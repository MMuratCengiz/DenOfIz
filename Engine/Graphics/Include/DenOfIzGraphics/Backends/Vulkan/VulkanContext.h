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

#pragma once

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp>

#include <DenOfIzCore/Common.h>
#include <unordered_map>
#include "vk_mem_alloc.h"
#include "../Interface/IDevice.h"
#include "../Common/ShaderCompiler.h"

namespace DenOfIz
{

struct VulkanDeviceInfo
{
	vk::PhysicalDevice device;
	vk::PhysicalDeviceProperties properties;
	vk::PhysicalDeviceFeatures features;

	std::vector<vk::ExtensionProperties> extensionProperties;
	std::vector<vk::QueueFamilyProperties> queueFamilies;
};

struct QueueFamily
{
	uint32_t index;
	VkQueueFamilyProperties properties;
};

enum class QueueType
{
	Graphics,
	Presentation,
	Transfer,
};

struct VulkanContext
{
public:
	vk::Instance Instance;
	vk::PhysicalDevice PhysicalDevice;
	vk::Device LogicalDevice;
	VmaAllocator Vma;
	ImageFormat SurfaceImageFormat;
	vk::ColorSpaceKHR ColorSpace;
	vk::PresentModeKHR PresentMode;
	vk::SwapchainKHR SwapChain;
	vk::SurfaceKHR RenderSurface;
	vk::SurfaceKHR Surface;
	std::vector<vk::Image> SwapChainImages;
	std::vector<vk::ImageView> SwapChainImageViews;
	vk::Image depthImage;

	vk::CommandPool TransferQueueCommandPool;
	vk::CommandPool GraphicsQueueCommandPool;
	vk::CommandPool ComputeQueueCommandPool;

	vk::Extent2D SurfaceExtent{};

	SDL_Window* Window;
	std::unordered_map<QueueType, QueueFamily> QueueFamilies;
	std::unordered_map<QueueType, vk::Queue> Queues;

	//Todo move to generic RenderContext when created.
	ShaderCompiler ShaderCompiler;
};

}