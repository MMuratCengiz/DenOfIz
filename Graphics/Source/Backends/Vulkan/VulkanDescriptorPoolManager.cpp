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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanDescriptorPoolManager.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanDescriptorPoolManager::VulkanDescriptorPool::VulkanDescriptorPool( const VkDevice &device, const uint32_t numSets ) : m_device( device ), m_numSets( numSets )
{
    std::vector<VkDescriptorPoolSize> poolSizes;

    VkDescriptorPoolSize poolSize{ };
    VkDescriptorType     supportedTypes[] = { VK_DESCRIPTOR_TYPE_SAMPLER,
                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                              VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                              VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                                              VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                              VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                              VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR };

    for ( const VkDescriptorType &type : supportedTypes )
    {
        poolSize.type            = type;
        poolSize.descriptorCount = numSets * 4;
        poolSizes.push_back( poolSize );
    }

    VkDescriptorPoolCreateInfo poolInfo{ };
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size( );
    poolInfo.pPoolSizes    = poolSizes.data( );
    poolInfo.maxSets       = numSets;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VK_CHECK_RESULT( vkCreateDescriptorPool( device, &poolInfo, nullptr, &m_pool ) );
}

VulkanDescriptorPoolManager::VulkanDescriptorPool::~VulkanDescriptorPool( )
{
    if ( m_pool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool( m_device, m_pool, nullptr );
    }
}

VulkanDescriptorPoolManager::VulkanDescriptorPoolManager( const VkDevice &device, const VkPhysicalDevice &physicalDevice ) : m_device( device )
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );

    m_maxSets     = std::min( deviceProperties.limits.maxDescriptorSetUniformBuffers, m_maxSets );
    m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );
}

void VulkanDescriptorPoolManager::AllocateDescriptorSets( const VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet *sets )
{
    if ( allocateInfo.descriptorSetCount > m_maxSets )
    {
        spdlog::error( "Descriptor set count {} exceeds maximum set count {}", allocateInfo.descriptorSetCount, m_maxSets );
        return;
    }

    if ( m_currentPool->m_setsAllocated + allocateInfo.descriptorSetCount > m_currentPool->m_numSets )
    {
        m_pools.push_back( std::move( m_currentPool ) );
        m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );
    }

    VkDescriptorSetAllocateInfo copyAllocateInfo = allocateInfo;
    copyAllocateInfo.descriptorPool              = m_currentPool->m_pool;

    const VkResult result = vkAllocateDescriptorSets( m_device, &copyAllocateInfo, sets );
    if ( result == VK_SUCCESS )
    {
        m_currentPool->m_setsAllocated += allocateInfo.descriptorSetCount;
    }
    else if ( result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL )
    {
        m_pools.push_back( std::move( m_currentPool ) );
        m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );

        copyAllocateInfo.descriptorPool = m_currentPool->m_pool;
        VK_CHECK_RESULT( vkAllocateDescriptorSets( m_device, &copyAllocateInfo, sets ) );
        m_currentPool->m_setsAllocated += allocateInfo.descriptorSetCount;
    }
    else
    {
        VK_CHECK_RESULT( result );
    }
}
