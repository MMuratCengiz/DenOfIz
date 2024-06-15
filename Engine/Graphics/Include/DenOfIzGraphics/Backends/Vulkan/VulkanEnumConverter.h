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
#ifdef BUILD_VK

#include "VulkanContext.h"
#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

namespace DenOfIz
{

class VulkanEnumConverter
{
public:
	static vk::ShaderStageFlagBits ConvertShaderStage(const ShaderStage& shaderStage)
	{
		switch (shaderStage)
		{
		case ShaderStage::Vertex:
			return vk::ShaderStageFlagBits::eVertex;
		case ShaderStage::Hull:
			return vk::ShaderStageFlagBits::eTessellationControl;
		case ShaderStage::Domain:
			return vk::ShaderStageFlagBits::eTessellationEvaluation;
		case ShaderStage::Geometry:
			return vk::ShaderStageFlagBits::eGeometry;
		case ShaderStage::Fragment:
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

	static vk::SampleCountFlagBits ConvertSampleCount(const MSAASampleCount& sampleCount)
	{
		switch (sampleCount)
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

	static vk::ImageLayout ConvertImageLayout(const ImageLayout& imageLayout)
	{
		switch (imageLayout)
		{
		case Undefined:
			return vk::ImageLayout::eUndefined;
		case General:
			return vk::ImageLayout::eGeneral;
		case ColorAttachmentOptimal:
			return vk::ImageLayout::eColorAttachmentOptimal;
		case DepthStencilAttachmentOptimal:
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;
		case DepthStencilReadOnlyOptimal:
			return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
		case ShaderReadOnlyOptimal:
			return vk::ImageLayout::eShaderReadOnlyOptimal;
		case TransferSrcOptimal:
			return vk::ImageLayout::eTransferSrcOptimal;
		case TransferDstOptimal:
			return vk::ImageLayout::eTransferDstOptimal;
		case PreInitialized:
			return vk::ImageLayout::ePreinitialized;
		case DepthReadOnlyStencilAttachmentOptimal:
			return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
		case DepthAttachmentStencilReadOnlyOptimal:
			return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
		case DepthAttachmentOptimal:
			return vk::ImageLayout::eDepthAttachmentOptimal;
		case DepthReadOnlyOptimal:
			return vk::ImageLayout::eDepthReadOnlyOptimal;
		case StencilAttachmentOptimal:
			return vk::ImageLayout::eStencilAttachmentOptimal;
		case StencilReadOnlyOptimal:
			return vk::ImageLayout::eStencilReadOnlyOptimal;
		case ReadOnlyOptimal:
			return vk::ImageLayout::eReadOnlyOptimal;
		case AttachmentOptimal:
			return vk::ImageLayout::eAttachmentOptimal;
		case PresentSrc:
			return vk::ImageLayout::ePresentSrcKHR;
		case VideoDecodeDst:
			return vk::ImageLayout::eVideoDecodeDstKHR;
		case VideoDecodeSrc:
			return vk::ImageLayout::eVideoDecodeSrcKHR;
		case VideoDecodeDpb:
			return vk::ImageLayout::eVideoDecodeDpbKHR;
		case SharedPresent:
			return vk::ImageLayout::eSharedPresentKHR;
		case FragmentShadingRateAttachmentOptimal:
			return vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;
		case RenderingLocalRead:
			return vk::ImageLayout::eRenderingLocalReadKHR;
		case VideoEncodeDst:
			return vk::ImageLayout::eVideoEncodeDstKHR;
		case VideoEncodeSrc:
			return vk::ImageLayout::eVideoEncodeSrcKHR;
		case VideoEncodeDpb:
			return vk::ImageLayout::eVideoEncodeDpbKHR;
		}

		return vk::ImageLayout::eUndefined;
	}

	static vk::StencilOp ConvertStencilOp(const StencilOp& stencilOp)
	{
		switch (stencilOp)
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

	static vk::CompareOp ConvertCompareOp(const CompareOp& compareOp)
	{
		switch (compareOp)
		{
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

	static vk::AttachmentLoadOp ConvertLoadOp(const LoadOp& loadOp)
	{
		switch (loadOp)
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

	static vk::AttachmentStoreOp ConvertStoreOp(const StoreOp& storeOp)
	{
		switch (storeOp)
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

	static vk::Filter ConvertFilter(const Filter& filter)
	{
		switch (filter)
		{
		case Filter::Nearest:
			return vk::Filter::eNearest;
		case Filter::Linear:
			return vk::Filter::eLinear;
		case Filter::CubicIMG:
			return vk::Filter::eCubicIMG;
		case Filter::CubicEXT:
			return vk::Filter::eCubicEXT;
		}

		return vk::Filter::eLinear;
	}

	static vk::SamplerAddressMode ConvertAddressMode(const SamplerAddressMode& addressMode)
	{
		switch (addressMode)
		{
		case SamplerAddressMode::Repeat:
			return vk::SamplerAddressMode::eRepeat;
		case SamplerAddressMode::MirroredRepeat:
			return vk::SamplerAddressMode::eMirroredRepeat;
		case SamplerAddressMode::ClampToEdge:
			return vk::SamplerAddressMode::eClampToEdge;
		case SamplerAddressMode::ClampToBorder:
			return vk::SamplerAddressMode::eClampToBorder;
		case SamplerAddressMode::MirrorClampToEdge:
			return vk::SamplerAddressMode::eMirrorClampToEdge;
		}
		return vk::SamplerAddressMode::eClampToBorder;
	}

	static vk::SamplerMipmapMode ConvertMipmapMode(const MipmapMode& mipmapMode)
	{
		switch (mipmapMode)
		{
		case MipmapMode::Nearest:
			return vk::SamplerMipmapMode::eNearest;
		case MipmapMode::Linear:
			return vk::SamplerMipmapMode::eLinear;
		}

		return vk::SamplerMipmapMode::eLinear;
	}

	static vk::BufferUsageFlags ConvertBufferUsage(BufferUsage usage)
	{
		vk::BufferUsageFlags flags = {};
		if (usage.CopySrc)
		{
			flags |= vk::BufferUsageFlagBits::eTransferSrc;
		}
		if (usage.CopyDst)
		{
			flags |= vk::BufferUsageFlagBits::eTransferDst;
		}
		if (usage.IndexBuffer)
		{
			flags |= vk::BufferUsageFlagBits::eIndexBuffer;
		}
		if (usage.VertexBuffer)
		{
			flags |= vk::BufferUsageFlagBits::eVertexBuffer;
		}
		if (usage.UniformBuffer)
		{
			flags |= vk::BufferUsageFlagBits::eUniformBuffer;
		}
		if (usage.Storage)
		{
			flags |= vk::BufferUsageFlagBits::eStorageBuffer;
		}
		if (usage.Indirect)
		{
			flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
		}
		if (usage.AccelerationStructureScratch)
		{
			flags |= vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		}
		if (usage.BottomLevelAccelerationStructureInput || usage.TopLevelAccelerationStructureInput)
		{
			flags |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		}

		return flags;
	}

	static vk::ImageAspectFlagBits ConvertImageAspect(ImageAspect aspect)
	{
		switch (aspect)
		{
		case ImageAspect::Color:
			return vk::ImageAspectFlagBits::eColor;
		case ImageAspect::Depth:
			return vk::ImageAspectFlagBits::eDepth;
		case ImageAspect::Stencil:
			return vk::ImageAspectFlagBits::eStencil;
		case ImageAspect::Metadata:
			return vk::ImageAspectFlagBits::eMetadata;
		case ImageAspect::Plane0:
			return vk::ImageAspectFlagBits::ePlane0;
		case ImageAspect::Plane1:
			return vk::ImageAspectFlagBits::ePlane1;
		case ImageAspect::Plane2:
			return vk::ImageAspectFlagBits::ePlane2;
		case ImageAspect::None:
			return vk::ImageAspectFlagBits::eNone;
		}

		return vk::ImageAspectFlagBits::eNone;
	}

	static vk::ImageUsageFlagBits ConvertImageUsage(ImageMemoryUsage usage)
	{
		switch (usage)
		{
		case ImageMemoryUsage::TransferSrc:
			return vk::ImageUsageFlagBits::eTransferSrc;
		case ImageMemoryUsage::TransferDst:
			return vk::ImageUsageFlagBits::eTransferDst;
		case ImageMemoryUsage::Sampled:
			return vk::ImageUsageFlagBits::eSampled;
		case ImageMemoryUsage::Storage:
			return vk::ImageUsageFlagBits::eStorage;
		case ImageMemoryUsage::ColorAttachment:
			return vk::ImageUsageFlagBits::eColorAttachment;
		case ImageMemoryUsage::DepthStencilAttachment:
			return vk::ImageUsageFlagBits::eDepthStencilAttachment;
		case ImageMemoryUsage::TransientAttachment:
			return vk::ImageUsageFlagBits::eTransientAttachment;
		case ImageMemoryUsage::InputAttachment:
			return vk::ImageUsageFlagBits::eInputAttachment;
		case ImageMemoryUsage::VideoDecodeDst:
			return vk::ImageUsageFlagBits::eVideoDecodeDstKHR;
		case ImageMemoryUsage::VideoDecodeSrc:
			return vk::ImageUsageFlagBits::eVideoDecodeSrcKHR;
		case ImageMemoryUsage::VideoDecodeDpb:
			return vk::ImageUsageFlagBits::eVideoDecodeDpbKHR;
		case ImageMemoryUsage::FragmentDensityMap:
			return vk::ImageUsageFlagBits::eFragmentDensityMapEXT;
		case ImageMemoryUsage::FragmentShadingRateAttachment:
			return vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;
		case ImageMemoryUsage::HostTransferEXT:
			return vk::ImageUsageFlagBits::eHostTransferEXT;
		case ImageMemoryUsage::VideoEncodeDst:
			return vk::ImageUsageFlagBits::eVideoEncodeDstKHR;
		case ImageMemoryUsage::VideoEncodeSrc:
			return vk::ImageUsageFlagBits::eVideoEncodeSrcKHR;
		case ImageMemoryUsage::VideoEncodeDpb:
			return vk::ImageUsageFlagBits::eVideoEncodeDpbKHR;
		case ImageMemoryUsage::AttachmentFeedbackLoop:
			return vk::ImageUsageFlagBits::eAttachmentFeedbackLoopEXT;
		}

		return vk::ImageUsageFlagBits::eColorAttachment;
	}

	// Weird naming on Vma or my side, either way location = usage.
	static VmaMemoryUsage ConvertMemoryLocation(HeapType location)
	{
		switch (location)
		{
		case HeapType::Auto:
			return VMA_MEMORY_USAGE_AUTO;
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

	static vk::Format ConvertImageFormat(const ImageFormat& imageFormat)
	{
		switch (imageFormat)
		{
		case ImageFormat::Undefined:
			return vk::Format::eUndefined;
		case ImageFormat::R32G32B32A32Float:
			return vk::Format::eR32G32B32A32Sfloat;
		case ImageFormat::R32G32B32A32Uint:
			return vk::Format::eR32G32B32A32Uint;
		case ImageFormat::R32G32B32A32Sint:
			return vk::Format::eR32G32B32A32Sint;
		case ImageFormat::R32G32B32Float:
			return vk::Format::eR32G32B32Sfloat;
		case ImageFormat::R32G32B32Uint:
			return vk::Format::eR32G32B32Uint;
		case ImageFormat::R32G32B32Sint:
			return vk::Format::eR32G32B32Sint;
		case ImageFormat::R16G16B16A16Float:
			return vk::Format::eR16G16B16A16Sfloat;
		case ImageFormat::R16G16B16A16Unorm:
			return vk::Format::eR16G16B16A16Unorm;
		case ImageFormat::R16G16B16A16Uint:
			return vk::Format::eR16G16B16A16Uint;
		case ImageFormat::R16G16B16A16Snorm:
			return vk::Format::eR16G16B16A16Snorm;
		case ImageFormat::R16G16B16A16Sint:
			return vk::Format::eR16G16B16A16Sint;
		case ImageFormat::R32G32Float:
			return vk::Format::eR32G32Sfloat;
		case ImageFormat::R32G32Uint:
			return vk::Format::eR32G32Uint;
		case ImageFormat::R32G32Sint:
			return vk::Format::eR32G32Sint;
		case ImageFormat::R10G10B10A2Unorm:
			return vk::Format::eA2R10G10B10UnormPack32;
		case ImageFormat::R10G10B10A2Uint:
			return vk::Format::eA2R10G10B10UintPack32;
		case ImageFormat::R8G8B8A8Unorm:
			return vk::Format::eR8G8B8A8Unorm;
		case ImageFormat::R8G8B8A8UnormSrgb:
			return vk::Format::eR8G8B8A8Srgb;
		case ImageFormat::R8G8B8A8Uint:
			return vk::Format::eR8G8B8A8Uint;
		case ImageFormat::R8G8B8A8Snorm:
			return vk::Format::eR8G8B8A8Snorm;
		case ImageFormat::R8G8B8A8Sint:
			return vk::Format::eR8G8B8A8Sint;
		case ImageFormat::R16G16Float:
			return vk::Format::eR16G16Sfloat;
		case ImageFormat::R16G16Unorm:
			return vk::Format::eR16G16Unorm;
		case ImageFormat::R16G16Uint:
			return vk::Format::eR16G16Uint;
		case ImageFormat::R16G16Snorm:
			return vk::Format::eR16G16Snorm;
		case ImageFormat::R16G16Sint:
			return vk::Format::eR16G16Sint;
		case ImageFormat::D32Float:
			return vk::Format::eD32Sfloat;
		case ImageFormat::R32Float:
			return vk::Format::eR32Sfloat;
		case ImageFormat::R32Uint:
			return vk::Format::eR32Uint;
		case ImageFormat::R32Sint:
			return vk::Format::eR32Sint;
		case ImageFormat::D24UnormS8Uint:
			return vk::Format::eD24UnormS8Uint;
		case ImageFormat::R8G8Unorm:
			return vk::Format::eR8G8Unorm;
		case ImageFormat::R8G8Uint:
			return vk::Format::eR8G8Uint;
		case ImageFormat::R8G8Snorm:
			return vk::Format::eR8G8Snorm;
		case ImageFormat::R8G8Sint:
			return vk::Format::eR8G8Sint;
		case ImageFormat::R16Float:
			return vk::Format::eR16Sfloat;
		case ImageFormat::D16Unorm:
			return vk::Format::eD16Unorm;
		case ImageFormat::R16Unorm:
			return vk::Format::eR16Unorm;
		case ImageFormat::R16Uint:
			return vk::Format::eR16Uint;
		case ImageFormat::R16Snorm:
			return vk::Format::eR16Snorm;
		case ImageFormat::R16Sint:
			return vk::Format::eR16Sint;
		case ImageFormat::R8Unorm:
			return vk::Format::eR8Unorm;
		case ImageFormat::R8Uint:
			return vk::Format::eR8Uint;
		case ImageFormat::R8Snorm:
			return vk::Format::eR8Snorm;
		case ImageFormat::R8Sint:
			return vk::Format::eR8Sint;
		case ImageFormat::BC1Unorm:
			return vk::Format::eBc1RgbaUnormBlock;
		case ImageFormat::BC1UnormSrgb:
			return vk::Format::eBc1RgbUnormBlock;
		case ImageFormat::BC2Unorm:
			return vk::Format::eBc2UnormBlock;
		case ImageFormat::BC2UnormSrgb:
			return vk::Format::eBc2SrgbBlock;
		case ImageFormat::BC3Unorm:
			return vk::Format::eBc3UnormBlock;
		case ImageFormat::BC3UnormSrgb:
			return vk::Format::eBc3SrgbBlock;
		case ImageFormat::BC4Unorm:
			return vk::Format::eBc4UnormBlock;
		case ImageFormat::BC4Snorm:
			return vk::Format::eBc4SnormBlock;
		case ImageFormat::BC5Unorm:
			return vk::Format::eBc5UnormBlock;
		case ImageFormat::BC5Snorm:
			return vk::Format::eBc5SnormBlock;
		case ImageFormat::B8G8R8A8Unorm:
			return vk::Format::eB8G8R8A8Unorm;
		case ImageFormat::BC6HUfloat16:
			return vk::Format::eBc6HUfloatBlock;
		case ImageFormat::BC6HSfloat16:
			return vk::Format::eBc6HSfloatBlock;
		case ImageFormat::BC7Unorm:
			return vk::Format::eBc7UnormBlock;
		case ImageFormat::BC7UnormSrgb:
			return vk::Format::eBc7SrgbBlock;
		}

		return vk::Format::eUndefined;
	}

	static vk::DescriptorType ConvertBindingTypeToDescriptorType(const ResourceBindingType& type)
	{
		switch (type)
		{
		case ResourceBindingType::Sampler:
			return vk::DescriptorType::eSampler;
		case ResourceBindingType::Texture:
		case ResourceBindingType::TextureReadWrite:
			return vk::DescriptorType::eSampledImage;
		case ResourceBindingType::StorageImage:
			return vk::DescriptorType::eStorageImage;
		case ResourceBindingType::Buffer:
		case ResourceBindingType::BufferReadWrite:
			return vk::DescriptorType::eUniformBuffer;
		case ResourceBindingType::Storage:
			return vk::DescriptorType::eStorageBuffer;
		case ResourceBindingType::BufferDynamic:
			return vk::DescriptorType::eUniformBufferDynamic;
		case ResourceBindingType::StorageDynamic:
			return vk::DescriptorType::eStorageBufferDynamic;
		case ResourceBindingType::AccelerationStructure:
			return vk::DescriptorType::eAccelerationStructureKHR;
		}

		return vk::DescriptorType::eSampler;
	}

	static vk::PrimitiveTopology ConvertPrimitiveTopology(const PrimitiveTopology& topology)
	{
		switch (topology)
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

	static vk::PipelineBindPoint ConvertPipelineBindPoint(const BindPoint& point)
	{
		switch (point)
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

}

#endif
