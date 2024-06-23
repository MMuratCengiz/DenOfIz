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

#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include "VulkanCommandList.h"

namespace DenOfIz
{
    class VulkanCommandPool final : public ICommandListPool
    {
    private:
        VulkanContext *m_context;
        std::vector<std::unique_ptr<VulkanCommandList>> m_commandLists;
        CommandListPoolDesc m_createInfo;
    public:
        VulkanCommandPool(VulkanContext* context, const CommandListPoolDesc &desc)
        {
            for (uint32_t i = 0; i < desc.CommandListCount; i++)
            {
                CommandListDesc commandListDesc{};
                commandListDesc.QueueType = desc.QueueType;

                m_commandLists.emplace_back(std::make_unique<VulkanCommandList>(context, commandListDesc));
            }
        }

        std::vector<ICommandList *> GetCommandLists() override
        {
            std::vector<ICommandList *> commandLists;
            for (auto &commandList : m_commandLists)
            {
                commandLists.push_back(commandList.get());
            }
            return commandLists;
        }

        ~VulkanCommandPool() override
        {
        }

    };
} // namespace DenOfIz