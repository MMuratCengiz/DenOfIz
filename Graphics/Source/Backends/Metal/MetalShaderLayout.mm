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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalShaderLayout.h"
#include <algorithm>
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

static DenOfIz_ResourceBindingType BindingTypeFromRange( const IRDescriptorRangeType type )
{
    switch ( type )
    {
    case IRDescriptorRangeTypeSRV:
        return DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE;
    case IRDescriptorRangeTypeUAV:
        return DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS;
    case IRDescriptorRangeTypeSampler:
        return DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER;
    default:
        return DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;
    }
}

static void HashCombine( uint64_t &hash, const uint64_t value )
{
    hash ^= value;
    hash *= 0x100000001B3ull;
}

IRDescriptorRangeType MetalShaderLayout::RangeType( const DenOfIz_ResourceBindingType type )
{
    return MetalRootSignatureBuilder::RangeType( type );
}

uint64_t MetalShaderLayout::BindingKey( const DenOfIz_ResourceBindingType type, const uint32_t binding )
{
    return ( static_cast<uint64_t>( type ) << 32 ) | binding;
}

const uint32_t *MetalSpaceLayout::TableIndex( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    const auto it = TableIndices.find( MetalShaderLayout::BindingKey( type, binding ) );
    return it == TableIndices.end( ) ? nullptr : &it->second;
}

const uint32_t *MetalSpaceLayout::RootDescriptorOffset( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    const auto it = RootDescriptorOffsets.find( MetalShaderLayout::BindingKey( type, binding ) );
    return it == RootDescriptorOffsets.end( ) ? nullptr : &it->second;
}

std::vector<MetalRootSignatureBinding> MetalShaderLayout::GlobalBindings( const DenOfIz_ShaderReflectDesc &reflectDesc )
{
    std::vector<MetalRootSignatureBinding> bindings;
    for ( uint32_t i = 0; i < reflectDesc.RootConstants.NumElements; ++i )
    {
        const DenOfIz_RootConstantBindingDesc &rootConstant = reflectDesc.RootConstants.Elements[ i ];
        MetalRootSignatureBinding             &binding      = bindings.emplace_back( );
        binding.Type                                        = IRDescriptorRangeTypeCBV;
        binding.RegisterSpace                               = DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE;
        binding.Binding                                     = rootConstant.Binding;
        binding.NumConstantBytes                            = static_cast<uint32_t>( rootConstant.NumBytes );
        binding.IsRootConstant                              = true;
    }

    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        const DenOfIz_BindGroupLayoutDesc &layout = reflectDesc.BindGroupLayouts.Elements[ i ];
        for ( uint32_t j = 0; j < layout.Bindings.NumElements; ++j )
        {
            const DenOfIz_BindingDesc        &bindingDesc = layout.Bindings.Elements[ j ];
            const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( bindingDesc.Descriptor );

            MetalRootSignatureBinding &binding = bindings.emplace_back( );
            binding.Type                       = RangeType( bindingType );
            binding.RegisterSpace              = layout.RegisterSpace;
            binding.Binding                    = bindingDesc.Binding;
            binding.NumDescriptors             = std::max( bindingDesc.ArraySize, 1u );
            binding.IsBindless                 = bindingDesc.IsBindless;
            binding.IsRootDescriptor           = layout.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE && bindingType != DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER;
        }
    }
    return bindings;
}

std::vector<MetalRootSignatureBinding> MetalShaderLayout::LocalBindings( const DenOfIz_LocalRootSignatureDescArray &localRootSignatures )
{
    std::vector<MetalRootSignatureBinding> bindings;
    for ( uint32_t i = 0; i < localRootSignatures.NumElements; ++i )
    {
        const DenOfIz_LocalRootSignatureDesc &localRootSignature = localRootSignatures.Elements[ i ];
        for ( uint32_t j = 0; j < localRootSignature.ResourceBindings.NumElements; ++j )
        {
            const DenOfIz_LocalResourceBindingDesc &bindingDesc = localRootSignature.ResourceBindings.Elements[ j ];
            const DenOfIz_ResourceBindingType       bindingType = DenOfIz_ResourceBindingType_FromDescriptor( bindingDesc.Descriptor );
            const IRDescriptorRangeType             rangeType   = RangeType( bindingType );

            const bool alreadyAdded = std::any_of( bindings.begin( ), bindings.end( ),
                                                   [ & ]( const MetalRootSignatureBinding &existing )
                                                   {
                                                       return existing.Type == rangeType && existing.RegisterSpace == bindingDesc.RegisterSpace &&
                                                              existing.Binding == bindingDesc.Binding;
                                                   } );
            if ( alreadyAdded )
            {
                continue;
            }

            MetalRootSignatureBinding &binding = bindings.emplace_back( );
            binding.Type                       = rangeType;
            binding.RegisterSpace              = bindingDesc.RegisterSpace;
            binding.Binding                    = bindingDesc.Binding;
            binding.NumDescriptors             = static_cast<uint32_t>( std::max( bindingDesc.ArraySize, 1 ) );
            if ( bindingType == DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER )
            {
                binding.IsRootConstant   = true;
                binding.NumConstantBytes = static_cast<uint32_t>( bindingDesc.NumBytes );
            }
            else if ( bindingDesc.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE && bindingType != DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER )
            {
                binding.IsRootDescriptor = true;
            }
        }
    }
    return bindings;
}

MetalShaderLayout::MetalShaderLayout( const std::vector<MetalRootSignatureBinding> &bindings )
{
    const MetalRootSignatureBuilder builder( bindings );
    m_numBytes = builder.NumBytes( );

    for ( const MetalRootParameterLocation &location : builder.Locations( ) )
    {
        if ( location.Kind == MetalRootParameterKind::RootConstant )
        {
            m_rootConstants.push_back( { location.RegisterSpace, location.Binding, location.OffsetBytes, location.SizeBytes } );
            continue;
        }

        ContainerUtilities::EnsureSize( m_spaces, location.RegisterSpace );
        if ( !m_spaces[ location.RegisterSpace ] )
        {
            m_spaces[ location.RegisterSpace ]                = std::make_unique<MetalSpaceLayout>( );
            m_spaces[ location.RegisterSpace ]->RegisterSpace = location.RegisterSpace;
        }
        MetalSpaceLayout &space = *m_spaces[ location.RegisterSpace ];
        switch ( location.Kind )
        {
        case MetalRootParameterKind::CbvSrvUavTable:
            space.CbvSrvUavTableOffset = location.OffsetBytes;
            break;
        case MetalRootParameterKind::SamplerTable:
            space.SamplerTableOffset = location.OffsetBytes;
            break;
        case MetalRootParameterKind::RootDescriptor:
            space.RootDescriptorOffsets[ BindingKey( BindingTypeFromRange( location.Type ), location.Binding ) ] = location.OffsetBytes;
            break;
        default:
            break;
        }
    }

    for ( const MetalDescriptorTableEntry &entry : builder.TableEntries( ) )
    {
        MetalSpaceLayout &space = *m_spaces[ entry.RegisterSpace ];
        space.TableIndices[ BindingKey( BindingTypeFromRange( entry.Type ), entry.Binding ) ] = entry.TableIndex;
        if ( entry.Type == IRDescriptorRangeTypeSampler )
        {
            space.SamplerTableSize = std::max( space.SamplerTableSize, entry.TableIndex + entry.NumDescriptors );
        }
        else
        {
            space.CbvSrvUavTableSize = std::max( space.CbvSrvUavTableSize, entry.TableIndex + entry.NumDescriptors );
        }
    }

    for ( auto &spacePtr : m_spaces )
    {
        if ( !spacePtr )
        {
            continue;
        }
        MetalSpaceLayout &space = *spacePtr;
        uint64_t          hash  = 0xCBF29CE484222325ull;
        HashCombine( hash, space.RegisterSpace );
        HashCombine( hash, space.CbvSrvUavTableSize );
        HashCombine( hash, space.SamplerTableSize );
        for ( const MetalDescriptorTableEntry &entry : builder.TableEntries( ) )
        {
            if ( entry.RegisterSpace != space.RegisterSpace )
            {
                continue;
            }
            HashCombine( hash, entry.Type );
            HashCombine( hash, entry.Binding );
            HashCombine( hash, entry.NumDescriptors );
            HashCombine( hash, entry.TableIndex );
        }
        std::vector<std::pair<uint64_t, uint32_t>> rootDescriptors( space.RootDescriptorOffsets.begin( ), space.RootDescriptorOffsets.end( ) );
        std::sort( rootDescriptors.begin( ), rootDescriptors.end( ) );
        for ( const auto &[ key, offset ] : rootDescriptors )
        {
            HashCombine( hash, key );
            HashCombine( hash, offset );
        }
        space.Hash = hash;
    }

    m_hash = 0xCBF29CE484222325ull;
    HashCombine( m_hash, m_numBytes );
    for ( const auto &rootConstant : m_rootConstants )
    {
        HashCombine( m_hash, rootConstant.RegisterSpace );
        HashCombine( m_hash, rootConstant.Binding );
        HashCombine( m_hash, rootConstant.Offset );
        HashCombine( m_hash, rootConstant.NumBytes );
    }
    for ( const auto &space : m_spaces )
    {
        HashCombine( m_hash, space ? space->Hash : 0 );
    }
}

const MetalSpaceLayout *MetalShaderLayout::Space( const uint32_t registerSpace ) const
{
    if ( registerSpace >= m_spaces.size( ) )
    {
        return nullptr;
    }
    return m_spaces[ registerSpace ].get( );
}

const std::vector<std::unique_ptr<MetalSpaceLayout>> &MetalShaderLayout::Spaces( ) const
{
    return m_spaces;
}

const MetalRootConstantLayout *MetalShaderLayout::RootConstant( const uint32_t binding ) const
{
    for ( const auto &rootConstant : m_rootConstants )
    {
        if ( rootConstant.Binding == binding )
        {
            return &rootConstant;
        }
    }
    return nullptr;
}

const MetalRootConstantLayout *MetalShaderLayout::RootConstant( const uint32_t registerSpace, const uint32_t binding ) const
{
    for ( const auto &rootConstant : m_rootConstants )
    {
        if ( rootConstant.RegisterSpace == registerSpace && rootConstant.Binding == binding )
        {
            return &rootConstant;
        }
    }
    return nullptr;
}

const MetalSpaceLayout *MetalShaderLayout::FindTable( const DenOfIz_ResourceBindingType type, const uint32_t binding, uint32_t &outIndex ) const
{
    for ( const auto &space : m_spaces )
    {
        if ( !space )
        {
            continue;
        }
        if ( const uint32_t *index = space->TableIndex( type, binding ) )
        {
            outIndex = *index;
            return space.get( );
        }
    }
    return nullptr;
}

uint32_t MetalShaderLayout::NumBytes( ) const
{
    return m_numBytes;
}

uint64_t MetalShaderLayout::Hash( ) const
{
    return m_hash;
}

const std::vector<MetalRootConstantLayout> &MetalShaderLayout::RootConstants( ) const
{
    return m_rootConstants;
}
