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
#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "../Common/ShaderCompiler.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "vma/vk_mem_alloc.h"

namespace DenOfIz
{

    struct QueueFamily
    {
        uint32_t                Index;
        VkQueueFamilyProperties Properties;
    };

    struct VulkanContext
    {
        PhysicalDevice SelectedDeviceInfo;

        vk::Instance       Instance;
        vk::PhysicalDevice GPU;
        vk::Device         LogicalDevice;
        VmaAllocator       Vma;
        Format             SurfaceImageFormat;
        vk::ColorSpaceKHR  ColorSpace;
        vk::PresentModeKHR PresentMode;

        vk::SurfaceKHR             Surface;
        vk::SwapchainKHR           SwapChain;
        std::vector<vk::Image>     SwapChainImages;
        std::vector<vk::ImageView> SwapChainImageViews;
        vk::Image                  DepthImage;

        vk::CommandPool    TransferQueueCommandPool;
        vk::CommandPool    GraphicsQueueCommandPool;
        vk::CommandPool    ComputeQueueCommandPool;
        vk::DescriptorPool DescriptorPool;

        vk::Extent2D SurfaceExtent{};

        GraphicsWindowHandle                      *Window;
        std::unordered_map<QueueType, QueueFamily> QueueFamilies;
        std::unordered_map<QueueType, vk::Queue>   Queues;

        // Todo move to generic RenderContext when created.
        ShaderCompiler ShaderCompiler;
        bool           IsDeviceLost = false;
    };

} // namespace DenOfIz

#define VK_CHECK_RESULT(R) assert((R) == vk::Result::eSuccess)

#endif
