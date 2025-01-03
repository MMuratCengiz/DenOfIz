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

#include <DenOfIzGraphics/Backends/Metal/MetalSemaphore.h>

using namespace DenOfIz;

MetalSemaphore::MetalSemaphore( MetalContext *context ) : m_context( context )
{
    m_fence      = [m_context->Device newEvent];
    m_fenceValue = 0;
    m_signaled   = false;
}

void MetalSemaphore::Notify( )
{
}

void MetalSemaphore::WaitFor( id<MTLCommandBuffer> commandBuffer )
{
    if ( m_signaled )
    {
        [commandBuffer encodeWaitForEvent:m_fence value:m_fenceValue];
        m_signaled = false;
    }
}

void MetalSemaphore::NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer )
{
    m_fenceValue = ( m_fenceValue + 1 ) % MAX_FENCE_VALUE;
    [commandBuffer encodeSignalEvent:m_fence value:m_fenceValue];
    m_signaled = true;
}
