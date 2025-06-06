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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
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
            spdlog::critical("Root constant binding index is out of range. Make sure all bindings are provided in ascending order.");
        }
        const auto &rootConstant     = m_desc.RootConstants.GetElement( trueIndex );
        m_rootConstants[ trueIndex ] = { .Offset = m_numRootConstantBytes, .NumBytes = rootConstant.NumBytes };
        m_numRootConstantBytes += rootConstant.NumBytes;
    }

    m_descriptorOffsets.resize( bindingsBySpace.rbegin( )->first + 1 );
    int currentTLABOffset = m_numRootConstantBytes / sizeof( uint64_t );
    
    bool hasBindlessResources = desc.BindlessResources.NumElements( ) > 0;
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
            if ( offsets.CbvSrvUavTableOffset == UINT_MAX )
            {
                if ( hasBindlessResources && space == 1 )
                {
                    offsets.CbvSrvUavTableOffset = 2;
                }
                else
                {
                    offsets.CbvSrvUavTableOffset = currentTLABOffset++;
                }
                m_numTLABAddresses++;
            }

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
            // Only use hardcoded offsets when bindless resources are present
            if ( hasBindlessResources && space == 0 )
            {
                offsets.SamplerTableOffset = 1; // Parameter[1] for bindless case
            }
            else
            {
                offsets.SamplerTableOffset = currentTLABOffset++;
            }
            m_numTLABAddresses++;
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
            // For bindless resources, we need to override the normal TLAB assignment
            // Parameter[0]: Space 0 bindless SRV 
            // Parameter[1]: Space 0 sampler
            // Parameter[2]: Space 1 CBV
            if ( bindlessResource.RegisterSpace == 0 )
            {
                offsets.CbvSrvUavTableOffset = 0;
            }
            else
            {
                offsets.CbvSrvUavTableOffset = 2;
            }
            m_numTLABAddresses++;
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
        spdlog::error("Invalid register space");
        return 0;
    }
    return m_descriptorOffsets[ registerSpace ].CbvSrvUavTableOffset;
}

const uint32_t MetalRootSignature::CbvSrvUavTableSize( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_descriptorOffsets.size( ) )
    {
        spdlog::error("Invalid register space");
        return 0;
    }
    return m_descriptorOffsets[ registerSpace ].CbvSrvUavResourceCount;
}

const uint32_t MetalRootSignature::CbvSrvUavResourceIndex( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        spdlog::error("Invalid register space");
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
        spdlog::error("Resource binding with type[ {} ],binding[ {} ],register[{} ] not found.", static_cast<int>( slot.Type ), slot.Binding, slot.RegisterSpace);
        return 0;
    }
    return resourceIndex;
}

const MTLRenderStages MetalRootSignature::CbvSrvUavResourceShaderStages( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        spdlog::error("Invalid register space");
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
        spdlog::error("Invalid register space");
        return 0;
    }

    return m_descriptorOffsets[ slot.RegisterSpace ].SamplerResourceStages[ slot.Binding ];
}

const uint32_t MetalRootSignature::SamplerTableOffset( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_descriptorOffsets.size( ) )
    {
        spdlog::error("Invalid register space");
        return 0;
    }

    return m_descriptorOffsets[ registerSpace ].SamplerTableOffset;
}

const uint32_t MetalRootSignature::SamplerResourceIndex( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_descriptorOffsets.size( ) )
    {
        spdlog::error("Invalid register space");
        return 0;
    }

    return m_descriptorOffsets[ slot.RegisterSpace ].SamplerResourceIndices[ slot.Binding ];
}

MetalRootSignature::~MetalRootSignature( )
{
}
