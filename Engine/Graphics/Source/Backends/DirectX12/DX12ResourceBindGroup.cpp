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

DX12ResourceBindGroup::DX12ResourceBindGroup(DX12Context *context, ResourceBindGroupDesc desc) : m_context(context)
{
    DX12RootSignature *rootSignature = static_cast<DX12RootSignature *>(desc.RootSignature);
    DZ_NOT_NULL(rootSignature);

    m_rootSignature = rootSignature->GetRootSignature();
}

void DX12ResourceBindGroup::BindTexture(ITextureResource *resource)
{
    DZ_NOT_NULL(resource);

    DX12TextureResource *dxResource = static_cast<DX12TextureResource *>(resource);

    uint64_t index = m_cbvSrvUavParams.size();
    std::unique_ptr<DX12DescriptorHeap> &heap = m_context->ShaderVisibleCbvSrvUavDescriptorHeap;

    RootParameterHandle& handle = m_cbvSrvUavParams.emplace_back(RootParameterHandle{});
    handle.Type      = dxResource->GetRootParameterType();
    handle.Index     = index;
    handle.GpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUStartHandle().ptr + index * heap->GetDescriptorSize());
}

void DX12ResourceBindGroup::BindBuffer(IBufferResource *resource)
{
    DZ_NOT_NULL(resource);

    DX12BufferResource *dxResource = static_cast<DX12BufferResource *>(resource);
    m_resources.push_back(dxResource->GetResource());

    uint64_t index = m_cbvSrvUavParams.size();
    std::unique_ptr<DX12DescriptorHeap> &heap = m_context->ShaderVisibleCbvSrvUavDescriptorHeap;

    RootParameterHandle& handle = m_cbvSrvUavParams.emplace_back(RootParameterHandle{});
    handle.Type      = dxResource->GetRootParameterType();
    handle.Index     = index;
    handle.GpuHandle = D3D12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUStartHandle().ptr + index * heap->GetDescriptorSize());
}
