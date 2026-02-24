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

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Handle to a query pool for GPU performance measurement.
     *
     * A query pool manages a collection of GPU queries for measuring occlusion,
     * pipeline statistics, or timestamps. Queries are recorded via CommandList
     * functions (BeginQuery, EndQuery) and results retrieved after GPU execution.
     *
     * @par Backend Implementations
     * - DirectX12: ID3D12QueryHeap + ID3D12Resource (readback buffer)
     * - Vulkan: VkQueryPool
     * - Metal: MTLCounterSampleBuffer (statistics) / MTLBuffer (occlusion)
     * - WebGPU: WGPUQuerySet + WGPUBuffer (readback)
     *
     * @par Query Lifecycle
     * 1. Create pool with DenOfIz_LogicalDevice_CreateQueryPool
     * 2. Reset queries: DenOfIz_CommandList_ResetQuery
     * 3. Record queries: DenOfIz_CommandList_BeginQuery / EndQuery
     * 4. Resolve results: DenOfIz_CommandList_ResolveQuery
     * 5. Submit command list and wait for completion
     * 6. Read results: DenOfIz_QueryPool_GetQueryData
     *
     * @par Usage Example
     * @code
     * DenOfIz_QueryPoolDesc poolDesc = {0};
     * poolDesc.Type       = DENOFIZ_QUERY_TYPE_TIMESTAMP;
     * poolDesc.NumQueries = 2;
     *
     * DenOfIz_QueryPool queryPool;
     * DenOfIz_LogicalDevice_CreateQueryPool(device, &poolDesc, &queryPool);
     *
     * // In command list recording:
     * DenOfIz_QueryDesc queryDesc = { .Index = 0 };
     * DenOfIz_CommandList_ResetQuery(cmd, queryPool, 0, 2);
     * DenOfIz_CommandList_BeginQuery(cmd, queryPool, &queryDesc);
     * // ... draw calls ...
     * queryDesc.Index = 1;
     * DenOfIz_CommandList_EndQuery(cmd, queryPool, &queryDesc);
     * DenOfIz_CommandList_ResolveQuery(cmd, queryPool, 0, 2);
     *
     * // After GPU execution completes:
     * DenOfIz_QueryData data;
     * DenOfIz_QueryPool_GetQueryData(queryPool, 0, &data);
     * double frequency;
     * DenOfIz_QueryPool_GetTimestampFrequency(queryPool, &frequency);
     * double elapsedMs = (data.EndTimestamp - data.BeginTimestamp) / frequency * 1000.0;
     * @endcode
     *
     * @see DenOfIz_QueryPoolDesc
     * @see DenOfIz_QueryData
     * @see DenOfIz_LogicalDevice_CreateQueryPool
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_QueryPool )

    /**
     * @brief Descriptor for creating a query pool.
     */
    typedef struct DenOfIz_QueryPoolDesc
    {
        /**
         * @brief Type of queries this pool will contain.
         *
         * - DENOFIZ_QUERY_TYPE_OCCLUSION: Counts pixels passing depth/stencil tests
         * - DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS: Shader invocation counts (requires PipelineStatisticFlags)
         * - DENOFIZ_QUERY_TYPE_TIMESTAMP: GPU timestamps for profiling
         */
        DenOfIz_QueryType Type;

        /**
         * @brief Number of queries in the pool.
         *
         * Must be > 0. For timestamp queries, typically use pairs (begin/end).
         */
        uint32_t NumQueries;

        /**
         * @brief Flags for pipeline statistics queries.
         *
         * Only used when Type is DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS.
         * Combine flags with bitwise OR to select which statistics to collect.
         *
         * @see DenOfIz_QueryPipelineStatisticFlagBits
         */
        DenOfIz_QueryPipelineStatisticFlags PipelineStatisticFlags;
    } DenOfIz_QueryPoolDesc;

    /**
     * @brief Returns the query type of the pool.
     *
     * @param queryPool Valid query pool handle.
     * @param[out] outType Receives the query type.
     *
     * @par Valid Usage
     * - @p queryPool must be a valid DenOfIz_QueryPool handle
     * - @p outType must not be NULL
     */
    DZ_API void DenOfIz_QueryPool_GetType( DenOfIz_QueryPool queryPool, DenOfIz_QueryType *outType );

    /**
     * @brief Returns the number of queries in the pool.
     *
     * @param queryPool Valid query pool handle.
     * @param[out] outNumQueries Receives the query count.
     *
     * @par Valid Usage
     * - @p queryPool must be a valid DenOfIz_QueryPool handle
     * - @p outNumQueries must not be NULL
     */
    DZ_API void DenOfIz_QueryPool_GetNumQueries( DenOfIz_QueryPool queryPool, uint32_t *outNumQueries );

    /**
     * @brief Returns the timestamp frequency for converting timestamps to time.
     *
     * The frequency is in ticks per second. To convert a timestamp delta to
     * milliseconds: (endTimestamp - beginTimestamp) / frequency * 1000.0
     *
     * @param queryPool Valid query pool handle.
     * @param[out] outFrequency Receives the frequency in Hz (ticks per second).
     *
     * @par Valid Usage
     * - @p queryPool must be a valid DenOfIz_QueryPool handle
     * - @p outFrequency must not be NULL
     * - Pool should be created with DENOFIZ_QUERY_TYPE_TIMESTAMP for meaningful results
     */
    DZ_API void DenOfIz_QueryPool_GetTimestampFrequency( DenOfIz_QueryPool queryPool, double *outFrequency );

    /**
     * @brief Retrieves the results of a completed query.
     *
     * Queries must be resolved (via DenOfIz_CommandList_ResolveQuery) and the
     * command list must have completed execution before reading results.
     *
     * @param queryPool Valid query pool handle.
     * @param queryIndex Index of the query to read. Must be < NumQueries.
     * @param[out] outData Receives the query results.
     *
     * @par Valid Usage
     * - @p queryPool must be a valid DenOfIz_QueryPool handle
     * - @p queryIndex must be < the pool's NumQueries
     * - @p outData must not be NULL
     * - The query must have been resolved and GPU execution completed
     *
     * @par Return Values (via outData)
     * - outData->Valid: true if query completed successfully
     * - outData->BeginTimestamp / EndTimestamp: For timestamp queries
     * - outData->OcclusionCounts: For occlusion queries
     * - outData->PipelineStats: For pipeline statistics queries
     */
    DZ_API void DenOfIz_QueryPool_GetQueryData( DenOfIz_QueryPool queryPool, uint32_t queryIndex, DenOfIz_QueryData *outData );

    /**
     * @brief Destroys a query pool and releases its resources.
     *
     * @param queryPool Query pool handle to destroy.
     *
     * @par Valid Usage
     * - @p queryPool must be a valid DenOfIz_QueryPool handle or DENOFIZ_NULL_HANDLE
     * - The pool must not be in use by a pending command list
     */
    DZ_API void DenOfIz_QueryPool_Destroy( DenOfIz_QueryPool queryPool );

#ifdef __cplusplus
}
#endif
