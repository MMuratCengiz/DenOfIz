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

#include <DenOfIzGraphics/Backends/DirectX12/Resource/DX12Semaphore.h>

using namespace DenOfIz;

DX12Semaphore::DX12Semaphore(DX12Context *context)
{
    m_context = context;
    m_fenceValue = 1;
    context->D3DDevice->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put()));
    m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
}

DX12Semaphore::~DX12Semaphore() { m_fence = nullptr; }

void DX12Semaphore::Wait()
{
    if ( m_fence->GetCompletedValue() != 1 )
    {
        THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
        m_fence->Signal(0);
    }
}

void DX12Semaphore::Notify()
{
    m_fence->Signal(1);
}
