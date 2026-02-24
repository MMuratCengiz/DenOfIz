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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUShadowState.h"

using namespace DenOfIz;

WebGPUShadowState DenOfIz::g_webGPUShadowState;

void WebGPUShadowState::SetBufferMapped( const WGPUBuffer &buffer, void *data, const size_t size, const WGPUMapMode mode )
{
    std::lock_guard lock( m_mutex );
    m_bufferStates[ buffer ] = { true, data, size, mode };
}

void WebGPUShadowState::SetBufferUnmapped( const WGPUBuffer &buffer )
{
    std::lock_guard lock( m_mutex );
    const auto      it = m_bufferStates.find( buffer );
    if ( it != m_bufferStates.end( ) )
    {
        it->second.IsMapped   = false;
        it->second.MappedData = nullptr;
        it->second.MappedSize = 0;
        it->second.MapMode    = static_cast<WGPUMapMode>( 0 );
    }
}

bool WebGPUShadowState::IsBufferMapped( const WGPUBuffer &buffer ) const
{
    std::lock_guard lock( m_mutex );
    const auto      it = m_bufferStates.find( buffer );
    return it != m_bufferStates.end( ) && it->second.IsMapped;
}

WebGPUShadowState::BufferState WebGPUShadowState::GetBufferState( const WGPUBuffer &buffer ) const
{
    std::lock_guard lock( m_mutex );
    const auto      it = m_bufferStates.find( buffer );
    return it != m_bufferStates.end( ) ? it->second : BufferState{ };
}

void WebGPUShadowState::RemoveBuffer( const WGPUBuffer &buffer )
{
    std::lock_guard lock( m_mutex );
    m_bufferStates.erase( buffer );
}

void WebGPUShadowState::SetTextureUsage( const WGPUTexture &texture, const uint32_t usage )
{
    std::lock_guard lock( m_mutex );
    m_textureStates[ texture ].CurrentUsage = usage;
}

uint32_t WebGPUShadowState::GetTextureUsage( const WGPUTexture &texture ) const
{
    std::lock_guard lock( m_mutex );
    const auto      it = m_textureStates.find( texture );
    return it != m_textureStates.end( ) ? it->second.CurrentUsage : 0;
}

void WebGPUShadowState::RemoveTexture( const WGPUTexture &texture )
{
    std::lock_guard lock( m_mutex );
    m_textureStates.erase( texture );
}
