/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

VkDescriptorPool VulkanDescriptorPoolManager::AllocateDescriptorSets( const VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet *sets )
{
    if ( allocateInfo.descriptorSetCount > m_maxSets )
    {
        spdlog::error( "Descriptor set count {} exceeds maximum set count {}", allocateInfo.descriptorSetCount, m_maxSets );
        return VK_NULL_HANDLE;
    }

    std::lock_guard lock( m_mutex );
    if ( m_currentPool->m_setsAllocated + allocateInfo.descriptorSetCount > m_currentPool->m_numSets )
    {
        m_pools.push_back( std::move( m_currentPool ) );
        m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );
    }

    VkDescriptorSetAllocateInfo copyAllocateInfo = allocateInfo;
    copyAllocateInfo.descriptorPool              = m_currentPool->m_pool;

    VkResult result = vkAllocateDescriptorSets( m_device, &copyAllocateInfo, sets );
    if ( result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL )
    {
        m_pools.push_back( std::move( m_currentPool ) );
        m_currentPool = std::make_unique<VulkanDescriptorPool>( m_device, m_maxSets );

        copyAllocateInfo.descriptorPool = m_currentPool->m_pool;
        result                          = vkAllocateDescriptorSets( m_device, &copyAllocateInfo, sets );
    }
    if ( result != VK_SUCCESS )
    {
        VK_CHECK_RESULT( result );
        return VK_NULL_HANDLE;
    }
    m_currentPool->m_setsAllocated += allocateInfo.descriptorSetCount;
    return m_currentPool->m_pool;
}

void VulkanDescriptorPoolManager::FreeDescriptorSet( const VkDescriptorPool pool, const VkDescriptorSet set )
{
    if ( pool == VK_NULL_HANDLE || set == VK_NULL_HANDLE )
    {
        return;
    }

    std::lock_guard lock( m_mutex );
    if ( m_currentPool && m_currentPool->m_pool == pool )
    {
        VK_CHECK_RESULT( vkFreeDescriptorSets( m_device, pool, 1, &set ) );
        m_currentPool->m_setsAllocated--;
        return;
    }
    for ( auto it = m_pools.begin( ); it != m_pools.end( ); ++it )
    {
        if ( ( *it )->m_pool != pool )
        {
            continue;
        }
        VK_CHECK_RESULT( vkFreeDescriptorSets( m_device, pool, 1, &set ) );
        if ( --( *it )->m_setsAllocated == 0 )
        {
            m_pools.erase( it );
        }
        return;
    }
}
