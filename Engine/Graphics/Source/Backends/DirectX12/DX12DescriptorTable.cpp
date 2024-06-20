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

#include <DenOfIzGraphics/Backends/DirectX12/DX12DescriptorTable.h>

using namespace DenOfIz;

DX12DescriptorTable::DX12DescriptorTable(DX12Context *context, DescriptorTableCreateInfo createInfo)
{
    DX12RootSignature *rootSignature = static_cast<DX12RootSignature *>(createInfo.RootSignature);
    NOT_NULL(rootSignature);

    m_rootSignature = rootSignature->GetRootSignature();
}

void DX12DescriptorTable::BindImage(ITextureResource *resource)
{
    NOT_NULL(resource);

    DX12ImageResource *dx12Resource = static_cast<DX12ImageResource *>(resource);
}

void DX12DescriptorTable::BindBuffer(IBufferResource *resource)
{
    NOT_NULL(resource);

    DX12BufferResource *dx12Resource = static_cast<DX12BufferResource *>(resource);
    m_resources.push_back(dx12Resource->GetResource());
}
