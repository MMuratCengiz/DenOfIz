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
#include "DenOfIzGraphics/Assets/Shaders/ShaderIncludeHandler.h"
#include "DenOfIzGraphics/Utilities/Common.h"
#include "RayTracing/RayTracingData.h"

typedef enum DenOfIz_ShaderStageFlagBits
{
    DENOFIZ_SHADER_STAGE_NONE_BIT         = 0,
    DENOFIZ_SHADER_STAGE_VERTEX_BIT       = 1 << 0,
    DENOFIZ_SHADER_STAGE_PIXEL_BIT        = 1 << 1,
    DENOFIZ_SHADER_STAGE_GEOMETRY_BIT     = 1 << 2,
    DENOFIZ_SHADER_STAGE_HULL_BIT         = 1 << 3,
    DENOFIZ_SHADER_STAGE_DOMAIN_BIT       = 1 << 4,
    DENOFIZ_SHADER_STAGE_COMPUTE_BIT      = 1 << 5,
    DENOFIZ_SHADER_STAGE_RAY_GEN_BIT      = 1 << 6,
    DENOFIZ_SHADER_STAGE_ANY_HIT_BIT      = 1 << 7,
    DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT  = 1 << 8,
    DENOFIZ_SHADER_STAGE_MISS_BIT         = 1 << 9,
    DENOFIZ_SHADER_STAGE_INTERSECTION_BIT = 1 << 10,
    DENOFIZ_SHADER_STAGE_CALLABLE_BIT     = 1 << 11,
    DENOFIZ_SHADER_STAGE_TASK_BIT         = 1 << 12,
    DENOFIZ_SHADER_STAGE_MESH_BIT         = 1 << 13,
    DENOFIZ_SHADER_STAGE_ALL_GRAPHICS_BIT = DENOFIZ_SHADER_STAGE_VERTEX_BIT | DENOFIZ_SHADER_STAGE_PIXEL_BIT | DENOFIZ_SHADER_STAGE_GEOMETRY_BIT | DENOFIZ_SHADER_STAGE_HULL_BIT |
                                            DENOFIZ_SHADER_STAGE_DOMAIN_BIT | DENOFIZ_SHADER_STAGE_TASK_BIT | DENOFIZ_SHADER_STAGE_MESH_BIT,
    DENOFIZ_SHADER_STAGE_ALL_BIT = 0x3FFF
} DenOfIz_ShaderStageFlagBits;
typedef uint32_t DenOfIz_ShaderStageFlags;

typedef struct DenOfIz_ThreadGroupDesc
{
    uint32_t X;
    uint32_t Y;
    uint32_t Z;
} DenOfIz_ThreadGroupDesc;

typedef struct DenOfIz_ThreadGroupDescArray
{
    DenOfIz_ThreadGroupDesc *Elements;
    size_t                   NumElements;
} DenOfIz_ThreadGroupDescArray;

typedef enum DenOfIz_TargetIL
{
    DENOFIZ_TARGET_IL_DXIL,
    DENOFIZ_TARGET_IL_MSL,
    DENOFIZ_TARGET_IL_SPIRV,
    DENOFIZ_TARGET_IL_WGSL
} DenOfIz_TargetIL;

typedef enum DenOfIz_RayTracingStage
{
    DENOFIZ_RAYTRACING_STAGE_RAY_GEN,
    DENOFIZ_RAYTRACING_STAGE_HIT_GROUP,
    DENOFIZ_RAYTRACING_STAGE_MISS,
    DENOFIZ_RAYTRACING_STAGE_CALLABLE
} DenOfIz_RayTracingStage;

typedef struct DenOfIz_ShaderRayTracingDesc
{
    uint32_t MaxNumPayloadBytes;
    uint32_t MaxNumAttributeBytes;
    uint32_t MaxRecursionDepth;
} DenOfIz_ShaderRayTracingDesc;

typedef struct DenOfIz_RayTracingShaderDesc
{
    // For metal, it needs to know before compiling what the hit group is otherwise intersection shaders do not work
    DenOfIz_HitGroupType             HitGroupType;
    DenOfIz_ResourceBindingSlotArray LocalBindings;
    // Local bindings are used to mark resources as local, so they are not included in the global resource list
    // The binding will be added to the corresponding ShaderDataLayoutDesc in the corresponding index of DenOfIz_LocalRootSignatureDesc at
    // ShaderReflectDesc.LocalRootSignatures[shaderIndex], where shaderIndex is the index of the shader in the order of shaders provided to CompileDesc.
} DenOfIz_RayTracingShaderDesc;

typedef struct DenOfIz_BindlessSlot
{
    DenOfIz_ResourceDescriptorFlags Descriptor;
    uint32_t                        Binding;
    uint32_t                        RegisterSpace;
    uint32_t                        MaxArraySize;
} DenOfIz_BindlessSlot;

typedef struct DenOfIz_BindlessSlotArray
{
    DenOfIz_BindlessSlot *Elements;
    size_t                NumElements;
} DenOfIz_BindlessSlotArray;

typedef struct DenOfIz_BindlessDesc
{
    DenOfIz_BindlessSlotArray BindlessArrays;
} DenOfIz_BindlessDesc;

typedef enum DenOfIz_CodePage
{
    DENOFIZ_CODE_PAGE_ACP,
    DENOFIZ_CODE_PAGE_UTF8,
    DENOFIZ_CODE_PAGE_UTF16,
    DENOFIZ_CODE_PAGE_UTF32
} DenOfIz_CodePage;

typedef struct DenOfIz_ShaderStageDesc
{
    DenOfIz_ShaderStageFlags     Stage;
    DenOfIz_CodePage             CodePage;
    DenOfIz_StringView           Path;
    DenOfIz_ByteArray            Data;
    DenOfIz_StringViewArray      Defines;
    DenOfIz_StringView           EntryPoint;
    DenOfIz_RayTracingShaderDesc RayTracing;
    DenOfIz_BindlessDesc         Bindless;
} DenOfIz_ShaderStageDesc;

typedef struct DenOfIz_ShaderStageDescArray
{
    DenOfIz_ShaderStageDesc *Elements;
    size_t                   NumElements;
} DenOfIz_ShaderStageDescArray;

typedef struct DenOfIz_CompiledShaderStage
{
    DenOfIz_ShaderStageFlags     Stage;
    DenOfIz_ByteArray            DXIL;
    DenOfIz_ByteArray            MSL;
    DenOfIz_ByteArray            SPIRV;
    DenOfIz_ByteArray            WGSL;
    DenOfIz_ByteArray            Reflection;
    DenOfIz_StringView           EntryPoint;
    DenOfIz_RayTracingShaderDesc RayTracing;
    DenOfIz_BindlessDesc         Bindless;
    DenOfIz_ThreadGroupDesc      ThreadGroup;
} DenOfIz_CompiledShaderStage;

typedef struct DenOfIz_CompiledShaderStageArray
{
    DenOfIz_CompiledShaderStage **Elements;
    size_t                        NumElements;
} DenOfIz_CompiledShaderStageArray;

typedef struct DenOfIz_ShaderProgramDesc
{
    DenOfIz_ShaderStageDescArray ShaderStages;
    DenOfIz_ShaderRayTracingDesc RayTracing;
    DenOfIz_ShaderIncludeHandler IncludeHandler;
} DenOfIz_ShaderProgramDesc;
