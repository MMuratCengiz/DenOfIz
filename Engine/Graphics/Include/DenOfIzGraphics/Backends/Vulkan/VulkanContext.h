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
#ifdef BUILD_VK

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzCore/Common.h>
#include <unordered_map>
#include "vk_mem_alloc.h"
#include "../Common/ShaderCompiler.h"

namespace DenOfIz
{

struct VulkanDeviceInfo
{
	vk::PhysicalDevice Device;
	vk::PhysicalDeviceProperties Properties;
	vk::PhysicalDeviceFeatures Features;

	std::vector<vk::ExtensionProperties> ExtensionProperties;
	std::vector<vk::QueueFamilyProperties> QueueFamilies;
};

struct QueueFamily
{
	uint32_t Index;
	VkQueueFamilyProperties Properties;
};

struct VulkanContext
{
	vk::Instance Instance;
	vk::PhysicalDevice PhysicalDevice;
	vk::Device LogicalDevice;
	VmaAllocator Vma;
	ImageFormat SurfaceImageFormat;
	vk::ColorSpaceKHR ColorSpace;
	vk::PresentModeKHR PresentMode;

	vk::SurfaceKHR Surface;
	vk::SwapchainKHR SwapChain;
	std::vector<vk::Image> SwapChainImages;
	std::vector<vk::ImageView> SwapChainImageViews;
	vk::Image DepthImage;

	vk::CommandPool TransferQueueCommandPool;
	vk::CommandPool GraphicsQueueCommandPool;
	vk::CommandPool ComputeQueueCommandPool;
	vk::DescriptorPool DescriptorPool;

	vk::Extent2D SurfaceExtent{};

	SDL_Window* Window;
	std::unordered_map<QueueType, QueueFamily> QueueFamilies;
	std::unordered_map<QueueType, vk::Queue> Queues;

	//Todo move to generic RenderContext when created.
	ShaderCompiler ShaderCompiler;
	PhysicalDeviceInfo SelectedDeviceInfo;
	bool IsDeviceLost = false;
};

}

#define VK_CHECK_RESULT(R) assert((R) == vk::Result::eSuccess)

#endif
