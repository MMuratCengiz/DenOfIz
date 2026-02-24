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
    DZ_API void            DenOfIz_BatchResourceCopy_Submit( DenOfIz_BatchResourceCopy batchResourceCopy, DenOfIz_Semaphore notify );
    DZ_API void            DenOfIz_BatchResourceCopy_Destroy( DenOfIz_BatchResourceCopy batchResourceCopy );

#ifdef __cplusplus
}
#endif
