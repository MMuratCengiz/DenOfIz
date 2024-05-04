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

#include <fstream>
#include <utility>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanSurface.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

VulkanSurface::VulkanSurface(VulkanContext* context)
		:context(context)
{
	CreateSurface();
}

void VulkanSurface::CreateSurface()
{
	vk::SurfaceCapabilitiesKHR capabilities;

	capabilities = context->PhysicalDevice.getSurfaceCapabilitiesKHR(context->Surface);

	CreateSwapChain(capabilities);
}

void VulkanSurface::CreateSwapChain(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
{
	ChooseExtent2D(surfaceCapabilities);

	vk::SwapchainCreateInfoKHR createInfo{};

	uint32_t imageCount = std::min(surfaceCapabilities.maxImageCount, surfaceCapabilities.minImageCount + 1);

	createInfo.surface = context->Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = VulkanEnumConverter::ConvertImageFormat(context->SurfaceImageFormat);
	createInfo.imageColorSpace = context->ColorSpace;
	createInfo.imageExtent = context->SurfaceExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	const uint32_t qfIndexes[2] = { context->QueueFamilies.at(QueueType::Graphics).index, context->QueueFamilies.at(QueueType::Presentation).index };

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
	createInfo.presentMode = context->PresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = context->SwapChain;

	context->SwapChain = context->LogicalDevice.createSwapchainKHR(createInfo);
	CreateSwapChainImages(VulkanEnumConverter::ConvertImageFormat(context->SurfaceImageFormat));
}

void VulkanSurface::CreateSwapChainImages(vk::Format format)
{
	context->SwapChainImages = context->LogicalDevice.getSwapchainImagesKHR(context->SwapChain);

	context->SwapChainImageViews.resize(context->SwapChainImages.size());

	int index = 0;
	for (auto image : context->SwapChainImages)
	{
		CreateImageView(context->SwapChainImageViews[index++], image, format, vk::ImageAspectFlagBits::eColor);
	}
}

void VulkanSurface::ChooseExtent2D(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		context->SurfaceExtent.width = capabilities.currentExtent.width;
		context->SurfaceExtent.height = capabilities.currentExtent.height;
		return;
	}

	SDL_Surface* surface = SDL_GetWindowSurface(context->Window);
	context->SurfaceExtent.width = std::clamp((uint32_t)surface->w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	context->SurfaceExtent.height = std::clamp((uint32_t)surface->h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
}

void VulkanSurface::CreateImageView(vk::ImageView& imageView, const vk::Image& image, const vk::Format& format, const vk::ImageAspectFlags& aspectFlags)
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

	imageView = context->LogicalDevice.createImageView(imageViewCreateInfo);
}

VulkanSurface::~VulkanSurface()
{
	Dispose();
	context->LogicalDevice.destroySwapchainKHR(context->SwapChain);
}

void VulkanSurface::Dispose()
{
	for (auto& imageView : context->SwapChainImageViews)
	{
		context->LogicalDevice.destroyImageView(imageView);
	}
}