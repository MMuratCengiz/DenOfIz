/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandList.h>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanDescriptorPoolManager.h"
#include "VulkanInclude.h"

namespace DenOfIz
{
    struct QueueFamily
    {
        uint32_t                Index;
        VkQueueFamilyProperties Properties;
    };

    enum class VulkanQueueType
    {
        Graphics,
        Copy,
        Compute,
        Presentation,
    };

    struct VulkanContext
    {
        bool                   IsDeviceLost = false;
        DenOfIz_PhysicalDevice SelectedDeviceInfo;

        VkInstance       Instance;
        VkPhysicalDevice PhysicalDevice;
        VkDevice         LogicalDevice;
        VmaAllocator     Vma;

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingProperties;

        VkCommandPool TransferQueueCommandPool;
        VkCommandPool GraphicsQueueCommandPool;
        VkCommandPool ComputeQueueCommandPool;

        DenOfIz_GraphicsWindowHandle                     Window;
        std::unique_ptr<VulkanDescriptorPoolManager>     DescriptorPoolManager;
        std::unordered_map<VulkanQueueType, QueueFamily> QueueFamilies;
        std::unordered_map<VulkanQueueType, VkQueue>     Queues;

        InternalAutoSync *AutoSync;

        std::vector<VkQueue> SubmitQueues;
        std::mutex           CompletionFenceMutex;
        std::vector<VkFence> FreeCompletionFences;

        void    RegisterSubmitQueue( VkQueue queue );
        VkFence AcquireCompletionFence( );
        void    ReleaseCompletionFence( VkFence fence );

        bool                              DebugUtilsEnabled             = false;
        PFN_vkCmdBeginDebugUtilsLabelEXT  vkCmdBeginDebugUtilsLabelEXT  = nullptr;
        PFN_vkCmdEndDebugUtilsLabelEXT    vkCmdEndDebugUtilsLabelEXT    = nullptr;
        PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT = nullptr;

        bool CheckDeviceLost( VkResult result, const char *operation );
    };

} // namespace DenOfIz
