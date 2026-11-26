/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandPool.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"

using namespace DenOfIz;

VulkanCommandPool::VulkanCommandPool( VulkanContext *context, const DenOfIz_CommandListPoolDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_commandQueue = dynamic_cast<VulkanCommandQueue *>( DENOFIZ_FROM_HANDLE( ICommandQueue, desc.CommandQueue ) );
    m_commandPools.reserve( desc.NumCommandLists );

    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        VkCommandPool           commandPool;
        VkCommandPoolCreateInfo commandPoolCreateInfo{ };
        commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = m_commandQueue->GetQueueFamilyIndex( );

        vkCreateCommandPool( m_context->LogicalDevice, &commandPoolCreateInfo, nullptr, &commandPool );
        m_commandPools.push_back( commandPool );

        DenOfIz_CommandListDesc commandListDesc{ };
        commandListDesc.QueueType = GetQueueType( );

        m_commandLists.emplace_back( std::make_unique<VulkanCommandList>( context, commandListDesc, commandPool ) );
        m_commandListPtrs.push_back( m_commandLists[ i ].get( ) );
    }
}

DenOfIz_CommandListArray VulkanCommandPool::GetCommandLists( )
{
    DenOfIz_CommandListArray commandLists;
    commandLists.Elements    = reinterpret_cast<DenOfIz_CommandList *>( m_commandListPtrs.data( ) );
    commandLists.NumElements = static_cast<uint32_t>( m_commandListPtrs.size( ) );
    return commandLists;
}

DenOfIz_QueueType VulkanCommandPool::GetQueueType( ) const
{
    return m_commandQueue->GetQueueType( );
}

VulkanCommandPool::~VulkanCommandPool( )
{
    m_commandLists.clear( );
    m_commandListPtrs.clear( );
    for ( const VkCommandPool commandPool : m_commandPools )
    {
        if ( commandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool( m_context->LogicalDevice, commandPool, nullptr );
        }
    }
    m_commandPools.clear( );
}
