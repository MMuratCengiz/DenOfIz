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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Fence.h>
#include "DenOfIzGraphics/Backends/DirectX12/DX12Context.h"

using namespace DenOfIz;

DX12Fence::DX12Fence(DX12Context *context) : m_context(context)
{
    m_context->D3DDevice->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put()));
    m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
}

DX12Fence::~DX12Fence()
{
}

void DX12Fence::Wait()
{
    if ( !m_submitted )
    {
        m_submitted = true;
        return;
    }

    if ( m_fence->GetCompletedValue() != 1 )
    {
        THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
    }
    m_fence->Signal(0);
}

void DX12Fence::Reset()
{
    m_submitted = true;
}
