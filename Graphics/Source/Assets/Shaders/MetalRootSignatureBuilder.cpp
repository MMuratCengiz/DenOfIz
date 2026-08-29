/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "DenOfIzGraphicsInternal/Assets/Shaders/MetalRootSignatureBuilder.h"
#include <algorithm>
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

int MetalRootSignatureBuilder::TypeOrder( const IRDescriptorRangeType type )
{
    switch ( type )
    {
    case IRDescriptorRangeTypeCBV:
        return 0;
    case IRDescriptorRangeTypeSRV:
        return 1;
    case IRDescriptorRangeTypeUAV:
        return 2;
    case IRDescriptorRangeTypeSampler:
        return 3;
    }
    return 4;
}

IRDescriptorRangeType MetalRootSignatureBuilder::RangeType( const DenOfIz_ResourceBindingType type )
{
    switch ( type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        return IRDescriptorRangeTypeSRV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        return IRDescriptorRangeTypeUAV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        return IRDescriptorRangeTypeSampler;
    default:
        return IRDescriptorRangeTypeCBV;
    }
}

static IRRootParameterType RootDescriptorParameterType( const IRDescriptorRangeType type )
{
    switch ( type )
    {
    case IRDescriptorRangeTypeSRV:
        return IRRootParameterTypeSRV;
    case IRDescriptorRangeTypeUAV:
        return IRRootParameterTypeUAV;
    default:
        return IRRootParameterTypeCBV;
    }
}

static uint32_t AlignTo( const uint32_t value, const uint32_t alignment )
{
    return ( value + alignment - 1 ) / alignment * alignment;
}

MetalRootSignatureBuilder::MetalRootSignatureBuilder( std::vector<MetalRootSignatureBinding> bindings )
{
    std::sort( bindings.begin( ), bindings.end( ),
               []( const MetalRootSignatureBinding &a, const MetalRootSignatureBinding &b )
               {
                   if ( a.RegisterSpace != b.RegisterSpace )
                   {
                       return a.RegisterSpace < b.RegisterSpace;
                   }
                   if ( TypeOrder( a.Type ) != TypeOrder( b.Type ) )
                   {
                       return TypeOrder( a.Type ) < TypeOrder( b.Type );
                   }
                   return a.Binding < b.Binding;
               } );

    for ( const auto &binding : bindings )
    {
        ContainerUtilities::EnsureSize( m_spaces, binding.RegisterSpace );
        SpaceParameters &space = m_spaces[ binding.RegisterSpace ];

        if ( binding.IsRootConstant )
        {
            IRRootConstants &constants = space.Constants.emplace_back( );
            constants.RegisterSpace    = binding.RegisterSpace;
            constants.ShaderRegister   = binding.Binding;
            constants.Num32BitValues   = AlignTo( binding.NumConstantBytes, 4 ) / 4;
            continue;
        }

        if ( binding.IsRootDescriptor && binding.Type != IRDescriptorRangeTypeSampler )
        {
            IRRootDescriptor1 &descriptor = space.Descriptors.emplace_back( );
            descriptor.RegisterSpace      = binding.RegisterSpace;
            descriptor.ShaderRegister     = binding.Binding;
            descriptor.Flags              = IRRootDescriptorFlagNone;
            space.DescriptorTypes.push_back( RootDescriptorParameterType( binding.Type ) );
            continue;
        }

        IRDescriptorRange1 range                = { };
        range.RangeType                         = binding.Type;
        range.BaseShaderRegister                = binding.Binding;
        range.RegisterSpace                     = binding.RegisterSpace;
        range.NumDescriptors                    = std::max( binding.NumDescriptors, 1u );
        range.Flags                             = binding.IsBindless ? IRDescriptorRangeFlagDescriptorsVolatile : IRDescriptorRangeFlagNone;
        range.OffsetInDescriptorsFromTableStart = 0;

        std::vector<IRDescriptorRange1> &ranges = binding.Type == IRDescriptorRangeTypeSampler ? space.SamplerRanges : space.CbvSrvUavRanges;
        if ( !ranges.empty( ) )
        {
            const IRDescriptorRange1 &previous    = ranges.back( );
            range.OffsetInDescriptorsFromTableStart = previous.OffsetInDescriptorsFromTableStart + previous.NumDescriptors;
        }
        ranges.push_back( range );

        MetalDescriptorTableEntry &entry = m_tableEntries.emplace_back( );
        entry.Type                       = binding.Type;
        entry.RegisterSpace              = binding.RegisterSpace;
        entry.Binding                    = binding.Binding;
        entry.NumDescriptors             = range.NumDescriptors;
        entry.TableIndex                 = range.OffsetInDescriptorsFromTableStart;
    }

    BuildRootParameters( );

    IRVersionedRootSignatureDescriptor descriptor = Descriptor( );
    IRError                           *error      = nullptr;
    m_rootSignature                               = IRRootSignatureCreateFromDescriptor( &descriptor, &error );
    if ( error )
    {
        spdlog::error( "Error producing IRRootSignature, error code [ {} ]", IRErrorGetCode( error ) );
        IRErrorDestroy( error );
    }

    ResolveLocations( );
}

MetalRootSignatureBuilder::~MetalRootSignatureBuilder( )
{
    if ( m_rootSignature )
    {
        IRRootSignatureDestroy( m_rootSignature );
        m_rootSignature = nullptr;
    }
}

void MetalRootSignatureBuilder::BuildRootParameters( )
{
    for ( auto &space : m_spaces )
    {
        for ( const auto &constants : space.Constants )
        {
            IRRootParameter1 &parameter = m_rootParameters.emplace_back( );
            parameter.ParameterType     = IRRootParameterType32BitConstants;
            parameter.ShaderVisibility  = IRShaderVisibilityAll;
            parameter.Constants         = constants;

            MetalRootParameterLocation &location = m_locations.emplace_back( );
            location.Kind                        = MetalRootParameterKind::RootConstant;
            location.Type                        = IRDescriptorRangeTypeCBV;
            location.RegisterSpace               = constants.RegisterSpace;
            location.Binding                     = constants.ShaderRegister;
            location.SizeBytes                   = constants.Num32BitValues * 4;
        }
    }

    for ( auto &space : m_spaces )
    {
        if ( !space.CbvSrvUavRanges.empty( ) )
        {
            IRRootParameter1 &parameter                   = m_rootParameters.emplace_back( );
            parameter.ParameterType                       = IRRootParameterTypeDescriptorTable;
            parameter.ShaderVisibility                    = IRShaderVisibilityAll;
            parameter.DescriptorTable.NumDescriptorRanges = static_cast<uint32_t>( space.CbvSrvUavRanges.size( ) );
            parameter.DescriptorTable.pDescriptorRanges   = space.CbvSrvUavRanges.data( );

            MetalRootParameterLocation &location = m_locations.emplace_back( );
            location.Kind                        = MetalRootParameterKind::CbvSrvUavTable;
            location.Type                        = IRDescriptorRangeTypeCBV;
            location.RegisterSpace               = space.CbvSrvUavRanges.front( ).RegisterSpace;
            location.Binding                     = 0;
            location.SizeBytes                   = sizeof( uint64_t );
        }
        if ( !space.SamplerRanges.empty( ) )
        {
            IRRootParameter1 &parameter                   = m_rootParameters.emplace_back( );
            parameter.ParameterType                       = IRRootParameterTypeDescriptorTable;
            parameter.ShaderVisibility                    = IRShaderVisibilityAll;
            parameter.DescriptorTable.NumDescriptorRanges = static_cast<uint32_t>( space.SamplerRanges.size( ) );
            parameter.DescriptorTable.pDescriptorRanges   = space.SamplerRanges.data( );

            MetalRootParameterLocation &location = m_locations.emplace_back( );
            location.Kind                        = MetalRootParameterKind::SamplerTable;
            location.Type                        = IRDescriptorRangeTypeSampler;
            location.RegisterSpace               = space.SamplerRanges.front( ).RegisterSpace;
            location.Binding                     = 0;
            location.SizeBytes                   = sizeof( uint64_t );
        }
        for ( size_t i = 0; i < space.Descriptors.size( ); ++i )
        {
            IRRootParameter1 &parameter = m_rootParameters.emplace_back( );
            parameter.ParameterType     = space.DescriptorTypes[ i ];
            parameter.ShaderVisibility  = IRShaderVisibilityAll;
            parameter.Descriptor        = space.Descriptors[ i ];

            MetalRootParameterLocation &location = m_locations.emplace_back( );
            location.Kind                        = MetalRootParameterKind::RootDescriptor;
            switch ( space.DescriptorTypes[ i ] )
            {
            case IRRootParameterTypeSRV:
                location.Type = IRDescriptorRangeTypeSRV;
                break;
            case IRRootParameterTypeUAV:
                location.Type = IRDescriptorRangeTypeUAV;
                break;
            default:
                location.Type = IRDescriptorRangeTypeCBV;
                break;
            }
            location.RegisterSpace = space.Descriptors[ i ].RegisterSpace;
            location.Binding       = space.Descriptors[ i ].ShaderRegister;
            location.SizeBytes     = sizeof( uint64_t );
        }
    }

    uint32_t offset = 0;
    for ( auto &location : m_locations )
    {
        const uint32_t alignment = location.Kind == MetalRootParameterKind::RootConstant ? 4 : 8;
        offset                   = AlignTo( offset, alignment );
        location.OffsetBytes     = offset;
        offset += location.SizeBytes;
    }
    m_numBytes = AlignTo( offset, 8 );
}

void MetalRootSignatureBuilder::ResolveLocations( )
{
    if ( !m_rootSignature )
    {
        return;
    }

    const size_t count = IRRootSignatureGetResourceCount( m_rootSignature );
    if ( count != m_locations.size( ) )
    {
        spdlog::error( "IRRootSignature reports {} resources, expected {}. Falling back to computed top level argument buffer offsets.", count, m_locations.size( ) );
        return;
    }
    if ( count == 0 )
    {
        return;
    }

    std::vector<IRResourceLocation> resourceLocations( count );
    IRRootSignatureGetResourceLocations( m_rootSignature, resourceLocations.data( ) );

    uint32_t end = 0;
    for ( size_t i = 0; i < count; ++i )
    {
        m_locations[ i ].OffsetBytes = resourceLocations[ i ].topLevelOffset;
        m_locations[ i ].SizeBytes   = static_cast<uint32_t>( resourceLocations[ i ].sizeBytes );
        end                          = std::max( end, m_locations[ i ].OffsetBytes + m_locations[ i ].SizeBytes );
    }
    m_numBytes = AlignTo( end, 8 );
}

IRVersionedRootSignatureDescriptor MetalRootSignatureBuilder::Descriptor( ) const
{
    IRVersionedRootSignatureDescriptor descriptor = { };
    descriptor.version                            = IRRootSignatureVersion_1_1;
    descriptor.desc_1_1.Flags             = static_cast<IRRootSignatureFlags>( IRRootSignatureFlagCBVSRVUAVHeapDirectlyIndexed | IRRootSignatureFlagSamplerHeapDirectlyIndexed );
    descriptor.desc_1_1.NumParameters     = static_cast<uint32_t>( m_rootParameters.size( ) );
    descriptor.desc_1_1.pParameters       = const_cast<IRRootParameter1 *>( m_rootParameters.data( ) );
    descriptor.desc_1_1.NumStaticSamplers = 0;
    descriptor.desc_1_1.pStaticSamplers   = nullptr;
    return descriptor;
}

IRRootSignature *MetalRootSignatureBuilder::RootSignature( ) const
{
    return m_rootSignature;
}

const std::vector<IRRootParameter1> &MetalRootSignatureBuilder::RootParameters( ) const
{
    return m_rootParameters;
}

const std::vector<MetalRootParameterLocation> &MetalRootSignatureBuilder::Locations( ) const
{
    return m_locations;
}

const std::vector<MetalDescriptorTableEntry> &MetalRootSignatureBuilder::TableEntries( ) const
{
    return m_tableEntries;
}

uint32_t MetalRootSignatureBuilder::NumBytes( ) const
{
    return m_numBytes;
}
