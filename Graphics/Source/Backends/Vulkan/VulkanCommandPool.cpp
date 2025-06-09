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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandPool.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"

using namespace DenOfIz;

VulkanCommandPool::VulkanCommandPool( VulkanContext *context, const CommandListPoolDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_commandQueue = dynamic_cast<VulkanCommandQueue *>( desc.CommandQueue );

    VkCommandPoolCreateInfo commandPoolCreateInfo{ };
    commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = m_commandQueue->GetQueueFamilyIndex( );

    vkCreateCommandPool( m_context->LogicalDevice, &commandPoolCreateInfo, nullptr, &m_commandPool );

    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        CommandListDesc commandListDesc{ };
        commandListDesc.QueueType = GetQueueType( );

        m_commandLists.emplace_back( std::make_unique<VulkanCommandList>( context, commandListDesc, m_commandPool ) );
        m_commandListPtrs.push_back( m_commandLists[ i ].get( ) );
    }
}

ICommandListArray VulkanCommandPool::GetCommandLists( )
{
    ICommandListArray commandLists;
    commandLists.Elements    = reinterpret_cast<ICommandList **>( m_commandListPtrs.data( ) );
    commandLists.NumElements = static_cast<uint32_t>( m_commandListPtrs.size( ) );
    return commandLists;
}

QueueType VulkanCommandPool::GetQueueType( ) const
{
    return m_commandQueue->GetQueueType( );
}
