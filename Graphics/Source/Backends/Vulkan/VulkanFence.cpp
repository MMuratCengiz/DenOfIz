/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanFence.h"

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
    if ( m_context->IsDeviceLost )
    {
        return;
    }
    const VkResult result = vkWaitForFences( m_context->LogicalDevice, 1, &m_fence, true, UINT64_MAX );
    if ( m_context->CheckDeviceLost( result, "vkWaitForFences" ) )
    {
        return;
    }
    VK_CHECK_RESULT( result );
}

void VulkanFence::Reset( )
{
    if ( m_context->IsDeviceLost )
    {
        return;
    }
    const VkResult result = vkResetFences( m_context->LogicalDevice, 1, &m_fence );
    if ( m_context->CheckDeviceLost( result, "vkResetFences" ) )
    {
        return;
    }
    VK_CHECK_RESULT( result );
}

VulkanFence::~VulkanFence( )
{
    vkDestroyFence( m_context->LogicalDevice, m_fence, nullptr );
}

VkFence VulkanFence::GetFence( ) const
{
    return m_fence;
}
