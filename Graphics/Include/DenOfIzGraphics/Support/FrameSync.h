/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
     * Waits for the frame's previous work and acquires the back buffer. Returns DENOFIZ_SUCCESS when there is something to render to, on
     * any other value the frame must be skipped and EndFrame must not be called.
     */
    DZ_API DenOfIz_Result DenOfIz_FrameSync_BeginFrame( DenOfIz_FrameSync frameSync, DenOfIz_Frame *outFrame );
    /** Submits the frame's command list, waiting on the optional waitSemaphores, and presents. */
    DZ_API DenOfIz_Result DenOfIz_FrameSync_EndFrame( DenOfIz_FrameSync frameSync, const DenOfIz_SemaphoreArray *waitSemaphores );
    DZ_API void           DenOfIz_FrameSync_GetFrameFence( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_Fence *outFence );
    DZ_API void           DenOfIz_FrameSync_GetCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_CommandList *outCommandList );
    /** Waits for every frame's submitted work. */
    DZ_API void DenOfIz_FrameSync_WaitIdle( DenOfIz_FrameSync frameSync );
    /** Destroys its fences and command lists immediately, call WaitIdle first. The swap chain and queue are not destroyed. */
    DZ_API void DenOfIz_FrameSync_Destroy( DenOfIz_FrameSync frameSync );

#ifdef __cplusplus
}
#endif
