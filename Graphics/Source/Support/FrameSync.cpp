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

#include "DenOfIzGraphics/Support/FrameSync.h"

#include <vector>

namespace DenOfIz
{
    class FrameSync
    {
        uint32_t m_numFrames;

        std::vector<DenOfIz_Fence> m_frameFences;
        DenOfIz_CommandListPool    m_commandListPool;

        uint32_t m_currentFrame = 0;
        uint32_t m_nextFrame    = 0;

        DenOfIz_LogicalDevice    m_device;
        DenOfIz_SwapChain        m_swapChain;
        DenOfIz_CommandQueue     m_commandQueue;
        DenOfIz_CommandListArray m_commandLists;

    public:
        explicit FrameSync( const DenOfIz_FrameSyncDesc &desc );
        ~FrameSync( );
        uint32_t              NextFrame( );
        DenOfIz_Fence         GetFrameFence( uint32_t frame ) const;
        DenOfIz_CommandList   GetCommandList( uint32_t frame ) const;
        void                  ExecuteCommandList( uint32_t frame, const DenOfIz_SemaphoreArray &additionalSemaphores = { } ) const;
        void                  ExecuteCommandList( uint32_t frame, const DenOfIz_SemaphoreArray &waitSemaphores, const DenOfIz_SemaphoreArray &signalSemaphores ) const;
        uint32_t              AcquireNextImage( ) const;
        DenOfIz_PresentResult Present( uint32_t imageIndex ) const;
        void                  WaitIdle( ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

FrameSync::FrameSync( const DenOfIz_FrameSyncDesc &desc ) :
    m_numFrames( desc.NumFrames ), m_device( desc.Device ), m_swapChain( desc.SwapChain ), m_commandQueue( desc.CommandQueue )
{
    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = DENOFIZ_TO_HANDLE( m_commandQueue );
    poolDesc.NumCommandLists = m_numFrames;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &poolDesc, &m_commandListPool );

    m_frameFences.reserve( m_numFrames );

    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        DenOfIz_Fence fence;
        DenOfIz_LogicalDevice_CreateFence( m_device, &fence );
        m_frameFences.push_back( fence );
    }

    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &m_commandLists );
}

FrameSync::~FrameSync( )
{
    DenOfIz_CommandListPool_Destroy( m_commandListPool );
    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        DenOfIz_Fence_Destroy( m_frameFences[ i ] );
    }
}

uint32_t FrameSync::NextFrame( )
{
    m_currentFrame = m_nextFrame;
    m_nextFrame    = ( m_nextFrame + 1 ) % m_numFrames;
    DenOfIz_Fence_Wait( m_frameFences[ m_currentFrame ] );
    return m_currentFrame;
}

DenOfIz_Fence FrameSync::GetFrameFence( const uint32_t frame ) const
{
    return m_frameFences[ frame ];
}

DenOfIz_CommandList FrameSync::GetCommandList( const uint32_t frame ) const
{
    return m_commandLists.Elements[ frame ];
}

void FrameSync::ExecuteCommandList( const uint32_t frame, const DenOfIz_SemaphoreArray &additionalSemaphores ) const
{
    ExecuteCommandList( frame, additionalSemaphores, { } );
}

void FrameSync::ExecuteCommandList( const uint32_t frame, const DenOfIz_SemaphoreArray &waitSemaphores, const DenOfIz_SemaphoreArray &signalSemaphores ) const
{
    DenOfIz_CommandList commandListHandle = DENOFIZ_TO_HANDLE( GetCommandList( frame ) );

    DenOfIz_ExecuteCommandListsDesc desc{ };
    desc.Signal                   = DENOFIZ_TO_HANDLE( m_frameFences[ frame ] );
    desc.WaitSemaphores           = waitSemaphores;
    desc.SignalSemaphores         = signalSemaphores;
    desc.CommandLists.Elements    = &commandListHandle;
    desc.CommandLists.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( m_commandQueue, &desc );
}

uint32_t FrameSync::AcquireNextImage( ) const
{
    uint32_t nextImage;
    DenOfIz_SwapChain_AcquireNextImage( m_swapChain, &nextImage );
    return nextImage;
}

DenOfIz_PresentResult FrameSync::Present( const uint32_t imageIndex ) const
{
    return DenOfIz_SwapChain_Present( m_swapChain, imageIndex );
}

void FrameSync::WaitIdle( ) const
{
    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        DenOfIz_Fence_Wait( m_frameFences[ m_currentFrame ] );
    }
}

#define FRAME_SYNC_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FrameSync, handle )

extern "C"
{

    void DenOfIz_FrameSync_Create( const DenOfIz_FrameSyncDesc *desc, DenOfIz_FrameSync *outFrameSync )
    {
        if ( desc == NULL || outFrameSync == NULL )
        {
            return;
        }

        DenOfIz_FrameSyncDesc cppDesc{ };
        cppDesc.Device       = desc->Device;
        cppDesc.SwapChain    = desc->SwapChain;
        cppDesc.CommandQueue = desc->CommandQueue;
        cppDesc.NumFrames    = desc->NumFrames;

        FrameSync *frameSync = new FrameSync( cppDesc );
        *outFrameSync        = DENOFIZ_TO_HANDLE( frameSync );
    }

    void DenOfIz_FrameSync_NextFrame( DenOfIz_FrameSync frameSync, uint32_t *outFrameIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) || outFrameIndex == NULL )
        {
            return;
        }

        *outFrameIndex = FRAME_SYNC_IMPL( frameSync )->NextFrame( );
    }

    void DenOfIz_FrameSync_GetFrameFence( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_Fence *outFence )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) || outFence == NULL )
        {
            return;
        }

        *outFence = DENOFIZ_TO_HANDLE( FRAME_SYNC_IMPL( frameSync )->GetFrameFence( frame ) );
    }

    void DenOfIz_FrameSync_GetCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_CommandList *outCommandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) || outCommandList == NULL )
        {
            return;
        }

        *outCommandList = DENOFIZ_TO_HANDLE( FRAME_SYNC_IMPL( frameSync )->GetCommandList( frame ) );
    }

    void DenOfIz_FrameSync_ExecuteCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, const DenOfIz_SemaphoreArray *additionalSemaphores )
    {
        DenOfIz_SemaphoreArray        emptySemaphores = { };
        const DenOfIz_SemaphoreArray *waitSemaphores  = additionalSemaphores != NULL ? additionalSemaphores : &emptySemaphores;
        DenOfIz_FrameSync_ExecuteCommandListWithSemaphores( frameSync, frame, waitSemaphores, &emptySemaphores );
    }

    void DenOfIz_FrameSync_ExecuteCommandListWithSemaphores( DenOfIz_FrameSync frameSync, uint32_t frame, const DenOfIz_SemaphoreArray *waitSemaphores,
                                                             const DenOfIz_SemaphoreArray *signalSemaphores )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) )
        {
            return;
        }

        DenOfIz_SemaphoreArray emptyWait   = { };
        DenOfIz_SemaphoreArray emptySignal = { };

        const DenOfIz_SemaphoreArray *actualWait   = waitSemaphores != NULL ? waitSemaphores : &emptyWait;
        const DenOfIz_SemaphoreArray *actualSignal = signalSemaphores != NULL ? signalSemaphores : &emptySignal;

        FRAME_SYNC_IMPL( frameSync )->ExecuteCommandList( frame, *actualWait, *actualSignal );
    }

    void DenOfIz_FrameSync_AcquireNextImage( DenOfIz_FrameSync frameSync, uint32_t *outImageIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) || outImageIndex == NULL )
        {
            return;
        }

        *outImageIndex = FRAME_SYNC_IMPL( frameSync )->AcquireNextImage( );
    }

    void DenOfIz_FrameSync_Present( DenOfIz_FrameSync frameSync, uint32_t imageIndex, DenOfIz_PresentResult *outResult )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) || outResult == NULL )
        {
            return;
        }

        *outResult = FRAME_SYNC_IMPL( frameSync )->Present( imageIndex );
    }

    void DenOfIz_FrameSync_WaitIdle( DenOfIz_FrameSync frameSync )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) )
        {
            return;
        }

        FRAME_SYNC_IMPL( frameSync )->WaitIdle( );
    }

    void DenOfIz_FrameSync_Destroy( DenOfIz_FrameSync frameSync )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) )
        {
            return;
        }

        delete FRAME_SYNC_IMPL( frameSync );
    }
}
