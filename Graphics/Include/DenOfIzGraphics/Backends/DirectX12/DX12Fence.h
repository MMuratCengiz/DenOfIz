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

#include <DenOfIzGraphics/Backends/Interface/IFence.h>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12Fence final : public IFence
    {
        constexpr static UINT64   INITIAL_FENCE_VALUE = 0;
        DX12Context              *m_context;
        wil::com_ptr<ID3D12Fence> m_fence;
        UINT32                    m_fenceValue = 1;
        Wrappers::Event           m_fenceEvent;
        bool                      m_submitted = false;

    public:
        explicit DX12Fence( DX12Context *context );
        [[nodiscard]] ID3D12Fence *GetFence( ) const;
        ~DX12Fence( ) override;
        void Wait( ) override;
        void Reset( ) override;
        void NotifyCommandQueue( ID3D12CommandQueue *commandQueue );
    };

} // namespace DenOfIz
