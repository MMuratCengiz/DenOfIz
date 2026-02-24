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
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a bottom-level acceleration structure (BLAS) for ray tracing.
     *
     * A BLAS contains the actual geometry (triangle meshes or procedural AABBs) used for
     * ray intersection testing. It represents a single object or mesh that can be instanced
     * multiple times in a TLAS with different transforms.
     *
     * BLASes are the lower level of the two-level acceleration structure hierarchy. For
     * triangle geometry, the BLAS references vertex and index buffers. For procedural
     * geometry, it references AABBs that bound custom intersection shaders.
     *
     * The BLAS must be built on the GPU via DenOfIz_CommandList_BuildBottomLevelAS before
     * it can be referenced by a TLAS.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12Resource (acceleration structure buffer) + D3D12_RAYTRACING_GEOMETRY_DESC array
     * - **Vulkan**: VkAccelerationStructureKHR + VkAccelerationStructureGeometryKHR array
     * - **Metal**: MTLAccelerationStructure with MTLPrimitiveAccelerationStructureDescriptor
     * - **WebGPU**: Not supported
     *
     * @par Usage
     * 1. Create vertex/index buffers with DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_GEOMETRY_BIT
     * 2. Create BLAS with geometry descriptors
     * 3. Allocate scratch buffer using BuildNumBytes()
     * 4. Build BLAS via DenOfIz_CommandList_BuildBottomLevelAS
     * 5. Add pipeline barrier to transition from write to read state
     * 6. Reference BLAS in TLAS instances
     *
     * @par Example
     * @code
     * // Create triangle geometry BLAS
     * DenOfIz_ASGeometryDesc geometry = {0};
     * geometry.Type                   = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
     * geometry.Flags                  = DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;
     * geometry.Triangles.VertexBuffer = vertexBuffer;
     * geometry.Triangles.VertexFormat = DENOFIZ_FORMAT_R32G32B32_FLOAT;
     * geometry.Triangles.VertexStride = 3 * sizeof(float);
     * geometry.Triangles.NumVertices  = 3;
     * geometry.Triangles.IndexBuffer  = indexBuffer;
     * geometry.Triangles.IndexType    = DENOFIZ_INDEX_TYPE_UINT16;
     * geometry.Triangles.NumIndices   = 3;
     *
     * DenOfIz_BottomLevelASDesc blasDesc = {0};
     * blasDesc.BuildFlags             = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
     * blasDesc.Geometries.Elements    = &geometry;
     * blasDesc.Geometries.NumElements = 1;
     *
     * DenOfIz_BottomLevelAS blas;
     * DenOfIz_LogicalDevice_CreateBottomLevelAS(device, &blasDesc, &blas);
     *
     * // Get scratch buffer size
     * size_t scratchSize = 0;
     * DenOfIz_BottomLevelAS_BuildNumBytes(blas, &scratchSize);
     *
     * // ... create scratch buffer with scratchSize bytes ...
     *
     * // Build BLAS in command list
     * DenOfIz_BuildBottomLevelASDesc buildDesc = {0};
     * buildDesc.BottomLevelAS  = blas;
     * buildDesc.ScratchBuffer  = scratchBuffer;
     * DenOfIz_CommandList_BuildBottomLevelAS(cmd, &buildDesc);
     *
     * // Barrier before using in TLAS
     * DenOfIz_MemoryBarrierDesc barrier = {0};
     * barrier.BottomLevelAS = blas;
     * barrier.OldState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
     * barrier.NewState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;
     * @endcode
     *
     * @see DenOfIz_TopLevelAS
     * @see DenOfIz_LogicalDevice_CreateBottomLevelAS
     * @see DenOfIz_CommandList_BuildBottomLevelAS
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BottomLevelAS )

    /**
     * @brief Flags controlling BLAS geometry behavior during ray tracing.
     */
    typedef enum DenOfIz_BLASGeometryFlagBits
    {
        DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT                          = 1 << 0, /**< Geometry is opaque; any-hit shader will not be invoked. */
        DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT = 1 << 1, /**< Guarantees any-hit shader invoked at most once per primitive. */
    } DenOfIz_BLASGeometryFlagBits;
    typedef uint32_t DenOfIz_BLASGeometryFlags;

    /**
     * @brief Describes triangle geometry for a bottom-level acceleration structure.
     *
     * Triangle geometry is the most common type for standard meshes. The vertex and
     * index buffers must have been created with DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_GEOMETRY_BIT.
     */
    typedef struct DenOfIz_ASGeometryTriangleDesc
    {
        DenOfIz_Buffer    VertexBuffer; /**< Buffer containing vertex positions. */
        uint32_t          VertexOffset; /**< Byte offset into vertex buffer. */
        uint32_t          VertexStride; /**< Bytes between consecutive vertices. */
        uint32_t          NumVertices;  /**< Number of vertices. */
        DenOfIz_Format    VertexFormat; /**< Format of position data (R32G32B32_FLOAT or R16G16B16A16_FLOAT). */
        DenOfIz_Buffer    IndexBuffer;  /**< Buffer containing indices, or DENOFIZ_NULL_HANDLE for non-indexed. */
        uint32_t          IndexOffset;  /**< Byte offset into index buffer. */
        uint32_t          NumIndices;   /**< Number of indices (0 if non-indexed). */
        DenOfIz_IndexType IndexType;    /**< Format of index data (UINT16 or UINT32). */
    } DenOfIz_ASGeometryTriangleDesc;

    /**
     * @brief Axis-aligned bounding box for procedural geometry.
     *
     * Defines bounds for a procedural primitive that will be tested via
     * intersection shader. The AABB defines the volume where rays may hit.
     */
    typedef struct DenOfIz_AABBBoundingBox
    {
        float MinX; /**< Minimum X coordinate. */
        float MinY; /**< Minimum Y coordinate. */
        float MinZ; /**< Minimum Z coordinate. */
        float MaxX; /**< Maximum X coordinate. */
        float MaxY; /**< Maximum Y coordinate. */
        float MaxZ; /**< Maximum Z coordinate. */
    } DenOfIz_AABBBoundingBox;

    /**
     * @brief Describes AABB (procedural) geometry for a bottom-level acceleration structure.
     *
     * AABB geometry is used for procedural primitives that require custom intersection
     * shaders (spheres, cylinders, signed distance fields, etc.). The buffer contains
     * an array of DenOfIz_AABBBoundingBox structures.
     */
    typedef struct DenOfIz_ASGeometryAABBDesc
    {
        DenOfIz_Buffer Buffer;   /**< Buffer containing AABBs (array of DenOfIz_AABBBoundingBox). */
        uint32_t       Offset;   /**< Byte offset into buffer. */
        uint32_t       Stride;   /**< Bytes between consecutive AABBs (typically sizeof(DenOfIz_AABBBoundingBox)). */
        uint32_t       NumAABBs; /**< Number of AABBs in the buffer. */
    } DenOfIz_ASGeometryAABBDesc;

    /**
     * @brief Describes a single geometry within a bottom-level acceleration structure.
     *
     * A BLAS can contain multiple geometries of the same type (all triangles or all AABBs).
     * Mixing triangle and AABB geometry in a single BLAS is not supported.
     */
    typedef struct DenOfIz_ASGeometryDesc
    {
        DenOfIz_HitGroupType           Type;      /**< Geometry type: TRIANGLES or AABBS. */
        DenOfIz_ASGeometryTriangleDesc Triangles; /**< Triangle geometry data (used when Type is TRIANGLES). */
        DenOfIz_ASGeometryAABBDesc     AABBs;     /**< AABB geometry data (used when Type is AABBS). */
        uint32_t                       Flags;     /**< Combination of DenOfIz_BLASGeometryFlagBits. */
    } DenOfIz_ASGeometryDesc;

    typedef struct DenOfIz_ASGeometryDescArray
    {
        DenOfIz_ASGeometryDesc *Elements;
        uint32_t                NumElements;
    } DenOfIz_ASGeometryDescArray;

    /**
     * @brief Describes parameters for creating a bottom-level acceleration structure.
     */
    typedef struct DenOfIz_BottomLevelASDesc
    {
        DenOfIz_ASGeometryDescArray Geometries; /**< Array of geometry descriptors. All must be same type. */
        DenOfIz_ASBuildFlagBits     BuildFlags; /**< Combination of DenOfIz_ASBuildFlagBits. */
    } DenOfIz_BottomLevelASDesc;

    /**
     * @brief Retrieves the scratch buffer size required to build the BLAS.
     *
     * The scratch buffer is temporary memory used during acceleration structure build.
     * It must be allocated as an RW buffer and passed to DenOfIz_CommandList_BuildBottomLevelAS.
     * The same scratch buffer can be reused for multiple builds if large enough.
     *
     * @param bottomLevelAS Valid BLAS handle.
     * @param[out] outNumBytes Receives the required scratch buffer size in bytes.
     *
     * @par Valid Usage
     * - @p bottomLevelAS must be a valid DenOfIz_BottomLevelAS handle
     * - @p outNumBytes must not be NULL
     */
    DZ_API void DenOfIz_BottomLevelAS_BuildNumBytes( DenOfIz_BottomLevelAS bottomLevelAS, size_t *outNumBytes );

    /**
     * @brief Destroys a bottom-level acceleration structure and releases resources.
     *
     * Ensure no TLAS references this BLAS before destruction.
     *
     * @param bottomLevelAS BLAS handle to destroy. May be DENOFIZ_NULL_HANDLE.
     */
    DZ_API void DenOfIz_BottomLevelAS_Destroy( DenOfIz_BottomLevelAS bottomLevelAS );

#ifdef __cplusplus
}
#endif
