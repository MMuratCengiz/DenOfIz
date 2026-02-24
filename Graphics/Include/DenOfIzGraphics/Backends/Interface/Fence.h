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

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a GPU-CPU synchronization primitive.
     *
     * A fence allows the CPU to wait for GPU work to complete. Fences are signaled by the GPU
     * when command list execution finishes and can be waited on by the CPU to ensure work
     * has completed before reusing resources.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12Fence with Win32 Event for CPU wait
     * - **Vulkan**: VkFence
     * - **Metal**: dispatch_semaphore_t with command buffer completion handler
     * - **WebGPU**: Emulated via wgpuQueueOnSubmittedWorkDone callback
     *
     * @par Usage
     * Fences are used for CPU-GPU synchronization in frame pacing. A typical pattern is to
     * create one fence per frame-in-flight and wait on the fence before reusing that frame's
     * resources. The fence is passed to ExecuteCommandLists via the Signal field and is
     * automatically signaled when the GPU completes execution.
     *
     * @par Example
     * @code
     * DenOfIz_Fence frameFences[3];
     * for (uint32_t i = 0; i < 3; ++i) {
     *     DenOfIz_LogicalDevice_CreateFence(device, &frameFences[i]);
     * }
     *
     * // In render loop:
     * DenOfIz_Fence_Wait(frameFences[currentFrame]);  // Wait for frame N-2 to complete
     * DenOfIz_Fence_Reset(frameFences[currentFrame]);
     *
     * DenOfIz_CommandList_Begin(commandList);
     * // ... record commands ...
     * DenOfIz_CommandList_End(commandList);
     *
     * DenOfIz_ExecuteCommandListsDesc execDesc = {0};
     * execDesc.Signal                   = frameFences[currentFrame];
     * execDesc.CommandLists.Elements    = &commandList;
     * execDesc.CommandLists.NumElements = 1;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &execDesc);
     *
     * currentFrame = (currentFrame + 1) % 3;
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateFence
     * @see DenOfIz_ExecuteCommandListsDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Fence )

    /**
     * @brief Blocks the calling thread until the fence is signaled.
     *
     * Waits indefinitely for the GPU to signal this fence. The fence is signaled when
     * all command lists submitted with this fence in ExecuteCommandListsDesc::Signal
     * have completed execution.
     *
     * @param fence Fence handle to wait on.
     *
     * @par Valid Usage
     * - @p fence must be a valid DenOfIz_Fence handle or DENOFIZ_NULL_HANDLE
     * - If @p fence is DENOFIZ_NULL_HANDLE, this function returns immediately
     * - The fence must have been submitted to a queue before waiting, otherwise
     *   behavior is undefined (may return immediately or block indefinitely)
     *
     * @note This is a blocking operation. For non-blocking status checks, use semaphores.
     */
    DZ_API void DenOfIz_Fence_Wait( DenOfIz_Fence fence );

    /**
     * @brief Resets the fence to unsignaled state.
     *
     * After waiting on a fence, reset it before reusing for another submission.
     * On some backends this is a no-op as the fence is automatically reset on signal.
     *
     * @param fence Fence handle to reset.
     *
     * @par Valid Usage
     * - @p fence must be a valid DenOfIz_Fence handle or DENOFIZ_NULL_HANDLE
     * - If @p fence is DENOFIZ_NULL_HANDLE, this function returns immediately
     * - The fence should be in signaled state (after Wait completes)
     * - Do not reset a fence that is still pending GPU signal
     */
    DZ_API void DenOfIz_Fence_Reset( DenOfIz_Fence fence );

    /**
     * @brief Destroys the fence and releases associated resources.
     *
     * @param fence Fence handle to destroy.
     *
     * @par Valid Usage
     * - @p fence must be a valid DenOfIz_Fence handle or DENOFIZ_NULL_HANDLE
     * - If @p fence is DENOFIZ_NULL_HANDLE, this function returns immediately
     * - The fence must not be pending signal (call Wait first)
     * - After destruction, the handle is invalid and must not be used
     */
    DZ_API void DenOfIz_Fence_Destroy( DenOfIz_Fence fence );

#ifdef __cplusplus
}
#endif
