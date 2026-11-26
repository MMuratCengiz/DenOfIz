/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Built on the GPU with DenOfIz_CommandList_BuildBottomLevelAS before a TLAS can reference it. */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BottomLevelAS )

    typedef enum DenOfIz_BLASGeometryFlagBits
    {
        DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT                          = 1 << 0, /**< Any hit shaders are not invoked. */
        DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT = 1 << 1,
    } DenOfIz_BLASGeometryFlagBits;
    typedef uint32_t DenOfIz_BLASGeometryFlags;

    /** Vertex and index buffers need DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT. Offsets and strides are in bytes. */
    typedef struct DenOfIz_ASGeometryTriangleDesc
    {
        DenOfIz_Buffer    VertexBuffer;
        uint32_t          VertexOffset;
        uint32_t          VertexStride;
        uint32_t          NumVertices;
        DenOfIz_Format    VertexFormat;
        DenOfIz_Buffer    IndexBuffer;
        uint32_t          IndexOffset;
        uint32_t          NumIndices; /**< Zero is non indexed geometry, IndexBuffer is then ignored. */
        DenOfIz_IndexType IndexType;
    } DenOfIz_ASGeometryTriangleDesc;

    typedef struct DenOfIz_AABBBoundingBox
    {
        float MinX;
        float MinY;
        float MinZ;
        float MaxX;
        float MaxY;
        float MaxZ;
    } DenOfIz_AABBBoundingBox;

    /** Buffer holds DenOfIz_AABBBoundingBox values and needs DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT. */
    typedef struct DenOfIz_ASGeometryAABBDesc
    {
        DenOfIz_Buffer Buffer;
        uint32_t       Offset; /**< Bytes. */
        uint32_t       Stride; /**< Bytes, usually sizeof(DenOfIz_AABBBoundingBox). */
        uint32_t       NumAABBs;
    } DenOfIz_ASGeometryAABBDesc;

    typedef struct DenOfIz_ASGeometryDesc
    {
        DenOfIz_HitGroupType           Type;
        DenOfIz_ASGeometryTriangleDesc Triangles; /**< When Type is TRIANGLES. */
        DenOfIz_ASGeometryAABBDesc     AABBs;     /**< When Type is AABBS. */
        uint32_t                       Flags;     /**< DenOfIz_BLASGeometryFlags. */
    } DenOfIz_ASGeometryDesc;

    typedef struct DenOfIz_ASGeometryDescArray
    {
        DenOfIz_ASGeometryDesc *Elements;
        uint32_t                NumElements;
    } DenOfIz_ASGeometryDescArray;

    typedef struct DenOfIz_BottomLevelASDesc
    {
        DenOfIz_ASGeometryDescArray Geometries; /**< All of the same Type. */
        DenOfIz_ASBuildFlagBits     BuildFlags;
    } DenOfIz_BottomLevelASDesc;

    /** Scratch bytes DenOfIz_CommandList_BuildBottomLevelAS needs, in a HEAP_TYPE_GPU buffer with STORAGE usage. */
    DZ_API void DenOfIz_BottomLevelAS_BuildNumBytes( DenOfIz_BottomLevelAS bottomLevelAS, size_t *outNumBytes );
    /**
     * Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. TLASes that instance it
     * read its memory, so they must not be used afterwards.
     */
    DZ_API void DenOfIz_BottomLevelAS_Destroy( DenOfIz_BottomLevelAS bottomLevelAS );

#ifdef __cplusplus
}
#endif
