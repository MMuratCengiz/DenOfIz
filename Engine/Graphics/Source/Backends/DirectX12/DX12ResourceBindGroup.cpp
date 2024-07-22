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

#include <DenOfIzGraphics/Backends/DirectX12/DX12ResourceBindGroup.h>

using namespace DenOfIz;

DX12ResourceBindGroup::DX12ResourceBindGroup(DX12Context *context, ResourceBindGroupDesc desc) : m_context(context), m_desc(desc)
{
    DX12RootSignature *rootSignature = static_cast<DX12RootSignature *>(desc.RootSignature);
    DZ_NOT_NULL(rootSignature);
    m_rootSignature = rootSignature;
}

void DX12ResourceBindGroup::Update(UpdateDesc desc)
{
    m_cbvSrvUavCount = 0;
    m_samplerCount   = 0;

    IResourceBindGroup::Update(desc);
}

void DX12ResourceBindGroup::BindTexture(ITextureResource *resource)
{
    DZ_NOT_NULL(resource);
    // Todo is this always the case?
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindBuffer(IBufferResource *resource)
{
    DZ_NOT_NULL(resource);
    // Todo is this always the case?
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindSampler(ISampler *sampler)
{
    DZ_NOT_NULL(sampler);
    m_samplerCount++;
}
