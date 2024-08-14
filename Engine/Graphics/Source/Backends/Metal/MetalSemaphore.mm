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
    m_semaphore = dispatch_semaphore_create( 0 );
}

void MetalSemaphore::Wait( )
{
    dispatch_semaphore_wait( m_semaphore, DISPATCH_TIME_FOREVER );
}

void MetalSemaphore::Notify( )
{
    dispatch_semaphore_signal( m_semaphore );
}

void MetalSemaphore::NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer>& commandBuffer )
{
    [commandBuffer addCompletedHandler:^( id<MTLCommandBuffer> _unused ) { this->Notify(); }];
}
