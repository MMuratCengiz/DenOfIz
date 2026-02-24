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

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a pipeline cache for storing and reusing compiled pipeline state.
     *
     * A pipeline cache stores the compiled bytecode of pipeline state objects (PSOs) to disk,
     * allowing reuse across application sessions. This reduces pipeline creation time on
     * subsequent launches since shader compilation can be skipped for cached pipelines.
     *
     * The cache is populated automatically when pipelines are created with the cache
     * parameter. Cache data can be retrieved via GetData and persisted to disk, then
     * provided to a new cache via PipelineCacheDesc.Data on next launch.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12PipelineLibrary
     * - **Vulkan**: VkPipelineCache
     * - **Metal**: MTLBinaryArchive
     * - **WebGPU**: Not supported (stub implementation, returns empty data)
     *
     * @par Usage
     * 1. Create cache with existing data (or empty for first run)
     * 2. Pass cache to pipeline creation via DenOfIz_PipelineDesc.Cache
     * 3. After creating all pipelines, retrieve data via GetData
     * 4. Save data to disk for next application launch
     *
     * @par Example
     * @code
     * // Load from file or create empty
     * DenOfIz_PipelineCacheDesc cacheDesc = {0};
     * cacheDesc.Data     = loadedCacheData;  // NULL for first run
     * cacheDesc.DataSize = loadedCacheSize;  // 0 for first run
     *
     * DenOfIz_PipelineCache cache;
     * DenOfIz_LogicalDevice_CreatePipelineCache(device, &cacheDesc, &cache);
     *
     * // Create pipelines with cache
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.Cache = cache;
     * // ... configure pipeline ...
     * DenOfIz_LogicalDevice_CreatePipeline(device, &pipelineDesc, &pipeline);
     *
     * // Save cache for next launch
     * size_t numBytes = 0;
     * DenOfIz_PipelineCache_GetDataNumBytes(cache, &numBytes);
     * DenOfIz_ByteArray cacheData = {0};
     * cacheData.Elements    = malloc(numBytes);
     * cacheData.NumElements = numBytes;
     * DenOfIz_PipelineCache_GetData(cache, &cacheData);
     * // Write cacheData to disk...
     *
     * DenOfIz_PipelineCache_Destroy(cache);
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreatePipelineCache
     * @see DenOfIz_PipelineCacheDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_PipelineCache )

    /**
     * @brief Describes parameters for creating a pipeline cache.
     *
     * To create an empty cache for first run, set both Data and DataSize to 0.
     * To load a previously saved cache, provide the data retrieved from
     * DenOfIz_PipelineCache_GetData in a prior session.
     */
    typedef struct DenOfIz_PipelineCacheDesc
    {
        void  *Data;     /**< Pointer to previously saved cache data, or NULL for empty cache. */
        size_t DataSize; /**< Size of cache data in bytes, or 0 for empty cache. */
    } DenOfIz_PipelineCacheDesc;

    /**
     * @brief Retrieves the size in bytes needed to store the pipeline cache data.
     *
     * Call this before GetData to allocate a buffer of sufficient size.
     *
     * @param cache Valid pipeline cache handle.
     * @param[out] outNumBytes Receives the size in bytes. Returns 0 if cache is empty.
     *
     * @par Valid Usage
     * - @p cache must be a valid DenOfIz_PipelineCache handle
     * - @p outNumBytes must not be NULL
     */
    DZ_API void DenOfIz_PipelineCache_GetDataNumBytes( DenOfIz_PipelineCache cache, size_t *outNumBytes );

    /**
     * @brief Retrieves the pipeline cache data for persistence.
     *
     * The returned data can be saved to disk and later provided to
     * DenOfIz_PipelineCacheDesc.Data when creating a new cache.
     *
     * @param cache Valid pipeline cache handle.
     * @param[out] data Pre-allocated byte array to receive cache data.
     *                  NumElements must be >= value returned by GetDataNumBytes.
     *
     * @par Valid Usage
     * - @p cache must be a valid DenOfIz_PipelineCache handle
     * - @p data must not be NULL
     * - @p data->Elements must point to a buffer of at least GetDataNumBytes() size
     * - @p data->NumElements must be >= GetDataNumBytes()
     */
    DZ_API void DenOfIz_PipelineCache_GetData( DenOfIz_PipelineCache cache, DenOfIz_ByteArray *data );

    /**
     * @brief Destroys a pipeline cache and releases associated resources.
     *
     * @param cache Pipeline cache handle to destroy. May be DENOFIZ_NULL_HANDLE.
     */
    DZ_API void DenOfIz_PipelineCache_Destroy( DenOfIz_PipelineCache cache );

#ifdef __cplusplus
}
#endif
