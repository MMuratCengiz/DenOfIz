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

#include <DenOfIzGraphics/Backends/DirectX12/Resource/DX12Fence.h>
#include "DenOfIzGraphics/Backends/DirectX12/DX12Context.h"

using namespace DenOfIz;

DX12Fence::DX12Fence(DX12Context* context)
{
	context->D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));
}

DX12Fence::~DX12Fence()
{
	m_fence.Reset();
}

void DX12Fence::Wait()
{

}

void DX12Fence::Reset()
{
}

