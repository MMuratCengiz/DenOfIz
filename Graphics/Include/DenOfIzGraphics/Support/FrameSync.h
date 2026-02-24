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

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FrameSync )

    typedef struct DenOfIz_FrameSyncDesc
    {
        DenOfIz_LogicalDevice Device;
        DenOfIz_SwapChain     SwapChain;
        DenOfIz_CommandQueue  CommandQueue;
        uint32_t              NumFrames;
    } DenOfIz_FrameSyncDesc;

    DZ_API void DenOfIz_FrameSync_Create( const DenOfIz_FrameSyncDesc *desc, DenOfIz_FrameSync *outFrameSync );
    DZ_API void DenOfIz_FrameSync_NextFrame( DenOfIz_FrameSync frameSync, uint32_t *outFrameIndex );
    DZ_API void DenOfIz_FrameSync_GetFrameFence( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_Fence *outFence );
    DZ_API void DenOfIz_FrameSync_GetCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, DenOfIz_CommandList *outCommandList );
    DZ_API void DenOfIz_FrameSync_ExecuteCommandList( DenOfIz_FrameSync frameSync, uint32_t frame, const DenOfIz_SemaphoreArray *additionalSemaphores );
    DZ_API void DenOfIz_FrameSync_ExecuteCommandListWithSemaphores( DenOfIz_FrameSync frameSync, uint32_t frame, const DenOfIz_SemaphoreArray *waitSemaphores,
                                                                    const DenOfIz_SemaphoreArray *signalSemaphores );
    DZ_API void DenOfIz_FrameSync_AcquireNextImage( DenOfIz_FrameSync frameSync, uint32_t *outImageIndex );
    DZ_API void DenOfIz_FrameSync_Present( DenOfIz_FrameSync frameSync, uint32_t imageIndex, DenOfIz_PresentResult *outResult );
    DZ_API void DenOfIz_FrameSync_WaitIdle( DenOfIz_FrameSync frameSync );
    DZ_API void DenOfIz_FrameSync_Destroy( DenOfIz_FrameSync frameSync );

#ifdef __cplusplus
}
#endif
