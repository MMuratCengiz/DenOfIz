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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalFence.h"

using namespace DenOfIz;
MetalFence::MetalFence( MetalContext *context ) : m_context( context )
{
    m_context   = context;
    m_fence     = dispatch_semaphore_create( 0 );
    m_submitted = false;
}

MetalFence::~MetalFence( )
{
}

void MetalFence::Wait( )
{
    if ( m_submitted )
    {
        dispatch_semaphore_wait( m_fence, DISPATCH_TIME_FOREVER );
        m_submitted = false;
    }
}

void MetalFence::Reset( )
{
    m_submitted = true;
}

void MetalFence::Notify( )
{
    if ( m_fence )
    {
        dispatch_semaphore_signal( m_fence );
    }
}

void MetalFence::NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer )
{
    dispatch_semaphore_t fence = m_fence;
    [commandBuffer addCompletedHandler:^( id<MTLCommandBuffer> _unused ) {
      if ( fence )
      {
          dispatch_semaphore_signal( fence );
      }
    }];
}
