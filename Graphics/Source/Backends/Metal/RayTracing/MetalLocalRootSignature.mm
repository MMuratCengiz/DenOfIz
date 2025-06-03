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

#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include <algorithm>

using namespace DenOfIz;

MetalLocalRootSignature::MetalLocalRootSignature( MetalContext *context, const LocalRootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    auto sortedBindings = SortResourceBindings( desc.ResourceBindings );

    uint32_t cbvOffsetBytes = 0;
    uint32_t srvUavOffset   = 0;
    uint32_t samplerOffset  = 0;
    for ( uint32_t i = 0; i < sortedBindings.NumElements( ); ++i )
    {
        const auto &binding = sortedBindings.GetElement( i );

        if ( binding.BindingType == ResourceBindingType::ConstantBuffer )
        {
            m_totalInlineDataBytes += binding.Reflection.NumBytes;
            ContainerUtilities::EnsureSize( m_inlineDataOffsets, binding.Binding );
            ContainerUtilities::EnsureSize( m_inlineDataNumBytes, binding.Binding );
            m_inlineDataOffsets[ binding.Binding ]  = cbvOffsetBytes;
            m_inlineDataNumBytes[ binding.Binding ] = binding.Reflection.NumBytes;
            cbvOffsetBytes += binding.Reflection.NumBytes;
        }
        else if ( binding.BindingType == ResourceBindingType::Sampler )
        {
            ContainerUtilities::EnsureSize( m_samplerBindings, binding.Binding );
            m_samplerBindings[ binding.Binding ] = { .DescriptorTableIndex = samplerOffset++, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
        else if ( binding.BindingType == ResourceBindingType::ShaderResource )
        {
            ContainerUtilities::EnsureSize( m_srvBindings, binding.Binding );
            m_srvBindings[ binding.Binding ] = { .DescriptorTableIndex = srvUavOffset++, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
        else if ( binding.BindingType == ResourceBindingType::UnorderedAccess )
        {
            ContainerUtilities::EnsureSize( m_uavBindings, binding.Binding );
            m_uavBindings[ binding.Binding ] = { .DescriptorTableIndex = srvUavOffset++, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
    }
}

uint32_t MetalLocalRootSignature::NumInlineBytes( ) const
{
    return m_totalInlineDataBytes;
}

uint32_t MetalLocalRootSignature::NumSrvUavs( ) const
{
    return m_srvBindings.size( ) + m_uavBindings.size( );
}

uint32_t MetalLocalRootSignature::NumSamplers( ) const
{
    return m_samplerBindings.size( );
}

const uint32_t MetalLocalRootSignature::InlineDataOffset( uint32_t binding ) const
{
    if ( binding >= m_inlineDataOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return 0;
    }
    return m_inlineDataOffsets[ binding ];
}

const uint32_t MetalLocalRootSignature::InlineNumBytes( uint32_t binding ) const
{
    if ( binding >= m_inlineDataNumBytes.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return 0;
    }
    return m_inlineDataNumBytes[ binding ];
}

const MetalLocalBindingDesc &MetalLocalRootSignature::UavBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_uavBindings ) )
    {
        return empty;
    }
    return m_uavBindings[ binding ];
}

const MetalLocalBindingDesc &MetalLocalRootSignature::SrvBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_srvBindings ) )
    {
        return empty;
    }
    return m_srvBindings[ binding ];
}

const MetalLocalBindingDesc &MetalLocalRootSignature::SamplerBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_samplerBindings ) )
    {
        return empty;
    }
    return m_samplerBindings[ binding ];
}

bool MetalLocalRootSignature::EnsureSize( uint32_t binding, const std::vector<MetalLocalBindingDesc> &bindings ) const
{
    if ( binding >= bindings.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return false;
    }

    return true;
}
