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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h>

using namespace DenOfIz;

VulkanSwapChain::VulkanSwapChain(VulkanContext* context, const SwapChainCreateInfo& createInfo)
		:m_context(context), m_createInfo(createInfo)
{
	CreateSwapChain();
}

void VulkanSwapChain::CreateSwapChain()
{
 	const vk::SurfaceCapabilitiesKHR capabilities = m_context->PhysicalDevice.getSurfaceCapabilitiesKHR(m_context->Surface);

	ChooseExtent2D(capabilities);

	vk::SwapchainCreateInfoKHR createInfo{};

	const uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

	createInfo.surface = m_context->Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = VulkanEnumConverter::ConvertImageFormat(m_context->SurfaceImageFormat);
	createInfo.imageColorSpace = m_context->ColorSpace;
	createInfo.imageExtent = vk::Extent2D(m_width, m_height);
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

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = m_context->PresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = m_swapChain;

	m_swapChain = m_context->LogicalDevice.createSwapchainKHR(createInfo);
	CreateSwapChainImages(VulkanEnumConverter::ConvertImageFormat(m_context->SurfaceImageFormat));
}

void VulkanSwapChain::CreateSwapChainImages(const vk::Format format)
{
	m_swapChainImages = m_context->LogicalDevice.getSwapchainImagesKHR(m_swapChain);
	m_swapChainImageViews.resize(m_swapChainImages.size());

	int index = 0;
	for (auto image : m_swapChainImages)
	{
		CreateImageView(m_swapChainImageViews[index], image, format, vk::ImageAspectFlagBits::eColor);
		m_renderTargets.push_back(std::make_unique<VulkanImageResource>(image, m_swapChainImageViews[index], format, vk::ImageAspectFlagBits::eColor));
		index++;
	}
}

void VulkanSwapChain::ChooseExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (m_createInfo.Width != 0 || m_createInfo.Height != 0)
	{
		m_width = m_createInfo.Width;
		m_height = m_createInfo.Height;
		return;
	}

	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		m_width = capabilities.currentExtent.width;
		m_height = capabilities.currentExtent.height;
		return;
	}

	const GraphicsWindowSurface surface = m_context->Window->GetSurface();
	m_width = std::clamp(static_cast<uint32_t>(surface.Width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	m_height = std::clamp(static_cast<uint32_t>(surface.Height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
}

void VulkanSwapChain::CreateImageView(vk::ImageView& imageView, const vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspectFlags) const
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

void VulkanSwapChain::Dispose() const
{
	for (const auto& imageView : m_swapChainImageViews)
	{
		m_context->LogicalDevice.destroyImageView(imageView);
	}
}

uint32_t VulkanSwapChain::AcquireNextImage(ISemaphore* imageReadySemaphore)
{
	VulkanSemaphore* semaphore = dynamic_cast<VulkanSemaphore*>(imageReadySemaphore);
	auto image = m_context->LogicalDevice.acquireNextImageKHR(m_swapChain, UINT64_MAX, semaphore->GetSemaphore(), nullptr);
	if (image.result == vk::Result::eErrorOutOfDateKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	if (image.result != vk::Result::eSuccess && image.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	return image.value;
}

VulkanSwapChain::~VulkanSwapChain()
{
	Dispose();
	m_context->LogicalDevice.destroySwapchainKHR(m_swapChain);
}

ImageFormat VulkanSwapChain::GetPreferredFormat()
{
	// Todo missing cases
	switch (m_context->PhysicalDevice.getSurfaceFormatsKHR(m_context->Surface)[0].format)
	{
	case vk::Format::eB8G8R8A8Unorm:
		return ImageFormat::B8G8R8A8Unorm;
	case vk::Format::eR8G8B8A8Unorm:
		return ImageFormat::R8G8B8A8Unorm;
	case vk::Format::eR8G8B8A8Srgb:
		return ImageFormat::R8G8B8A8UnormSrgb;
	default:
		return ImageFormat::R8G8B8A8Unorm;
	}
}

void VulkanSwapChain::Resize(uint32_t width, uint32_t height)
{

}
