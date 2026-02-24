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

#include "BottomLevelAS.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a top-level acceleration structure (TLAS) for ray tracing.
     *
     * A TLAS organizes instances of bottom-level acceleration structures (BLASes) with
     * per-instance transforms. It is the top level of the two-level BVH hierarchy used
     * by hardware ray tracing. Rays are traced against the TLAS, which references the
     * underlying geometry through BLAS instances.
     *
     * The TLAS must be built on the GPU via DenOfIz_CommandList_BuildTopLevelAS before
     * use in ray tracing. Instance transforms can be updated dynamically if the TLAS
     * was created with DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12Resource (acceleration structure buffer) + D3D12_RAYTRACING_INSTANCE_DESC array
     * - **Vulkan**: VkAccelerationStructureKHR + VkAccelerationStructureInstanceKHR array
     * - **Metal**: MTLAccelerationStructure with MTLInstanceAccelerationStructureDescriptor
     * - **WebGPU**: Not supported
     *
     * @par Usage
     * 1. Create BLAS(es) for scene geometry
     * 2. Create TLAS with instance descriptors referencing BLASes
     * 3. Allocate scratch buffer using BuildNumBytes()
     * 4. Build TLAS via DenOfIz_CommandList_BuildTopLevelAS
     * 5. Bind to shader via DenOfIz_BindGroup_SrvTopLevelAS
     * 6. Trace rays via DenOfIz_CommandList_DispatchRays
     *
     * @par Example
     * @code
     * // Create TLAS with one instance
     * DenOfIz_ASInstanceDesc instance = {0};
     * instance.BLAS      = bottomLevelAS;
     * instance.Transform = DENOFIZ_FLOAT4X4_IDENTITY;
     * instance.Mask      = 0xFF;
     *
     * DenOfIz_TopLevelASDesc tlasDesc = {0};
     * tlasDesc.BuildFlags            = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
     * tlasDesc.Instances.Elements    = &instance;
     * tlasDesc.Instances.NumElements = 1;
     *
     * DenOfIz_TopLevelAS tlas;
     * DenOfIz_LogicalDevice_CreateTopLevelAS(device, &tlasDesc, &tlas);
     *
     * // Get scratch buffer size and build
     * size_t scratchSize = 0;
     * DenOfIz_TopLevelAS_BuildNumBytes(tlas, &scratchSize);
     *
     * // ... create scratch buffer with scratchSize bytes ...
     *
     * DenOfIz_BuildTopLevelASDesc buildDesc = {0};
     * buildDesc.TopLevelAS    = tlas;
     * buildDesc.ScratchBuffer = scratchBuffer;
     * DenOfIz_CommandList_BuildTopLevelAS(cmd, &buildDesc);
     *
     * // Bind for ray tracing
     * DenOfIz_BindGroup_SrvTopLevelAS(bindGroup, 0, tlas);
     * @endcode
     *
     * @see DenOfIz_BottomLevelAS
     * @see DenOfIz_LogicalDevice_CreateTopLevelAS
     * @see DenOfIz_CommandList_BuildTopLevelAS
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_TopLevelAS )

    /**
     * @brief Describes a single instance within a top-level acceleration structure.
     *
     * Each instance references a BLAS and provides per-instance data including
     * transform, hit group offset, instance ID, and visibility mask.
     */
    typedef struct DenOfIz_ASInstanceDesc
    {
        DenOfIz_BottomLevelAS BLAS;                        /**< Bottom-level AS containing the geometry. */
        DenOfIz_Float4x4      Transform;                   /**< 3x4 row-major transform matrix (use DENOFIZ_FLOAT4X4_IDENTITY for no transform). */
        uint32_t              ContributionToHitGroupIndex; /**< Offset into shader binding table for this instance's hit group. */
        uint32_t              ID;                          /**< User-defined instance ID accessible in shaders via InstanceID(). */
        uint32_t              Mask;                        /**< 8-bit visibility mask for ray/instance intersection culling. */
    } DenOfIz_ASInstanceDesc;

    typedef struct DenOfIz_ASInstanceDescArray
    {
        DenOfIz_ASInstanceDesc *Elements;
        uint32_t                NumElements;
    } DenOfIz_ASInstanceDescArray;

    /**
     * @brief Describes parameters for creating a top-level acceleration structure.
     */
    typedef struct DenOfIz_TopLevelASDesc
    {
        DenOfIz_ASInstanceDescArray Instances;  /**< Array of BLAS instances. */
        uint32_t                    BuildFlags; /**< Combination of DenOfIz_ASBuildFlagBits. */
    } DenOfIz_TopLevelASDesc;

    /**
     * @brief Describes transform updates for TLAS instances.
     *
     * Used with UpdateInstanceTransforms to update instance transforms without
     * full rebuild. TLAS must have been created with DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT.
     */
    typedef struct DenOfIz_UpdateTransformsDesc
    {
        DenOfIz_Float4x4Array Transforms; /**< New transforms, one per instance in creation order. */
    } DenOfIz_UpdateTransformsDesc;

    /**
     * @brief Updates instance transforms for a TLAS.
     *
     * Updates the transform matrices for TLAS instances. The TLAS must have been
     * created with DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT. After calling this function,
     * you must rebuild the TLAS with DENOFIZ_AS_BUILD_PERFORM_UPDATE_BIT to apply
     * the changes.
     *
     * @param topLevelAS Valid TLAS handle created with ALLOW_UPDATE flag.
     * @param desc New transforms for instances.
     *
     * @par Valid Usage
     * - @p topLevelAS must be a valid DenOfIz_TopLevelAS handle
     * - @p topLevelAS must have been created with DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT
     * - @p desc must not be NULL
     * - @p desc->Transforms.NumElements must match the instance count
     */
    DZ_API void DenOfIz_TopLevelAS_UpdateInstanceTransforms( DenOfIz_TopLevelAS topLevelAS, const DenOfIz_UpdateTransformsDesc *desc );

    /**
     * @brief Retrieves the scratch buffer size required to build the TLAS.
     *
     * The scratch buffer is temporary memory used during acceleration structure build.
     * It must be allocated as an RW buffer and passed to DenOfIz_CommandList_BuildTopLevelAS.
     *
     * @param topLevelAS Valid TLAS handle.
     * @param[out] outNumBytes Receives the required scratch buffer size in bytes.
     *
     * @par Valid Usage
     * - @p topLevelAS must be a valid DenOfIz_TopLevelAS handle
     * - @p outNumBytes must not be NULL
     */
    DZ_API void DenOfIz_TopLevelAS_BuildNumBytes( DenOfIz_TopLevelAS topLevelAS, size_t *outNumBytes );

    /**
     * @brief Destroys a top-level acceleration structure and releases resources.
     *
     * @param topLevelAS TLAS handle to destroy. May be DENOFIZ_NULL_HANDLE.
     */
    DZ_API void DenOfIz_TopLevelAS_Destroy( DenOfIz_TopLevelAS topLevelAS );

#ifdef __cplusplus
}
#endif
