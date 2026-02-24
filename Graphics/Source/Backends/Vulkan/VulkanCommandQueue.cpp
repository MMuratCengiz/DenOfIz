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

VulkanCommandQueue::VulkanCommandQueue( VulkanContext *context, const DenOfIz_CommandQueueDesc &desc ) :
    m_context( context ), m_desc( desc ), m_queueFamilyIndex( 0 ), m_queueIndex( 0 )
{
    const VkQueueFlags requiredFlags = DenOfIz_VulkanEnumConverter_ConvertQueueFlags( desc.QueueType );

    FindQueueFamilyIndex( requiredFlags );

    vkGetDeviceQueue( m_context->LogicalDevice, m_queueFamilyIndex, m_queueIndex, &m_queue );

    if ( m_queue == VK_NULL_HANDLE )
    {
        spdlog::critical( "Failed to create queue" );
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

void VulkanCommandQueue::ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc )
{
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore>          waitSemaphores;
    std::vector<uint64_t>             timelineWaitValues;
    uint32_t                          numBinaryWaitSemaphores = 0;

    ISwapChain *targetSwapChain = nullptr;
    for ( uint32_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; ++i )
    {
        ISwapChain *swapChain = nullptr;
        if ( m_context->AutoSync->NeedsSwapChainSync( swapChain ) )
        {
            const auto *vulkanSwapChain = dynamic_cast<VulkanSwapChain *>( swapChain );
            waitSemaphores.push_back( vulkanSwapChain->GetCurrentAcquireSemaphore( ) );
            waitStages.push_back( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT );
            timelineWaitValues.push_back( 0 );
            numBinaryWaitSemaphores++;
            targetSwapChain = swapChain;
            break;
        }
    }

    for ( size_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; i++ )
    {
        const auto *vulkanSemaphore = dynamic_cast<VulkanSemaphore *>( DENOFIZ_FROM_HANDLE( ISemaphore, executeCommandListsDesc.WaitSemaphores.Elements[ i ] ) );
        waitSemaphores.push_back( vulkanSemaphore->GetSemaphore( ) );
        waitStages.push_back( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
        timelineWaitValues.push_back( vulkanSemaphore->GetTimelineValue( ) );
    }

    std::vector<VkSemaphore> signalSemaphores;
    std::vector<uint64_t>    timelineSignalValues;
    uint32_t                 numBinarySignalSemaphores = 0;

    for ( size_t i = 0; i < executeCommandListsDesc.SignalSemaphores.NumElements; i++ )
    {
        auto *vulkanSemaphore = dynamic_cast<VulkanSemaphore *>( DENOFIZ_FROM_HANDLE( ISemaphore, executeCommandListsDesc.SignalSemaphores.Elements[ i ] ) );
        vulkanSemaphore->Notify( );
        signalSemaphores.push_back( vulkanSemaphore->GetSemaphore( ) );
        timelineSignalValues.push_back( vulkanSemaphore->GetTimelineValue( ) );
    }

    if ( targetSwapChain )
    {
        auto             *vulkanSwapChain         = dynamic_cast<VulkanSwapChain *>( targetSwapChain );
        const VkSemaphore renderCompleteSemaphore = vulkanSwapChain->GetCurrentRenderCompleteSemaphore( );
        signalSemaphores.push_back( renderCompleteSemaphore );
        timelineSignalValues.push_back( 0 );
        numBinarySignalSemaphores++;
        vulkanSwapChain->SetLastUsedRenderCompleteSemaphore( renderCompleteSemaphore );
    }

    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.reserve( executeCommandListsDesc.CommandLists.NumElements );

    for ( size_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; i++ )
    {
        auto *vulkanCmdList = dynamic_cast<VulkanCommandList *>( DENOFIZ_FROM_HANDLE( ICommandList, executeCommandListsDesc.CommandLists.Elements[ i ] ) );
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

    VkTimelineSemaphoreSubmitInfo timelineSubmitInfo{ };
    if ( !timelineWaitValues.empty( ) || !timelineSignalValues.empty( ) )
    {
        timelineSubmitInfo.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineSubmitInfo.pNext                     = nullptr;
        timelineSubmitInfo.waitSemaphoreValueCount   = timelineWaitValues.size( );
        timelineSubmitInfo.pWaitSemaphoreValues      = timelineWaitValues.data( );
        timelineSubmitInfo.signalSemaphoreValueCount = timelineSignalValues.size( );
        timelineSubmitInfo.pSignalSemaphoreValues    = timelineSignalValues.data( );
        submitInfo.pNext                             = &timelineSubmitInfo;
    }

    VkFence fence = VK_NULL_HANDLE;
    if ( DENOFIZ_HANDLE_IS_VALID( executeCommandListsDesc.Signal ) )
    {
        auto *vulkanFence = dynamic_cast<VulkanFence *>( DENOFIZ_FROM_HANDLE( IFence, executeCommandListsDesc.Signal ) );
        vulkanFence->Reset( );
        fence = vulkanFence->GetFence( );
    }

    VK_CHECK_RESULT( vkQueueSubmit( m_queue, 1, &submitInfo, fence ) );
    for ( uint32_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; ++i )
    {
        m_context->AutoSync->ClearCommandListSync( );
    }
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

        if ( m_desc.QueueType == DENOFIZ_QUEUE_TYPE_GRAPHICS && isGraphicsQueue )
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

    spdlog::warn( "Could not find queue of required type. Using default queue family" );
    m_queueFamilyIndex = 0;
    m_queueIndex       = 0;
}

VkQueue VulkanCommandQueue::GetQueue( ) const
{
    return m_queue;
}

DenOfIz_QueueType VulkanCommandQueue::GetQueueType( ) const
{
    return m_desc.QueueType;
}
