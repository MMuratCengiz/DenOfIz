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

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h>
#include "VulkanCommandList.h"
#include "VulkanCommandQueue.h"

namespace DenOfIz
{
    class VulkanCommandPool final : public ICommandListPool
    {
        VulkanContext                                  *m_context;
        std::vector<std::unique_ptr<VulkanCommandList>> m_commandLists;
        std::vector<VulkanCommandList *>                m_commandListPtrs;
        DenOfIz_CommandListPoolDesc                     m_desc;
        std::vector<VkCommandPool>                      m_commandPools; // One per command list for thread safety
        VulkanCommandQueue                             *m_commandQueue;

    public:
        VulkanCommandPool( VulkanContext *context, const DenOfIz_CommandListPoolDesc &desc );
        DenOfIz_CommandListArray        GetCommandLists( ) override;
        [[nodiscard]] DenOfIz_QueueType GetQueueType( ) const;
        ~VulkanCommandPool( ) override;
    };
} // namespace DenOfIz
