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
#include "CommandQueue.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a command list pool.
     *
     * A command list pool manages allocation and reuse of command lists. It is associated
     * with a command queue and allocates command lists compatible with that queue type.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12CommandAllocator (one per command list)
     * - **Vulkan**: VkCommandPool
     * - **Metal**: Allocates MTLCommandBuffer from MTLCommandQueue
     * - **WebGPU**: Manages WGPUCommandEncoder allocation
     *
     * @par Usage
     * Command list pools are created via DenOfIz_LogicalDevice_CreateCommandListPool.
     * Use GetCommandLists to retrieve the allocated command lists.
     *
     * @see DenOfIz_LogicalDevice_CreateCommandListPool
     * @see DenOfIz_CommandList
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandListPool )

    /**
     * @brief Describes command list pool creation parameters.
     */
    typedef struct DenOfIz_CommandListPoolDesc
    {
        DenOfIz_CommandQueue CommandQueue;    /**< Queue to allocate command lists for. */
        uint32_t             NumCommandLists; /**< Number of command lists to allocate. */
    } DenOfIz_CommandListPoolDesc;

    /**
     * @brief Retrieves the command lists allocated by the pool.
     *
     * @param pool Valid command list pool handle.
     * @param[out] outCommandLists Receives array of command list handles.
     *
     * @par Valid Usage
     * - @p pool must be a valid DenOfIz_CommandListPool handle
     * - @p outCommandLists must not be NULL
     */
    DZ_API void DenOfIz_CommandListPool_GetCommandLists( DenOfIz_CommandListPool pool, DenOfIz_CommandListArray *outCommandLists );

    /**
     * @brief Destroys the command list pool and all its command lists.
     *
     * @param pool Command list pool handle to destroy.
     *
     * @par Valid Usage
     * - No command lists from this pool may be executing on a queue
     * - After destruction, all command list handles from this pool are invalid
     */
    DZ_API void DenOfIz_CommandListPool_Destroy( DenOfIz_CommandListPool pool );

#ifdef __cplusplus
}
#endif
