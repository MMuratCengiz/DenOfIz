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
#include "DX12CommandList.h"

namespace DenOfIz
{
    class DX12CommandListPool : public ICommandListPool
    {
    private:
        DX12Context *m_context;
        std::vector<wil::com_ptr<ID3D12CommandAllocator>> m_commandAllocators;
        wil::com_ptr<ID3D12GraphicsCommandList> m_commandList;
        std::vector<std::unique_ptr<DX12CommandList>> m_commandLists;
        CommandListPoolDesc m_desc;
    public:
        DX12CommandListPool(DX12Context *context, CommandListPoolDesc desc) : m_desc(desc)
        {
            DZ_NOT_NULL(context);
            DZ_ASSERTM(desc.CommandListCount > 0, "CommandListCount must be greater than 0");

            m_context = context;
            for (uint32_t i = 0; i < desc.CommandListCount; i++)
            {
                wil::com_ptr<ID3D12CommandAllocator> commandAllocator;

                D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType(desc.QueueType);
                THROW_IF_FAILED(context->D3DDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(commandAllocator.put())));
                m_commandAllocators.push_back(std::move(commandAllocator));
            }

            D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType(m_desc.QueueType);

            THROW_IF_FAILED(context->D3DDevice->CreateCommandList(0, commandListType, m_commandAllocators[0].get(), nullptr, IID_PPV_ARGS(m_commandList.put())));
            m_commandList->Close();

            CommandListDesc commandListCreateInfo{};
            commandListCreateInfo.QueueType = m_desc.QueueType;

            for (uint32_t i = 0; i < desc.CommandListCount; i++)
            {
                m_commandLists.push_back(std::make_unique<DX12CommandList>(m_context, m_commandAllocators[i], m_commandList, commandListCreateInfo));
            }
        }

        virtual std::vector<ICommandList*> GetCommandLists() override
        {
            std::vector<ICommandList*> commandLists;
            for (auto &commandList : m_commandLists)
            {
                commandLists.push_back(commandList.get());
            }
            return commandLists;
        }

        ~DX12CommandListPool() override = default;
    };
} // namespace DenOfIz
