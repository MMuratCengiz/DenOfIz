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

#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <cstdbool>
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

typedef enum DenOfIz_FormatSubType
{
    DENOFIZ_FORMAT_SUB_TYPE_UNDEFINED,
    DENOFIZ_FORMAT_SUB_TYPE_FLOAT,
    DENOFIZ_FORMAT_SUB_TYPE_UINT,
    DENOFIZ_FORMAT_SUB_TYPE_SINT,
    DENOFIZ_FORMAT_SUB_TYPE_UNORM,
    DENOFIZ_FORMAT_SUB_TYPE_SNORM,
    DENOFIZ_FORMAT_SUB_TYPE_TYPELESS,
    DENOFIZ_FORMAT_SUB_TYPE_SRGB,
} DenOfIz_FormatSubType;

typedef enum DenOfIz_Format
{
    DENOFIZ_FORMAT_UNDEFINED,
    DENOFIZ_FORMAT_R32G32B32A32_FLOAT,
    DENOFIZ_FORMAT_R32G32B32A32_SINT,
    DENOFIZ_FORMAT_R32G32B32A32_TYPELESS,
    DENOFIZ_FORMAT_R32G32B32A32_UINT,
    DENOFIZ_FORMAT_R32G32B32_FLOAT,
    DENOFIZ_FORMAT_R32G32B32_UINT,
    DENOFIZ_FORMAT_R32G32B32_SINT,
    DENOFIZ_FORMAT_R16G16B16A16_FLOAT,
    DENOFIZ_FORMAT_R16G16B16A16_SNORM,
    DENOFIZ_FORMAT_R16G16B16A16_SINT,
    DENOFIZ_FORMAT_R16G16B16A16_TYPELESS,
    DENOFIZ_FORMAT_R16G16B16A16_UNORM,
    DENOFIZ_FORMAT_R16G16B16A16_UINT,
    DENOFIZ_FORMAT_R32G32_FLOAT,
    DENOFIZ_FORMAT_R32G32_SINT,
    DENOFIZ_FORMAT_R32G32_TYPELESS,
    DENOFIZ_FORMAT_R32G32_UINT,
    DENOFIZ_FORMAT_R10G10B10A2_TYPELESS,
    DENOFIZ_FORMAT_R10G10B10A2_UINT,
    DENOFIZ_FORMAT_R10G10B10A2_UNORM,
    DENOFIZ_FORMAT_R8G8B8A8_UINT,
    DENOFIZ_FORMAT_R8G8B8A8_SINT,
    DENOFIZ_FORMAT_R8G8B8A8_SNORM,
    DENOFIZ_FORMAT_R8G8B8A8_TYPELESS,
    DENOFIZ_FORMAT_R8G8B8A8_UNORM,
    DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB,
    DENOFIZ_FORMAT_R16G16_FLOAT,
    DENOFIZ_FORMAT_R16G16_SINT,
    DENOFIZ_FORMAT_R16G16_SNORM,
    DENOFIZ_FORMAT_R16G16_TYPELESS,
    DENOFIZ_FORMAT_R16G16_UINT,
    DENOFIZ_FORMAT_R16G16_UNORM,
    DENOFIZ_FORMAT_D32_FLOAT,
    DENOFIZ_FORMAT_R32_FLOAT,
    DENOFIZ_FORMAT_R32_SINT,
    DENOFIZ_FORMAT_R32_TYPELESS,
    DENOFIZ_FORMAT_R32_UINT,
    DENOFIZ_FORMAT_D24_UNORM_S8_UINT,
    DENOFIZ_FORMAT_R8G8_SNORM,
    DENOFIZ_FORMAT_R8G8_TYPELESS,
    DENOFIZ_FORMAT_R8G8_UINT,
    DENOFIZ_FORMAT_R8G8_UNORM,
    DENOFIZ_FORMAT_R8G8_SINT,
    DENOFIZ_FORMAT_D16_UNORM,
    DENOFIZ_FORMAT_R16_FLOAT,
    DENOFIZ_FORMAT_R16_SINT,
    DENOFIZ_FORMAT_R16_SNORM,
    DENOFIZ_FORMAT_R16_TYPELESS,
    DENOFIZ_FORMAT_R16_UINT,
    DENOFIZ_FORMAT_R16_UNORM,
    DENOFIZ_FORMAT_R8_SINT,
    DENOFIZ_FORMAT_R8_SNORM,
    DENOFIZ_FORMAT_R8_TYPELESS,
    DENOFIZ_FORMAT_R8_UINT,
    DENOFIZ_FORMAT_R8_UNORM,
    DENOFIZ_FORMAT_BC1_UNORM,
    DENOFIZ_FORMAT_BC1_UNORM_SRGB,
    DENOFIZ_FORMAT_BC2_UNORM,
    DENOFIZ_FORMAT_BC2_UNORM_SRGB,
    DENOFIZ_FORMAT_BC3_UNORM,
    DENOFIZ_FORMAT_BC3_UNORM_SRGB,
    DENOFIZ_FORMAT_BC4_UNORM,
    DENOFIZ_FORMAT_BC4_SNORM,
    DENOFIZ_FORMAT_BC5_UNORM,
    DENOFIZ_FORMAT_BC5_SNORM,
    DENOFIZ_FORMAT_B8G8R8A8_UNORM,
    DENOFIZ_FORMAT_BC6H_UFLOAT16,
    DENOFIZ_FORMAT_BC6H_SFLOAT16,
    DENOFIZ_FORMAT_BC7_UNORM,
    DENOFIZ_FORMAT_BC7_UNORM_SRGB
} DenOfIz_Format;

typedef enum DenOfIz_PrimitiveTopology
{
    DENOFIZ_PRIMITIVE_TOPOLOGY_POINT,
    DENOFIZ_PRIMITIVE_TOPOLOGY_LINE,
    DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE,
    DENOFIZ_PRIMITIVE_TOPOLOGY_PATCH
} DenOfIz_PrimitiveTopology;

typedef enum DenOfIz_IndexType
{
    DENOFIZ_INDEX_TYPE_UINT16,
    DENOFIZ_INDEX_TYPE_UINT32
} DenOfIz_IndexType;

typedef enum DenOfIz_ViewMask
{
    DENOFIZ_VIEW_MASK_NONE = 0,
    DENOFIZ_VIEW_MASK_R    = 0x00000001,
    DENOFIZ_VIEW_MASK_G    = 0x00000002,
    DENOFIZ_VIEW_MASK_B    = 0x00000004,
    DENOFIZ_VIEW_MASK_A    = 0x00000008
} DenOfIz_ViewMask;

typedef enum DenOfIz_MSAASampleCount
{
    DENOFIZ_MSAA_SAMPLE_COUNT_0,
    DENOFIZ_MSAA_SAMPLE_COUNT_1,
    DENOFIZ_MSAA_SAMPLE_COUNT_2,
    DENOFIZ_MSAA_SAMPLE_COUNT_4,
    DENOFIZ_MSAA_SAMPLE_COUNT_8,
    DENOFIZ_MSAA_SAMPLE_COUNT_16,
    DENOFIZ_MSAA_SAMPLE_COUNT_32,
    DENOFIZ_MSAA_SAMPLE_COUNT_64,
} DenOfIz_MSAASampleCount;

typedef enum DenOfIz_HeapType
{
    DENOFIZ_HEAP_TYPE_GPU,
    DENOFIZ_HEAP_TYPE_CPU,
    DENOFIZ_HEAP_TYPE_CPU_GPU,
    DENOFIZ_HEAP_TYPE_GPU_CPU
} DenOfIz_HeapType;

typedef enum DenOfIz_TextureAspect
{
    DENOFIZ_TEXTURE_ASPECT_COLOR,
    DENOFIZ_TEXTURE_ASPECT_DEPTH,
    DENOFIZ_TEXTURE_ASPECT_STENCIL,
    DENOFIZ_TEXTURE_ASPECT_METADATA,
    DENOFIZ_TEXTURE_ASPECT_PLANE0,
    DENOFIZ_TEXTURE_ASPECT_PLANE1,
    DENOFIZ_TEXTURE_ASPECT_PLANE2,
    DENOFIZ_TEXTURE_ASPECT_NONE
} DenOfIz_TextureAspect;

typedef enum DenOfIz_SamplerAddressMode
{
    DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT,
    DENOFIZ_SAMPLER_ADDRESS_MODE_MIRROR,
    DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
} DenOfIz_SamplerAddressMode;

typedef enum DenOfIz_MipmapMode
{
    DENOFIZ_MIPMAP_MODE_NEAREST,
    DENOFIZ_MIPMAP_MODE_LINEAR
} DenOfIz_MipmapMode;

typedef enum DenOfIz_Filter
{
    DENOFIZ_FILTER_NEAREST,
    DENOFIZ_FILTER_LINEAR,
} DenOfIz_Filter;

typedef enum DenOfIz_CompareOp
{
    DENOFIZ_COMPARE_OP_NEVER,
    DENOFIZ_COMPARE_OP_EQUAL,
    DENOFIZ_COMPARE_OP_NOT_EQUAL,
    DENOFIZ_COMPARE_OP_ALWAYS,
    DENOFIZ_COMPARE_OP_LESS,
    DENOFIZ_COMPARE_OP_LESS_OR_EQUAL,
    DENOFIZ_COMPARE_OP_GREATER,
    DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL
} DenOfIz_CompareOp;

typedef enum DenOfIz_StencilOp
{
    DENOFIZ_STENCIL_OP_KEEP,
    DENOFIZ_STENCIL_OP_ZERO,
    DENOFIZ_STENCIL_OP_REPLACE,
    DENOFIZ_STENCIL_OP_INCREMENT_AND_CLAMP,
    DENOFIZ_STENCIL_OP_DECREMENT_AND_CLAMP,
    DENOFIZ_STENCIL_OP_INVERT,
    DENOFIZ_STENCIL_OP_INCREMENT_AND_WRAP,
    DENOFIZ_STENCIL_OP_DECREMENT_AND_WRAP
} DenOfIz_StencilOp;

typedef enum DenOfIz_BufferUsageFlagBits
{
    DENOFIZ_BUFFER_USAGE_NONE_BIT                   = 0,
    DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT               = 1 << 0,
    DENOFIZ_BUFFER_USAGE_COPY_DST_BIT               = 1 << 1,
    DENOFIZ_BUFFER_USAGE_INDEX_BIT                  = 1 << 2,
    DENOFIZ_BUFFER_USAGE_VERTEX_BIT                 = 1 << 3,
    DENOFIZ_BUFFER_USAGE_UNIFORM_BIT                = 1 << 4,
    DENOFIZ_BUFFER_USAGE_STORAGE_BIT                = 1 << 5,
    DENOFIZ_BUFFER_USAGE_INDIRECT_BIT               = 1 << 6,
    DENOFIZ_BUFFER_USAGE_QUERY_RESOLVE_BIT          = 1 << 7,
    DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT = 1 << 8,
    DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT   = 1 << 9,
    DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT  = 1 << 10,
} DenOfIz_BufferUsageFlagBits;
typedef uint32_t DenOfIz_BufferUsageFlags;

typedef enum DenOfIz_TextureUsageFlagBits
{
    DENOFIZ_TEXTURE_USAGE_NONE_BIT              = 0,
    DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT          = 1 << 0,
    DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT          = 1 << 1,
    DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT   = 1 << 2,
    DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT   = 1 << 3,
    DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT = 1 << 4,
    DENOFIZ_TEXTURE_USAGE_CUBE_BIT              = 1 << 5,
} DenOfIz_TextureUsageFlagBits;
typedef uint32_t DenOfIz_TextureUsageFlags;

typedef enum DenOfIz_ResourceDescriptorFlagBits
{
    DENOFIZ_RESOURCE_DESCRIPTOR_NONE_BIT                   = 0,
    DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT                 = 1 << 0,
    DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT              = 1 << 1,
    DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT                = 1 << 2,
    DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT             = 1 << 3,
    DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT          = 1 << 4,
    DENOFIZ_RESOURCE_DESCRIPTOR_DEPTH_STENCIL_BIT          = 1 << 5,
    DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT                = 1 << 6,
    DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT         = 1 << 7,
    DENOFIZ_RESOURCE_DESCRIPTOR_ROOT_CONSTANT_BIT          = 1 << 8,
    DENOFIZ_RESOURCE_DESCRIPTOR_INDEX_BUFFER_BIT           = 1 << 9,
    DENOFIZ_RESOURCE_DESCRIPTOR_VERTEX_BUFFER_BIT          = 1 << 10,
    DENOFIZ_RESOURCE_DESCRIPTOR_INDIRECT_BUFFER_BIT        = 1 << 11,
    DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_CUBE_BIT           = 1 << 12,
    DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT = 1 << 13,
    DENOFIZ_RESOURCE_DESCRIPTOR_STRUCTURED_BUFFER_BIT      = 1 << 14,
    DENOFIZ_RESOURCE_DESCRIPTOR_RAW_BUFFER_BIT             = 1 << 15
} DenOfIz_ResourceDescriptorFlagBits;
typedef uint32_t DenOfIz_ResourceDescriptorFlags;

typedef enum DenOfIz_ResourceUsageFlagBits
{
    DENOFIZ_RESOURCE_USAGE_NONE_BIT                            = 0,
    DENOFIZ_RESOURCE_USAGE_UNDEFINED_BIT                       = 1 << 0,
    DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT      = 1 << 1,
    DENOFIZ_RESOURCE_USAGE_INDEX_BUFFER_BIT                    = 1 << 2,
    DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT                   = 1 << 3,
    DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT                = 1 << 4,
    DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT                     = 1 << 5,
    DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT                      = 1 << 6,
    DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT                 = 1 << 7,
    DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT           = 1 << 8,
    DENOFIZ_RESOURCE_USAGE_STREAM_OUT_BIT                      = 1 << 9,
    DENOFIZ_RESOURCE_USAGE_INDIRECT_ARGUMENT_BIT               = 1 << 10,
    DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT                        = 1 << 11,
    DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT                        = 1 << 12,
    DENOFIZ_RESOURCE_USAGE_GENERIC_READ_BIT                    = 1 << 13,
    DENOFIZ_RESOURCE_USAGE_PRESENT_BIT                         = 1 << 14,
    DENOFIZ_RESOURCE_USAGE_COMMON_BIT                          = 1 << 15,
    DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT     = 1 << 16,
    DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT    = 1 << 17,
    DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_GEOMETRY_BIT = 1 << 18,
    DENOFIZ_RESOURCE_USAGE_SHADER_BINDING_TABLE_BIT            = 1 << 19
} DenOfIz_ResourceUsageFlagBits;
typedef uint32_t DenOfIz_ResourceUsageFlags;

typedef enum DenOfIz_ResourceBindingType
{
    DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER  = 0,
    DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE  = 1,
    DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS = 2,
    DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER          = 3
} DenOfIz_ResourceBindingType;

typedef struct DenOfIz_ResourceBindingSlot
{
    DenOfIz_ResourceBindingType Type;
    uint32_t                    Binding;
    uint32_t                    RegisterSpace;
} DenOfIz_ResourceBindingSlot;

typedef struct DenOfIz_ResourceBindingSlotArray
{
    DenOfIz_ResourceBindingSlot *Elements;
    uint32_t                     NumElements;
} DenOfIz_ResourceBindingSlotArray;

typedef enum DenOfIz_LoadOp
{
    DENOFIZ_LOAD_OP_CLEAR,
    DENOFIZ_LOAD_OP_LOAD,
    DENOFIZ_LOAD_OP_DONT_CARE
} DenOfIz_LoadOp;

typedef enum DenOfIz_StoreOp
{
    DENOFIZ_STORE_OP_STORE,
    DENOFIZ_STORE_OP_NONE,
    DENOFIZ_STORE_OP_DONT_CARE
} DenOfIz_StoreOp;

typedef enum DenOfIz_QueueType
{
    DENOFIZ_QUEUE_TYPE_GRAPHICS,
    DENOFIZ_QUEUE_TYPE_COMPUTE,
    DENOFIZ_QUEUE_TYPE_COPY
} DenOfIz_QueueType;

typedef enum DenOfIz_QueryType
{
    DENOFIZ_QUERY_TYPE_OCCLUSION,
    DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS,
    DENOFIZ_QUERY_TYPE_TIMESTAMP
} DenOfIz_QueryType;

typedef enum DenOfIz_QueryPipelineStatisticFlagBits
{
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_NONE_BIT                        = 0,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_VERTICES_BIT     = 1 << 0,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_INPUT_ASSEMBLY_PRIMITIVES_BIT   = 1 << 1,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_VERTEX_SHADER_INVOCATIONS_BIT   = 1 << 2,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_INVOCATIONS_BIT = 1 << 3,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_GEOMETRY_SHADER_PRIMITIVES_BIT  = 1 << 4,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_INVOCATIONS_BIT        = 1 << 5,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_CLIPPING_PRIMITIVES_BIT         = 1 << 6,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_PIXEL_SHADER_INVOCATIONS_BIT    = 1 << 7,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_HULL_SHADER_INVOCATIONS_BIT     = 1 << 8,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_DOMAIN_SHADER_INVOCATIONS_BIT   = 1 << 9,
    DENOFIZ_QUERY_PIPELINE_STATISTIC_FLAG_COMPUTE_SHADER_INVOCATIONS_BIT  = 1 << 10,
} DenOfIz_QueryPipelineStatisticFlagBits;
typedef uint32_t DenOfIz_QueryPipelineStatisticFlags;

typedef enum DenOfIz_PipelineStageFlagBits
{
    DENOFIZ_PIPELINE_STAGE_FLAG_NONE_BIT                         = 0,
    DENOFIZ_PIPELINE_STAGE_FLAG_TOP_OF_PIPE_BIT                  = 1 << 0,
    DENOFIZ_PIPELINE_STAGE_FLAG_DRAW_INDIRECT_BIT                = 1 << 1,
    DENOFIZ_PIPELINE_STAGE_FLAG_VERTEX_INPUT_BIT                 = 1 << 2,
    DENOFIZ_PIPELINE_STAGE_FLAG_VERTEX_SHADER_BIT                = 1 << 3,
    DENOFIZ_PIPELINE_STAGE_FLAG_HULL_SHADER_BIT                  = 1 << 4,
    DENOFIZ_PIPELINE_STAGE_FLAG_DOMAIN_SHADER_BIT                = 1 << 5,
    DENOFIZ_PIPELINE_STAGE_FLAG_GEOMETRY_SHADER_BIT              = 1 << 6,
    DENOFIZ_PIPELINE_STAGE_FLAG_PIXEL_SHADER_BIT                 = 1 << 7,
    DENOFIZ_PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS_BIT         = 1 << 8,
    DENOFIZ_PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS_BIT          = 1 << 9,
    DENOFIZ_PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT_BIT      = 1 << 10,
    DENOFIZ_PIPELINE_STAGE_FLAG_COMPUTE_SHADER_BIT               = 1 << 11,
    DENOFIZ_PIPELINE_STAGE_FLAG_TRANSFER_BIT                     = 1 << 12,
    DENOFIZ_PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE_BIT               = 1 << 13,
    DENOFIZ_PIPELINE_STAGE_FLAG_HOST_BIT                         = 1 << 14,
    DENOFIZ_PIPELINE_STAGE_FLAG_ALL_GRAPHICS_BIT                 = 1 << 15,
    DENOFIZ_PIPELINE_STAGE_FLAG_ALL_COMMANDS_BIT                 = 1 << 16,
    DENOFIZ_PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER_BIT           = 1 << 17,
    DENOFIZ_PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD_BIT = 1 << 18,
    DENOFIZ_PIPELINE_STAGE_FLAG_TASK_SHADER_BIT                  = 1 << 19,
    DENOFIZ_PIPELINE_STAGE_FLAG_MESH_SHADER_BIT                  = 1 << 20,
} DenOfIz_PipelineStageFlagBits;
typedef uint32_t DenOfIz_PipelineStageFlags;

typedef struct DenOfIz_PipelineStatisticsData
{
    uint64_t InputAssemblyVertices;
    uint64_t InputAssemblyPrimitives;
    uint64_t VertexShaderInvocations;
    uint64_t GeometryShaderInvocations;
    uint64_t GeometryShaderPrimitives;
    uint64_t ClippingInvocations;
    uint64_t ClippingPrimitives;
    uint64_t PixelShaderInvocations;
    uint64_t HullShaderInvocations;
    uint64_t DomainShaderInvocations;
    uint64_t ComputeShaderInvocations;
} DenOfIz_PipelineStatisticsData;

typedef struct DenOfIz_QueryDesc
{
    uint32_t Index;
} DenOfIz_QueryDesc;

typedef struct DenOfIz_QueryData
{
    bool                           Valid;
    uint64_t                       BeginTimestamp;
    uint64_t                       EndTimestamp;
    uint64_t                       OcclusionCounts;
    DenOfIz_PipelineStatisticsData PipelineStats;
} DenOfIz_QueryData;

typedef struct DenOfIz_DeviceConstants
{
    uint32_t StorageBufferAlignment;
    uint32_t ConstantBufferAlignment;
    uint32_t BufferTextureAlignment;
    uint32_t BufferTextureRowAlignment;
} DenOfIz_DeviceConstants;

typedef struct DenOfIz_PhysicalDeviceCapabilities
{
    bool DedicatedCopyQueue;
    bool RayTracing;
    bool ComputeShaders;
    bool Tearing;
    bool Tessellation;
    bool GeometryShaders;
    bool HDR;
    bool MeshShaders;
    bool VariableRateShading;
    bool SamplerFeedback;
    bool DrawIndirectCount;
    bool ConservativeRasterization;
    bool ShaderInt16;
    bool ShaderFloat16;
    bool TiledResources;
    bool MultiDrawIndirect;
    bool QueryStatistics;
    bool SrvArray;
    bool Bindless;
} DenOfIz_PhysicalDeviceCapabilities;

typedef struct DenOfIz_PhysicalDeviceProperties
{
    bool         IsDedicated;
    unsigned int MemoryAvailableInMb;
} DenOfIz_PhysicalDeviceProperties;

typedef struct DenOfIz_PhysicalDevice
{
    long                               Id;
    DenOfIz_StringView                 Name;
    DenOfIz_PhysicalDeviceProperties   Properties;
    DenOfIz_PhysicalDeviceCapabilities Capabilities;
    DenOfIz_DeviceConstants            Constants;
} DenOfIz_PhysicalDevice;

typedef struct DenOfIz_PhysicalDeviceArray
{
    DenOfIz_PhysicalDevice *Elements;
    uint32_t                NumElements;
} DenOfIz_PhysicalDeviceArray;

#ifndef DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE
#define DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE 30
#endif

#ifndef DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE
#define DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE 31
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    DZ_API uint32_t              DenOfIz_Format_NumBytes( DenOfIz_Format format );
    DZ_API DenOfIz_Format        DenOfIz_Format_ToTypeless( DenOfIz_Format format );
    DZ_API DenOfIz_FormatSubType DenOfIz_Format_GetSubType( DenOfIz_Format format );
    DZ_API bool                  DenOfIz_Format_IsBC( DenOfIz_Format format );
    DZ_API uint32_t              DenOfIz_Format_BlockSize( DenOfIz_Format format );

    DZ_API bool DenOfIz_Format_IsDepth( DenOfIz_Format format );
    DZ_API bool DenOfIz_Format_IsStencil( DenOfIz_Format format );
    DZ_API bool DenOfIz_Format_IsDepthOnly( DenOfIz_Format format );
    DZ_API bool DenOfIz_Format_IsDepthStencil( DenOfIz_Format format );
    DZ_API bool DenOfIz_Format_HasStencilComponent( DenOfIz_Format format );

    DZ_API DenOfIz_ResourceBindingType DenOfIz_ResourceBindingType_FromDescriptor( uint32_t descriptor );
    DZ_API uint32_t                    DenOfIz_ResourceBindingSlot_Key( const DenOfIz_ResourceBindingSlot *slot );

    DZ_API int DenOfIz_MSAASampleCount_ToNumSamples( DenOfIz_MSAASampleCount sampleCount );
#ifdef __cplusplus
}
#endif

#endif
