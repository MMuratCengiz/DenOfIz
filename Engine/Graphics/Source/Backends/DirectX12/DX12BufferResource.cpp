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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include "DenOfIzGraphics/Backends/DirectX12/DX12Fence.h"
#include "directxtk12/BufferHelpers.h"

using namespace DenOfIz;

DX12BufferResource::DX12BufferResource(DX12Context *context, const BufferDesc &desc) : m_context(context), m_desc(desc)
{
    m_stride                   = GetImageFormatSize(m_desc.Format);
    m_numBytes                 = m_desc.NumBytes;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::UnorderedAccess) )
    {
        flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::AccelerationStructure) )
    {
        flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(DX12DescriptorHeap::RoundUp(m_numBytes), flags);

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType                 = DX12EnumConverter::ConvertHeapType(m_desc.HeapType);
    // Remove the following line to once dependency to The Forge is removed !TF!
    allocationDesc.CreationNodeMask = 1;
    allocationDesc.VisibleNodeMask  = 1;
    // --

    D3D12_RESOURCE_STATES start_state = DX12EnumConverter::ConvertResourceState(m_desc.InitialState);

    HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, start_state, NULL, &m_allocation, IID_PPV_ARGS(&m_resource));
    THROW_IF_FAILED(hr);
    std::wstring name = std::wstring(Name.begin(), Name.end());
    m_resource->SetName(name.c_str());

    if ( !m_desc.Descriptor.None() )
    {
        CreateBufferView();
    }
}

void DX12BufferResource::CreateBufferView()
{

    std::unique_ptr<DX12DescriptorHeap> &heap = m_desc.HeapType == HeapType::CPU_GPU || m_desc.HeapType == HeapType::GPU
                                                    ? m_context->ShaderVisibleCbvSrvUavDescriptorHeap
                                                    : m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

    // The views here are set explicitly in CommandList::BindVertexBuffer and CommandList::BindIndexBuffer
    DZ_RETURN_IF(m_desc.Descriptor.Any({ ResourceDescriptor::VertexBuffer, ResourceDescriptor::IndexBuffer }));

    m_cpuHandle = heap->GetNextCPUHandleOffset(1);
    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::UniformBuffer) )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.BufferLocation                  = m_allocation->GetResource()->GetGPUVirtualAddress();
        desc.SizeInBytes                     = DX12DescriptorHeap::RoundUp(m_numBytes);
        m_context->D3DDevice->CreateConstantBufferView(&desc, m_cpuHandle);
        m_rootParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    }
    else
    {
        uint64_t stride = m_desc.BufferView.Stride;

        if ( m_desc.Descriptor.IsSet(ResourceDescriptor::UnorderedAccess) || m_desc.HeapType == HeapType::GPU )
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
            desc.Format                           = DX12EnumConverter::ConvertFormat(m_desc.Format);
            desc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement              = m_desc.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements         = m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.CounterOffsetInBytes = 0;
            desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
            m_context->D3DDevice->CreateUnorderedAccessView(m_resource.get(), nullptr, &desc, m_cpuHandle);
            m_rootParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        }
        else
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format                          = DX12EnumConverter::ConvertFormat(m_desc.Format);
            desc.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
            desc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Buffer.FirstElement             = m_desc.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements         = m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            m_context->D3DDevice->CreateShaderResourceView(m_resource.get(), &desc, m_cpuHandle);
            m_rootParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        }
    }
}

void DX12BufferResource::MapMemory()
{
    DZ_ASSERTM(m_mappedMemory == nullptr, std::format("Memory already mapped {}", Name.c_str()));
    THROW_IF_FAILED(m_resource->Map(0, NULL, &m_mappedMemory));
}

void DX12BufferResource::CopyData(const void *data, uint32_t size)
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped, buffer: {}", Name.c_str()));
    memcpy(m_mappedMemory, data, size);
}

void *DX12BufferResource::ReadData()
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped, buffer: {}", Name.c_str()));
    return m_mappedMemory;
}

void DX12BufferResource::UnmapMemory()
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped, buffer: {}", Name.c_str()));
    m_resource->Unmap(0, NULL);
    m_mappedMemory = nullptr;
}
DX12BufferResource::~DX12BufferResource()
{
    if ( m_mappedMemory != nullptr )
    {
        LOG(WARNING) << "Memory not unmapped before lifetime of the buffer.";
        m_resource->Unmap(0, NULL);
        m_mappedMemory = nullptr;
    }
}
