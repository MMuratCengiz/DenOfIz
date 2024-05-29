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
#include <DenOfIzGraphics/Backends/Interface/IRenderPass.h>

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
		case ShaderStage::TessellationControl:
			return vk::ShaderStageFlagBits::eTessellationControl;
		case ShaderStage::TessellationEvaluation:
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

	static vk::BufferUsageFlagBits ConvertBufferUsage(BufferMemoryUsage usage)
	{
		switch (usage)
		{
		case BufferMemoryUsage::TransferSrc:
			return vk::BufferUsageFlagBits::eTransferSrc;
		case BufferMemoryUsage::TransferDst:
			return vk::BufferUsageFlagBits::eTransferDst;
		case BufferMemoryUsage::UniformTexelBuffer:
			return vk::BufferUsageFlagBits::eUniformTexelBuffer;
		case BufferMemoryUsage::StorageTexelBuffer:
			return vk::BufferUsageFlagBits::eStorageTexelBuffer;
		case BufferMemoryUsage::UniformBuffer:
			return vk::BufferUsageFlagBits::eUniformBuffer;
		case BufferMemoryUsage::StorageBuffer:
			return vk::BufferUsageFlagBits::eStorageBuffer;
		case BufferMemoryUsage::IndexBuffer:
			return vk::BufferUsageFlagBits::eIndexBuffer;
		case BufferMemoryUsage::VertexBuffer:
			return vk::BufferUsageFlagBits::eVertexBuffer;
		case BufferMemoryUsage::IndirectBuffer:
			return vk::BufferUsageFlagBits::eIndirectBuffer;
		case BufferMemoryUsage::ShaderDeviceAddress:
			return vk::BufferUsageFlagBits::eShaderDeviceAddress;
		case BufferMemoryUsage::TransformFeedbackBufferEXT:
			return vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;
		case BufferMemoryUsage::TransformFeedbackCounterBufferEXT:
			return vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT;
		case BufferMemoryUsage::ConditionalRenderingEXT:
			return vk::BufferUsageFlagBits::eConditionalRenderingEXT;
		case BufferMemoryUsage::RayTracingNV:
			return vk::BufferUsageFlagBits::eRayTracingNV;
		case BufferMemoryUsage::ShaderDeviceAddressEXT:
			return vk::BufferUsageFlagBits::eShaderDeviceAddressEXT;
		case BufferMemoryUsage::SamplerDescriptorBufferEXT:
			return vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT;
		case BufferMemoryUsage::ResourceDescriptorBufferEXT:
			return vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT;
		case BufferMemoryUsage::PushDescriptorsDescriptorBufferEXT:
			return vk::BufferUsageFlagBits::ePushDescriptorsDescriptorBufferEXT;
		case BufferMemoryUsage::MicromapBuildInputReadOnlyEXT:
			return vk::BufferUsageFlagBits::eMicromapBuildInputReadOnlyEXT;
		case BufferMemoryUsage::MicromapStorageEXT:
			return vk::BufferUsageFlagBits::eMicromapStorageEXT;
		case BufferMemoryUsage::VideoDecodeSrc:
			return vk::BufferUsageFlagBits::eVideoDecodeSrcKHR;
		case BufferMemoryUsage::VideoDecodeDst:
			return vk::BufferUsageFlagBits::eVideoDecodeDstKHR;
		case BufferMemoryUsage::AccelerationStructureBuildInputReadOnly:
			return vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
		case BufferMemoryUsage::AccelerationStructureStorage:
			return vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
		case BufferMemoryUsage::ShaderBindingTable:
			return vk::BufferUsageFlagBits::eShaderBindingTableKHR;
		case BufferMemoryUsage::VideoEncodeDst:
			return vk::BufferUsageFlagBits::eVideoEncodeDstKHR;
		case BufferMemoryUsage::VideoEncodeSrc:
			return vk::BufferUsageFlagBits::eVideoEncodeSrcKHR;
		}

		return vk::BufferUsageFlagBits::eVertexBuffer;
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
	static VmaMemoryUsage ConvertMemoryLocation(MemoryLocation location)
	{
		switch (location)
		{
		case MemoryLocation::Auto:
			return VMA_MEMORY_USAGE_AUTO;
		case MemoryLocation::GPU:
			return VMA_MEMORY_USAGE_GPU_ONLY;
		case MemoryLocation::CPU:
			return VMA_MEMORY_USAGE_CPU_COPY;
		case MemoryLocation::CPU_GPU:
			return VMA_MEMORY_USAGE_CPU_TO_GPU;
		case MemoryLocation::GPU_CPU:
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
		case ImageFormat::R4G4UnormPack8:
			return vk::Format::eR4G4UnormPack8;
		case ImageFormat::R4G4B4A4UnormPack16:
			return vk::Format::eR4G4B4A4UnormPack16;
		case ImageFormat::B4G4R4A4UnormPack16:
			return vk::Format::eB4G4R4A4UnormPack16;
		case ImageFormat::R5G6B5UnormPack16:
			return vk::Format::eR5G6B5UnormPack16;
		case ImageFormat::B5G6R5UnormPack16:
			return vk::Format::eB5G6R5UnormPack16;
		case ImageFormat::R5G5B5A1UnormPack16:
			return vk::Format::eR5G5B5A1UnormPack16;
		case ImageFormat::B5G5R5A1UnormPack16:
			return vk::Format::eB5G5R5A1UnormPack16;
		case ImageFormat::A1R5G5B5UnormPack16:
			return vk::Format::eA1R5G5B5UnormPack16;
		case ImageFormat::R8Unorm:
			return vk::Format::eR8Unorm;
		case ImageFormat::R8Snorm:
			return vk::Format::eR8Snorm;
		case ImageFormat::R8Uscaled:
			return vk::Format::eR8Uscaled;
		case ImageFormat::R8Sscaled:
			return vk::Format::eR8Sscaled;
		case ImageFormat::R8Uint:
			return vk::Format::eR8Uint;
		case ImageFormat::R8Sint:
			return vk::Format::eR8Sint;
		case ImageFormat::R8Srgb:
			return vk::Format::eR8Srgb;
		case ImageFormat::R8G8Unorm:
			return vk::Format::eR8G8Unorm;
		case ImageFormat::R8G8Snorm:
			return vk::Format::eR8G8Snorm;
		case ImageFormat::R8G8Uscaled:
			return vk::Format::eR8G8Uscaled;
		case ImageFormat::R8G8Sscaled:
			return vk::Format::eR8G8Sscaled;
		case ImageFormat::R8G8Uint:
			return vk::Format::eR8G8Uint;
		case ImageFormat::R8G8Sint:
			return vk::Format::eR8G8Sint;
		case ImageFormat::R8G8Srgb:
			return vk::Format::eR8G8Srgb;
		case ImageFormat::R8G8B8Unorm:
			return vk::Format::eR8G8B8Unorm;
		case ImageFormat::R8G8B8Snorm:
			return vk::Format::eR8G8B8Snorm;
		case ImageFormat::R8G8B8Uscaled:
			return vk::Format::eR8G8B8Uscaled;
		case ImageFormat::R8G8B8Sscaled:
			return vk::Format::eR8G8B8Sscaled;
		case ImageFormat::R8G8B8Uint:
			return vk::Format::eR8G8B8Uint;
		case ImageFormat::R8G8B8Sint:
			return vk::Format::eR8G8B8Sint;
		case ImageFormat::R8G8B8Srgb:
			return vk::Format::eR8G8B8Srgb;
		case ImageFormat::B8G8R8Unorm:
			return vk::Format::eB8G8R8Unorm;
		case ImageFormat::B8G8R8Snorm:
			return vk::Format::eB8G8R8Snorm;
		case ImageFormat::B8G8R8Uscaled:
			return vk::Format::eB8G8R8Uscaled;
		case ImageFormat::B8G8R8Sscaled:
			return vk::Format::eB8G8R8Sscaled;
		case ImageFormat::B8G8R8Uint:
			return vk::Format::eB8G8R8Uint;
		case ImageFormat::B8G8R8Sint:
			return vk::Format::eB8G8R8Sint;
		case ImageFormat::B8G8R8Srgb:
			return vk::Format::eB8G8R8Srgb;
		case ImageFormat::R8G8B8A8Unorm:
			return vk::Format::eR8G8B8A8Unorm;
		case ImageFormat::R8G8B8A8Snorm:
			return vk::Format::eR8G8B8A8Snorm;
		case ImageFormat::R8G8B8A8Uscaled:
			return vk::Format::eR8G8B8A8Uscaled;
		case ImageFormat::R8G8B8A8Sscaled:
			return vk::Format::eR8G8B8A8Sscaled;
		case ImageFormat::R8G8B8A8Uint:
			return vk::Format::eR8G8B8A8Uint;
		case ImageFormat::R8G8B8A8Sint:
			return vk::Format::eR8G8B8A8Sint;
		case ImageFormat::R8G8B8A8Srgb:
			return vk::Format::eR8G8B8A8Srgb;
		case ImageFormat::B8G8R8A8Unorm:
			return vk::Format::eB8G8R8A8Unorm;
		case ImageFormat::B8G8R8A8Snorm:
			return vk::Format::eB8G8R8A8Snorm;
		case ImageFormat::B8G8R8A8Uscaled:
			return vk::Format::eB8G8R8A8Uscaled;
		case ImageFormat::B8G8R8A8Sscaled:
			return vk::Format::eB8G8R8A8Sscaled;
		case ImageFormat::B8G8R8A8Uint:
			return vk::Format::eB8G8R8A8Uint;
		case ImageFormat::B8G8R8A8Sint:
			return vk::Format::eB8G8R8A8Sint;
		case ImageFormat::B8G8R8A8Srgb:
			return vk::Format::eB8G8R8A8Srgb;
		case ImageFormat::A8B8G8R8UnormPack32:
			return vk::Format::eA8B8G8R8UnormPack32;
		case ImageFormat::A8B8G8R8SnormPack32:
			return vk::Format::eA8B8G8R8SnormPack32;
		case ImageFormat::A8B8G8R8UscaledPack32:
			return vk::Format::eA8B8G8R8UscaledPack32;
		case ImageFormat::A8B8G8R8SscaledPack32:
			return vk::Format::eA8B8G8R8SscaledPack32;
		case ImageFormat::A8B8G8R8UintPack32:
			return vk::Format::eA8B8G8R8UintPack32;
		case ImageFormat::A8B8G8R8SintPack32:
			return vk::Format::eA8B8G8R8SintPack32;
		case ImageFormat::A8B8G8R8SrgbPack32:
			return vk::Format::eA8B8G8R8SrgbPack32;
		case ImageFormat::A2R10G10B10UnormPack32:
			return vk::Format::eA2R10G10B10UnormPack32;
		case ImageFormat::A2R10G10B10SnormPack32:
			return vk::Format::eA2R10G10B10SnormPack32;
		case ImageFormat::A2R10G10B10UscaledPack32:
			return vk::Format::eA2R10G10B10UscaledPack32;
		case ImageFormat::A2R10G10B10SscaledPack32:
			return vk::Format::eA2R10G10B10SscaledPack32;
		case ImageFormat::A2R10G10B10UintPack32:
			return vk::Format::eA2R10G10B10UintPack32;
		case ImageFormat::A2R10G10B10SintPack32:
			return vk::Format::eA2R10G10B10SintPack32;
		case ImageFormat::A2B10G10R10UnormPack32:
			return vk::Format::eA2B10G10R10UnormPack32;
		case ImageFormat::A2B10G10R10SnormPack32:
			return vk::Format::eA2B10G10R10SnormPack32;
		case ImageFormat::A2B10G10R10UscaledPack32:
			return vk::Format::eA2B10G10R10UscaledPack32;
		case ImageFormat::A2B10G10R10SscaledPack32:
			return vk::Format::eA2B10G10R10SscaledPack32;
		case ImageFormat::A2B10G10R10UintPack32:
			return vk::Format::eA2B10G10R10UintPack32;
		case ImageFormat::A2B10G10R10SintPack32:
			return vk::Format::eA2B10G10R10SintPack32;
		case ImageFormat::R16Unorm:
			return vk::Format::eR16Unorm;
		case ImageFormat::R16Snorm:
			return vk::Format::eR16Snorm;
		case ImageFormat::R16Uscaled:
			return vk::Format::eR16Uscaled;
		case ImageFormat::R16Sscaled:
			return vk::Format::eR16Sscaled;
		case ImageFormat::R16Uint:
			return vk::Format::eR16Uint;
		case ImageFormat::R16Sint:
			return vk::Format::eR16Sint;
		case ImageFormat::R16Sfloat:
			return vk::Format::eR16Sfloat;
		case ImageFormat::R16G16Unorm:
			return vk::Format::eR16G16Unorm;
		case ImageFormat::R16G16Snorm:
			return vk::Format::eR16G16Snorm;
		case ImageFormat::R16G16Uscaled:
			return vk::Format::eR16G16Uscaled;
		case ImageFormat::R16G16Sscaled:
			return vk::Format::eR16G16Sscaled;
		case ImageFormat::R16G16Uint:
			return vk::Format::eR16G16Uint;
		case ImageFormat::R16G16Sint:
			return vk::Format::eR16G16Sint;
		case ImageFormat::R16G16Sfloat:
			return vk::Format::eR16G16Sfloat;
		case ImageFormat::R16G16B16Unorm:
			return vk::Format::eR16G16B16Unorm;
		case ImageFormat::R16G16B16Snorm:
			return vk::Format::eR16G16B16Snorm;
		case ImageFormat::R16G16B16Uscaled:
			return vk::Format::eR16G16B16Uscaled;
		case ImageFormat::R16G16B16Sscaled:
			return vk::Format::eR16G16B16Sscaled;
		case ImageFormat::R16G16B16Uint:
			return vk::Format::eR16G16B16Uint;
		case ImageFormat::R16G16B16Sint:
			return vk::Format::eR16G16B16Sint;
		case ImageFormat::R16G16B16Sfloat:
			return vk::Format::eR16G16B16Sfloat;
		case ImageFormat::R16G16B16A16Unorm:
			return vk::Format::eR16G16B16A16Unorm;
		case ImageFormat::R16G16B16A16Snorm:
			return vk::Format::eR16G16B16A16Snorm;
		case ImageFormat::R16G16B16A16Uscaled:
			return vk::Format::eR16G16B16A16Uscaled;
		case ImageFormat::R16G16B16A16Sscaled:
			return vk::Format::eR16G16B16A16Sscaled;
		case ImageFormat::R16G16B16A16Uint:
			return vk::Format::eR16G16B16A16Uint;
		case ImageFormat::R16G16B16A16Sint:
			return vk::Format::eR16G16B16A16Sint;
		case ImageFormat::R16G16B16A16Sfloat:
			return vk::Format::eR16G16B16A16Sfloat;
		case ImageFormat::R32Uint:
			return vk::Format::eR32Uint;
		case ImageFormat::R32Sint:
			return vk::Format::eR32Sint;
		case ImageFormat::R32Sfloat:
			return vk::Format::eR32Sfloat;
		case ImageFormat::R32G32Uint:
			return vk::Format::eR32G32Uint;
		case ImageFormat::R32G32Sint:
			return vk::Format::eR32G32Sint;
		case ImageFormat::R32G32Sfloat:
			return vk::Format::eR32G32Sfloat;
		case ImageFormat::R32G32B32Uint:
			return vk::Format::eR32G32B32Uint;
		case ImageFormat::R32G32B32Sint:
			return vk::Format::eR32G32B32Sint;
		case ImageFormat::R32G32B32Sfloat:
			return vk::Format::eR32G32B32Sfloat;
		case ImageFormat::R32G32B32A32Uint:
			return vk::Format::eR32G32B32A32Uint;
		case ImageFormat::R32G32B32A32Sint:
			return vk::Format::eR32G32B32A32Sint;
		case ImageFormat::R32G32B32A32Sfloat:
			return vk::Format::eR32G32B32A32Sfloat;
		case ImageFormat::R64Uint:
			return vk::Format::eR64Uint;
		case ImageFormat::R64Sint:
			return vk::Format::eR64Sint;
		case ImageFormat::R64Sfloat:
			return vk::Format::eR64Sfloat;
		case ImageFormat::R64G64Uint:
			return vk::Format::eR64G64Uint;
		case ImageFormat::R64G64Sint:
			return vk::Format::eR64G64Sint;
		case ImageFormat::R64G64Sfloat:
			return vk::Format::eR64G64Sfloat;
		case ImageFormat::R64G64B64Uint:
			return vk::Format::eR64G64B64Uint;
		case ImageFormat::R64G64B64Sint:
			return vk::Format::eR64G64B64Sint;
		case ImageFormat::R64G64B64Sfloat:
			return vk::Format::eR64G64B64Sfloat;
		case ImageFormat::R64G64B64A64Uint:
			return vk::Format::eR64G64B64A64Uint;
		case ImageFormat::R64G64B64A64Sint:
			return vk::Format::eR64G64B64A64Sint;
		case ImageFormat::R64G64B64A64Sfloat:
			return vk::Format::eR64G64B64A64Sfloat;
		case ImageFormat::B10G11R11UfloatPack32:
			return vk::Format::eB10G11R11UfloatPack32;
		case ImageFormat::E5B9G9R9UfloatPack32:
			return vk::Format::eE5B9G9R9UfloatPack32;
		case ImageFormat::D16Unorm:
			return vk::Format::eD16Unorm;
		case ImageFormat::X8D24UnormPack32:
			return vk::Format::eX8D24UnormPack32;
		case ImageFormat::D32Sfloat:
			return vk::Format::eD32Sfloat;
		case ImageFormat::S8Uint:
			return vk::Format::eS8Uint;
		case ImageFormat::D16UnormS8Uint:
			return vk::Format::eD16UnormS8Uint;
		case ImageFormat::D24UnormS8Uint:
			return vk::Format::eD24UnormS8Uint;
		case ImageFormat::D32SfloatS8Uint:
			return vk::Format::eD32SfloatS8Uint;
		case ImageFormat::Bc1RgbUnormBlock:
			return vk::Format::eBc1RgbUnormBlock;
		case ImageFormat::Bc1RgbSrgbBlock:
			return vk::Format::eBc1RgbSrgbBlock;
		case ImageFormat::Bc1RgbaUnormBlock:
			return vk::Format::eBc1RgbaUnormBlock;
		case ImageFormat::Bc1RgbaSrgbBlock:
			return vk::Format::eBc1RgbaSrgbBlock;
		case ImageFormat::Bc2UnormBlock:
			return vk::Format::eBc2UnormBlock;
		case ImageFormat::Bc2SrgbBlock:
			return vk::Format::eBc2SrgbBlock;
		case ImageFormat::Bc3UnormBlock:
			return vk::Format::eBc3UnormBlock;
		case ImageFormat::Bc3SrgbBlock:
			return vk::Format::eBc3SrgbBlock;
		case ImageFormat::Bc4UnormBlock:
			return vk::Format::eBc4UnormBlock;
		case ImageFormat::Bc4SnormBlock:
			return vk::Format::eBc4SnormBlock;
		case ImageFormat::Bc5UnormBlock:
			return vk::Format::eBc5UnormBlock;
		case ImageFormat::Bc5SnormBlock:
			return vk::Format::eBc5SnormBlock;
		case ImageFormat::Bc6HUfloatBlock:
			return vk::Format::eBc6HUfloatBlock;
		case ImageFormat::Bc6HSfloatBlock:
			return vk::Format::eBc6HSfloatBlock;
		case ImageFormat::Bc7UnormBlock:
			return vk::Format::eBc7UnormBlock;
		case ImageFormat::Bc7SrgbBlock:
			return vk::Format::eBc7SrgbBlock;
		case ImageFormat::Etc2R8G8B8UnormBlock:
			return vk::Format::eEtc2R8G8B8UnormBlock;
		case ImageFormat::Etc2R8G8B8SrgbBlock:
			return vk::Format::eEtc2R8G8B8SrgbBlock;
		case ImageFormat::Etc2R8G8B8A1UnormBlock:
			return vk::Format::eEtc2R8G8B8A1UnormBlock;
		case ImageFormat::Etc2R8G8B8A1SrgbBlock:
			return vk::Format::eEtc2R8G8B8A1SrgbBlock;
		case ImageFormat::Etc2R8G8B8A8UnormBlock:
			return vk::Format::eEtc2R8G8B8A8UnormBlock;
		case ImageFormat::Etc2R8G8B8A8SrgbBlock:
			return vk::Format::eEtc2R8G8B8A8SrgbBlock;
		case ImageFormat::EacR11UnormBlock:
			return vk::Format::eEacR11UnormBlock;
		case ImageFormat::EacR11SnormBlock:
			return vk::Format::eEacR11SnormBlock;
		case ImageFormat::EacR11G11UnormBlock:
			return vk::Format::eEacR11G11UnormBlock;
		case ImageFormat::EacR11G11SnormBlock:
			return vk::Format::eEacR11G11SnormBlock;
		case ImageFormat::Astc4x4UnormBlock:
			return vk::Format::eAstc4x4UnormBlock;
		case ImageFormat::Astc4x4SrgbBlock:
			return vk::Format::eAstc4x4SrgbBlock;
		case ImageFormat::Astc5x4UnormBlock:
			return vk::Format::eAstc5x4UnormBlock;
		case ImageFormat::Astc5x4SrgbBlock:
			return vk::Format::eAstc5x4SrgbBlock;
		case ImageFormat::Astc5x5UnormBlock:
			return vk::Format::eAstc5x5UnormBlock;
		case ImageFormat::Astc5x5SrgbBlock:
			return vk::Format::eAstc5x5SrgbBlock;
		case ImageFormat::Astc6x5UnormBlock:
			return vk::Format::eAstc6x5UnormBlock;
		case ImageFormat::Astc6x5SrgbBlock:
			return vk::Format::eAstc6x5SrgbBlock;
		case ImageFormat::Astc6x6UnormBlock:
			return vk::Format::eAstc6x6UnormBlock;
		case ImageFormat::Astc6x6SrgbBlock:
			return vk::Format::eAstc6x6SrgbBlock;
		case ImageFormat::Astc8x5UnormBlock:
			return vk::Format::eAstc8x5UnormBlock;
		case ImageFormat::Astc8x5SrgbBlock:
			return vk::Format::eAstc8x5SrgbBlock;
		case ImageFormat::Astc8x6UnormBlock:
			return vk::Format::eAstc8x6UnormBlock;
		case ImageFormat::Astc8x6SrgbBlock:
			return vk::Format::eAstc8x6SrgbBlock;
		case ImageFormat::Astc8x8UnormBlock:
			return vk::Format::eAstc8x8UnormBlock;
		case ImageFormat::Astc8x8SrgbBlock:
			return vk::Format::eAstc8x8SrgbBlock;
		case ImageFormat::Astc10x5UnormBlock:
			return vk::Format::eAstc10x5UnormBlock;
		case ImageFormat::Astc10x5SrgbBlock:
			return vk::Format::eAstc10x5SrgbBlock;
		case ImageFormat::Astc10x6UnormBlock:
			return vk::Format::eAstc10x6UnormBlock;
		case ImageFormat::Astc10x6SrgbBlock:
			return vk::Format::eAstc10x6SrgbBlock;
		case ImageFormat::Astc10x8UnormBlock:
			return vk::Format::eAstc10x8UnormBlock;
		case ImageFormat::Astc10x8SrgbBlock:
			return vk::Format::eAstc10x8SrgbBlock;
		case ImageFormat::Astc10x10UnormBlock:
			return vk::Format::eAstc10x10UnormBlock;
		case ImageFormat::Astc10x10SrgbBlock:
			return vk::Format::eAstc10x10SrgbBlock;
		case ImageFormat::Astc12x10UnormBlock:
			return vk::Format::eAstc12x10UnormBlock;
		case ImageFormat::Astc12x10SrgbBlock:
			return vk::Format::eAstc12x10SrgbBlock;
		case ImageFormat::Astc12x12UnormBlock:
			return vk::Format::eAstc12x12UnormBlock;
		case ImageFormat::Astc12x12SrgbBlock:
			return vk::Format::eAstc12x12SrgbBlock;
		case ImageFormat::G8B8G8R8422Unorm:
			return vk::Format::eG8B8G8R8422Unorm;
		case ImageFormat::B8G8R8G8422Unorm:
			return vk::Format::eB8G8R8G8422Unorm;
		case ImageFormat::G8B8R83Plane420Unorm:
			return vk::Format::eG8B8R83Plane420Unorm;
		case ImageFormat::G8B8R82Plane420Unorm:
			return vk::Format::eG8B8R82Plane420Unorm;
		case ImageFormat::G8B8R83Plane422Unorm:
			return vk::Format::eG8B8R83Plane422Unorm;
		case ImageFormat::G8B8R82Plane422Unorm:
			return vk::Format::eG8B8R82Plane422Unorm;
		case ImageFormat::G8B8R83Plane444Unorm:
			return vk::Format::eG8B8R83Plane444Unorm;
		case ImageFormat::R10X6UnormPack16:
			return vk::Format::eR10X6UnormPack16;
		case ImageFormat::R10X6G10X6Unorm2Pack16:
			return vk::Format::eR10X6G10X6Unorm2Pack16;
		case ImageFormat::R10X6G10X6B10X6A10X6Unorm4Pack16:
			return vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16;
		case ImageFormat::G10X6B10X6G10X6R10X6422Unorm4Pack16:
			return vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16;
		case ImageFormat::B10X6G10X6R10X6G10X6422Unorm4Pack16:
			return vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16;
		case ImageFormat::G10X6B10X6R10X63Plane420Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16;
		case ImageFormat::G10X6B10X6R10X62Plane420Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16;
		case ImageFormat::G10X6B10X6R10X63Plane422Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16;
		case ImageFormat::G10X6B10X6R10X62Plane422Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16;
		case ImageFormat::G10X6B10X6R10X63Plane444Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16;
		case ImageFormat::R12X4UnormPack16:
			return vk::Format::eR12X4UnormPack16;
		case ImageFormat::R12X4G12X4Unorm2Pack16:
			return vk::Format::eR12X4G12X4Unorm2Pack16;
		case ImageFormat::R12X4G12X4B12X4A12X4Unorm4Pack16:
			return vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16;
		case ImageFormat::G12X4B12X4G12X4R12X4422Unorm4Pack16:
			return vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16;
		case ImageFormat::B12X4G12X4R12X4G12X4422Unorm4Pack16:
			return vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16;
		case ImageFormat::G12X4B12X4R12X43Plane420Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16;
		case ImageFormat::G12X4B12X4R12X42Plane420Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16;
		case ImageFormat::G12X4B12X4R12X43Plane422Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16;
		case ImageFormat::G12X4B12X4R12X42Plane422Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16;
		case ImageFormat::G12X4B12X4R12X43Plane444Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16;
		case ImageFormat::G16B16G16R16422Unorm:
			return vk::Format::eG16B16G16R16422Unorm;
		case ImageFormat::B16G16R16G16422Unorm:
			return vk::Format::eB16G16R16G16422Unorm;
		case ImageFormat::G16B16R163Plane420Unorm:
			return vk::Format::eG16B16R163Plane420Unorm;
		case ImageFormat::G16B16R162Plane420Unorm:
			return vk::Format::eG16B16R162Plane420Unorm;
		case ImageFormat::G16B16R163Plane422Unorm:
			return vk::Format::eG16B16R163Plane422Unorm;
		case ImageFormat::G16B16R162Plane422Unorm:
			return vk::Format::eG16B16R162Plane422Unorm;
		case ImageFormat::G16B16R163Plane444Unorm:
			return vk::Format::eG16B16R163Plane444Unorm;
		case ImageFormat::G8B8R82Plane444Unorm:
			return vk::Format::eG8B8R82Plane444Unorm;
		case ImageFormat::G10X6B10X6R10X62Plane444Unorm3Pack16:
			return vk::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16;
		case ImageFormat::G12X4B12X4R12X42Plane444Unorm3Pack16:
			return vk::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16;
		case ImageFormat::G16B16R162Plane444Unorm:
			return vk::Format::eG16B16R162Plane444Unorm;
		case ImageFormat::A4R4G4B4UnormPack16:
			return vk::Format::eA4R4G4B4UnormPack16;
		case ImageFormat::A4B4G4R4UnormPack16:
			return vk::Format::eA4B4G4R4UnormPack16;

		}

		return vk::Format::eUndefined;
	}

	static vk::DescriptorType ConvertBindingTypeToDescriptorType(const ResourceBindingType& type)
	{
		switch (type)
		{
		case ResourceBindingType::Sampler:
			return vk::DescriptorType::eSampler;
		case ResourceBindingType::CombinedImageSampler:
			return vk::DescriptorType::eCombinedImageSampler;
		case ResourceBindingType::SampledImage:
			return vk::DescriptorType::eSampledImage;
		case ResourceBindingType::StorageImage:
			return vk::DescriptorType::eStorageImage;
		case ResourceBindingType::UniformTexelBuffer:
			return vk::DescriptorType::eUniformTexelBuffer;
		case ResourceBindingType::StorageTexelBuffer:
			return vk::DescriptorType::eStorageTexelBuffer;
		case ResourceBindingType::UniformBuffer:
			return vk::DescriptorType::eUniformBuffer;
		case ResourceBindingType::StorageBuffer:
			return vk::DescriptorType::eStorageBuffer;
		case ResourceBindingType::UniformBufferDynamic:
			return vk::DescriptorType::eUniformBufferDynamic;
		case ResourceBindingType::StorageBufferDynamic:
			return vk::DescriptorType::eStorageBufferDynamic;
		case ResourceBindingType::InputAttachment:
			return vk::DescriptorType::eInputAttachment;
		case ResourceBindingType::InlineUniformBlock:
			return vk::DescriptorType::eInlineUniformBlock;
		case ResourceBindingType::AccelerationStructure:
			return vk::DescriptorType::eAccelerationStructureKHR;
		}

		return vk::DescriptorType::eSampler;
	}

	static vk::ImageLayout GetImageVkLayout(const RenderTargetType& targetType)
	{
		switch (targetType)
		{
		case RenderTargetType::Color:
			return vk::ImageLayout::eColorAttachmentOptimal;
		case RenderTargetType::Depth:
			return vk::ImageLayout::eDepthAttachmentOptimal;
		case RenderTargetType::Stencil:
			return vk::ImageLayout::eStencilAttachmentOptimal;
		case RenderTargetType::DepthAndStencil:
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;
		}

		return vk::ImageLayout::eColorAttachmentOptimal;
	}

	static vk::ImageUsageFlags GetVkUsageFlags(const RenderTargetType& targetType)
	{
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eColorAttachment;

		if (targetType == RenderTargetType::Depth || targetType == RenderTargetType::DepthAndStencil)
		{
			usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		}

		if (targetType == RenderTargetType::Color || targetType == RenderTargetType::Depth)
		{
			usageFlags |= vk::ImageUsageFlagBits::eSampled;
		}

		return usageFlags;
	}

	static vk::ImageAspectFlags GetOutputImageVkAspect(const RenderTargetType& renderTargetType)
	{
		vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;

		if (renderTargetType == RenderTargetType::Depth)
		{
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		}
		else if (renderTargetType == RenderTargetType::Stencil)
		{
			aspectFlags = vk::ImageAspectFlagBits::eStencil;
		}
		else if (renderTargetType == RenderTargetType::DepthAndStencil)
		{
			aspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		}

		return aspectFlags;
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
