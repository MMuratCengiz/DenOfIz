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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanSemaphore.h>

using namespace DenOfIz;

VulkanSemaphore::VulkanSemaphore( VulkanContext *context ) : m_context( context )
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{ };
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = VK_SEMAPHORE_TYPE_BINARY;
    vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphore );
}

void VulkanSemaphore::Wait( )
{
    VkSemaphoreWaitInfo waitInfo{ };
    waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores    = &m_semaphore;
    waitInfo.pValues        = &m_value;
    waitInfo.flags          = VK_SEMAPHORE_WAIT_ANY_BIT;

    VK_CHECK_RESULT( vkWaitSemaphores( m_context->LogicalDevice, &waitInfo, UINT64_MAX ) );
    m_value = 0;
}

void VulkanSemaphore::Notify( )
{
    VkSemaphoreSignalInfo signalInfo{ };
    signalInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.semaphore = m_semaphore;
    signalInfo.value     = m_value;

    VK_CHECK_RESULT( vkSignalSemaphore( m_context->LogicalDevice, &signalInfo ) );
}

VulkanSemaphore::~VulkanSemaphore( )
{
    vkDestroySemaphore( m_context->LogicalDevice, m_semaphore, nullptr );
}

VkSemaphore VulkanSemaphore::GetSemaphore( ) const
{
    return m_semaphore;
}
