/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Collects the pipelines created with it (DenOfIz_PipelineDesc::PipelineCache). Its data can be saved and passed to a new cache in a
     * later session to skip shader compilation.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_PipelineCache )

    /** Data from DenOfIz_PipelineCache_GetData, NULL for an empty cache. DX12 does not copy it: keep it alive until the cache is destroyed. */
    typedef struct DenOfIz_PipelineCacheDesc
    {
        void  *Data;
        size_t DataSize;
    } DenOfIz_PipelineCacheDesc;

    DZ_API void DenOfIz_PipelineCache_GetDataNumBytes( DenOfIz_PipelineCache cache, size_t *outNumBytes );
    /**
     * Serializes into caller allocated memory: Elements must hold at least GetDataNumBytes bytes. NumElements is set to the bytes written,
     * and left unchanged when nothing was written.
     */
    DZ_API void DenOfIz_PipelineCache_GetData( DenOfIz_PipelineCache cache, DenOfIz_ByteArray *data );
    DZ_API void DenOfIz_PipelineCache_Destroy( DenOfIz_PipelineCache cache );

#ifdef __cplusplus
}
#endif
