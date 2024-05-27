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

VulkanSwapChain::VulkanSwapChain(VulkanContext* context)
	:m_context(context)
{
	m_swapChainImageAvailable = std::make_unique<VulkanLock>(this->m_context, LockType::Semaphore);
	m_swapChainImageRendered = std::make_unique<VulkanLock>(this->m_context, LockType::Semaphore);

	CreateSurface();
}

void VulkanSwapChain::CreateSurface() const
{
	const vk::SurfaceCapabilitiesKHR capabilities = m_context->PhysicalDevice.getSurfaceCapabilitiesKHR(m_context->Surface);

	CreateSwapChain(capabilities);
}

void VulkanSwapChain::CreateSwapChain(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) const
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

void VulkanSwapChain::CreateSwapChainImages(const vk::Format format) const
{
	m_context->SwapChainImages = m_context->LogicalDevice.getSwapchainImagesKHR(m_context->SwapChain);
	m_context->SwapChainImageViews.resize(m_context->SwapChainImages.size());

	int index = 0;
	for (auto image : m_context->SwapChainImages)
	{
		CreateImageView(m_context->SwapChainImageViews[index++], image, format, vk::ImageAspectFlagBits::eColor);
	}
}

void VulkanSwapChain::ChooseExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		m_context->SurfaceExtent.width = capabilities.currentExtent.width;
		m_context->SurfaceExtent.height = capabilities.currentExtent.height;
		return;
	}

	const SDL_Surface* pSurface = SDL_GetWindowSurface(m_context->Window);
	m_context->SurfaceExtent.width = std::clamp(static_cast<uint32_t>(pSurface->w), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	m_context->SurfaceExtent.height = std::clamp(static_cast<uint32_t>(pSurface->h), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
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
	for (const auto& imageView : m_context->SwapChainImageViews)
	{
		m_context->LogicalDevice.destroyImageView(imageView);
	}
}

uint32_t VulkanSwapChain::AcquireNextImage()
{
	auto image = m_context->LogicalDevice.acquireNextImageKHR(m_context->SwapChain, UINT64_MAX, m_swapChainImageAvailable->GetVkSemaphore(), nullptr);
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

VulkanLock* VulkanSwapChain::GetImageAvailableLock()
{
	return m_swapChainImageAvailable.get();
}

VulkanLock* VulkanSwapChain::GetImageRenderedLock()
{
	return m_swapChainImageRendered.get();
}

VulkanSwapChain::~VulkanSwapChain()
{
	Dispose();
	m_context->LogicalDevice.destroySwapchainKHR(m_context->SwapChain);
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
		return ImageFormat::R8G8B8A8Srgb;
	default:
		return ImageFormat::R8G8B8A8Unorm;
	}
}

void VulkanSwapChain::Resize(uint32_t width, uint32_t height)
{

}
