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
#include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>

using namespace DenOfIz;

CommandListRing::CommandListRing( ILogicalDevice *logicalDevice ) : m_logicalDevice( logicalDevice )
{
    CommandListPoolDesc createInfo{ };
    createInfo.QueueType = QueueType::Graphics;
    for ( uint32_t i = 0; i < m_desc.NumFrames; i++ )
    {
        createInfo.NumCommandLists = m_desc.NumCommandListsPerFrame;
        m_commandListPools.push_back( std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( createInfo ) ) );
    }

    if ( m_desc.CreateSyncObjects )
    {
        m_frameFences.resize( m_desc.NumFrames );
        m_imageReadySemaphores.resize( m_desc.NumFrames );
        m_imageRenderedSemaphores.resize( m_desc.NumFrames );

        for ( uint8_t i = 0; i < m_desc.NumFrames; ++i )
        {
            m_frameFences[ i ]             = std::unique_ptr<IFence>( logicalDevice->CreateFence( ) );
            m_imageReadySemaphores[ i ]    = std::unique_ptr<ISemaphore>( logicalDevice->CreateSemaphore( ) );
            m_imageRenderedSemaphores[ i ] = std::unique_ptr<ISemaphore>( logicalDevice->CreateSemaphore( ) );
        }
    }
}

void CommandListRing::NextFrame( )
{
    m_currentFrame = m_frame;
    m_frame        = ( m_frame + 1 ) % m_commandListPools.size( );
    if ( m_desc.CreateSyncObjects )
    {
        m_frameFences[ m_currentFrame ]->Wait( );
    }
}

ICommandList *CommandListRing::FrameCommandList( const uint32_t index )
{
    if ( index >= m_desc.NumCommandListsPerFrame )
    {
        LOG( ERROR ) << "index cannot be larger or equal @ref CommandListRingDesc::NuNumCommandListsPerFrame";
    }

    m_currentFrame                            = m_frame;
    InteropArray<ICommandList *> commandLists = m_commandListPools[ m_frame ]->GetCommandLists( );
    const auto                   next         = commandLists.GetElement( index );
    m_frame                                   = ( m_frame + 1 ) % m_commandListPools.size( );
    if ( m_desc.CreateSyncObjects )
    {
        m_frameFences[ m_currentFrame ]->Wait( );
    }
    return next;
}

void CommandListRing::ExecuteAndPresent( ICommandList *commandList, ISwapChain *swapChain, const uint32_t image ) const
{
    ExecuteDesc executeDesc{ };
    if ( !m_desc.CreateSyncObjects )
    {
        LOG( WARNING ) << "Probable developer error, CreateSyncObjects = false, without it this command list may not be synchronized.";
    }
    else
    {
        executeDesc.Notify = m_frameFences[ m_currentFrame ].get( );
        executeDesc.WaitOnSemaphores.AddElement( m_imageReadySemaphores[ m_currentFrame ].get( ) );
        executeDesc.NotifySemaphores.AddElement( m_imageRenderedSemaphores[ m_currentFrame ].get( ) );
    }

    commandList->Execute( executeDesc );
    InteropArray<ISemaphore *> semaphores;
    semaphores.AddElement( m_imageReadySemaphores[ m_currentFrame ].get( ) );
    commandList->Present( swapChain, image, semaphores );
}

/// Execute the last commandList in the pool of the current frame, this is different as it will notify the built-in Sync object.
/// @param commandList the last command list in this pool
void CommandListRing::ExecuteLast( ICommandList *commandList ) const
{
    ExecuteDesc executeDesc{ };
    if ( !m_desc.CreateSyncObjects )
    {
        LOG( WARNING ) << "Probable developer error, CreateSyncObjects = false, without it this command list may not be synchronized.";
    }
    else
    {
        executeDesc.Notify = m_frameFences[ m_currentFrame ].get( );
    }
    commandList->Execute( executeDesc );
}

void CommandListRing::WaitIdle( ) const
{
    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        m_frameFences[ i ]->Wait( );
    }
}

[[nodiscard]] uint32_t CommandListRing::CurrentImage( ISwapChain *swapChain ) const
{
    if ( !m_desc.CreateSyncObjects )
    {
        LOG( ERROR ) << "When providing your own sync objects make sure to call ISwapChain::AcquireNextImage yourself, to ensure the call is synchronized";
    }
    return swapChain->AcquireNextImage( m_imageReadySemaphores[ m_currentFrame ].get( ) );
}

[[nodiscard]] uint32_t CommandListRing::CurrentFrame( ) const
{
    return m_currentFrame;
}
