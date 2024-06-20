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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>
#include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>

namespace DenOfIz
{

    class DX12Semaphore : public ISemaphore
    {
    private:
        DX12Context *m_context;
        wil::com_ptr<ID3D12Fence> m_fence;
        Microsoft::WRL::Wrappers::Event m_fenceEvent;
        UINT64 m_fenceValue;

    public:
        DX12Semaphore(DX12Context *context);
        ID3D12Fence *GetFence() const { return m_fence.get(); }
        ~DX12Semaphore() override;
        void Wait() override;
        void Notify() override;
    };
} // namespace DenOfIz
