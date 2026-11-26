/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipelineCache.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanPipelineCache::VulkanPipelineCache( VulkanContext *context, const DenOfIz_PipelineCacheDesc &desc ) : m_context( context )
{
    DZ_NOT_NULL( context );

    VkPipelineCacheCreateInfo createInfo = { };
    createInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pNext                     = nullptr;
    createInfo.flags                     = 0;
    createInfo.initialDataSize           = desc.DataSize;
    createInfo.pInitialData              = desc.Data;

    const VkResult result = vkCreatePipelineCache( m_context->LogicalDevice, &createInfo, nullptr, &m_pipelineCache );
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "Failed to create Vulkan pipeline cache" );
    }
}

VulkanPipelineCache::~VulkanPipelineCache( )
{
    if ( m_pipelineCache != VK_NULL_HANDLE )
    {
        vkDestroyPipelineCache( m_context->LogicalDevice, m_pipelineCache, nullptr );
        m_pipelineCache = VK_NULL_HANDLE;
    }
}

size_t VulkanPipelineCache::GetDataNumBytes( )
{
    if ( m_pipelineCache == VK_NULL_HANDLE )
    {
        return 0;
    }

    size_t dataSize = 0;
    vkGetPipelineCacheData( m_context->LogicalDevice, m_pipelineCache, &dataSize, nullptr );
    return dataSize;
}

bool VulkanPipelineCache::GetData( DenOfIz_ByteArray &data )
{
    if ( m_pipelineCache == VK_NULL_HANDLE || !data.Elements )
    {
        return false;
    }

    size_t   dataSize = 0;
    VkResult result   = vkGetPipelineCacheData( m_context->LogicalDevice, m_pipelineCache, &dataSize, nullptr );
    if ( result != VK_SUCCESS || dataSize == 0 || data.NumElements < dataSize )
    {
        return false;
    }

    result = vkGetPipelineCacheData( m_context->LogicalDevice, m_pipelineCache, &dataSize, data.Elements );
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "Failed to get pipeline cache data" );
        return false;
    }

    data.NumElements = dataSize;
    return true;
}

VkPipelineCache VulkanPipelineCache::GetCache( ) const
{
    return m_pipelineCache;
}
