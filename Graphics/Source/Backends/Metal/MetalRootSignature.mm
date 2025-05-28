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

#include <DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h>
#include <DenOfIzGraphics/Backends/Metal/MetalRootSignature.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <map>

using namespace DenOfIz;

MetalRootSignature::MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{

    std::map<uint32_t, std::vector<ResourceBindingDesc>> bindingsBySpace;
    auto                                                 sortedBindings = SortResourceBindings( m_desc.ResourceBindings );
    for ( int i = 0; i < sortedBindings.NumElements( ); i++ )
    {
        const auto &binding = m_desc.ResourceBindings.GetElement( i );
        bindingsBySpace[ binding.RegisterSpace ].push_back( binding );
    }

    m_rootConstants.resize( m_desc.RootConstants.NumElements( ) );
    for ( int i = 0; i < m_desc.RootConstants.NumElements( ); i++ )
    {
        const auto &trueIndex = m_desc.RootConstants.GetElement( i ).Binding;
        if ( trueIndex >= m_desc.RootConstants.NumElements( ) )
        {
            LOG( FATAL ) << "Root constant binding index is out of range. Make sure all bindings are provided in ascending order.";
        }
        const auto &rootConstant     = m_desc.RootConstants.GetElement( trueIndex );
        m_rootConstants[ trueIndex ] = { .Offset = m_numRootConstantBytes, .NumBytes = rootConstant.NumBytes };
        m_numRootConstantBytes += rootConstant.NumBytes;
    }

    m_descriptorOffsets.resize( bindingsBySpace.rbegin( )->first + 1 );
    int currentTLABOffset = m_numRootConstantBytes / sizeof( uint64_t );
    for ( const auto &[ space, bindings ] : bindingsBySpace )
    {
        auto &offsets = m_descriptorOffsets[ space ];

        bool     hasCbvSrvUav   = false;
        bool     hasSamplers    = false;
        uint32_t cbvSrvUavIndex = 0;
        uint32_t samplerIndex   = 0;

        for ( const auto &binding : bindings )
        {
            if ( binding.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
            {
                uint32_t hash                   = Utilities::HashInts( GetRootParameterType( binding.BindingType ), binding.RegisterSpace, binding.Binding );
                offsets.UniqueTLABIndex[ hash ] = currentTLABOffset++;
                m_numTLABAddresses++;
                continue;
            }

            if ( binding.BindingType == ResourceBindingType::Sampler )
            {
                hasSamplers = true;
            }
            else
            {
                hasCbvSrvUav = true;
            }
        }

        if ( hasCbvSrvUav )
        {
            m_numTLABAddresses++;
            offsets.CbvSrvUavTableOffset = currentTLABOffset++;

            for ( const auto &binding : bindings )
            {
                if ( binding.BindingType == ResourceBindingType::Sampler )
                {
                    continue;
                }

                ContainerUtilities::EnsureSize( offsets.CbvSrvUavResourceIndices, binding.Binding + 1 );
                auto &bindingIndices = offsets.CbvSrvUavResourceIndices[ binding.Binding ];
                switch ( binding.BindingType )
                {
                case ResourceBindingType::ConstantBuffer:
                    bindingIndices.Cbv       = cbvSrvUavIndex;
                    bindingIndices.CbvStages = MetalEnumConverter::ConvertRenderStages( binding.Stages );
                    break;
                case ResourceBindingType::ShaderResource:
                    bindingIndices.Srv       = cbvSrvUavIndex;
                    bindingIndices.SrvStages = MetalEnumConverter::ConvertRenderStages( binding.Stages );
                    break;
                case ResourceBindingType::UnorderedAccess:
                    bindingIndices.Uav       = cbvSrvUavIndex;
                    bindingIndices.UavStages = MetalEnumConverter::ConvertRenderStages( binding.Stages );
                    break;
                case ResourceBindingType::Sampler:
                    break;
                }
                cbvSrvUavIndex++;
            }

            offsets.CbvSrvUavResourceCount = cbvSrvUavIndex;
        }

        if ( hasSamplers )
        {
            m_numTLABAddresses++;
            offsets.SamplerTableOffset = currentTLABOffset++;
            for ( const auto &binding : bindings )
            {
                if ( binding.BindingType == ResourceBindingType::Sampler )
                {
                    ContainerUtilities::EnsureSize( offsets.SamplerResourceIndices, binding.Binding + 1 );
                    ContainerUtilities::EnsureSize( offsets.SamplerResourceStages, binding.Binding + 1 );
                    offsets.SamplerResourceIndices[ binding.Binding ] = samplerIndex;
                    offsets.SamplerResourceStages[ binding.Binding ]  = MetalEnumConverter::ConvertRenderStages( binding.Stages );
                    samplerIndex++;
                }
            }
        }
    };

    for ( int i = 0; i < desc.BindlessResources.NumElements( ); ++i )
    {
        const auto &bindlessResource = desc.BindlessResources.GetElement( i );
        AddBindlessResource( bindlessResource );
    }
}

const uint32_t MetalRootSignature::NumTLABAddresses( ) const
{
    return m_numTLABAddresses;
}

[[nodiscard]] const uint32_t &MetalRootSignature::NumRootConstantBytes( ) const
{
    return m_numRootConstantBytes;
}

const std::vector<MetalRootConstant> &MetalRootSignature::RootConstants( ) const
{
    return m_rootConstants;
}

IRRootParameterType MetalRootSignature::GetRootParameterType( ResourceBindingType type )
{
    switch ( type )
    {
    case ResourceBindingType::ConstantBuffer:
        return IRRootParameterTypeCBV;
    case ResourceBindingType::ShaderResource:
        return IRRootParameterTypeSRV;
    case ResourceBindingType::UnorderedAccess:
        return IRRootParameterTypeUAV;
    default:
        return IRRootParameterTypeCBV;
    }
}

void MetalRootSignature::AddBindlessResource( const BindlessResourceDesc &bindlessResource )
{
    ContainerUtilities::EnsureSize( m_descriptorOffsets, bindlessResource.RegisterSpace + 1 );
    auto &offsets = m_descriptorOffsets[ bindlessResource.RegisterSpace ];
    
    if ( bindlessResource.Type != ResourceBindingType::Sampler )
    {
        if ( offsets.CbvSrvUavTableOffset == UINT_MAX )
        {
            m_numTLABAddresses++;
            offsets.CbvSrvUavTableOffset = m_numTLABAddresses - 1;
        }
        
        ContainerUtilities::EnsureSize( offsets.CbvSrvUavResourceIndices, bindlessResource.Binding + bindlessResource.MaxArraySize );
        for ( uint32_t i = 0; i < bindlessResource.MaxArraySize; ++i )
        {
            auto &bindingIndices = offsets.CbvSrvUavResourceIndices[ bindlessResource.Binding + i ];
            switch ( bindlessResource.Type )
            {
            case ResourceBindingType::ConstantBuffer:
                bindingIndices.Cbv = offsets.CbvSrvUavResourceCount++;
                bindingIndices.CbvStages = MTLRenderStageVertex | MTLRenderStageFragment;
                break;
            case ResourceBindingType::ShaderResource:
                bindingIndices.Srv = offsets.CbvSrvUavResourceCount++;
                bindingIndices.SrvStages = MTLRenderStageVertex | MTLRenderStageFragment;
                break;
            case ResourceBindingType::UnorderedAccess:
                bindingIndices.Uav = offsets.CbvSrvUavResourceCount++;
                bindingIndices.UavStages = MTLRenderStageVertex | MTLRenderStageFragment;
                break;
            }
        }
    }
}

const uint32_t MetalRootSignature::TLABOffset( const ResourceBindingSlot &slot ) const
{
    uint32_t hash = Utilities::HashInts( MetalRootSignature::GetRootParameterType( slot.Type ), slot.RegisterSpace, slot.Binding );
    return m_descriptorOffsets[ slot.RegisterSpace ].UniqueTLABIndex.at( hash );
}

const uint32_t MetalRootSignature::CbvSrvUavTableOffset( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }
    return m_descriptorOffsets[ registerSpace ].CbvSrvUavTableOffset;
}

const uint32_t MetalRootSignature::CbvSrvUavResourceIndex( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }

    auto &bindingIndices = m_descriptorOffsets[ slot.RegisterSpace ].CbvSrvUavResourceIndices[ slot.Binding ];
    int   resourceIndex  = -1;
    switch ( slot.Type )
    {
    case ResourceBindingType::ConstantBuffer:
        resourceIndex = bindingIndices.Cbv;
        break;
    case ResourceBindingType::ShaderResource:
        resourceIndex = bindingIndices.Srv;
        break;
    case ResourceBindingType::UnorderedAccess:
        resourceIndex = bindingIndices.Uav;
        break;
    case ResourceBindingType::Sampler:
        break;
    }
    if ( resourceIndex == -1 )
    {
        LOG( ERROR ) << "Resource binding with type[" << static_cast<int>( slot.Type ) << "],binding[" << slot.Binding << "],register[" << slot.RegisterSpace << "] not found.";
        return 0;
    }
    return resourceIndex;
}

const MTLRenderStages MetalRootSignature::CbvSrvUavResourceShaderStages( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }

    auto &bindingIndices = m_descriptorOffsets[ slot.RegisterSpace ].CbvSrvUavResourceIndices[ slot.Binding ];
    switch ( slot.Type )
    {
    case ResourceBindingType::ConstantBuffer:
        return bindingIndices.CbvStages;
    case ResourceBindingType::ShaderResource:
        return bindingIndices.SrvStages;
    case ResourceBindingType::UnorderedAccess:
        return bindingIndices.UavStages;
    case ResourceBindingType::Sampler:
        break;
    }
    return MTLRenderStageVertex;
}

const MTLRenderStages MetalRootSignature::SamplerResourceShaderStages( const DenOfIz::ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }

    return m_descriptorOffsets[ slot.RegisterSpace ].SamplerResourceStages[ slot.Binding ];
}

const uint32_t MetalRootSignature::SamplerTableOffset( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }

    return m_descriptorOffsets[ registerSpace ].SamplerTableOffset;
}

const uint32_t MetalRootSignature::SamplerResourceIndex( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid register space";
        return 0;
    }

    return m_descriptorOffsets[ slot.RegisterSpace ].SamplerResourceIndices[ slot.Binding ];
}

MetalRootSignature::~MetalRootSignature( )
{
}
