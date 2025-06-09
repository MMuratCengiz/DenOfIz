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

#include "DenOfIzGraphics/Renderer/Sync/FrameSync.h"

using namespace DenOfIz;

FrameSync::FrameSync( const FrameSyncDesc &desc ) : m_numFrames( desc.NumFrames ), m_device( desc.Device ), m_swapChain( desc.SwapChain ), m_commandQueue( desc.CommandQueue )
{
    CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_commandQueue;
    poolDesc.NumCommandLists = m_numFrames;
    m_commandListPool        = std::unique_ptr<ICommandListPool>( m_device->CreateCommandListPool( poolDesc ) );

    m_frameFences.reserve( m_numFrames );
    m_imageAvailableSemaphores.reserve( m_numFrames );
    m_renderFinishedSemaphores.reserve( m_numFrames );

    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        m_frameFences.push_back( std::unique_ptr<IFence>( m_device->CreateFence( ) ) );
        m_imageAvailableSemaphores.push_back( std::unique_ptr<ISemaphore>( m_device->CreateSemaphore( ) ) );
        m_renderFinishedSemaphores.push_back( std::unique_ptr<ISemaphore>( m_device->CreateSemaphore( ) ) );
    }

    auto commandLists = m_commandListPool->GetCommandLists( );
    for ( uint32_t i = 0; i < commandLists.NumElements; ++i )
    {
        m_commandLists.push_back( commandLists.Elements[ i ] );
    }
}

uint32_t FrameSync::NextFrame( )
{
    m_currentFrame = m_nextFrame;
    m_nextFrame    = ( m_nextFrame + 1 ) % m_numFrames;
    m_frameFences[ m_currentFrame ]->Wait( );
    return m_currentFrame;
}

IFence *FrameSync::GetFrameFence( const uint32_t frame ) const
{
    return m_frameFences[ frame ].get( );
}

ISemaphore *FrameSync::GetPresentSignalSemaphore( const uint32_t frame ) const
{
    return m_renderFinishedSemaphores[ frame ].get( );
}

ICommandList *FrameSync::GetCommandList( const uint32_t frame ) const
{
    return m_commandLists[ frame ];
}

void FrameSync::ExecuteCommandList( const uint32_t frame, const ISemaphoreArray& additionalSemaphores ) const
{
    std::vector<ISemaphore*> waitSemaphores;
    waitSemaphores.push_back( m_imageAvailableSemaphores[ frame ].get( ) );
    for ( int i = 0; i < additionalSemaphores.NumElements; ++i )
    {
        waitSemaphores.push_back( additionalSemaphores.Elements[ i ] );
    }
    
    ISemaphore* signalSemaphore = m_renderFinishedSemaphores[ frame ].get( );
    ICommandList* commandList = GetCommandList( frame );
    
    ExecuteCommandListsDesc desc{ };
    desc.Signal = m_frameFences[ frame ].get( );
    desc.WaitSemaphores.Elements = waitSemaphores.data( );
    desc.WaitSemaphores.NumElements = static_cast<uint32_t>( waitSemaphores.size( ) );
    desc.SignalSemaphores.Elements = &signalSemaphore;
    desc.SignalSemaphores.NumElements = 1;
    desc.CommandLists.Elements = &commandList;
    desc.CommandLists.NumElements = 1;
    m_commandQueue->ExecuteCommandLists( desc );
}

uint32_t FrameSync::AcquireNextImage( const uint32_t frame ) const
{
    return m_swapChain->AcquireNextImage( m_imageAvailableSemaphores[ frame ].get( ) );
}

PresentResult FrameSync::Present( const uint32_t imageIndex ) const
{
    ISemaphore* waitSemaphore = m_renderFinishedSemaphores[ m_currentFrame ].get( );
    
    PresentDesc presentDesc{ };
    presentDesc.Image = imageIndex;
    presentDesc.WaitSemaphores.Elements = &waitSemaphore;
    presentDesc.WaitSemaphores.NumElements = 1;
    return m_swapChain->Present( presentDesc );
}

void FrameSync::WaitIdle( ) const
{
    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        m_frameFences[ m_currentFrame ]->Wait( );
    }
}
