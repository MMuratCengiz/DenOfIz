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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanEnumConverter
    {
    public:
        static vk::ShaderStageFlagBits ConvertShaderStage(const ShaderStage &shaderStage)
        {
            switch ( shaderStage )
            {
            case ShaderStage::Vertex:
                return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::Hull:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::Domain:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::Geometry:
                return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::Pixel:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::Compute:
                return vk::ShaderStageFlagBits::eCompute;
            case ShaderStage::AllGraphics:
                return vk::ShaderStageFlagBits::eAllGraphics;
            case ShaderStage::All:
                return vk::ShaderStageFlagBits::eAll;
            case ShaderStage::Raygen:
                return vk::ShaderStageFlagBits::eRaygenKHR;
            case ShaderStage::AnyHit:
                return vk::ShaderStageFlagBits::eAnyHitKHR;
            case ShaderStage::ClosestHit:
                return vk::ShaderStageFlagBits::eClosestHitKHR;
            case ShaderStage::Miss:
                return vk::ShaderStageFlagBits::eMissKHR;
            case ShaderStage::Intersection:
                return vk::ShaderStageFlagBits::eIntersectionKHR;
            case ShaderStage::Callable:
                return vk::ShaderStageFlagBits::eCallableKHR;
            case ShaderStage::Task:
                return vk::ShaderStageFlagBits::eTaskEXT;
            case ShaderStage::Mesh:
                return vk::ShaderStageFlagBits::eMeshEXT;
            }

            return vk::ShaderStageFlagBits::eVertex;
        }

        static vk::SampleCountFlagBits ConvertSampleCount(const MSAASampleCount &sampleCount)
        {
            switch ( sampleCount )
            {
            case MSAASampleCount::_0:
            case MSAASampleCount::_1:
                return vk::SampleCountFlagBits::e1;
            case MSAASampleCount::_2:
                return vk::SampleCountFlagBits::e2;
            case MSAASampleCount::_4:
                return vk::SampleCountFlagBits::e4;
            case MSAASampleCount::_8:
                return vk::SampleCountFlagBits::e8;
            case MSAASampleCount::_16:
                return vk::SampleCountFlagBits::e16;
            case MSAASampleCount::_32:
                return vk::SampleCountFlagBits::e32;
            case MSAASampleCount::_64:
                return vk::SampleCountFlagBits::e64;
            }

            return vk::SampleCountFlagBits::e1;
        }

        static vk::StencilOp ConvertStencilOp(const StencilOp &stencilOp)
        {
            switch ( stencilOp )
            {
            case StencilOp::Keep:
                return vk::StencilOp::eKeep;
            case StencilOp::Zero:
                return vk::StencilOp::eZero;
            case StencilOp::Replace:
                return vk::StencilOp::eReplace;
            case StencilOp::IncrementAndClamp:
                return vk::StencilOp::eIncrementAndClamp;
            case StencilOp::DecrementAndClamp:
                return vk::StencilOp::eDecrementAndClamp;
            case StencilOp::Invert:
                return vk::StencilOp::eInvert;
            case StencilOp::IncrementAndWrap:
                return vk::StencilOp::eIncrementAndWrap;
            case StencilOp::DecrementAndWrap:
                return vk::StencilOp::eDecrementAndWrap;
            }

            return vk::StencilOp::eZero;
        }

        static vk::CompareOp ConvertCompareOp(const CompareOp &compareOp)
        {
            switch ( compareOp )
            {
            case CompareOp::Never:
                return vk::CompareOp::eNever;
            case CompareOp::Always:
                return vk::CompareOp::eAlways;
            case CompareOp::Equal:
                return vk::CompareOp::eEqual;
            case CompareOp::NotEqual:
                return vk::CompareOp::eNotEqual;
            case CompareOp::Less:
                return vk::CompareOp::eLess;
            case CompareOp::LessOrEqual:
                return vk::CompareOp::eLessOrEqual;
            case CompareOp::Greater:
                return vk::CompareOp::eGreater;
            case CompareOp::GreaterOrEqual:
                return vk::CompareOp::eGreaterOrEqual;
            }

            return vk::CompareOp::eAlways;
        }

        static vk::AttachmentLoadOp ConvertLoadOp(const LoadOp &loadOp)
        {
            switch ( loadOp )
            {
            case LoadOp::Load:
                return vk::AttachmentLoadOp::eLoad;
            case LoadOp::Clear:
                return vk::AttachmentLoadOp::eClear;
            case LoadOp::Unidentified:
                return vk::AttachmentLoadOp::eDontCare;
            }

            return vk::AttachmentLoadOp::eLoad;
        }

        static vk::AttachmentStoreOp ConvertStoreOp(const StoreOp &storeOp)
        {
            switch ( storeOp )
            {
            case StoreOp::Store:
                return vk::AttachmentStoreOp::eStore;
            case StoreOp::None:
                return vk::AttachmentStoreOp::eNone;
            case StoreOp::Unidentified:
                return vk::AttachmentStoreOp::eDontCare;
            }

            return vk::AttachmentStoreOp::eStore;
        }

        static vk::Filter ConvertFilter(const Filter &filter)
        {
            switch ( filter )
            {
            case Filter::Nearest:
                return vk::Filter::eNearest;
            case Filter::Linear:
                return vk::Filter::eLinear;
            }

            return vk::Filter::eLinear;
        }

        static vk::SamplerAddressMode ConvertAddressMode(const SamplerAddressMode &addressMode)
        {
            switch ( addressMode )
            {
            case SamplerAddressMode::Repeat:
                return vk::SamplerAddressMode::eRepeat;
            case SamplerAddressMode::Mirror:
                return vk::SamplerAddressMode::eMirroredRepeat;
            case SamplerAddressMode::ClampToEdge:
                return vk::SamplerAddressMode::eClampToEdge;
            case SamplerAddressMode::ClampToBorder:
                return vk::SamplerAddressMode::eClampToBorder;
            }
            return vk::SamplerAddressMode::eRepeat;
        }

        static vk::SamplerMipmapMode ConvertMipmapMode(const MipmapMode &mipmapMode)
        {
            switch ( mipmapMode )
            {
            case MipmapMode::Nearest:
                return vk::SamplerMipmapMode::eNearest;
            case MipmapMode::Linear:
                return vk::SamplerMipmapMode::eLinear;
            }

            return vk::SamplerMipmapMode::eLinear;
        }

        // !IMPROVEMENT! This might be incorrect
        static vk::BufferUsageFlags ConvertBufferUsage(BitSet<ResourceDescriptor> usage, BitSet<ResourceState> initialState)
        {
            vk::BufferUsageFlags flags = {};
            if ( initialState.IsSet(ResourceState::CopySrc) )
            {
                flags |= vk::BufferUsageFlagBits::eTransferSrc;
            }
            if ( initialState.IsSet(ResourceState::CopyDst) )
            {
                flags |= vk::BufferUsageFlagBits::eTransferDst;
            }
            if ( usage.IsSet(ResourceDescriptor::IndexBuffer) )
            {
                flags |= vk::BufferUsageFlagBits::eIndexBuffer;
            }
            if ( usage.IsSet(ResourceDescriptor::VertexBuffer) )
            {
                flags |= vk::BufferUsageFlagBits::eVertexBuffer;
            }
            if ( usage.IsSet(ResourceDescriptor::UniformBuffer) )
            {
                flags |= vk::BufferUsageFlagBits::eUniformBuffer;
            }
            if ( usage.IsSet(ResourceDescriptor::Buffer) )
            {
                flags |= vk::BufferUsageFlagBits::eStorageBuffer;
            }
            if ( usage.IsSet(ResourceDescriptor::IndirectBuffer) )
            {
                flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
            }
            if ( usage.IsSet(ResourceDescriptor::AccelerationStructure) )
            {
                flags |= vk::BufferUsageFlagBits::eStorageBuffer;
            }

            if ( initialState.IsSet(ResourceState::AccelerationStructureWrite) )
            {
                flags |= vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
            }
            if ( initialState.IsSet(ResourceState::AccelerationStructureRead) )
            {
                flags |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
            }

            return flags;
        }

        static vk::ImageAspectFlagBits ConvertImageAspect(TextureAspect aspect)
        {
            switch ( aspect )
            {
            case TextureAspect::Color:
                return vk::ImageAspectFlagBits::eColor;
            case TextureAspect::Depth:
                return vk::ImageAspectFlagBits::eDepth;
            case TextureAspect::Stencil:
                return vk::ImageAspectFlagBits::eStencil;
            case TextureAspect::Metadata:
                return vk::ImageAspectFlagBits::eMetadata;
            case TextureAspect::Plane0:
                return vk::ImageAspectFlagBits::ePlane0;
            case TextureAspect::Plane1:
                return vk::ImageAspectFlagBits::ePlane1;
            case TextureAspect::Plane2:
                return vk::ImageAspectFlagBits::ePlane2;
            case TextureAspect::None:
                return vk::ImageAspectFlagBits::eNone;
            }

            return vk::ImageAspectFlagBits::eNone;
        }

        static vk::ImageUsageFlags ConvertTextureDescriptorToUsage(BitSet<ResourceDescriptor> descriptor, BitSet<ResourceState> initialState)
        {
            vk::ImageUsageFlags usage = {};
            if ( descriptor.IsSet(ResourceDescriptor::Sampler) )
            {
                usage |= vk::ImageUsageFlagBits::eSampled;
            }
            if ( descriptor.Any({ ResourceDescriptor::RWBuffer, ResourceDescriptor::RWTexture }) )
            {
                usage |= vk::ImageUsageFlagBits::eStorage;
            }
            if ( initialState.IsSet(ResourceState::RenderTarget) )
            {
                usage |= vk::ImageUsageFlagBits::eColorAttachment;
            }
            if ( initialState.Any({ ResourceState::DepthRead, ResourceState::DepthWrite }) )
            {
                usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }
            return usage;
        }

        // Weird naming on Vma or my side, either way location
        static VmaMemoryUsage ConvertHeapType(HeapType location)
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

        static vk::Format ConvertImageFormat(const Format &imageFormat)
        {
            switch ( imageFormat )
            {
            case Format::Undefined:
                return vk::Format::eUndefined;
            case Format::R32G32B32A32Float:
                return vk::Format::eR32G32B32A32Sfloat;
            case Format::R32G32B32A32Uint:
                return vk::Format::eR32G32B32A32Uint;
            case Format::R32G32B32A32Sint:
                return vk::Format::eR32G32B32A32Sint;
            case Format::R32G32B32Float:
                return vk::Format::eR32G32B32Sfloat;
            case Format::R32G32B32Uint:
                return vk::Format::eR32G32B32Uint;
            case Format::R32G32B32Sint:
                return vk::Format::eR32G32B32Sint;
            case Format::R16G16B16A16Float:
                return vk::Format::eR16G16B16A16Sfloat;
            case Format::R16G16B16A16Unorm:
                return vk::Format::eR16G16B16A16Unorm;
            case Format::R16G16B16A16Uint:
                return vk::Format::eR16G16B16A16Uint;
            case Format::R16G16B16A16Snorm:
                return vk::Format::eR16G16B16A16Snorm;
            case Format::R16G16B16A16Sint:
                return vk::Format::eR16G16B16A16Sint;
            case Format::R32G32Float:
                return vk::Format::eR32G32Sfloat;
            case Format::R32G32Uint:
                return vk::Format::eR32G32Uint;
            case Format::R32G32Sint:
                return vk::Format::eR32G32Sint;
            case Format::R10G10B10A2Unorm:
                return vk::Format::eA2R10G10B10UnormPack32;
            case Format::R10G10B10A2Uint:
                return vk::Format::eA2R10G10B10UintPack32;
            case Format::R8G8B8A8Unorm:
                return vk::Format::eR8G8B8A8Unorm;
            case Format::R8G8B8A8UnormSrgb:
                return vk::Format::eR8G8B8A8Srgb;
            case Format::R8G8B8A8Uint:
                return vk::Format::eR8G8B8A8Uint;
            case Format::R8G8B8A8Snorm:
                return vk::Format::eR8G8B8A8Snorm;
            case Format::R8G8B8A8Sint:
                return vk::Format::eR8G8B8A8Sint;
            case Format::R16G16Float:
                return vk::Format::eR16G16Sfloat;
            case Format::R16G16Unorm:
                return vk::Format::eR16G16Unorm;
            case Format::R16G16Uint:
                return vk::Format::eR16G16Uint;
            case Format::R16G16Snorm:
                return vk::Format::eR16G16Snorm;
            case Format::R16G16Sint:
                return vk::Format::eR16G16Sint;
            case Format::D32Float:
                return vk::Format::eD32Sfloat;
            case Format::R32Float:
                return vk::Format::eR32Sfloat;
            case Format::R32Uint:
                return vk::Format::eR32Uint;
            case Format::R32Sint:
                return vk::Format::eR32Sint;
            case Format::D24UnormS8Uint:
                return vk::Format::eD24UnormS8Uint;
            case Format::R8G8Unorm:
                return vk::Format::eR8G8Unorm;
            case Format::R8G8Uint:
                return vk::Format::eR8G8Uint;
            case Format::R8G8Snorm:
                return vk::Format::eR8G8Snorm;
            case Format::R8G8Sint:
                return vk::Format::eR8G8Sint;
            case Format::R16Float:
                return vk::Format::eR16Sfloat;
            case Format::D16Unorm:
                return vk::Format::eD16Unorm;
            case Format::R16Unorm:
                return vk::Format::eR16Unorm;
            case Format::R16Uint:
                return vk::Format::eR16Uint;
            case Format::R16Snorm:
                return vk::Format::eR16Snorm;
            case Format::R16Sint:
                return vk::Format::eR16Sint;
            case Format::R8Unorm:
                return vk::Format::eR8Unorm;
            case Format::R8Uint:
                return vk::Format::eR8Uint;
            case Format::R8Snorm:
                return vk::Format::eR8Snorm;
            case Format::R8Sint:
                return vk::Format::eR8Sint;
            case Format::BC1Unorm:
                return vk::Format::eBc1RgbaUnormBlock;
            case Format::BC1UnormSrgb:
                return vk::Format::eBc1RgbUnormBlock;
            case Format::BC2Unorm:
                return vk::Format::eBc2UnormBlock;
            case Format::BC2UnormSrgb:
                return vk::Format::eBc2SrgbBlock;
            case Format::BC3Unorm:
                return vk::Format::eBc3UnormBlock;
            case Format::BC3UnormSrgb:
                return vk::Format::eBc3SrgbBlock;
            case Format::BC4Unorm:
                return vk::Format::eBc4UnormBlock;
            case Format::BC4Snorm:
                return vk::Format::eBc4SnormBlock;
            case Format::BC5Unorm:
                return vk::Format::eBc5UnormBlock;
            case Format::BC5Snorm:
                return vk::Format::eBc5SnormBlock;
            case Format::B8G8R8A8Unorm:
                return vk::Format::eB8G8R8A8Unorm;
            case Format::BC6HUfloat16:
                return vk::Format::eBc6HUfloatBlock;
            case Format::BC6HSfloat16:
                return vk::Format::eBc6HSfloatBlock;
            case Format::BC7Unorm:
                return vk::Format::eBc7UnormBlock;
            case Format::BC7UnormSrgb:
                return vk::Format::eBc7SrgbBlock;
            case Format::R32G32B32A32Typeless:
                // No Typeless in Vulkan
                return vk::Format::eR32G32B32Sint;
            case Format::R16G16B16A16Typeless:
                return vk::Format::eR16G16B16A16Sint;
            case Format::R32G32Typeless:
                return vk::Format::eR32G32Sint;
            case Format::R10G10B10A2Typeless:
                return vk::Format::eA2R10G10B10UintPack32;
            case Format::R8G8B8A8Typeless:
                return vk::Format::eR8G8B8A8Sint;
            case Format::R16G16Typeless:
                return vk::Format::eR16G16Sint;
            case Format::R32Typeless:
                return vk::Format::eR32Sint;
            case Format::R8G8Typeless:
                return vk::Format::eR8G8Sint;
            case Format::R16Typeless:
                return vk::Format::eR16Sint;
            case Format::R8Typeless:
                return vk::Format::eR8Sint;
            }

            return vk::Format::eUndefined;
        }

        static vk::DescriptorType ConvertResourceDescriptorToDescriptorType(const BitSet<ResourceDescriptor> &descriptor)
        {
            if ( descriptor.IsSet(ResourceDescriptor::Sampler) )
            {
                return vk::DescriptorType::eSampler;
            }
            if ( descriptor.IsSet(ResourceDescriptor::Texture) )
            {
                return vk::DescriptorType::eSampledImage;
            }
            if ( descriptor.IsSet(ResourceDescriptor::RWTexture) )
            {
                return vk::DescriptorType::eStorageImage;
            }
            if ( descriptor.IsSet(ResourceDescriptor::UniformBuffer) )
            {
                return vk::DescriptorType::eUniformBuffer;
            }
            if ( descriptor.Any({ ResourceDescriptor::RWBuffer, ResourceDescriptor::Buffer }) )
            {
                return vk::DescriptorType::eStorageBuffer;
            }
            if ( descriptor.IsSet(ResourceDescriptor::AccelerationStructure) )
            {
                return vk::DescriptorType::eAccelerationStructureKHR;
            }

            return vk::DescriptorType::eStorageImage;
        }

        static vk::PrimitiveTopology ConvertPrimitiveTopology(const PrimitiveTopology &topology)
        {
            switch ( topology )
            {
            case PrimitiveTopology::Point:
                return vk::PrimitiveTopology::ePointList;
            case PrimitiveTopology::Line:
                return vk::PrimitiveTopology::eLineList;
            case PrimitiveTopology::Triangle:
                return vk::PrimitiveTopology::eTriangleList;
            case PrimitiveTopology::Patch:
                return vk::PrimitiveTopology::ePatchList;
            }

            return vk::PrimitiveTopology::eTriangleList;
        }

        static vk::PipelineBindPoint ConvertPipelineBindPoint(const BindPoint &point)
        {
            switch ( point )
            {
            case BindPoint::Graphics:
                return vk::PipelineBindPoint::eGraphics;
            case BindPoint::Compute:
                return vk::PipelineBindPoint::eCompute;
            case BindPoint::RayTracing:
                return vk::PipelineBindPoint::eRayTracingKHR;
            }

            return vk::PipelineBindPoint::eRayTracingKHR;
        }
    };

} // namespace DenOfIz
