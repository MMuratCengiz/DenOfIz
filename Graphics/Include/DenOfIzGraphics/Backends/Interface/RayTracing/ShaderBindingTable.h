/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
     * Shader records of a ray tracing pipeline. Bind the ray generation shader, miss shaders and hit groups, then Build once. Binding
     * copies the shader local data as it is at that moment. On DX12 and Vulkan Build uploads the records and blocks until the upload
     * finished, records can only be bound again after a Resize. Metal writes records straight into GPU visible memory, so do not bind
     * while the GPU uses the table.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderBindingTable )

    typedef struct DenOfIz_HitGroupBindingDesc
    {
        DenOfIz_HitGroupType    GeometryType;
        int                     Offset;             /**< Record index in the hit group region, see DenOfIz_ASInstanceDesc::ContributionToHitGroupIndex. */
        DenOfIz_StringView      HitGroupExportName; /**< DenOfIz_HitGroupDesc::Name. */
        DenOfIz_ShaderLocalData Data;               /**< Optional, at most MaxHitGroupDataBytes. */
    } DenOfIz_HitGroupBindingDesc;

    typedef struct DenOfIz_MissBindingDesc
    {
        int                     Offset;     /**< Record index in the miss region. */
        DenOfIz_StringView      ShaderName; /**< Entry point name. */
        DenOfIz_ShaderLocalData Data;       /**< Optional, at most MaxMissDataBytes. */
    } DenOfIz_MissBindingDesc;

    /** Written to the first ray generation record. */
    typedef struct DenOfIz_RayGenerationBindingDesc
    {
        DenOfIz_StringView      ShaderName; /**< Entry point name. */
        DenOfIz_ShaderLocalData Data;       /**< Optional, at most MaxRayGenDataBytes. */
    } DenOfIz_RayGenerationBindingDesc;

    /** Record counts per region. */
    typedef struct DenOfIz_SBTSizeDesc
    {
        uint32_t NumRayGenerationShaders;
        uint32_t NumMissShaders;
        uint32_t NumHitGroups;
    } DenOfIz_SBTSizeDesc;

    /** The Max*DataBytes fields size the shader local data part of each record of that region. */
    typedef struct DenOfIz_ShaderBindingTableDesc
    {
        DenOfIz_Pipeline    Pipeline; /**< Must outlive the table. */
        DenOfIz_SBTSizeDesc SizeDesc;
        uint32_t            MaxHitGroupDataBytes;
        uint32_t            MaxMissDataBytes;
        uint32_t            MaxRayGenDataBytes;
    } DenOfIz_ShaderBindingTableDesc;

    /**
     * Reallocates the table for new record counts and drops every record: bind them again, then Build. The previous GPU buffer is freed
     * right away, so the GPU must not be using the table.
     */
    DZ_API void DenOfIz_ShaderBindingTable_Resize( DenOfIz_ShaderBindingTable sbt, const DenOfIz_SBTSizeDesc *resizeDesc );
    DZ_API void DenOfIz_ShaderBindingTable_BindRayGenerationShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_RayGenerationBindingDesc *desc );
    DZ_API void DenOfIz_ShaderBindingTable_BindHitGroup( DenOfIz_ShaderBindingTable sbt, const DenOfIz_HitGroupBindingDesc *desc );
    DZ_API void DenOfIz_ShaderBindingTable_BindMissShader( DenOfIz_ShaderBindingTable sbt, const DenOfIz_MissBindingDesc *desc );
    DZ_API void DenOfIz_ShaderBindingTable_Build( DenOfIz_ShaderBindingTable sbt );
    /** Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_ShaderBindingTable_Destroy( DenOfIz_ShaderBindingTable sbt );

#ifdef __cplusplus
}
#endif
