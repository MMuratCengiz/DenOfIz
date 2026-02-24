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
     * @brief Opaque handle to a GPU-GPU synchronization primitive.
     *
     * A semaphore coordinates execution order between command queue submissions on the GPU.
     * Unlike fences (CPU-GPU sync), semaphores operate entirely on the GPU timeline.
     * One submission signals the semaphore, another waits on it before starting execution.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12Fence (timeline semaphore)
     * - **Vulkan**: VkSemaphore (timeline semaphore)
     * - **Metal**: MTLEvent with command buffer completion handler
     * - **WebGPU**: Emulated via atomic flag (limited functionality)
     *
     * @par Usage
     * Semaphores are used to order GPU work between submissions, such as ensuring a
     * shadow map pass completes before the main rendering pass that samples from it.
     * They are passed to ExecuteCommandLists via WaitSemaphores and SignalSemaphores.
     *
     * @par Example
     * @code
     * DenOfIz_Semaphore deferredSemaphore;
     * DenOfIz_LogicalDevice_CreateSemaphore(device, &deferredSemaphore);
     *
     * // First pass: render to texture, signal semaphore when done
     * DenOfIz_ExecuteCommandListsDesc exec1 = {0};
     * exec1.CommandLists.Elements        = &deferredCmd;
     * exec1.CommandLists.NumElements     = 1;
     * exec1.SignalSemaphores.Elements    = &deferredSemaphore;
     * exec1.SignalSemaphores.NumElements = 1;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &exec1);
     *
     * // Second pass: sample from texture, wait for semaphore first
     * DenOfIz_ExecuteCommandListsDesc exec2 = {0};
     * exec2.CommandLists.Elements      = &mainCmd;
     * exec2.CommandLists.NumElements   = 1;
     * exec2.WaitSemaphores.Elements    = &deferredSemaphore;
     * exec2.WaitSemaphores.NumElements = 1;
     * exec2.Signal                     = frameFence;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &exec2);
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateSemaphore
     * @see DenOfIz_ExecuteCommandListsDesc
     * @see DenOfIz_Fence (for CPU-GPU synchronization)
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Semaphore )

    /**
     * @brief Array of semaphore handles for batch operations.
     *
     * Used in ExecuteCommandListsDesc to specify multiple semaphores to wait on
     * or signal in a single submission.
     */
    typedef struct DenOfIz_SemaphoreArray
    {
        DenOfIz_Semaphore *Elements;    /**< Pointer to array of semaphore handles. */
        size_t             NumElements; /**< Number of semaphores in the array. */
    } DenOfIz_SemaphoreArray;

    /**
     * @brief Manually signals the semaphore from the CPU.
     *
     * In normal usage, semaphores are signaled by the GPU via SignalSemaphores in
     * ExecuteCommandLists. This function allows CPU-side signaling for special cases.
     *
     * @param semaphore Semaphore handle to signal.
     *
     * @par Valid Usage
     * - @p semaphore must be a valid DenOfIz_Semaphore handle or DENOFIZ_NULL_HANDLE
     * - If @p semaphore is DENOFIZ_NULL_HANDLE, this function returns immediately
     *
     * @note Typically not needed in standard rendering workflows. Use SignalSemaphores
     *       in ExecuteCommandLists for GPU-side signaling.
     */
    DZ_API void DenOfIz_Semaphore_Notify( DenOfIz_Semaphore semaphore );

    /**
     * @brief Queries whether the semaphore has been signaled.
     *
     * Non-blocking check of semaphore state. Returns true if the semaphore has been
     * signaled since the last wait or reset.
     *
     * @param semaphore Semaphore handle to query.
     * @param[out] outCompleted Receives true if signaled, false otherwise.
     *
     * @par Valid Usage
     * - @p semaphore must be a valid DenOfIz_Semaphore handle or DENOFIZ_NULL_HANDLE
     * - @p outCompleted must not be NULL (no-op if NULL)
     * - If @p semaphore is DENOFIZ_NULL_HANDLE, *outCompleted is set to false
     */
    DZ_API void DenOfIz_Semaphore_IsCompleted( DenOfIz_Semaphore semaphore, bool *outCompleted );

    /**
     * @brief Destroys the semaphore and releases associated resources.
     *
     * @param semaphore Semaphore handle to destroy.
     *
     * @par Valid Usage
     * - @p semaphore must be a valid DenOfIz_Semaphore handle or DENOFIZ_NULL_HANDLE
     * - If @p semaphore is DENOFIZ_NULL_HANDLE, this function returns immediately
     * - No pending GPU work should be waiting on or signaling this semaphore
     * - After destruction, the handle is invalid and must not be used
     */
    DZ_API void DenOfIz_Semaphore_Destroy( DenOfIz_Semaphore semaphore );

#ifdef __cplusplus
}
#endif
