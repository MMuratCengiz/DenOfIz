/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Recorded with DenOfIz_CommandList_ResetQuery, BeginQuery, EndQuery and ResolveQuery. A timestamp query is a pair: BeginQuery and
     * EndQuery with the same Index write its BeginTimestamp and EndTimestamp. Metal has no pipeline statistics queries.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_QueryPool )

    typedef struct DenOfIz_QueryPoolDesc
    {
        DenOfIz_QueryType                   Type;
        uint32_t                            NumQueries;
        DenOfIz_QueryPipelineStatisticFlags PipelineStatisticFlags; /**< PIPELINE_STATISTICS pools only. */
    } DenOfIz_QueryPoolDesc;

    DZ_API void DenOfIz_QueryPool_GetType( DenOfIz_QueryPool queryPool, DenOfIz_QueryType *outType );
    DZ_API void DenOfIz_QueryPool_GetNumQueries( DenOfIz_QueryPool queryPool, uint32_t *outNumQueries );
    /** Timestamp ticks per second. */
    DZ_API void DenOfIz_QueryPool_GetTimestampFrequency( DenOfIz_QueryPool queryPool, double *outFrequency );
    /**
     * Reads a query once the command list that resolved it finished executing. Valid is false when the result could not be read, only
     * Vulkan also reports results that are not available yet.
     */
    DZ_API void DenOfIz_QueryPool_GetQueryData( DenOfIz_QueryPool queryPool, uint32_t queryIndex, DenOfIz_QueryData *outData );
    /** Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_QueryPool_Destroy( DenOfIz_QueryPool queryPool );

#ifdef __cplusplus
}
#endif
