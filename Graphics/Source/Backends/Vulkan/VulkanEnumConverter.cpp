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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

VkQueueFlags DenOfIz_VulkanEnumConverter_ConvertQueueFlags( const DenOfIz_QueueType queueType )
{
    switch ( queueType )
    {
    case DENOFIZ_QUEUE_TYPE_GRAPHICS:
        return VK_QUEUE_GRAPHICS_BIT;
    case DENOFIZ_QUEUE_TYPE_COMPUTE:
        return VK_QUEUE_COMPUTE_BIT;
    case DENOFIZ_QUEUE_TYPE_COPY:
        return VK_QUEUE_TRANSFER_BIT;
    }
    return VK_QUEUE_GRAPHICS_BIT;
}

VkShaderStageFlags DenOfIz_VulkanEnumConverter_ConvertShaderStage( const DenOfIz_ShaderStageFlags shaderStages )
{
    VkShaderStageFlags result = 0;

    if ( shaderStages & DENOFIZ_SHADER_STAGE_VERTEX_BIT )
    {
        result |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_HULL_BIT )
    {
        result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
    {
        result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
    {
        result |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
    {
        result |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
    {
        result |= VK_SHADER_STAGE_COMPUTE_BIT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_RAY_GEN_BIT )
    {
        result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_ANY_HIT_BIT )
    {
        result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT )
    {
        result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_MISS_BIT )
    {
        result |= VK_SHADER_STAGE_MISS_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_INTERSECTION_BIT )
    {
        result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_CALLABLE_BIT )
    {
        result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_TASK_BIT )
    {
        result |= VK_SHADER_STAGE_TASK_BIT_EXT;
    }
    if ( shaderStages & DENOFIZ_SHADER_STAGE_MESH_BIT )
    {
        result |= VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    return result != 0 ? result : VK_SHADER_STAGE_ALL;
}

VkSampleCountFlagBits DenOfIz_VulkanEnumConverter_ConvertSampleCount( const DenOfIz_MSAASampleCount sampleCount )
{
    switch ( sampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_0:
    case DENOFIZ_MSAA_SAMPLE_COUNT_1:
        return VK_SAMPLE_COUNT_1_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_2:
        return VK_SAMPLE_COUNT_2_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_4:
        return VK_SAMPLE_COUNT_4_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_8:
        return VK_SAMPLE_COUNT_8_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_16:
        return VK_SAMPLE_COUNT_16_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_32:
        return VK_SAMPLE_COUNT_32_BIT;
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        return VK_SAMPLE_COUNT_64_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkStencilOp DenOfIz_VulkanEnumConverter_ConvertStencilOp( const DenOfIz_StencilOp stencilOp )
{
    switch ( stencilOp )
    {
    case DENOFIZ_STENCIL_OP_KEEP:
        return VK_STENCIL_OP_KEEP;
    case DENOFIZ_STENCIL_OP_ZERO:
        return VK_STENCIL_OP_ZERO;
    case DENOFIZ_STENCIL_OP_REPLACE:
        return VK_STENCIL_OP_REPLACE;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_CLAMP:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_CLAMP:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case DENOFIZ_STENCIL_OP_INVERT:
        return VK_STENCIL_OP_INVERT;
    case DENOFIZ_STENCIL_OP_INCREMENT_AND_WRAP:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case DENOFIZ_STENCIL_OP_DECREMENT_AND_WRAP:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }

    return VK_STENCIL_OP_ZERO;
}

VkCompareOp DenOfIz_VulkanEnumConverter_ConvertCompareOp( const DenOfIz_CompareOp compareOp )
{
    switch ( compareOp )
    {
    case DENOFIZ_COMPARE_OP_NEVER:
        return VK_COMPARE_OP_NEVER;
    case DENOFIZ_COMPARE_OP_ALWAYS:
        return VK_COMPARE_OP_ALWAYS;
    case DENOFIZ_COMPARE_OP_EQUAL:
        return VK_COMPARE_OP_EQUAL;
    case DENOFIZ_COMPARE_OP_NOT_EQUAL:
        return VK_COMPARE_OP_NOT_EQUAL;
    case DENOFIZ_COMPARE_OP_LESS:
        return VK_COMPARE_OP_LESS;
    case DENOFIZ_COMPARE_OP_LESS_OR_EQUAL:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case DENOFIZ_COMPARE_OP_GREATER:
        return VK_COMPARE_OP_GREATER;
    case DENOFIZ_COMPARE_OP_GREATER_OR_EQUAL:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    }

    return VK_COMPARE_OP_ALWAYS;
}

VkAttachmentLoadOp DenOfIz_VulkanEnumConverter_ConvertLoadOp( const DenOfIz_LoadOp loadOp )
{
    switch ( loadOp )
    {
    case DENOFIZ_LOAD_OP_LOAD:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case DENOFIZ_LOAD_OP_CLEAR:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case DENOFIZ_LOAD_OP_DONT_CARE:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

VkAttachmentStoreOp DenOfIz_VulkanEnumConverter_ConvertStoreOp( const DenOfIz_StoreOp storeOp )
{
    switch ( storeOp )
    {
    case DENOFIZ_STORE_OP_STORE:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case DENOFIZ_STORE_OP_NONE:
        return VK_ATTACHMENT_STORE_OP_NONE;
    case DENOFIZ_STORE_OP_DONT_CARE:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_STORE_OP_STORE;
}

VkBlendOp DenOfIz_VulkanEnumConverter_ConvertBlendOp( const DenOfIz_BlendOp op )
{
    switch ( op )
    {
    case DENOFIZ_BLEND_OP_ADD:
        return VK_BLEND_OP_ADD;
    case DENOFIZ_BLEND_OP_SUBTRACT:
        return VK_BLEND_OP_SUBTRACT;
    case DENOFIZ_BLEND_OP_REVERSE_SUBTRACT:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case DENOFIZ_BLEND_OP_MIN:
        return VK_BLEND_OP_MIN;
    case DENOFIZ_BLEND_OP_MAX:
        return VK_BLEND_OP_MAX;
    }

    return VK_BLEND_OP_ADD;
}

VkLogicOp DenOfIz_VulkanEnumConverter_ConvertLogicOp( const DenOfIz_LogicOp op )
{
    switch ( op )
    {
    case DENOFIZ_LOGIC_OP_CLEAR:
        return VK_LOGIC_OP_CLEAR;
    case DENOFIZ_LOGIC_OP_AND:
        return VK_LOGIC_OP_AND;
    case DENOFIZ_LOGIC_OP_AND_REVERSE:
        return VK_LOGIC_OP_AND_REVERSE;
    case DENOFIZ_LOGIC_OP_COPY:
        return VK_LOGIC_OP_COPY;
    case DENOFIZ_LOGIC_OP_AND_INVERTED:
        return VK_LOGIC_OP_AND_INVERTED;
    case DENOFIZ_LOGIC_OP_NO_OP:
        return VK_LOGIC_OP_NO_OP;
    case DENOFIZ_LOGIC_OP_XOR:
        return VK_LOGIC_OP_XOR;
    case DENOFIZ_LOGIC_OP_OR:
        return VK_LOGIC_OP_OR;
    case DENOFIZ_LOGIC_OP_NOR:
        return VK_LOGIC_OP_NOR;
    case DENOFIZ_LOGIC_OP_EQUIV:
        return VK_LOGIC_OP_EQUIVALENT;
    case DENOFIZ_LOGIC_OP_INVERT:
        return VK_LOGIC_OP_INVERT;
    case DENOFIZ_LOGIC_OP_OR_REVERSE:
        return VK_LOGIC_OP_OR_REVERSE;
    case DENOFIZ_LOGIC_OP_COPY_INVERTED:
        return VK_LOGIC_OP_COPY_INVERTED;
    case DENOFIZ_LOGIC_OP_OR_INVERTED:
        return VK_LOGIC_OP_OR_INVERTED;
    case DENOFIZ_LOGIC_OP_NAND:
        return VK_LOGIC_OP_NAND;
    case DENOFIZ_LOGIC_OP_SET:
        return VK_LOGIC_OP_SET;
    }

    return VK_LOGIC_OP_CLEAR;
}

VkBlendFactor DenOfIz_VulkanEnumConverter_ConvertBlend( const DenOfIz_Blend blend )
{
    switch ( blend )
    {
    case DENOFIZ_BLEND_ZERO:
        return VK_BLEND_FACTOR_ZERO;
    case DENOFIZ_BLEND_ONE:
        return VK_BLEND_FACTOR_ONE;
    case DENOFIZ_BLEND_SRC_COLOR:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case DENOFIZ_BLEND_DST_COLOR:
        return VK_BLEND_FACTOR_DST_COLOR;
    case DENOFIZ_BLEND_SRC_ALPHA:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case DENOFIZ_BLEND_DST_ALPHA:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case DENOFIZ_BLEND_SRC_ALPHA_SATURATE:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case DENOFIZ_BLEND_SRC1_COLOR:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case DENOFIZ_BLEND_SRC1_ALPHA:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case DENOFIZ_BLEND_INV_SRC_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case DENOFIZ_BLEND_INV_SRC_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case DENOFIZ_BLEND_INV_DST_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case DENOFIZ_BLEND_INV_DST_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case DENOFIZ_BLEND_INV_SRC1_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case DENOFIZ_BLEND_INV_SRC1_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }

    return VK_BLEND_FACTOR_ZERO;
}

VkFilter DenOfIz_VulkanEnumConverter_ConvertFilter( const DenOfIz_Filter filter )
{
    switch ( filter )
    {
    case DENOFIZ_FILTER_NEAREST:
        return VK_FILTER_NEAREST;
    case DENOFIZ_FILTER_LINEAR:
        return VK_FILTER_LINEAR;
    }

    return VK_FILTER_LINEAR;
}

VkSamplerAddressMode DenOfIz_VulkanEnumConverter_ConvertAddressMode( const DenOfIz_SamplerAddressMode addressMode )
{
    switch ( addressMode )
    {
    case DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_MIRROR:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkSamplerMipmapMode DenOfIz_VulkanEnumConverter_ConvertMipmapMode( const DenOfIz_MipmapMode mipmapMode )
{
    switch ( mipmapMode )
    {
    case DENOFIZ_MIPMAP_MODE_NEAREST:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case DENOFIZ_MIPMAP_MODE_LINEAR:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

// TODO !IMPROVEMENT! This might be incorrect
VkBufferUsageFlags DenOfIz_VulkanEnumConverter_ConvertBufferUsage( const DenOfIz_ResourceUsageFlags descriptor, const DenOfIz_ResourceUsageFlags usages )
{
    VkBufferUsageFlags flags = { };
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDEX_BUFFER_BIT )
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_VERTEX_BUFFER_BIT )
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT )
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT || descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ||
         descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_STRUCTURED_BUFFER_BIT )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDIRECT_BUFFER_BIT )
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_SHADER_BINDING_TABLE_BIT )
    {
        flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_GEOMETRY_BIT )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if ( usages & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    return flags;
}

VkImageAspectFlagBits DenOfIz_VulkanEnumConverter_ConvertImageAspect( const DenOfIz_TextureAspect aspect )
{
    switch ( aspect )
    {
    case DENOFIZ_TEXTURE_ASPECT_COLOR:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case DENOFIZ_TEXTURE_ASPECT_DEPTH:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case DENOFIZ_TEXTURE_ASPECT_STENCIL:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case DENOFIZ_TEXTURE_ASPECT_METADATA:
        return VK_IMAGE_ASPECT_METADATA_BIT;
    case DENOFIZ_TEXTURE_ASPECT_PLANE0:
        return VK_IMAGE_ASPECT_PLANE_0_BIT;
    case DENOFIZ_TEXTURE_ASPECT_PLANE1:
        return VK_IMAGE_ASPECT_PLANE_1_BIT;
    case DENOFIZ_TEXTURE_ASPECT_PLANE2:
        return VK_IMAGE_ASPECT_PLANE_2_BIT;
    case DENOFIZ_TEXTURE_ASPECT_NONE:
        return VK_IMAGE_ASPECT_NONE;
    }

    return VK_IMAGE_ASPECT_NONE;
}

VkImageUsageFlags DenOfIz_VulkanEnumConverter_ConvertTextureDescriptorToUsage( const DenOfIz_ResourceDescriptorFlags descriptor, const DenOfIz_ResourceUsageFlags initialState )
{
    VkImageUsageFlags usage = { };

    if ( initialState & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT || initialState & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return usage;
}

VmaMemoryUsage DenOfIz_VulkanEnumConverter_ConvertHeapType( const DenOfIz_HeapType location )
{
    switch ( location )
    {
    case DENOFIZ_HEAP_TYPE_GPU:
        return VMA_MEMORY_USAGE_GPU_ONLY;
    case DENOFIZ_HEAP_TYPE_CPU:
        return VMA_MEMORY_USAGE_CPU_COPY;
    case DENOFIZ_HEAP_TYPE_CPU_GPU:
        return VMA_MEMORY_USAGE_CPU_TO_GPU;
    case DENOFIZ_HEAP_TYPE_GPU_CPU:
        return VMA_MEMORY_USAGE_GPU_TO_CPU;
    }

    return VMA_MEMORY_USAGE_AUTO;
}

VkFormat DenOfIz_VulkanEnumConverter_ConvertImageFormat( const DenOfIz_Format imageFormat )
{
    switch ( imageFormat )
    {
    case DENOFIZ_FORMAT_UNDEFINED:
        return VK_FORMAT_UNDEFINED;
    case DENOFIZ_FORMAT_R32G32B32A32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case DENOFIZ_FORMAT_R32G32B32A32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case DENOFIZ_FORMAT_R32G32B32A32_SINT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case DENOFIZ_FORMAT_R32G32B32_FLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case DENOFIZ_FORMAT_R32G32B32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case DENOFIZ_FORMAT_R32G32B32_SINT:
        return VK_FORMAT_R32G32B32_SINT;
    case DENOFIZ_FORMAT_R16G16B16A16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case DENOFIZ_FORMAT_R16G16B16A16_UNORM:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case DENOFIZ_FORMAT_R16G16B16A16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case DENOFIZ_FORMAT_R16G16B16A16_SNORM:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case DENOFIZ_FORMAT_R16G16B16A16_SINT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case DENOFIZ_FORMAT_R32G32_FLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case DENOFIZ_FORMAT_R32G32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case DENOFIZ_FORMAT_R32G32_SINT:
        return VK_FORMAT_R32G32_SINT;
    case DENOFIZ_FORMAT_R10G10B10A2_UNORM:
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case DENOFIZ_FORMAT_R10G10B10A2_UINT:
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case DENOFIZ_FORMAT_R8G8B8A8_UINT:
        return VK_FORMAT_R8G8B8A8_UINT;
    case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case DENOFIZ_FORMAT_R8G8B8A8_SINT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case DENOFIZ_FORMAT_R16G16_FLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case DENOFIZ_FORMAT_R16G16_UNORM:
        return VK_FORMAT_R16G16_UNORM;
    case DENOFIZ_FORMAT_R16G16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case DENOFIZ_FORMAT_R16G16_SNORM:
        return VK_FORMAT_R16G16_SNORM;
    case DENOFIZ_FORMAT_R16G16_SINT:
        return VK_FORMAT_R16G16_SINT;
    case DENOFIZ_FORMAT_D32_FLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case DENOFIZ_FORMAT_R32_FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case DENOFIZ_FORMAT_R32_UINT:
        return VK_FORMAT_R32_UINT;
    case DENOFIZ_FORMAT_R32_SINT:
        return VK_FORMAT_R32_SINT;
    case DENOFIZ_FORMAT_D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case DENOFIZ_FORMAT_R8G8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case DENOFIZ_FORMAT_R8G8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case DENOFIZ_FORMAT_R8G8_SNORM:
        return VK_FORMAT_R8G8_SNORM;
    case DENOFIZ_FORMAT_R8G8_SINT:
        return VK_FORMAT_R8G8_SINT;
    case DENOFIZ_FORMAT_R16_FLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case DENOFIZ_FORMAT_D16_UNORM:
        return VK_FORMAT_D16_UNORM;
    case DENOFIZ_FORMAT_R16_UNORM:
        return VK_FORMAT_R16_UNORM;
    case DENOFIZ_FORMAT_R16_UINT:
        return VK_FORMAT_R16_UINT;
    case DENOFIZ_FORMAT_R16_SNORM:
        return VK_FORMAT_R16_SNORM;
    case DENOFIZ_FORMAT_R16_SINT:
        return VK_FORMAT_R16_SINT;
    case DENOFIZ_FORMAT_R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case DENOFIZ_FORMAT_R8_UINT:
        return VK_FORMAT_R8_UINT;
    case DENOFIZ_FORMAT_R8_SNORM:
        return VK_FORMAT_R8_SNORM;
    case DENOFIZ_FORMAT_R8_SINT:
        return VK_FORMAT_R8_SINT;
    case DENOFIZ_FORMAT_BC1_UNORM:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC2_UNORM:
        return VK_FORMAT_BC2_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
        return VK_FORMAT_BC2_SRGB_BLOCK;
    case DENOFIZ_FORMAT_BC3_UNORM:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
        return VK_FORMAT_BC3_SRGB_BLOCK;
    case DENOFIZ_FORMAT_BC4_UNORM:
        return VK_FORMAT_BC4_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC4_SNORM:
        return VK_FORMAT_BC4_SNORM_BLOCK;
    case DENOFIZ_FORMAT_BC5_UNORM:
        return VK_FORMAT_BC5_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC5_SNORM:
        return VK_FORMAT_BC5_SNORM_BLOCK;
    case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case DENOFIZ_FORMAT_BC6H_UFLOAT16:
        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case DENOFIZ_FORMAT_BC6H_SFLOAT16:
        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case DENOFIZ_FORMAT_BC7_UNORM:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    case DENOFIZ_FORMAT_BC7_UNORM_SRGB:
        return VK_FORMAT_BC7_SRGB_BLOCK;
    case DENOFIZ_FORMAT_R32G32B32A32_TYPELESS:
        // No Typeless in Vulkan
        return VK_FORMAT_R32G32B32_SINT;
    case DENOFIZ_FORMAT_R16G16B16A16_TYPELESS:
        return VK_FORMAT_R16G16B16A16_SINT;
    case DENOFIZ_FORMAT_R32G32_TYPELESS:
        return VK_FORMAT_R32G32_SINT;
    case DENOFIZ_FORMAT_R10G10B10A2_TYPELESS:
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case DENOFIZ_FORMAT_R8G8B8A8_TYPELESS:
        return VK_FORMAT_R8G8B8A8_SINT;
    case DENOFIZ_FORMAT_R16G16_TYPELESS:
        return VK_FORMAT_R16G16_SINT;
    case DENOFIZ_FORMAT_R32_TYPELESS:
        return VK_FORMAT_R32_SINT;
    case DENOFIZ_FORMAT_R8G8_TYPELESS:
        return VK_FORMAT_R8G8_SINT;
    case DENOFIZ_FORMAT_R16_TYPELESS:
        return VK_FORMAT_R16_SINT;
    case DENOFIZ_FORMAT_R8_TYPELESS:
        return VK_FORMAT_R8_SINT;
    }

    return VK_FORMAT_UNDEFINED;
}

VkDescriptorType DenOfIz_VulkanEnumConverter_ConvertResourceDescriptorToDescriptorType( const DenOfIz_ResourceDescriptorFlags descriptor )
{
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT )
    {
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT )
    {
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT )
    {
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT )
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT )
    {
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_STRUCTURED_BUFFER_BIT |
                        DENOFIZ_RESOURCE_DESCRIPTOR_RAW_BUFFER_BIT ) )
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }

    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

VkPrimitiveTopology DenOfIz_VulkanEnumConverter_ConvertPrimitiveTopology( const DenOfIz_PrimitiveTopology topology )
{
    switch ( topology )
    {
    case DENOFIZ_PRIMITIVE_TOPOLOGY_POINT:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_LINE:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case DENOFIZ_PRIMITIVE_TOPOLOGY_PATCH:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }

    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPipelineBindPoint DenOfIz_VulkanEnumConverter_ConvertPipelineBindPoint( const DenOfIz_BindPoint point )
{
    switch ( point )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case DENOFIZ_BIND_POINT_COMPUTE:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    case DENOFIZ_BIND_POINT_RAYTRACING:
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    case DENOFIZ_BIND_POINT_MESH:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    }

    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

VkImageUsageFlags DenOfIz_VulkanEnumConverter_ConvertTextureUsage( const DenOfIz_ResourceDescriptorFlags descriptor, const DenOfIz_ResourceUsageFlags usage )
{
    VkImageUsageFlags flags = 0;
    if ( ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT ) || ( usage & DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT ) )
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ( descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT )
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( ( usage & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT ) || ( usage & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT ) )
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if ( usage & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( flags == 0 )
    {
        spdlog::warn( "No suitable descriptor specified for texture." );
    }
    return flags;
}

// TODO !IMPROVEMENT! This needs to be more complete
VkImageLayout DenOfIz_VulkanEnumConverter_ConvertTextureDescriptorToLayout( const DenOfIz_ResourceUsageFlags initialState )
{
    if ( initialState & ( DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT | DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT ) )
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if ( initialState & ( DENOFIZ_RESOURCE_USAGE_COMMON_BIT | DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT ) )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if ( ( initialState & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT ) && ( initialState & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT ) )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if ( initialState & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkBuildAccelerationStructureFlagsKHR DenOfIz_VulkanEnumConverter_ConvertAccelerationStructureBuildFlags( const DenOfIz_ASBuildFlags buildFlags )
{
    VkBuildAccelerationStructureFlagsKHR flags = 0;
    if ( buildFlags & DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    }
    if ( buildFlags & DENOFIZ_AS_BUILD_ALLOW_COMPACTION_BIT )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    }
    if ( buildFlags & DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    }
    if ( buildFlags & DENOFIZ_AS_BUILD_PREFER_FAST_BUILD_BIT )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    }
    if ( buildFlags & DENOFIZ_AS_BUILD_LOW_MEMORY_BIT )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
    }
    return flags;
}

VkPipelineStageFlagBits DenOfIz_VulkanEnumConverter_ConvertPipelineStageFlags( const DenOfIz_PipelineStageFlags pipelineStage )
{
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_TOP_OF_PIPE_BIT )
    {
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_DRAW_INDIRECT_BIT )
    {
        return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_VERTEX_INPUT_BIT )
    {
        return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_VERTEX_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_HULL_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_DOMAIN_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_GEOMETRY_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_PIXEL_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_EARLY_FRAGMENT_TESTS_BIT )
    {
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS_BIT )
    {
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT_BIT )
    {
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_COMPUTE_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_TRANSFER_BIT )
    {
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE_BIT )
    {
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_HOST_BIT )
    {
        return VK_PIPELINE_STAGE_HOST_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_ALL_GRAPHICS_BIT )
    {
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_ALL_COMMANDS_BIT )
    {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD_BIT )
    {
        return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_TASK_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
    }
    if ( pipelineStage & DENOFIZ_PIPELINE_STAGE_FLAG_MESH_SHADER_BIT )
    {
        return VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
    }

    return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
}

VkQueryType DenOfIz_VulkanEnumConverter_ConvertQueryType( const DenOfIz_QueryType queryType )
{
    switch ( queryType )
    {
    case DENOFIZ_QUERY_TYPE_OCCLUSION:
        return VK_QUERY_TYPE_OCCLUSION;
    case DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS:
        return VK_QUERY_TYPE_PIPELINE_STATISTICS;
    case DENOFIZ_QUERY_TYPE_TIMESTAMP:
        return VK_QUERY_TYPE_TIMESTAMP;
    default:
        return VK_QUERY_TYPE_OCCLUSION;
    }
}

VkBufferUsageFlags DenOfIz_VulkanEnumConverter_ConvertBufferUsageToVk( const DenOfIz_BufferUsageFlags usage )
{
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    if ( usage & DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_COPY_DST_BIT )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_INDEX_BIT )
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_VERTEX_BIT )
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_UNIFORM_BIT )
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_INDIRECT_BIT )
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT )
    {
        flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }
    if ( usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }

    return flags;
}

VkImageUsageFlags DenOfIz_VulkanEnumConverter_ConvertTextureUsageToVk( const DenOfIz_TextureUsageFlags usage, const DenOfIz_Format format )
{
    VkImageUsageFlags flags = 0;

    if ( usage & DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usage & DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( usage & DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT )
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ( usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT )
    {
        if ( DenOfIz_Format_IsDepth( format ) || DenOfIz_Format_IsDepthStencil( format ) )
        {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else
        {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    if ( flags == 0 )
    {
        spdlog::warn( "No suitable usage specified for texture." );
    }

    return flags;
}

VkImageAspectFlags DenOfIz_VulkanEnumConverter_GetImageAspectFromFormat( const DenOfIz_Format format )
{
    if ( DenOfIz_Format_IsDepthStencil( format ) )
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    if ( DenOfIz_Format_IsDepth( format ) )
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}
