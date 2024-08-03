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

#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <unordered_map>
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Backends/Common/ShaderCompiler.h"
#include "VulkanInclude.h"
#include "VulkanDescriptorPoolManager.h"

namespace DenOfIz
{
    struct QueueFamily
    {
        uint32_t                Index;
        VkQueueFamilyProperties Properties;
    };

    struct VulkanContext
    {
        bool           IsDeviceLost = false;
        PhysicalDevice SelectedDeviceInfo;

        VkInstance       Instance;
        VkPhysicalDevice PhysicalDevice;
        VkDevice         LogicalDevice;
        VmaAllocator     Vma;

        VkCommandPool TransferQueueCommandPool;
        VkCommandPool GraphicsQueueCommandPool;
        VkCommandPool ComputeQueueCommandPool;

        GraphicsWindowHandle                        *Window;
        std::unique_ptr<VulkanDescriptorPoolManager> DescriptorPoolManager;
        std::unordered_map<QueueType, QueueFamily>   QueueFamilies;
        std::unordered_map<QueueType, VkQueue>       Queues;
    };

} // namespace DenOfIz
