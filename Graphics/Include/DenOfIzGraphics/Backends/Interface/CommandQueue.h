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

#include "CommandList.h"
#include "DenOfIzGraphics/Handle.h"
#include "Fence.h"
#include "Semaphore.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a command queue for GPU work submission.
     *
     * A command queue is used to submit recorded command lists for GPU execution.
     * Different queue types support different operations (graphics, compute, copy).
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12CommandQueue
     * - **Vulkan**: VkQueue
     * - **Metal**: MTLCommandQueue
     * - **WebGPU**: WGPUQueue
     *
     * @par Usage
     * Command queues execute command lists submitted via ExecuteCommandLists.
     * Synchronization is done via fences and semaphores.
     *
     * @par Example
     * @code
     * DenOfIz_ExecuteCommandListsDesc execDesc = {0};
     * execDesc.CommandLists.Elements           = &commandList;
     * execDesc.CommandLists.NumElements        = 1;
     * execDesc.Signal                          = fence;
     * execDesc.WaitSemaphores.Elements         = &acquireSemaphore;
     * execDesc.WaitSemaphores.NumElements      = 1;
     * execDesc.SignalSemaphores.Elements       = &presentSemaphore;
     * execDesc.SignalSemaphores.NumElements    = 1;
     *
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &execDesc);
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateCommandQueue
     * @see DenOfIz_CommandQueue_ExecuteCommandLists
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandQueue )

    /**
     * @brief Flags for command queue creation.
     */
    typedef struct DenOfIz_CommandQueueFlags
    {
        bool RequirePresentationSupport; /**< Queue must support presenting to swapchains. */
    } DenOfIz_CommandQueueFlags;

    /**
     * @brief Queue scheduling priority.
     */
    typedef enum DenOfIz_QueuePriority
    {
        DENOFIZ_QUEUE_PRIORITY_LOW,    /**< Low priority queue. */
        DENOFIZ_QUEUE_PRIORITY_NORMAL, /**< Normal priority queue (default). */
        DENOFIZ_QUEUE_PRIORITY_HIGH    /**< High priority queue. */
    } DenOfIz_QueuePriority;

    /**
     * @brief Describes command queue creation parameters.
     */
    typedef struct DenOfIz_CommandQueueDesc
    {
        DenOfIz_QueueType         QueueType; /**< Type of queue (Graphics, Compute, or Copy). */
        DenOfIz_QueuePriority     Priority;  /**< Scheduling priority. */
        DenOfIz_CommandQueueFlags Flags;     /**< Additional flags. */
    } DenOfIz_CommandQueueDesc;

    /**
     * @brief Describes command list execution parameters.
     *
     * Configures which command lists to execute and synchronization primitives.
     */
    typedef struct DenOfIz_ExecuteCommandListsDesc
    {
        DenOfIz_Fence            Signal;           /**< Fence to signal when execution completes. May be NULL. */
        DenOfIz_CommandListArray CommandLists;     /**< Command lists to execute. */
        DenOfIz_SemaphoreArray   WaitSemaphores;   /**< Semaphores to wait on before execution. */
        DenOfIz_SemaphoreArray   SignalSemaphores; /**< Semaphores to signal after execution. */
    } DenOfIz_ExecuteCommandListsDesc;

    /**
     * @brief Waits for all submitted work on the queue to complete.
     *
     * Blocks the calling thread until all command lists submitted to this queue finish.
     *
     * @param queue Valid command queue handle.
     *
     * @par Valid Usage
     * - @p queue must be a valid DenOfIz_CommandQueue handle
     *
     * @note This is a heavyweight synchronization. Prefer fences for fine-grained sync.
     */
    DZ_API void DenOfIz_CommandQueue_WaitIdle( DenOfIz_CommandQueue queue );

    /**
     * @brief Submits command lists for execution on the GPU.
     *
     * Command lists are executed in order. The optional fence is signaled when all
     * command lists complete. Wait semaphores must be signaled before execution begins,
     * and signal semaphores are signaled when execution completes.
     *
     * @param queue Valid command queue handle.
     * @param executeCommandListsDesc Execution parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p queue must be a valid DenOfIz_CommandQueue handle
     * - @p executeCommandListsDesc must not be NULL
     * - All command lists must be in ended state (after End, before Begin)
     * - Command lists must be compatible with the queue type
     * - Wait semaphores must eventually be signaled to avoid deadlock
     *
     * @par Example
     * @code
     * DenOfIz_ExecuteCommandListsDesc execDesc = {0};
     * execDesc.CommandLists.Elements    = &commandList;
     * execDesc.CommandLists.NumElements = 1;
     * execDesc.Signal                   = frameFence;
     *
     * DenOfIz_CommandQueue_ExecuteCommandLists(graphicsQueue, &execDesc);
     * DenOfIz_Fence_Wait(frameFence);
     * @endcode
     */
    DZ_API void DenOfIz_CommandQueue_ExecuteCommandLists( DenOfIz_CommandQueue queue, const DenOfIz_ExecuteCommandListsDesc *executeCommandListsDesc );

    /**
     * @brief Destroys the command queue.
     *
     * @param queue Command queue handle to destroy.
     *
     * @par Valid Usage
     * - All submitted work must be complete before destruction
     * - After destruction, the handle is invalid
     */
    DZ_API void DenOfIz_CommandQueue_Destroy( DenOfIz_CommandQueue queue );

#ifdef __cplusplus
}
#endif
