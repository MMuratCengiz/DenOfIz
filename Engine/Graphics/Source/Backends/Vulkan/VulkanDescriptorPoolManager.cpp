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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanDescriptorPoolManager.h>

using namespace DenOfIz;

VulkanDescriptorPoolManager::VulkanDescriptorPool::VulkanDescriptorPool( const VkDevice &device, uint32_t numSets ) : m_device( device ), m_numSets( numSets )
{
    std::vector<VkDescriptorPoolSize> poolSizes;

    VkDescriptorPoolSize poolSize{ };
    for ( int i = 0; i < VK_DESCRIPTOR_TYPE_MAX_ENUM; i++ )
    {
        poolSize.type            = static_cast<VkDescriptorType>( i );
        poolSize.descriptorCount = numSets;
        poolSizes.push_back( poolSize );
    }

    VkDescriptorPoolCreateInfo poolInfo{ };
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size( );
    poolInfo.pPoolSizes    = poolSizes.data( );
    poolInfo.maxSets       = numSets * poolSizes.size( );
}

VulkanDescriptorPoolManager::VulkanDescriptorPool::~VulkanDescriptorPool( )
{
    vkDestroyDescriptorPool( m_device, m_pool, nullptr );
}

VulkanDescriptorPoolManager::VulkanDescriptorPoolManager( const VkDevice &device ) : m_device( device )
{
    m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );
}

VulkanDescriptorPoolManager::~VulkanDescriptorPoolManager( )
{
}

void VulkanDescriptorPoolManager::AllocateDescriptorSets( const VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet *sets )
{
    if ( allocateInfo.descriptorSetCount > m_maxSets )
    {
        LOG( ERROR ) << "Descriptor set count exceeds maximum set count";
    }

    if ( m_currentPool->m_setsAllocated + allocateInfo.descriptorSetCount > m_currentPool->m_numSets )
    {
        m_pools.push_back( std::move( m_currentPool ) );
        m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );
    }

    VkDescriptorSetAllocateInfo copyAllocateInfo = allocateInfo;
    copyAllocateInfo.descriptorPool              = m_currentPool->m_pool;

    VK_CHECK_RESULT( vkAllocateDescriptorSets( m_device, &copyAllocateInfo, sets ) );
    m_currentPool->m_setsAllocated += allocateInfo.descriptorSetCount;
}
