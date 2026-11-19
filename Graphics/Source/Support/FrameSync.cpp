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
        uint32_t m_width        = 0;
        uint32_t m_height       = 0;
        bool     m_trackDepth   = true;

        DenOfIz_LogicalDevice    m_device;
        DenOfIz_SwapChain        m_swapChain;
        DenOfIz_CommandQueue     m_commandQueue;
        DenOfIz_ResourceTracking m_resourceTracking;
        DenOfIz_CommandListArray m_commandLists;

    public:
        explicit FrameSync( const DenOfIz_FrameSyncDesc &desc );
        ~FrameSync( );
        DenOfIz_Result      BeginFrame( DenOfIz_Frame &outFrame );
        DenOfIz_Result      EndFrame( const DenOfIz_SemaphoreArray &waitSemaphores ) const;
        DenOfIz_Fence       GetFrameFence( uint32_t frame ) const;
        DenOfIz_CommandList GetCommandList( uint32_t frame ) const;
        void                WaitIdle( ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

FrameSync::FrameSync( const DenOfIz_FrameSyncDesc &desc ) :
    m_numFrames( desc.NumFrames ), m_device( desc.Device ), m_swapChain( desc.SwapChain ), m_commandQueue( desc.CommandQueue ), m_resourceTracking( desc.ResourceTracking )
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

    DenOfIz_SwapChainDesc swapChainDesc{ };
    DenOfIz_SwapChain_GetDesc( m_swapChain, &swapChainDesc );
    m_width  = swapChainDesc.Width;
    m_height = swapChainDesc.Height;
}

FrameSync::~FrameSync( )
{
    DenOfIz_CommandListPool_Destroy( m_commandListPool );
    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        DenOfIz_Fence_Destroy( m_frameFences[ i ] );
    }
}

DenOfIz_Result FrameSync::BeginFrame( DenOfIz_Frame &outFrame )
{
    outFrame = { };

    m_currentFrame = m_nextFrame;
    m_nextFrame    = ( m_nextFrame + 1 ) % m_numFrames;
    DenOfIz_Fence_Wait( m_frameFences[ m_currentFrame ] );

    const DenOfIz_Result result = DenOfIz_SwapChain_AcquireNextImage( m_swapChain, &outFrame.BackBuffer );
    if ( result != DENOFIZ_SUCCESS )
    {
        return result;
    }

    DenOfIz_SwapChainDesc swapChainDesc{ };
    DenOfIz_SwapChain_GetDesc( m_swapChain, &swapChainDesc );
    DenOfIz_SwapChain_GetDepthBuffer( m_swapChain, &outFrame.DepthBuffer );

    outFrame.Index       = m_currentFrame;
    outFrame.CommandList = m_commandLists.Elements[ m_currentFrame ];
    outFrame.Width       = swapChainDesc.Width;
    outFrame.Height      = swapChainDesc.Height;
    outFrame.Resized     = swapChainDesc.Width != m_width || swapChainDesc.Height != m_height;
    m_width              = swapChainDesc.Width;
    m_height             = swapChainDesc.Height;

    if ( DENOFIZ_HANDLE_IS_VALID( m_resourceTracking ) )
    {
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, outFrame.BackBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
        if ( DENOFIZ_HANDLE_IS_VALID( outFrame.DepthBuffer ) && ( m_trackDepth || outFrame.Resized ) )
        {
            DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, outFrame.DepthBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
            m_trackDepth = false;
        }
    }
    return DENOFIZ_SUCCESS;
}

DenOfIz_Result FrameSync::EndFrame( const DenOfIz_SemaphoreArray &waitSemaphores ) const
{
    DenOfIz_CommandList commandList = m_commandLists.Elements[ m_currentFrame ];

    DenOfIz_ExecuteCommandListsDesc desc{ };
    desc.Signal                   = m_frameFences[ m_currentFrame ];
    desc.WaitSemaphores           = waitSemaphores;
    desc.CommandLists.Elements    = &commandList;
    desc.CommandLists.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( m_commandQueue, &desc );

    return DenOfIz_SwapChain_Present( m_swapChain );
}

DenOfIz_Fence FrameSync::GetFrameFence( const uint32_t frame ) const
{
    return m_frameFences[ frame ];
}

DenOfIz_CommandList FrameSync::GetCommandList( const uint32_t frame ) const
{
    return m_commandLists.Elements[ frame ];
}

void FrameSync::WaitIdle( ) const
{
    for ( uint32_t i = 0; i < m_numFrames; ++i )
    {
        DenOfIz_Fence_Wait( m_frameFences[ i ] );
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

        FrameSync *frameSync = new FrameSync( *desc );
        *outFrameSync        = DENOFIZ_TO_HANDLE( frameSync );
    }

    DenOfIz_Result DenOfIz_FrameSync_BeginFrame( DenOfIz_FrameSync frameSync, DenOfIz_Frame *outFrame )
    {
        if ( outFrame == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }

        return FRAME_SYNC_IMPL( frameSync )->BeginFrame( *outFrame );
    }

    DenOfIz_Result DenOfIz_FrameSync_EndFrame( DenOfIz_FrameSync frameSync, const DenOfIz_SemaphoreArray *waitSemaphores )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( frameSync ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }

        const DenOfIz_SemaphoreArray noSemaphores = { };
        return FRAME_SYNC_IMPL( frameSync )->EndFrame( waitSemaphores != NULL ? *waitSemaphores : noSemaphores );
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
