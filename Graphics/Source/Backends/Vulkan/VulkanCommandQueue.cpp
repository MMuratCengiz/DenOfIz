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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanFence.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanSwapChain.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanCommandQueue::VulkanCommandQueue( VulkanContext *context, const CommandQueueDesc &desc ) : m_context( context ), m_desc( desc ), m_queueFamilyIndex( 0 ), m_queueIndex( 0 )
{
    const VkQueueFlags requiredFlags = VulkanEnumConverter::ConvertQueueFlags( desc.QueueType );

    FindQueueFamilyIndex( requiredFlags );

    vkGetDeviceQueue( m_context->LogicalDevice, m_queueFamilyIndex, m_queueIndex, &m_queue );

    if ( m_queue == VK_NULL_HANDLE )
    {
        spdlog::critical("Failed to create queue");
    }
}

VulkanCommandQueue::~VulkanCommandQueue( )
{
    WaitIdle( );
    m_queue = VK_NULL_HANDLE;
}

void VulkanCommandQueue::WaitIdle( )
{
    vkQueueWaitIdle( m_queue );
}

void VulkanCommandQueue::ExecuteCommandLists( const ExecuteCommandListsDesc &executeCommandListsDesc )
{
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore>          waitSemaphores;

    for ( int i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements( ); i++ )
    {
        const auto *vulkanSemaphore = dynamic_cast<VulkanSemaphore *>( executeCommandListsDesc.WaitSemaphores.GetElement( i ) );
        waitSemaphores.push_back( vulkanSemaphore->GetSemaphore( ) );
        waitStages.push_back( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
    }

    std::vector<VkSemaphore> signalSemaphores;
    for ( int i = 0; i < executeCommandListsDesc.SignalSemaphores.NumElements( ); i++ )
    {
        const auto *vulkanSemaphore = dynamic_cast<VulkanSemaphore *>( executeCommandListsDesc.SignalSemaphores.GetElement( i ) );
        signalSemaphores.push_back( vulkanSemaphore->GetSemaphore( ) );
    }

    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.reserve( executeCommandListsDesc.CommandLists.NumElements( ) );

    for ( int i = 0; i < executeCommandListsDesc.CommandLists.NumElements( ); i++ )
    {
        auto *vulkanCmdList = dynamic_cast<VulkanCommandList *>( executeCommandListsDesc.CommandLists.GetElement( i ) );
        commandBuffers.push_back( vulkanCmdList->GetCommandBuffer( ) );
    }

    VkSubmitInfo submitInfo{ };
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = waitSemaphores.size( );
    submitInfo.pWaitSemaphores      = waitSemaphores.data( );
    submitInfo.pWaitDstStageMask    = waitStages.data( );
    submitInfo.commandBufferCount   = commandBuffers.size( );
    submitInfo.pCommandBuffers      = commandBuffers.data( );
    submitInfo.signalSemaphoreCount = signalSemaphores.size( );
    submitInfo.pSignalSemaphores    = signalSemaphores.data( );

    VkFence fence = VK_NULL_HANDLE;
    if ( executeCommandListsDesc.Signal )
    {
        auto *vulkanFence = dynamic_cast<VulkanFence *>( executeCommandListsDesc.Signal );
        vulkanFence->Reset( );
        fence = vulkanFence->GetFence( );
    }

    VK_CHECK_RESULT( vkQueueSubmit( m_queue, 1, &submitInfo, fence ) );
}

uint32_t VulkanCommandQueue::GetQueueFamilyIndex( ) const
{
    return m_queueFamilyIndex;
}

void VulkanCommandQueue::FindQueueFamilyIndex( const VkQueueFlags requiredFlags )
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( m_context->PhysicalDevice, &queueFamilyCount, nullptr );

    std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( m_context->PhysicalDevice, &queueFamilyCount, queueFamilies.data( ) );

    for ( uint32_t i = 0; i < queueFamilyCount; ++i )
    {
        const VkQueueFlags queueFlags      = queueFamilies[ i ].queueFlags;
        const bool         isGraphicsQueue = ( queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0;

        if ( m_desc.QueueType == QueueType::Graphics && isGraphicsQueue )
        {
            m_queueFamilyIndex = i;
            m_queueIndex       = 0;
            return;
        }

        const uint32_t matchingFlags = queueFlags & requiredFlags;
        if ( matchingFlags && ( queueFlags & ~requiredFlags ) == 0 )
        {
            m_queueFamilyIndex = i;
            m_queueIndex       = 0;
            return;
        }

        if ( matchingFlags && queueFlags - matchingFlags < UINT32_MAX && !isGraphicsQueue )
        {
            m_queueFamilyIndex = i;
            m_queueIndex       = 0;
            return;
        }
    }

    // Still not found:
    for ( uint32_t i = 0; i < queueFamilyCount; ++i )
    {
        if ( queueFamilies[ i ].queueFlags & requiredFlags )
        {
            m_queueFamilyIndex = i;
            m_queueIndex       = 0;
            return;
        }
    }

    spdlog::warn("Could not find queue of required type. Using default queue family");
    m_queueFamilyIndex = 0;
    m_queueIndex       = 0;
}

VkQueue VulkanCommandQueue::GetQueue( ) const
{
    return m_queue;
}

QueueType VulkanCommandQueue::GetQueueType( ) const
{
    return m_desc.QueueType;
}
