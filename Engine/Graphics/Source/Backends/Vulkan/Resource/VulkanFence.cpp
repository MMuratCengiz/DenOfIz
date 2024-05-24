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

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanFence.h>

using namespace DenOfIz;

VulkanFence::VulkanFence(VulkanContext* context)
		:m_context(context)
{
	m_fence = m_context->LogicalDevice.createFence(vk::FenceCreateFlags{});
}

void VulkanFence::Wait()
{
	const auto result = m_context->LogicalDevice.waitForFences(1, &m_fence, true, UINT64_MAX);
	VK_CHECK_RESULT(result);
}

void VulkanFence::Reset()
{
	const vk::Result result = m_context->LogicalDevice.resetFences(1, &m_fence);
	VK_CHECK_RESULT(result);
}

VulkanFence::~VulkanFence()
{
	m_context->LogicalDevice.destroyFence(m_fence);
}
