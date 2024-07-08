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

VulkanSemaphore::VulkanSemaphore(VulkanContext *context) : m_context(context)
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    m_semaphore = m_context->LogicalDevice.createSemaphore(semaphoreCreateInfo);
}

void VulkanSemaphore::Wait()
{
    vk::SemaphoreWaitInfo waitInfo{};
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores    = &m_semaphore;
    waitInfo.flags          = vk::SemaphoreWaitFlagBits::eAny;

    auto result = m_context->LogicalDevice.waitSemaphores(waitInfo, UINT64_MAX);
    VK_CHECK_RESULT(result);
}

void VulkanSemaphore::Notify()
{
    vk::SemaphoreSignalInfo signalInfo{};
    signalInfo.semaphore = m_semaphore;

    m_context->LogicalDevice.signalSemaphore(signalInfo);
}

VulkanSemaphore::~VulkanSemaphore()
{
    m_context->LogicalDevice.destroySemaphore(m_semaphore);
}
