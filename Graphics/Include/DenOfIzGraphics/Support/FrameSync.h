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

#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Backends/Interface/Fence.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Semaphore.h"
#include "DenOfIzGraphics/Backends/Interface/SwapChain.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/Result.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FrameSync )

    typedef struct DenOfIz_FrameSyncDesc
    {
        DenOfIz_LogicalDevice    Device;
        DenOfIz_SwapChain        SwapChain;
        DenOfIz_CommandQueue     CommandQueue;
        uint32_t                 NumFrames;
        DenOfIz_ResourceTracking ResourceTracking; /**< Optional. When set, BeginFrame tracks the back buffer and the depth buffer. */
    } DenOfIz_FrameSyncDesc;

    typedef struct DenOfIz_Frame
    {
        uint32_t            Index;       /**< In [0, NumFrames), selects per-frame resources. */
        DenOfIz_CommandList CommandList; /**< Command list of this frame, submitted by EndFrame. */
        DenOfIz_Texture     BackBuffer;  /**< Valid until EndFrame. */
        DenOfIz_Texture     DepthBuffer; /**< DENOFIZ_NULL_HANDLE when the swap chain has no depth buffer. */
        uint32_t            Width;       /**< Size of the back buffer in pixels. */
        uint32_t            Height;
        bool                Resized;     /**< The size changed since the previous frame. The GPU is idle: recreate resources sized after the swap chain now. */
    } DenOfIz_Frame;

    DZ_API void DenOfIz_FrameSync_Create( const DenOfIz_FrameSyncDesc *desc, DenOfIz_FrameSync *outFrameSync );
    /**
     * @brief Waits for the frame's previous work and acquires the back buffer.
     * @return DENOFIZ_SUCCESS when there is something to render to. On any other value the frame must be skipped, EndFrame must not be called.
     */
    DZ_API DenOfIz_Result DenOfIz_FrameSync_BeginFrame( DenOfIz_FrameSync frameSync, DenOfIz_Frame *outFrame );
    /**
     * @brief Submits the frame's command list and presents.
     * @param waitSemaphores Optional semaphores the frame's command list waits on.
     */
    DZ_API DenOfIz_Result DenOfIz_FrameSync_EndFrame( DenOfIz_FrameSync frameSync, const DenOfIz_SemaphoreArray *waitSemaphores );
    DZ_API void           DenOfIz_FrameSync_GetFrameFence( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_Fence *outFence );
    DZ_API void           DenOfIz_FrameSync_GetCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_CommandList *outCommandList );
    DZ_API void           DenOfIz_FrameSync_WaitIdle( DenOfIz_FrameSync frameSync );
    DZ_API void           DenOfIz_FrameSync_Destroy( DenOfIz_FrameSync frameSync );

#ifdef __cplusplus
}
#endif
