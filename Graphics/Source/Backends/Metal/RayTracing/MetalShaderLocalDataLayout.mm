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

#include <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderLocalDataLayout.h>
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>

using namespace DenOfIz;

MetalShaderLocalDataLayout::MetalShaderLocalDataLayout( MetalContext *context, const ShaderLocalDataLayoutDesc &desc ) : m_context( context ), m_desc( desc )
{
    for ( uint32_t i = 0; i < desc.ResourceBindings.NumElements( ); ++i )
    {
        const auto &binding = desc.ResourceBindings.GetElement( i );

        if ( binding.BindingType == ResourceBindingType::ConstantBuffer )
        {
            m_totalInlineDataBytes += binding.Reflection.NumBytes;
            ContainerUtilities::EnsureSize( m_inlineDataOffsets, binding.Binding );
            ContainerUtilities::EnsureSize( m_inlineDataNumBytes, binding.Binding );
            m_inlineDataOffsets[ binding.Binding ]  = binding.Reflection.LocalCbvOffset;
            m_inlineDataNumBytes[ binding.Binding ] = binding.Reflection.NumBytes;
        }
        else if ( binding.BindingType == ResourceBindingType::Sampler )
        {
            ContainerUtilities::EnsureSize( m_samplerBindings, binding.Binding );
            m_samplerBindings[ binding.Binding ] = { .TLABOffset = binding.Reflection.TLABOffset, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
        else if ( binding.BindingType == ResourceBindingType::ShaderResource )
        {
            ContainerUtilities::EnsureSize( m_srvBindings, binding.Binding );
            m_srvBindings[ binding.Binding ] = { .TLABOffset = binding.Reflection.TLABOffset, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
        else if ( binding.BindingType == ResourceBindingType::UnorderedAccess )
        {
            ContainerUtilities::EnsureSize( m_uavBindings, binding.Binding );
            m_uavBindings[ binding.Binding ] = { .TLABOffset = binding.Reflection.TLABOffset, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
        }
    }
}

uint32_t MetalShaderLocalDataLayout::NumInlineBytes( ) const
{
    return m_totalInlineDataBytes;
}

uint32_t MetalShaderLocalDataLayout::NumSrvUavs( ) const
{
    return m_srvBindings.size( ) + m_uavBindings.size( );
}

uint32_t MetalShaderLocalDataLayout::NumSamplers( ) const
{
    return m_samplerBindings.size( );
}

const uint32_t MetalShaderLocalDataLayout::InlineDataOffset( uint32_t binding ) const
{
    if ( binding >= m_inlineDataOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return 0;
    }
    return m_inlineDataOffsets[ binding ];
}

const uint32_t MetalShaderLocalDataLayout::InlineNumBytes( uint32_t binding ) const
{
    if ( binding >= m_inlineDataNumBytes.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return 0;
    }
    return m_inlineDataNumBytes[ binding ];
}

const MetalLocalBindingDesc &MetalShaderLocalDataLayout::UavBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_uavBindings ) )
    {
        return empty;
    }
    return m_uavBindings[ binding ];
}

const MetalLocalBindingDesc &MetalShaderLocalDataLayout::SrvBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_srvBindings ) )
    {
        return empty;
    }
    return m_srvBindings[ binding ];
}

const MetalLocalBindingDesc &MetalShaderLocalDataLayout::SamplerBinding( uint32_t binding ) const
{
    if ( !EnsureSize( binding, m_samplerBindings ) )
    {
        return empty;
    }
    return m_samplerBindings[ binding ];
}

bool MetalShaderLocalDataLayout::EnsureSize( uint32_t binding, const std::vector<MetalLocalBindingDesc> &bindings ) const
{
    if ( binding >= bindings.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        return false;
    }

    return true;
}
