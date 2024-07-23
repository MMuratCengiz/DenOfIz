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

    m_cbvSrvUavHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetNextHandle(m_desc.MaxNumBuffers + m_desc.MaxNumTextures);
    m_samplerHandle   = m_context->ShaderVisibleSamplerDescriptorHeap->GetNextHandle(m_desc.MaxNumSamplers);
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
    uint32_t offset = m_rootSignature->GetResourceOffset(resource->Name);
    reinterpret_cast<DX12TextureResource *>(resource)->CreateView(CpuHandleCbvSrvUav(offset));
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindBuffer(IBufferResource *resource)
{
    DZ_NOT_NULL(resource);
    // Todo is this always the case?
    uint32_t offset = m_rootSignature->GetResourceOffset(resource->Name);
    reinterpret_cast<DX12BufferResource *>(resource)->CreateView(CpuHandleCbvSrvUav(offset));
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindSampler(ISampler *sampler)
{
    DZ_NOT_NULL(sampler);
    uint32_t offset = m_rootSignature->GetResourceOffset(sampler->Name);
    reinterpret_cast<DX12Sampler *>(sampler)->CreateView(CpuHandleSampler(offset));
    m_samplerCount++;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ResourceBindGroup::CpuHandleCbvSrvUav(uint32_t binding)
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvUavHandle.Cpu, binding, m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetDescriptorSize());
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ResourceBindGroup::CpuHandleSampler(uint32_t binding)
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_samplerHandle.Cpu, binding, m_context->ShaderVisibleSamplerDescriptorHeap->GetDescriptorSize());
}
