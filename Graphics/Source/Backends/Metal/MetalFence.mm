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

MetalFence::MetalFence( MetalContext *context ) : m_context( context ), m_state( std::make_shared<State>( ) )
{
}

MetalFence::~MetalFence( )
{
}

void MetalFence::Wait( )
{
    std::unique_lock lock( m_state->Mutex );
    m_state->Condition.wait( lock, [ this ] { return m_state->CompletedValue >= m_state->SubmittedValue; } );
}

void MetalFence::Reset( )
{
    // Nothing to do: like DX12Fence, the next submission simply signals a newer value and Wait() waits for it.
}

void MetalFence::Notify( )
{
    {
        std::lock_guard lock( m_state->Mutex );
        m_state->CompletedValue = m_state->SubmittedValue;
    }
    m_state->Condition.notify_all( );
}

void MetalFence::NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer )
{
    std::shared_ptr<State> state = m_state;
    uint64_t               value;
    {
        std::lock_guard lock( state->Mutex );
        value = ++state->SubmittedValue;
    }

    @autoreleasepool
    {
        [commandBuffer addCompletedHandler:^( id<MTLCommandBuffer> _unused ) {
          {
              std::lock_guard lock( state->Mutex );
              if ( value > state->CompletedValue )
              {
                  state->CompletedValue = value;
              }
          }
          state->Condition.notify_all( );
        }];
    }
}
