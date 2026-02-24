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

#include "DenOfIzGraphics/Backends/Interface/Pipeline.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "ShaderLocalData.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Handle to a shader binding table for ray tracing pipelines.
     *
     * A shader binding table (SBT) connects ray tracing shaders (ray generation, miss, hit groups) with
     * their shader local data. It is built on the CPU and uploaded to GPU memory, then referenced during
     * DenOfIz_CommandList_DispatchRays to execute ray tracing.
     *
     * @par Backend Implementations
     * - DirectX12: GPU buffer with D3D12_DISPATCH_RAYS_DESC shader record regions
     * - Vulkan: GPU buffer with VkStridedDeviceAddressRegionKHR regions
     * - Metal: MTLBuffer with MTLAccelerationStructureInstanceDescriptor and shader identifiers
     * - WebGPU: Not supported (ray tracing not available)
     *
     * @par Usage
     * The SBT is the central structure for ray tracing execution. It organizes shaders into three categories:
     * - Ray generation shaders: Entry point for ray tracing (typically one shader)
     * - Miss shaders: Invoked when rays miss geometry (sky, environment, shadows)
     * - Hit groups: Invoked when rays intersect geometry (materials, lighting)
     *
     * Each shader can have associated shader local data for per-shader resources.
     *
     * The SBT must be built (DenOfIz_ShaderBindingTable_Build) before use in ray tracing.
     *
     * @par Workflow
     * 1. Create SBT with pipeline and size requirements
     * 2. Bind ray generation shader(s)
     * 3. Bind miss shader(s)
     * 4. Bind hit group(s) with optional shader local data
     * 5. Build the SBT to finalize GPU buffer
     * 6. Reference SBT in DenOfIz_DispatchRaysDesc during command list recording
     *
     * @par Example
     * @code
     * // Create ray tracing pipeline (omitted for brevity)
     * DenOfIz_Pipeline rtPipeline;
     * // ... create ray tracing pipeline ...
     *
     * // Create shader local data for hit group
     * DenOfIz_ShaderLocalData materialData;
     * // ... create and populate shader local data ...
     *
     * // Create shader binding table
     * DenOfIz_ShaderBindingTableDesc sbtDesc = {0};
     * sbtDesc.Pipeline = rtPipeline;
     * sbtDesc.SizeDesc.NumRayGenerationShaders = 1;
     * sbtDesc.SizeDesc.NumMissShaders = 1;
     * sbtDesc.SizeDesc.NumHitGroups = 1;
     * sbtDesc.MaxHitGroupDataBytes = 16; // Size of shader local data
     *
     * DenOfIz_ShaderBindingTable sbt;
     * DenOfIz_LogicalDevice_CreateShaderBindingTable(device, &sbtDesc, &sbt);
     *
     * // Bind ray generation shader
     * DenOfIz_RayGenerationBindingDesc rayGenDesc = {0};
     * rayGenDesc.ShaderName = DENOFIZ_STRING("MyRaygenShader");
     * DenOfIz_ShaderBindingTable_BindRayGenerationShader(sbt, &rayGenDesc);
     *
     * // Bind miss shader
     * DenOfIz_MissBindingDesc missDesc = {0};
     * missDesc.ShaderName = DENOFIZ_STRING("MyMissShader");
     * DenOfIz_ShaderBindingTable_BindMissShader(sbt, &missDesc);
     *
     * // Bind hit group with local data
     * DenOfIz_HitGroupBindingDesc hitGroupDesc = {0};
     * hitGroupDesc.HitGroupExportName = DENOFIZ_STRING("MyHitGroup");
     * hitGroupDesc.Data = materialData;
     * DenOfIz_ShaderBindingTable_BindHitGroup(sbt, &hitGroupDesc);
     *
     * // Build SBT
     * DenOfIz_ShaderBindingTable_Build(sbt);
     *
     * // Use in ray tracing
     * DenOfIz_DispatchRaysDesc dispatchDesc = {0};
     * dispatchDesc.Width = 1920;
     * dispatchDesc.Height = 1080;
     * dispatchDesc.Depth = 1;
     * dispatchDesc.ShaderBindingTable = sbt;
     * DenOfIz_CommandList_DispatchRays(commandList, &dispatchDesc);
     *
     * // Cleanup
     * DenOfIz_ShaderBindingTable_Destroy(sbt);
     * @endcode
     *
     * @see DenOfIz_ShaderBindingTableDesc
     * @see DenOfIz_CommandList_DispatchRays
     * @see DenOfIz_ShaderLocalData
     * @see DenOfIz_Pipeline (with DENOFIZ_BIND_POINT_RAYTRACING)
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderBindingTable )

    /**
     * @brief Descriptor for binding a hit group shader in the shader binding table.
     *
     * @param GeometryType Type of geometry the hit group handles (triangles or AABBs).
     * @param Offset Offset index for this hit group binding (used for multi-geometry scenes).
     * @param HitGroupExportName Name of the hit group as defined in the pipeline.
     * @param Data Optional shader local data for per-hit-group resources (materials, textures).
     */
    typedef struct DenOfIz_HitGroupBindingDesc
    {
        DenOfIz_HitGroupType    GeometryType;
        int                     Offset;
        DenOfIz_StringView      HitGroupExportName;
        DenOfIz_ShaderLocalData Data;
    } DenOfIz_HitGroupBindingDesc;

    /**
     * @brief Descriptor for binding a miss shader in the shader binding table.
     *
     * @param Offset Offset index for this miss shader binding (used for multiple miss shaders).
     * @param ShaderName Name of the miss shader as defined in the shader program.
     * @param Data Optional shader local data for per-miss-shader resources.
     */
    typedef struct DenOfIz_MissBindingDesc
    {
        int                     Offset;
        DenOfIz_StringView      ShaderName;
        DenOfIz_ShaderLocalData Data;
    } DenOfIz_MissBindingDesc;

    /**
     * @brief Descriptor for binding a ray generation shader in the shader binding table.
     *
     * @param ShaderName Name of the ray generation shader as defined in the shader program.
     * @param Data Optional shader local data for per-raygen resources.
     */
    typedef struct DenOfIz_RayGenerationBindingDesc
    {
        DenOfIz_StringView      ShaderName;
        DenOfIz_ShaderLocalData Data;
    } DenOfIz_RayGenerationBindingDesc;

    /**
     * @brief Size descriptor for shader binding table shader counts.
     *
     * @param NumRayGenerationShaders Number of ray generation shaders (typically 1).
     * @param NumMissShaders Number of miss shaders (one per ray type).
     * @param NumHitGroups Number of hit groups (typically one per material or geometry type).
     */
    typedef struct DenOfIz_SBTSizeDesc
    {
        uint32_t NumRayGenerationShaders;
        uint32_t NumMissShaders;
        uint32_t NumHitGroups;
    } DenOfIz_SBTSizeDesc;

    /**
     * @brief Descriptor for creating a shader binding table.
     *
     * @param Pipeline Ray tracing pipeline containing the shaders referenced by the SBT.
     * @param SizeDesc Number of shaders in each category (ray gen, miss, hit groups).
     * @param MaxHitGroupDataBytes Maximum size in bytes of shader local data per hit group.
     * @param MaxMissDataBytes Maximum size in bytes of shader local data per miss shader.
     * @param MaxRayGenDataBytes Maximum size in bytes of shader local data per ray generation shader.
     */
    typedef struct DenOfIz_ShaderBindingTableDesc
    {
        DenOfIz_Pipeline    Pipeline;
        DenOfIz_SBTSizeDesc SizeDesc;
        uint32_t            MaxHitGroupDataBytes;
        uint32_t            MaxMissDataBytes;
        uint32_t            MaxRayGenDataBytes;
    } DenOfIz_ShaderBindingTableDesc;

    /**
     * @brief Resizes the shader binding table to accommodate a different number of shaders.
     *
     * Reallocates internal GPU buffer to fit new shader counts. Must rebuild the SBT after resizing.
     *
     * @param sbt Valid shader binding table handle.
     * @param resizeDesc New size descriptor with updated shader counts.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - @p resizeDesc must not be NULL
     * - Must call DenOfIz_ShaderBindingTable_Build after resizing
     * - All previously bound shaders are invalidated and must be rebound
     */
    DZ_API void DenOfIz_ShaderBindingTable_Resize( DenOfIz_ShaderBindingTable sbt, const DenOfIz_SBTSizeDesc *resizeDesc );

    /**
     * @brief Binds a ray generation shader to the shader binding table.
     *
     * Ray generation shaders are the entry point for ray tracing. They typically generate primary rays
     * from the camera and trace them into the scene.
     *
     * @param sbt Valid shader binding table handle.
     * @param desc Ray generation binding descriptor.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - @p desc must not be NULL
     * - @p desc->ShaderName must match a ray generation shader in the associated pipeline
     * - If @p desc->Data is provided, it must be a valid DenOfIz_ShaderLocalData handle
     * - Must call DenOfIz_ShaderBindingTable_Build before using the SBT
     */
    DZ_API void DenOfIz_ShaderBindingTable_BindRayGenerationShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_RayGenerationBindingDesc *desc );

    /**
     * @brief Binds a hit group to the shader binding table.
     *
     * Hit groups define shader behavior when rays intersect geometry. Each hit group can contain
     * closest hit, any hit, and intersection shaders. Use shader local data to provide per-geometry
     * material parameters.
     *
     * @param sbt Valid shader binding table handle.
     * @param desc Hit group binding descriptor.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - @p desc must not be NULL
     * - @p desc->HitGroupExportName must match a hit group name in the associated pipeline
     * - @p desc->GeometryType must match the geometry type used in acceleration structures
     * - If @p desc->Data is provided, it must be a valid DenOfIz_ShaderLocalData handle
     * - If @p desc->Data is provided, its size must not exceed MaxHitGroupDataBytes
     * - Must call DenOfIz_ShaderBindingTable_Build before using the SBT
     */
    DZ_API void DenOfIz_ShaderBindingTable_BindHitGroup( DenOfIz_ShaderBindingTable sbt, const DenOfIz_HitGroupBindingDesc *desc );

    /**
     * @brief Binds a miss shader to the shader binding table.
     *
     * Miss shaders are invoked when rays do not intersect any geometry. Commonly used for
     * environment lighting, sky rendering, or shadow determination.
     *
     * @param sbt Valid shader binding table handle.
     * @param desc Miss shader binding descriptor.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - @p desc must not be NULL
     * - @p desc->ShaderName must match a miss shader in the associated pipeline
     * - If @p desc->Data is provided, it must be a valid DenOfIz_ShaderLocalData handle
     * - If @p desc->Data is provided, its size must not exceed MaxMissDataBytes
     * - Must call DenOfIz_ShaderBindingTable_Build before using the SBT
     */
    DZ_API void DenOfIz_ShaderBindingTable_BindMissShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_MissBindingDesc *desc );

    /**
     * @brief Builds the shader binding table and uploads it to GPU memory.
     *
     * Finalizes all shader bindings and copies the SBT data to GPU. Must be called after binding
     * all shaders and before using the SBT in DenOfIz_CommandList_DispatchRays.
     *
     * @param sbt Valid shader binding table handle.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - At least one ray generation shader must be bound
     * - Build must be called after any binding changes
     * - Rebuilding is required after Resize or any shader binding changes
     *
     * @see DenOfIz_ShaderBindingTable_BindRayGenerationShader
     * @see DenOfIz_ShaderBindingTable_BindHitGroup
     * @see DenOfIz_ShaderBindingTable_BindMissShader
     */
    DZ_API void DenOfIz_ShaderBindingTable_Build( DenOfIz_ShaderBindingTable sbt );

    /**
     * @brief Destroys the shader binding table and releases associated GPU resources.
     *
     * @param sbt Valid shader binding table handle to destroy.
     *
     * @par Valid Usage
     * - @p sbt must be a valid DenOfIz_ShaderBindingTable handle
     * - Must not be referenced in any pending or executing command lists
     * - All shader local data objects can be destroyed before or after the SBT
     */
    DZ_API void DenOfIz_ShaderBindingTable_Destroy( DenOfIz_ShaderBindingTable sbt );

#ifdef __cplusplus
}
#endif
