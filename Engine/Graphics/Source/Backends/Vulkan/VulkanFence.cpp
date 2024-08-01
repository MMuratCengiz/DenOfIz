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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanFence.h>

using namespace DenOfIz;

VulkanFence::VulkanFence( VulkanContext *context ) : m_context( context )
{
    VkFenceCreateInfo fenceCreateInfo{ };
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK_RESULT( vkCreateFence( m_context->LogicalDevice, &fenceCreateInfo, nullptr, &m_fence ) );
}

void VulkanFence::Wait( )
{
    VK_CHECK_RESULT( vkWaitForFences( m_context->LogicalDevice, 1, &m_fence, true, UINT64_MAX ) );
}

void VulkanFence::Reset( )
{
    VK_CHECK_RESULT( vkResetFences( m_context->LogicalDevice, 1, &m_fence ) );
}

VulkanFence::~VulkanFence( )
{
    vkDestroyFence( m_context->LogicalDevice, m_fence, nullptr );
}
