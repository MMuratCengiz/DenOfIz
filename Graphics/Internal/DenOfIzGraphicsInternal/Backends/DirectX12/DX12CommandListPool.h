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
        DX12Context                                         *m_context;
        std::vector<wil::com_ptr<ID3D12CommandAllocator>>    m_commandAllocators;
        std::vector<wil::com_ptr<ID3D12GraphicsCommandList>> m_dx12CommandLists;
        std::vector<std::unique_ptr<DX12CommandList>>        m_commandLists;
        std::vector<DX12CommandList *>                       m_commandListPtrs;
        CommandListPoolDesc                                  m_desc;

    public:
        DX12CommandListPool( DX12Context *context, CommandListPoolDesc desc );
        ICommandListArray GetCommandLists( ) override;
        ~DX12CommandListPool( ) override = default;
    };
} // namespace DenOfIz
