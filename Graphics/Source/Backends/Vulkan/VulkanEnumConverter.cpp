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
    case LoadOp::DontCare:
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
    case StoreOp::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_STORE_OP_STORE;
}

VkBlendOp VulkanEnumConverter::ConvertBlendOp( const BlendOp &op )
{
    switch ( op )
    {
    case BlendOp::Add:
        return VK_BLEND_OP_ADD;
    case BlendOp::Subtract:
        return VK_BLEND_OP_SUBTRACT;
    case BlendOp::ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOp::Min:
        return VK_BLEND_OP_MIN;
    case BlendOp::Max:
        return VK_BLEND_OP_MAX;
    }

    return VK_BLEND_OP_ADD;
}

VkLogicOp VulkanEnumConverter::ConvertLogicOp( const LogicOp &op )
{
    switch ( op )
    {
    case LogicOp::Clear:
        return VK_LOGIC_OP_CLEAR;
    case LogicOp::And:
        return VK_LOGIC_OP_AND;
    case LogicOp::AndReverse:
        return VK_LOGIC_OP_AND_REVERSE;
    case LogicOp::Copy:
        return VK_LOGIC_OP_COPY;
    case LogicOp::AndInverted:
        return VK_LOGIC_OP_AND_INVERTED;
    case LogicOp::Noop:
        return VK_LOGIC_OP_NO_OP;
    case LogicOp::Xor:
        return VK_LOGIC_OP_XOR;
    case LogicOp::Or:
        return VK_LOGIC_OP_OR;
    case LogicOp::Nor:
        return VK_LOGIC_OP_NOR;
    case LogicOp::Equiv:
        return VK_LOGIC_OP_EQUIVALENT;
    case LogicOp::Invert:
        return VK_LOGIC_OP_INVERT;
    case LogicOp::OrReverse:
        return VK_LOGIC_OP_OR_REVERSE;
    case LogicOp::CopyInverted:
        return VK_LOGIC_OP_COPY_INVERTED;
    case LogicOp::OrInverted:
        return VK_LOGIC_OP_OR_INVERTED;
    case LogicOp::Nand:
        return VK_LOGIC_OP_NAND;
    case LogicOp::Set:
        return VK_LOGIC_OP_SET;
    }

    return VK_LOGIC_OP_CLEAR;
}

VkBlendFactor VulkanEnumConverter::ConvertBlend( const Blend &blend )
{
    switch ( blend )
    {
    case Blend::Zero:
        return VK_BLEND_FACTOR_ZERO;
    case Blend::One:
        return VK_BLEND_FACTOR_ONE;
    case Blend::SrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case Blend::DstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case Blend::SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case Blend::DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case Blend::SrcAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case Blend::Src1Color:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case Blend::Src1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case Blend::InvSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case Blend::InvSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case Blend::InvDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case Blend::InvDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case Blend::InvSrc1Color:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case Blend::InvSrc1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }

    return VK_BLEND_FACTOR_ZERO;
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
VkBufferUsageFlags VulkanEnumConverter::ConvertBufferUsage( BitSet<ResourceDescriptor> descriptor, BitSet<ResourceUsage> usages )
{
    VkBufferUsageFlags flags = { };
    if ( usages.IsSet( ResourceUsage::CopySrc ) )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usages.IsSet( ResourceUsage::CopyDst ) )
    {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ( descriptor.IsSet( ResourceDescriptor::IndexBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if ( descriptor.IsSet( ResourceDescriptor::VertexBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if ( descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ( descriptor.Any( { ResourceDescriptor::Buffer, ResourceDescriptor::RWBuffer } ) )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if ( descriptor.IsSet( ResourceDescriptor::IndirectBuffer ) )
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if ( usages.IsSet( ResourceUsage::ShaderBindingTable ) )
    {
        flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }
    if ( usages.IsSet( ResourceUsage::AccelerationStructureGeometry ) )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if ( descriptor.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }
    if ( usages.IsSet( ResourceUsage::AccelerationStructureWrite ) )
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if ( usages.IsSet( ResourceUsage::AccelerationStructureRead ) )
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

VkImageUsageFlags VulkanEnumConverter::ConvertTextureDescriptorToUsage( BitSet<ResourceDescriptor> descriptor, BitSet<ResourceUsage> initialState )
{
    VkImageUsageFlags usage = { };

    if ( initialState.IsSet( ResourceUsage::CopySrc ) )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( initialState.IsSet( ResourceUsage::CopyDst ) )
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( initialState.IsSet( ResourceUsage::RenderTarget ) )
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( initialState.Any( { ResourceUsage::DepthRead, ResourceUsage::DepthWrite } ) )
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
    if ( descriptor.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    }
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

VkImageUsageFlags VulkanEnumConverter::ConvertTextureUsage( BitSet<ResourceDescriptor> descriptor, BitSet<ResourceUsage> usage )
{
    VkImageUsageFlags flags = 0;
    if ( descriptor.IsSet( ResourceDescriptor::Texture ) || usage.IsSet( ResourceUsage::ShaderResource ) )
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ( descriptor.Any( { ResourceDescriptor::RWBuffer, ResourceDescriptor::RWTexture, ResourceDescriptor::AccelerationStructure } ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( descriptor.IsSet( ResourceDescriptor::RenderTarget ) )
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( usage.IsSet( ResourceUsage::CopySrc ) )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ( usage.IsSet( ResourceUsage::CopyDst ) )
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ( usage.IsSet( ResourceUsage::AccelerationStructureWrite ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( usage.IsSet( ResourceUsage::AccelerationStructureRead ) )
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ( usage.IsSet( ResourceUsage::RenderTarget ) )
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ( usage.IsSet( ResourceUsage::DepthRead ) || usage.IsSet( ResourceUsage::DepthWrite ) )
    {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if ( usage.IsSet( ResourceUsage::UnorderedAccess ) )
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
VkImageLayout VulkanEnumConverter::ConvertTextureDescriptorToLayout( BitSet<ResourceUsage> initialState )
{
    if ( initialState.Any( { ResourceUsage::ShaderResource, ResourceUsage::PixelShaderResource } ) )
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if ( initialState.Any( { ResourceUsage::Common, ResourceUsage::UnorderedAccess } ) )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if ( initialState.IsSet( ResourceUsage::RenderTarget ) )
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if ( initialState.All( { ResourceUsage::DepthRead, ResourceUsage::DepthWrite } ) )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if ( initialState.IsSet( ResourceUsage::CopySrc ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if ( initialState.IsSet( ResourceUsage::CopyDst ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkBuildAccelerationStructureFlagsKHR VulkanEnumConverter::ConvertAccelerationStructureBuildFlags( BitSet<ASBuildFlags> buildFlags )
{
    VkBuildAccelerationStructureFlagsKHR flags = 0;
    if ( buildFlags.IsSet( ASBuildFlags::AllowUpdate ) )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    }
    if ( buildFlags.IsSet( ASBuildFlags::AllowCompaction ) )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    }
    if ( buildFlags.IsSet( ASBuildFlags::PreferFastTrace ) )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    }
    if ( buildFlags.IsSet( ASBuildFlags::PreferFastBuild ) )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    }
    if ( buildFlags.IsSet( ASBuildFlags::LowMemory ) )
    {
        flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
    }
    return flags;
}
