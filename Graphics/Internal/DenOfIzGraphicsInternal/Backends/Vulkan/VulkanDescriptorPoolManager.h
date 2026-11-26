/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <mutex>
#include <memory>
#include <vector>
#include "VulkanInclude.h"

namespace DenOfIz
{
    class VulkanDescriptorPoolManager final
    {
        class VulkanDescriptorPool final
        {
            friend class VulkanDescriptorPoolManager;
            VkDevice         m_device;
            VkDescriptorPool m_pool{ };
            uint32_t         m_numSets;
            uint32_t         m_setsAllocated = 0;

        public:
            VulkanDescriptorPool( const VkDevice &device, uint32_t numSets );
            ~VulkanDescriptorPool( );
        };

        VkDevice                                           m_device;
        std::mutex                                         m_mutex;
        std::vector<std::unique_ptr<VulkanDescriptorPool>> m_pools;
        std::unique_ptr<VulkanDescriptorPool>              m_currentPool;
        uint32_t                                           m_maxSets = 65536;

    public:
        explicit VulkanDescriptorPoolManager( const VkDevice &device, const VkPhysicalDevice &physicalDevice );
        ~VulkanDescriptorPoolManager( ) = default;

        // Ignores allocateInfo.descriptorPool. Returns the pool the sets came from, which FreeDescriptorSet needs, VK_NULL_HANDLE on failure.
        VkDescriptorPool AllocateDescriptorSets( const VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet *sets );
        // A pool that is no longer allocated from is destroyed when its last set is freed.
        void FreeDescriptorSet( VkDescriptorPool pool, VkDescriptorSet set );
    };
} // namespace DenOfIz
