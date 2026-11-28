/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Semaphore.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Data/TextureData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "Geometry.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_CopyToGpuBufferDesc
    {
        DenOfIz_Buffer        DstBuffer;
        uint64_t              DstBufferOffset;
        DenOfIz_ByteArrayView Data;
    } DenOfIz_CopyToGpuBufferDesc;

    typedef struct DenOfIz_CopyDataToTextureDesc
    {
        DenOfIz_Texture       DstTexture;
        DenOfIz_ByteArrayView Data;
        bool                  AutoAlign;
        uint32_t              Width;
        uint32_t              Height;
        uint32_t              ArrayLayer;
        uint32_t              MipLevel;
    } DenOfIz_CopyDataToTextureDesc;

    typedef struct DenOfIz_LoadTextureDesc
    {
        DenOfIz_StringView File;
        DenOfIz_Texture    DstTexture;
    } DenOfIz_LoadTextureDesc;

    typedef struct DenOfIz_LoadTextureFromDataDesc
    {
        DenOfIz_TextureData TextureData;
        DenOfIz_Texture     DstTexture;
    } DenOfIz_LoadTextureFromDataDesc;

    typedef struct DenOfIz_BatchResourceCopyDesc
    {
        DenOfIz_LogicalDevice Device;
        bool                  IssueBarriers;
    } DenOfIz_BatchResourceCopyDesc;

    /**
     * Records uploads between Begin and Submit and runs them on its own copy queue. With IssueBarriers a second submission on its own
     * graphics queue follows the copies and moves the textures written by CopyDataToTexture and the Load functions, and the buffers made
     * by the Create functions, to DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT. One instance serves any number of Begin and Submit cycles,
     * Begin and Destroy wait for the previous submission.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BatchResourceCopy )

    DZ_API void            DenOfIz_BatchResourceCopy_Create( const DenOfIz_BatchResourceCopyDesc *desc, DenOfIz_BatchResourceCopy *outBatchResourceCopy );
    DZ_API void            DenOfIz_BatchResourceCopy_Begin( DenOfIz_BatchResourceCopy batchResourceCopy );
    DZ_API void            DenOfIz_BatchResourceCopy_CopyToGPUBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyToGpuBufferDesc *copyDesc );
    DZ_API void            DenOfIz_BatchResourceCopy_CopyBufferRegion( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyBufferRegionDesc *copyDesc );
    DZ_API void            DenOfIz_BatchResourceCopy_CopyTextureRegion( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyTextureRegionDesc *copyDesc );
    DZ_API void            DenOfIz_BatchResourceCopy_CopyDataToTexture( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyDataToTextureDesc *copyDesc );
    DZ_API DenOfIz_Texture DenOfIz_BatchResourceCopy_CreateAndLoadTexture( DenOfIz_BatchResourceCopy batchResourceCopy, DenOfIz_StringView file );
    DZ_API void            DenOfIz_BatchResourceCopy_LoadTexture( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_LoadTextureDesc *loadDesc );
    DZ_API void            DenOfIz_BatchResourceCopy_LoadTextureFromData( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_LoadTextureFromDataDesc *loadDesc );
    DZ_API DenOfIz_Buffer  DenOfIz_BatchResourceCopy_CreateUniformBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_ByteArrayView *data, uint32_t numBytes );
    DZ_API DenOfIz_Buffer  DenOfIz_BatchResourceCopy_CreateGeometryVertexBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_GeometryData *geometryData );
    DZ_API DenOfIz_Buffer  DenOfIz_BatchResourceCopy_CreateGeometryIndexBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_GeometryData *geometryData );
    /**
     * notify is optional. Without it Submit returns once the uploads are finalised when IssueBarriers is set, and right after submitting
     * the copies otherwise. With it Submit never waits: the last submission of the batch signals notify, the barrier submission when
     * IssueBarriers is set and the copy submission otherwise, so work that waits on notify sees the uploaded data in its final state.
     */
    DZ_API void            DenOfIz_BatchResourceCopy_Submit( DenOfIz_BatchResourceCopy batchResourceCopy, DenOfIz_Semaphore notify );
    DZ_API void            DenOfIz_BatchResourceCopy_Destroy( DenOfIz_BatchResourceCopy batchResourceCopy );

#ifdef __cplusplus
}
#endif
