/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanSemaphore.h"

using namespace DenOfIz;

VulkanSemaphore::VulkanSemaphore( VulkanContext *context ) : m_context( context )
{
    VkSemaphoreTypeCreateInfo timelineCreateInfo{ };
    timelineCreateInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineCreateInfo.pNext         = nullptr;
    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineCreateInfo.initialValue  = 0;

    VkSemaphoreCreateInfo timelineSemaphoreCreateInfo{ };
    timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    timelineSemaphoreCreateInfo.pNext = &timelineCreateInfo;
    timelineSemaphoreCreateInfo.flags = 0;

    vkCreateSemaphore( m_context->LogicalDevice, &timelineSemaphoreCreateInfo, nullptr, &m_timelineSemaphore );
}

void VulkanSemaphore::Wait( ) const
{
    VkSemaphoreWaitInfo waitInfo{ };
    waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores    = &m_timelineSemaphore;
    waitInfo.pValues        = &m_timelineValue;
    waitInfo.flags          = VK_SEMAPHORE_WAIT_ANY_BIT;

    VK_CHECK_RESULT( vkWaitSemaphores( m_context->LogicalDevice, &waitInfo, UINT64_MAX ) );
}

void VulkanSemaphore::Notify( )
{
    m_timelineValue++;
}

VulkanSemaphore::~VulkanSemaphore( )
{
    vkDestroySemaphore( m_context->LogicalDevice, m_timelineSemaphore, nullptr );
}

bool VulkanSemaphore::IsCompleted( ) const
{
    if ( m_timelineValue == 0 )
    {
        return false;
    }
    uint64_t       currentValue;
    const VkResult result = vkGetSemaphoreCounterValue( m_context->LogicalDevice, m_timelineSemaphore, &currentValue );
    if ( result == VK_SUCCESS )
    {
        return currentValue >= m_timelineValue;
    }
    return false;
}

VkSemaphore VulkanSemaphore::GetSemaphore( ) const
{
    return m_timelineSemaphore;
}

uint64_t VulkanSemaphore::GetTimelineValue( ) const
{
    return m_timelineValue;
}
