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

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanImageResource.h>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h"

using namespace DenOfIz;

VulkanImageResource::VulkanImageResource(VulkanContext* context, ImageCreateInfo createInfo): m_context(context), m_createInfo(createInfo)
{
	vk::ImageCreateInfo imageCreateInfo{};

	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = createInfo.Width == 0 ? context->SurfaceExtent.width : createInfo.Width;
	imageCreateInfo.extent.height = createInfo.Height == 0 ? context->SurfaceExtent.height : createInfo.Height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.format = VulkanEnumConverter::ConvertImageFormat(createInfo.Format);
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.usage = VulkanEnumConverter::ConvertImageUsage(createInfo.ImageUsage);
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = VulkanEnumConverter::ConverSampleCount(createInfo.MSAASampleCount);
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(context->Vma, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage*>(&m_image), &m_allocation, nullptr);

	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.image = m_image;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = VulkanEnumConverter::ConvertImageFormat(createInfo.Format);
	imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.subresourceRange.aspectMask = VulkanEnumConverter::ConvertImageAspect(createInfo.Aspect);
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	m_imageView = context->LogicalDevice.createImageView(imageViewCreateInfo);
}

void VulkanImageResource::Allocate(const void* newImage)
{
	m_allocated = true;

	bool isEmptyImage = newImage == nullptr;
	data = newImage;

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_createInfo.Width, m_createInfo.Height)))) + 1;

	if (isEmptyImage)
	{
		m_mipLevels = 1;
	}

	const vk::Format format = VulkanEnumConverter::ConvertImageFormat(m_createInfo.Format);

	vk::ImageCreateInfo imageCreateInfo{};

	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = m_createInfo.Width;
	imageCreateInfo.extent.height = m_createInfo.Height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = m_mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(m_context->Vma, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage*>(&(m_image)),
			&(m_allocation),
			nullptr);

	vk::ImageViewCreateInfo imageViewCreateInfo{};

	imageViewCreateInfo.image = m_image;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = m_mipLevels;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	m_imageView = m_context->LogicalDevice.createImageView(imageViewCreateInfo);
	if (m_hasSampler)
	{
		vk::SamplerCreateInfo samplerCreateInfo{};

		samplerCreateInfo.magFilter = VulkanEnumConverter::ConvertFilter(m_samplerCreateInfo.MagFilter);
		samplerCreateInfo.minFilter = VulkanEnumConverter::ConvertFilter(m_samplerCreateInfo.MinFilter);
		samplerCreateInfo.addressModeU = VulkanEnumConverter::ConvertAddressMode(m_samplerCreateInfo.AddressModeU);
		samplerCreateInfo.addressModeV = VulkanEnumConverter::ConvertAddressMode(m_samplerCreateInfo.AddressModeV);
		samplerCreateInfo.addressModeW = VulkanEnumConverter::ConvertAddressMode(m_samplerCreateInfo.AddressModeW);
		samplerCreateInfo.anisotropyEnable = m_samplerCreateInfo.AnisotropyEnable;
		samplerCreateInfo.maxAnisotropy = m_samplerCreateInfo.MaxAnisotropy;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = m_samplerCreateInfo.CompareEnable;
		samplerCreateInfo.compareOp = VulkanEnumConverter::ConvertCompareOp(m_samplerCreateInfo.CompareOp);
		samplerCreateInfo.mipmapMode = VulkanEnumConverter::ConvertMipmapMode(m_samplerCreateInfo.MipmapMode);
		samplerCreateInfo.mipLodBias = m_samplerCreateInfo.MipLodBias;
		samplerCreateInfo.minLod = m_samplerCreateInfo.MinLod;
		samplerCreateInfo.maxLod = m_mipLevels;

		m_sampler = m_context->LogicalDevice.createSampler(samplerCreateInfo);
	}

	RETURN_IF(isEmptyImage);

	vk::Buffer stagingBuffer;
	VmaAllocation stagingAllocation;

	VulkanUtilities::InitStagingBuffer(m_context, stagingBuffer, stagingAllocation, newImage, m_createInfo.Width * m_createInfo.Height * 4);
	VulkanUtilities::RunOneTimeCommand(m_context, [&](const vk::CommandBuffer& commandBuffer)
	{
		vk::ImageMemoryBarrier memoryBarrier{};

		memoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		memoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memoryBarrier.image = m_image;
		memoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		memoryBarrier.subresourceRange.baseMipLevel = 0;
		memoryBarrier.subresourceRange.levelCount = m_mipLevels;
		memoryBarrier.subresourceRange.baseArrayLayer = 0;
		memoryBarrier.subresourceRange.layerCount = 1;
		memoryBarrier.srcAccessMask = {};
		memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

		vk::BufferImageCopy bufferImageCopy{};

		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.layerCount = 1;
		bufferImageCopy.imageOffset = vk::Offset3D{ 0, 0, 0 };
		bufferImageCopy.imageExtent = vk::Extent3D{ m_createInfo.Width, m_createInfo.Height, 1 };

		commandBuffer.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);
	});

	vmaDestroyBuffer(m_context->Vma, stagingBuffer, stagingAllocation);

	const vk::FormatProperties properties = m_context->PhysicalDevice.getFormatProperties(format);

	if ((properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear) != vk::FormatFeatureFlagBits::eSampledImageFilterLinear)
	{
		throw std::runtime_error("Unsupported device, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT must be  supported");
	}

	GenerateMipMaps();

	DescriptorInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	DescriptorInfo.imageView = m_imageView;
	DescriptorInfo.sampler = m_sampler;
}

void VulkanImageResource::AttachSampler(SamplerCreateInfo& info)
{
	m_samplerCreateInfo = info;
	m_hasSampler = true;
}

void VulkanImageResource::GenerateMipMaps() const
{
	int32_t mipWidth = m_createInfo.Width, mipHeight = m_createInfo.Height;

	VulkanUtilities::RunOneTimeCommand(m_context, [&](vk::CommandBuffer& commandBuffer)
	{
		/*
		 * Continuously copy current image( in mip level=index ) to the next, ie.
		 * Iteration 1: i - 1 = 0, 512x512 is copied to i as 512/2
		 * Iteration 2: i - 1 = 0, 512/2 is copied to i as 512/2/2
		 *
		 * In such form every mip level of the image is filled with an image with half the size of the previous image
		 */
		for (uint32_t index = 1; index < m_mipLevels; ++index)
		{
			vk::ImageMemoryBarrier memoryBarrier{};

			memoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			memoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarrier.image = m_image;
			memoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			memoryBarrier.subresourceRange.baseMipLevel = index - 1;
			memoryBarrier.subresourceRange.levelCount = 1;
			memoryBarrier.subresourceRange.baseArrayLayer = 0;
			memoryBarrier.subresourceRange.layerCount = 1;
			memoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

			vk::ImageBlit imageBlit{};

			imageBlit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			imageBlit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			imageBlit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
			imageBlit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

			imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageBlit.srcSubresource.mipLevel = index - 1;
			imageBlit.srcSubresource.baseArrayLayer = 0;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageBlit.dstSubresource.mipLevel = index;
			imageBlit.dstSubresource.baseArrayLayer = 0;
			imageBlit.dstSubresource.layerCount = 1;

			commandBuffer.blitImage(m_image, vk::ImageLayout::eTransferSrcOptimal, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &imageBlit,
					vk::Filter::eLinear);

			/* After the blit transition the image to the form that will be used by the shader */
			vk::ImageMemoryBarrier toShaderFormat{};

			toShaderFormat.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			toShaderFormat.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			toShaderFormat.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toShaderFormat.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			toShaderFormat.image = m_image;
			toShaderFormat.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			toShaderFormat.subresourceRange.baseMipLevel = index - 1;
			toShaderFormat.subresourceRange.levelCount = 1;
			toShaderFormat.subresourceRange.baseArrayLayer = 0;
			toShaderFormat.subresourceRange.layerCount = 1;
			toShaderFormat.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			toShaderFormat.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &toShaderFormat);

			if (mipWidth > 1)
			{
				mipWidth /= 2;
			}
			if (mipHeight > 1)
			{
				mipHeight /= 2;
			}
		}

		vk::ImageMemoryBarrier finalFormat{};

		finalFormat.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		finalFormat.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		finalFormat.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalFormat.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalFormat.image = m_image;
		finalFormat.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		finalFormat.subresourceRange.baseMipLevel = m_mipLevels - 1;
		finalFormat.subresourceRange.levelCount = 1;
		finalFormat.subresourceRange.baseArrayLayer = 0;
		finalFormat.subresourceRange.layerCount = 1;
		finalFormat.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		finalFormat.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &finalFormat);
	});
}

void VulkanImageResource::Deallocate()
{
	RETURN_IF(!m_allocated);

	vmaDestroyImage(m_context->Vma, m_image, m_allocation);
	m_context->LogicalDevice.destroyImageView(m_imageView);
	if (m_hasSampler)
	{
		m_context->LogicalDevice.destroySampler(m_sampler);
	}
}

VulkanImageResource::~VulkanImageResource()
{
	Deallocate();
}
