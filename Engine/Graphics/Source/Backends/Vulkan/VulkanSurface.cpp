/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanSurface.h>
#include <utility>

using namespace DenOfIz;

VulkanSurface::VulkanSurface(VulkanContext* context)
		:
		m_context(context)
{
	CreateSurface();
}

void VulkanSurface::CreateSurface() const
{
	const vk::SurfaceCapabilitiesKHR capabilities = m_context->PhysicalDevice.getSurfaceCapabilitiesKHR(m_context->Surface);

	CreateSwapChain(capabilities);
}

void VulkanSurface::CreateSwapChain(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) const
{
	ChooseExtent2D(surfaceCapabilities);

	vk::SwapchainCreateInfoKHR createInfo{};

	const uint32_t imageCount = std::min(surfaceCapabilities.maxImageCount, surfaceCapabilities.minImageCount + 1);

	createInfo.surface = m_context->Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = VulkanEnumConverter::ConvertImageFormat(m_context->SurfaceImageFormat);
	createInfo.imageColorSpace = m_context->ColorSpace;
	createInfo.imageExtent = m_context->SurfaceExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	const uint32_t qfIndexes[2] = { m_context->QueueFamilies.at(QueueType::Graphics).Index, m_context->QueueFamilies.at(QueueType::Presentation).Index };

	if (qfIndexes[0] != qfIndexes[1])
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = qfIndexes;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = m_context->PresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = m_context->SwapChain;

	m_context->SwapChain = m_context->LogicalDevice.createSwapchainKHR(createInfo);
	CreateSwapChainImages(VulkanEnumConverter::ConvertImageFormat(m_context->SurfaceImageFormat));
}

void VulkanSurface::CreateSwapChainImages(const vk::Format format) const
{
	m_context->SwapChainImages = m_context->LogicalDevice.getSwapchainImagesKHR(m_context->SwapChain);
	m_context->SwapChainImageViews.resize(m_context->SwapChainImages.size());

	int index = 0;
	for (auto image : m_context->SwapChainImages)
	{
		CreateImageView(m_context->SwapChainImageViews[index++], image, format, vk::ImageAspectFlagBits::eColor);
	}
}

void VulkanSurface::ChooseExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		m_context->SurfaceExtent.width = capabilities.currentExtent.width;
		m_context->SurfaceExtent.height = capabilities.currentExtent.height;
		return;
	}

	const GraphicsWindowSurface surface = m_context->Window->GetSurface();
	m_context->SurfaceExtent.width = std::clamp(static_cast<uint32_t>(surface.Width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	m_context->SurfaceExtent.height = std::clamp(static_cast<uint32_t>(surface.Height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
}

void VulkanSurface::CreateImageView(vk::ImageView& imageView, const vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspectFlags) const
{
	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	imageView = m_context->LogicalDevice.createImageView(imageViewCreateInfo);
}

VulkanSurface::~VulkanSurface()
{
	Dispose();
	m_context->LogicalDevice.destroySwapchainKHR(m_context->SwapChain);
}

void VulkanSurface::Dispose() const
{
	for (const auto& imageView : m_context->SwapChainImageViews)
	{
		m_context->LogicalDevice.destroyImageView(imageView);
	}
}
