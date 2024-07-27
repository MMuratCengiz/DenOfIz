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

DX12ResourceBindGroup::DX12ResourceBindGroup(DX12Context *context, ResourceBindGroupDesc desc) : IResourceBindGroup(desc), m_context(context)
{
    DX12RootSignature *rootSignature = static_cast<DX12RootSignature *>(desc.RootSignature);
    DZ_NOT_NULL(rootSignature);
    m_dx12RootSignature = rootSignature;

    m_offset = rootSignature->RegisterSpaceOffset(desc.RegisterSpace);
    if ( desc.NumBuffers + desc.NumTextures > 0 )
    {
        m_cbvSrvUavHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetNextHandle(m_desc.NumBuffers + m_desc.NumTextures);
    }
    if ( desc.NumSamplers > 0 )
    {
        m_samplerHandle = m_context->ShaderVisibleSamplerDescriptorHeap->GetNextHandle(m_desc.NumSamplers);
    }
}

void DX12ResourceBindGroup::Update(UpdateDesc desc)
{
    m_cbvSrvUavCount = 0;
    m_samplerCount   = 0;

    DZ_ASSERTM(desc.Buffers.size()  == m_desc.NumBuffers,  "Number of buffers being updated do not match.");
    DZ_ASSERTM(desc.Textures.size() == m_desc.NumTextures, "Number of textures being updated do not match.");
    DZ_ASSERTM(desc.Samplers.size() == m_desc.NumSamplers, "Number of sampler being updated do not match.");

    IResourceBindGroup::Update(desc);
}

void DX12ResourceBindGroup::BindTexture(const std::string &name, ITextureResource *resource)
{
    DZ_NOT_NULL(resource);
    uint32_t offset  = m_rootSignature->GetResourceOffset(m_desc.RegisterSpace, name);
    reinterpret_cast<DX12TextureResource *>(resource)->CreateView(CpuHandleCbvSrvUav(offset));
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindBuffer(const std::string &name, IBufferResource *resource)
{
    DZ_NOT_NULL(resource);
    uint32_t offset  = m_rootSignature->GetResourceOffset(m_desc.RegisterSpace, name);
    reinterpret_cast<DX12BufferResource *>(resource)->CreateView(CpuHandleCbvSrvUav(offset));
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindSampler(const std::string &name, ISampler *sampler)
{
    DZ_NOT_NULL(sampler);
    uint32_t offset  = m_rootSignature->GetResourceOffset(m_desc.RegisterSpace, name);
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
