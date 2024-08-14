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

#import "DenOfIzGraphics/Backends/Metal/MetalFence.h"

using namespace DenOfIz;
MetalFence::MetalFence( MetalContext *context ) : m_context( context )
{
    m_context = context;
    m_signaled = true;
}

MetalFence::~MetalFence( )
{
}

void MetalFence::Wait( )
{
    std::unique_lock<std::mutex> lock( m_mutex );
    if ( !m_signaled )
    {
        m_condition.wait( lock, [ this ] { return m_signaled; } );
    }
}

void MetalFence::Reset( )
{
    std::lock_guard<std::mutex> lock( m_mutex );
    m_signaled = false;
}

void MetalFence::Notify( )
{
    std::lock_guard<std::mutex> lock( m_mutex );
    m_signaled = true;
    m_condition.notify_all( );
}

void MetalFence::NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer>& commandBuffer )
{
    [commandBuffer addCompletedHandler:^( id<MTLCommandBuffer> _unused ) { this->Notify(); }];
}
