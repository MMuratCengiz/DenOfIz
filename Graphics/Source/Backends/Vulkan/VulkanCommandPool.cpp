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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanCommandPool.h>

using namespace DenOfIz;

VulkanCommandPool::VulkanCommandPool( VulkanContext *context, const CommandListPoolDesc &desc ) : m_context( nullptr ), m_createInfo( desc )
{
    for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
    {
        CommandListDesc commandListDesc{ };
        commandListDesc.QueueType = desc.QueueType;

        m_commandLists.emplace_back( std::make_unique<VulkanCommandList>( context, commandListDesc ) );
    }
}

CommandLists VulkanCommandPool::GetCommandLists( )
{
    CommandLists commandLists;
    commandLists.NumElements = m_commandLists.size( );
    for ( int i = 0; i < m_commandLists.size( ); i++ )
    {
        commandLists.Array[ i ] = m_commandLists[ i ].get( );
    }
    return commandLists;
}
