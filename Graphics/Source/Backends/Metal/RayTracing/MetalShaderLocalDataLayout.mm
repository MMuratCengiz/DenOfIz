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
#include <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderLocalDataLayout.h>

using namespace DenOfIz;

MetalShaderLocalDataLayout::MetalShaderLocalDataLayout( MetalContext *context, const ShaderLocalDataLayoutDesc &desc ) : m_context( context ), m_desc( desc )
{
    for ( uint32_t i = 0; i < desc.ResourceBindings.NumElements( ); ++i )
    {
        const auto &binding = desc.ResourceBindings.GetElement( i );

        m_bindings.resize( std::max<size_t>( m_bindings.size( ), binding.Binding + 1 ) );

        if ( binding.BindingType == ResourceBindingType::ConstantBuffer )
        {
            m_totalInlineDataBytes += binding.Reflection.NumBytes;
        }
        else if ( binding.BindingType == ResourceBindingType::Sampler )
        {
            m_numSamplers++;
        }
        else
        {
            m_numSrvUavs++;
        }

        m_bindings[ binding.Binding ] = { .TLABOffset = binding.Reflection.TLABOffset, .NumBytes = binding.Reflection.NumBytes, .Type = binding.BindingType };
    }
}

uint32_t MetalShaderLocalDataLayout::NumInlineBytes( ) const
{
    return m_totalInlineDataBytes;
}

uint32_t MetalShaderLocalDataLayout::NumSrvUavs( ) const
{
    return m_numSrvUavs;
}

uint32_t MetalShaderLocalDataLayout::NumSamplers( ) const
{
    return m_numSamplers;
}

const MetalLocalBindingDesc &MetalShaderLocalDataLayout::GetBinding( uint32_t binding ) const
{
    if ( binding >= m_bindings.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index(" << binding << ")";
        static MetalLocalBindingDesc empty = { };
        return empty;
    }
    return m_bindings[ binding ];
}
