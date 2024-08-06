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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

VkShaderStageFlagBits VulkanEnumConverter::ConvertShaderStage( const ShaderStage &shaderStage )
{
    switch ( shaderStage )
    {
    case ShaderStage::Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderStage::Hull:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderStage::Domain:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderStage::Geometry:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderStage::Pixel:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderStage::Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    case ShaderStage::AllGraphics:
        return VK_SHADER_STAGE_ALL_GRAPHICS;
    case ShaderStage::All:
        return VK_SHADER_STAGE_ALL;
    case ShaderStage::Raygen:
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case ShaderStage::AnyHit:
        return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    case ShaderStage::ClosestHit:
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    case ShaderStage::Miss:
        return VK_SHADER_STAGE_MISS_BIT_KHR;
    case ShaderStage::Intersection:
        return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    case ShaderStage::Callable:
        return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    case ShaderStage::Task:
        return VK_SHADER_STAGE_TASK_BIT_EXT;
    case ShaderStage::Mesh:
        return VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    return VK_SHADER_STAGE_VERTEX_BIT;
}

VkSampleCountFlagBits VulkanEnumConverter::ConvertSampleCount( const MSAASampleCount &sampleCount )
{
    switch ( sampleCount )
    {
    case MSAASampleCount::_0:
    case MSAASampleCount::_1:
        return VK_SAMPLE_COUNT_1_BIT;
    case MSAASampleCount::_2:
        return VK_SAMPLE_COUNT_2_BIT;
    case MSAASampleCount::_4:
        return VK_SAMPLE_COUNT_4_BIT;
    case MSAASampleCount::_8:
        return VK_SAMPLE_COUNT_8_BIT;
    case MSAASampleCount::_16:
        return VK_SAMPLE_COUNT_16_BIT;
    case MSAASampleCount::_32:
        return VK_SAMPLE_COUNT_32_BIT;
    case MSAASampleCount::_64:
        return VK_SAMPLE_COUNT_64_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkStencilOp VulkanEnumConverter::ConvertStencilOp( const StencilOp &stencilOp )
{
    switch ( stencilOp )
    {
    case StencilOp::Keep:
        return VK_STENCIL_OP_KEEP;
    case StencilOp::Zero:
        return VK_STENCIL_OP_ZERO;
    case StencilOp::Replace:
        return VK_STENCIL_OP_REPLACE;
    case StencilOp::IncrementAndClamp:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::DecrementAndClamp:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::Invert:
        return VK_STENCIL_OP_INVERT;
    case StencilOp::IncrementAndWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::DecrementAndWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }

    return VK_STENCIL_OP_ZERO;
}

VkCompareOp VulkanEnumConverter::ConvertCompareOp( const CompareOp &compareOp )
{
    switch ( compareOp )
    {
    case CompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case CompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    case CompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case CompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case CompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case CompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    }

    return VK_COMPARE_OP_ALWAYS;
}

VkAttachmentLoadOp VulkanEnumConverter::ConvertLoadOp( const LoadOp &loadOp )
{
    switch ( loadOp )
    {
    case LoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case LoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case LoadOp::Unidentified:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

VkAttachmentStoreOp VulkanEnumConverter::ConvertStoreOp( const StoreOp &storeOp )
{
    switch ( storeOp )
    {
    case StoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case StoreOp::None:
        return VK_ATTACHMENT_STORE_OP_NONE;
    case StoreOp::Unidentified:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_STORE_OP_STORE;
}

VkFilter VulkanEnumConverter::ConvertFilter( const Filter &filter )
{
    switch ( filter )
    {
    case Filter::Nearest:
        return VK_FILTER_NEAREST;
    case Filter::Linear:
        return VK_FILTER_LINEAR;
    }

    return VK_FILTER_LINEAR;
}

VkSamplerAddressMode VulkanEnumConverter::ConvertAddressMode( const SamplerAddressMode &addressMode )
{
    switch ( addressMode )
    {
    case SamplerAddressMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case SamplerAddressMode::Mirror:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case SamplerAddressMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case SamplerAddressMode::ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkSamplerMipmapMode VulkanEnumConverter::ConvertMipmapMode( const MipmapMode &mipmapMode )
{
    switch ( mipmapMode )
    {
    case MipmapMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case MipmapMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

// TODO !IMPROVEMENT! This might be incorrect
VkBufferUsageFlags VulkanEnumConverter::ConvertBufferUsage( BitSet<ResourceDescriptor> usage, BitSet<ResourceState> initialState )
{
    VkBufferUsageFlags flags = { };
    if ( initialState.IsSet( ResourceState::CopySrc ) )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ( initialState.IsSet( ResourceState::CopyDst ) )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::IndexBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::VertexBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::Buffer ) )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::IndirectBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if ( usage.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if ( initialState.IsSet( ResourceState::AccelerationStructureWrite ) )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if ( initialState.IsSet( ResourceState::AccelerationStructureRead ) )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    return flags;
}

VkImageAspectFlagBits VulkanEnumConverter::ConvertImageAspect( TextureAspect aspect )
{
    switch ( aspect )
    {
    case TextureAspect::Color:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case TextureAspect::Depth:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case TextureAspect::Stencil:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case TextureAspect::Metadata:
        return VK_IMAGE_ASPECT_METADATA_BIT;
    case TextureAspect::Plane0:
        return VK_IMAGE_ASPECT_PLANE_0_BIT;
    case TextureAspect::Plane1:
        return VK_IMAGE_ASPECT_PLANE_1_BIT;
    case TextureAspect::Plane2:
        return VK_IMAGE_ASPECT_PLANE_2_BIT;
    case TextureAspect::None:
        return VK_IMAGE_ASPECT_NONE;
    }

    return VK_IMAGE_ASPECT_NONE;
}

VkImageUsageFlags VulkanEnumConverter::ConvertTextureDescriptorToUsage( BitSet<ResourceDescriptor> descriptor, BitSet<ResourceState> initialState )
{
    VkImageUsageFlags usage = { };

    if ( initialState.IsSet( ResourceState::CopySrc ) )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( initialState.IsSet( ResourceState::CopyDst ) )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( initialState.IsSet( ResourceState::RenderTarget ) )
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( initialState.Any( { ResourceState::DepthRead, ResourceState::DepthWrite } ) )
    {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return usage;
}

VmaMemoryUsage VulkanEnumConverter::ConvertHeapType( HeapType location )
{
    switch ( location )
    {
    case HeapType::GPU:
        return VMA_MEMORY_USAGE_GPU_ONLY;
    case HeapType::CPU:
        return VMA_MEMORY_USAGE_CPU_COPY;
    case HeapType::CPU_GPU:
        return VMA_MEMORY_USAGE_CPU_TO_GPU;
    case HeapType::GPU_CPU:
        return VMA_MEMORY_USAGE_GPU_TO_CPU;
    }

    return VMA_MEMORY_USAGE_AUTO;
}

VkFormat VulkanEnumConverter::ConvertImageFormat( const Format &imageFormat )
{
    switch ( imageFormat )
    {
    case Format::Undefined:
        return VK_FORMAT_UNDEFINED;
    case Format::R32G32B32A32Float:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::R32G32B32A32Uint:
        return VK_FORMAT_R32G32B32A32_UINT;
    case Format::R32G32B32A32Sint:
        return VK_FORMAT_R32G32B32A32_SINT;
    case Format::R32G32B32Float:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Format::R32G32B32Uint:
        return VK_FORMAT_R32G32B32_UINT;
    case Format::R32G32B32Sint:
        return VK_FORMAT_R32G32B32_SINT;
    case Format::R16G16B16A16Float:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Format::R16G16B16A16Unorm:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case Format::R16G16B16A16Uint:
        return VK_FORMAT_R16G16B16A16_UINT;
    case Format::R16G16B16A16Snorm:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case Format::R16G16B16A16Sint:
        return VK_FORMAT_R16G16B16A16_SINT;
    case Format::R32G32Float:
        return VK_FORMAT_R32G32_SFLOAT;
    case Format::R32G32Uint:
        return VK_FORMAT_R32G32_UINT;
    case Format::R32G32Sint:
        return VK_FORMAT_R32G32_SINT;
    case Format::R10G10B10A2Unorm:
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case Format::R10G10B10A2Uint:
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case Format::R8G8B8A8Unorm:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::R8G8B8A8UnormSrgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::R8G8B8A8Uint:
        return VK_FORMAT_R8G8B8A8_UINT;
    case Format::R8G8B8A8Snorm:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case Format::R8G8B8A8Sint:
        return VK_FORMAT_R8G8B8A8_SINT;
    case Format::R16G16Float:
        return VK_FORMAT_R16G16_SFLOAT;
    case Format::R16G16Unorm:
        return VK_FORMAT_R16G16_UNORM;
    case Format::R16G16Uint:
        return VK_FORMAT_R16G16_UINT;
    case Format::R16G16Snorm:
        return VK_FORMAT_R16G16_SNORM;
    case Format::R16G16Sint:
        return VK_FORMAT_R16G16_SINT;
    case Format::D32Float:
        return VK_FORMAT_D32_SFLOAT;
    case Format::R32Float:
        return VK_FORMAT_R32_SFLOAT;
    case Format::R32Uint:
        return VK_FORMAT_R32_UINT;
    case Format::R32Sint:
        return VK_FORMAT_R32_SINT;
    case Format::D24UnormS8Uint:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case Format::R8G8Unorm:
        return VK_FORMAT_R8G8_UNORM;
    case Format::R8G8Uint:
        return VK_FORMAT_R8G8_UINT;
    case Format::R8G8Snorm:
        return VK_FORMAT_R8G8_SNORM;
    case Format::R8G8Sint:
        return VK_FORMAT_R8G8_SINT;
    case Format::R16Float:
        return VK_FORMAT_R16_SFLOAT;
    case Format::D16Unorm:
        return VK_FORMAT_D16_UNORM;
    case Format::R16Unorm:
        return VK_FORMAT_R16_UNORM;
    case Format::R16Uint:
        return VK_FORMAT_R16_UINT;
    case Format::R16Snorm:
        return VK_FORMAT_R16_SNORM;
    case Format::R16Sint:
        return VK_FORMAT_R16_SINT;
    case Format::R8Unorm:
        return VK_FORMAT_R8_UNORM;
    case Format::R8Uint:
        return VK_FORMAT_R8_UINT;
    case Format::R8Snorm:
        return VK_FORMAT_R8_SNORM;
    case Format::R8Sint:
        return VK_FORMAT_R8_SINT;
    case Format::BC1Unorm:
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case Format::BC1UnormSrgb:
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case Format::BC2Unorm:
        return VK_FORMAT_BC2_UNORM_BLOCK;
    case Format::BC2UnormSrgb:
        return VK_FORMAT_BC2_SRGB_BLOCK;
    case Format::BC3Unorm:
        return VK_FORMAT_BC3_UNORM_BLOCK;
    case Format::BC3UnormSrgb:
        return VK_FORMAT_BC3_SRGB_BLOCK;
    case Format::BC4Unorm:
        return VK_FORMAT_BC4_UNORM_BLOCK;
    case Format::BC4Snorm:
        return VK_FORMAT_BC4_SNORM_BLOCK;
    case Format::BC5Unorm:
        return VK_FORMAT_BC5_UNORM_BLOCK;
    case Format::BC5Snorm:
        return VK_FORMAT_BC5_SNORM_BLOCK;
    case Format::B8G8R8A8Unorm:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case Format::BC6HUfloat16:
        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case Format::BC6HSfloat16:
        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case Format::BC7Unorm:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    case Format::BC7UnormSrgb:
        return VK_FORMAT_BC7_SRGB_BLOCK;
    case Format::R32G32B32A32Typeless:
        // No Typeless in Vulkan
        return VK_FORMAT_R32G32B32_SINT;
    case Format::R16G16B16A16Typeless:
        return VK_FORMAT_R16G16B16A16_SINT;
    case Format::R32G32Typeless:
        return VK_FORMAT_R32G32_SINT;
    case Format::R10G10B10A2Typeless:
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case Format::R8G8B8A8Typeless:
        return VK_FORMAT_R8G8B8A8_SINT;
    case Format::R16G16Typeless:
        return VK_FORMAT_R16G16_SINT;
    case Format::R32Typeless:
        return VK_FORMAT_R32_SINT;
    case Format::R8G8Typeless:
        return VK_FORMAT_R8G8_SINT;
    case Format::R16Typeless:
        return VK_FORMAT_R16_SINT;
    case Format::R8Typeless:
        return VK_FORMAT_R8_SINT;
    }

    return VK_FORMAT_UNDEFINED;
}

VkDescriptorType VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( const BitSet<ResourceDescriptor> &descriptor )
{
    if ( descriptor.IsSet( ResourceDescriptor::Sampler ) )
    {
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    if ( descriptor.IsSet( ResourceDescriptor::Texture ) )
    {
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    if ( descriptor.IsSet( ResourceDescriptor::RWTexture ) )
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    if ( descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
    if ( descriptor.Any( { ResourceDescriptor::RWBuffer, ResourceDescriptor::Buffer } ) )
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    if ( descriptor.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    }

    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

VkPrimitiveTopology VulkanEnumConverter::ConvertPrimitiveTopology( const PrimitiveTopology &topology )
{
    switch ( topology )
    {
    case PrimitiveTopology::Point:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveTopology::Line:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveTopology::Triangle:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveTopology::Patch:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }

    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkPipelineBindPoint VulkanEnumConverter::ConvertPipelineBindPoint( const BindPoint &point )
{
    switch ( point )
    {
    case BindPoint::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case BindPoint::Compute:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    case BindPoint::RayTracing:
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    }

    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

VkImageUsageFlags VulkanEnumConverter::ConvertTextureUsage( BitSet<ResourceDescriptor> descriptor, BitSet<ResourceState> initialState )
{
    VkImageUsageFlags flags = 0;
    if ( descriptor.IsSet( ResourceDescriptor::Texture ) )
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ( descriptor.Any( { ResourceDescriptor::RWBuffer, ResourceDescriptor::RWTexture, ResourceDescriptor::AccelerationStructure } ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( initialState.IsSet( ResourceState::CopySrc ) )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( initialState.IsSet( ResourceState::CopyDst ) )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( initialState.IsSet( ResourceState::AccelerationStructureWrite ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( initialState.IsSet( ResourceState::AccelerationStructureRead ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( initialState.IsSet( ResourceState::RenderTarget ) )
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( initialState.IsSet( ResourceState::DepthRead ) || initialState.IsSet( ResourceState::DepthWrite ) )
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if ( initialState.IsSet( ResourceState::ShaderResource ) )
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ( initialState.IsSet( ResourceState::UnorderedAccess ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( flags == 0 )
    {
        LOG( WARNING ) << "No suitable descriptor specified for texture.";
    }
    return flags;
}

// TODO !IMPROVEMENT! This needs to be more complete
VkImageLayout VulkanEnumConverter::ConvertTextureDescriptorToLayout( BitSet<ResourceState> initialState )
{
    if ( initialState.IsSet( ResourceState::Common ) )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if ( initialState.IsSet( ResourceState::RenderTarget ) )
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if ( initialState.All( { ResourceState::DepthRead, ResourceState::DepthWrite } ) )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if ( initialState.Any( { ResourceState::ShaderResource, ResourceState::PixelShaderResource } ) )
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if ( initialState.IsSet( ResourceState::CopySrc ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if ( initialState.IsSet( ResourceState::CopyDst ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}
