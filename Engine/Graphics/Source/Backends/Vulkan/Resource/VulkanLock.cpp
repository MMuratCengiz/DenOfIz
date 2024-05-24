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

#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanLock.h>

using namespace DenOfIz;

VulkanLock::VulkanLock(VulkanContext* context, const LockType& lockType)
		:
		m_context(context)
{
	this->m_lockType = lockType;

	if (lockType == LockType::Fence)
	{
		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
		m_fence = this->m_context->LogicalDevice.createFence(fenceCreateInfo);
	}
	else
	{
		constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		m_semaphore = this->m_context->LogicalDevice.createSemaphore(semaphoreCreateInfo);
	}
}

void VulkanLock::Wait()
{
	if (m_lockType == LockType::Fence)
	{
		const auto result = m_context->LogicalDevice.waitForFences(1, &m_fence, true, UINT64_MAX);
		VK_CHECK_RESULT(result);
	}
	else
	{
		vk::SemaphoreWaitInfo waitInfo{};
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_semaphore;
		waitInfo.flags = vk::SemaphoreWaitFlagBits::eAny;

		auto result = m_context->LogicalDevice.waitSemaphores(waitInfo, UINT64_MAX);
		VK_CHECK_RESULT(result);
	}
}

void VulkanLock::Reset()
{
	if (m_lockType == LockType::Fence)
	{
		const vk::Result result = m_context->LogicalDevice.resetFences(1, &m_fence);
		VK_CHECK_RESULT(result);
	}
	else
	{
		// No matching functionality
	}
}

void VulkanLock::Notify()
{
	if (m_lockType == LockType::Fence)
	{
		// No notify on client
	}
	else
	{
		m_context->LogicalDevice.signalSemaphore(m_semaphore);
	}
}

VulkanLock::~VulkanLock()
{
	if (m_lockType == LockType::Fence)
	{
		m_context->LogicalDevice.destroyFence(m_fence);
	}
	else
	{
		m_context->LogicalDevice.destroySemaphore(m_semaphore);
	}
}

#endif
