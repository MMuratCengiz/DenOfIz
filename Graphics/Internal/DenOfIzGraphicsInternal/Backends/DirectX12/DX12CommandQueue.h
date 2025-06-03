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

#include <DenOfIzGraphics/Backends/Interface/ICommandQueue.h>
#include "DX12CommandList.h"
#include "DX12Context.h"

namespace DenOfIz
{
    class DZ_API DX12CommandQueue final : public ICommandQueue
    {
        DX12Context                     *m_context;
        CommandQueueDesc                 m_desc;
        wil::com_ptr<ID3D12CommandQueue> m_commandQueue;
        wil::com_ptr<ID3D12Fence1>       m_fence;

    public:
        DX12CommandQueue( DX12Context *context, const CommandQueueDesc &desc );
        ~DX12CommandQueue( ) override;

        void          WaitIdle( ) override;
        void          ExecuteCommandLists( const ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        ID3D12CommandQueue *GetCommandQueue( ) const;
        QueueType           GetQueueType( ) const;
    };
} // namespace DenOfIz
