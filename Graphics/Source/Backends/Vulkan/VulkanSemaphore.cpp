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
