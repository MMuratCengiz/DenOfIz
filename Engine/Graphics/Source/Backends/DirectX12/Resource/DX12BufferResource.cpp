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

#include <DenOfIzGraphics/Backends/DirectX12/Resource/DX12BufferResource.h>

using namespace DenOfIz;

DX12BufferResource::DX12BufferResource(DX12Context *context, const BufferCreateInfo &createInfo) : m_context(context), m_createInfo(createInfo) {}

void DX12BufferResource::Allocate(const void *data)
{
    bool useStagingBuffer = m_createInfo.HeapType == HeapType::GPU_CPU || m_createInfo.HeapType == HeapType::GPU;

    ID3D12Resource2 *stagingBuffer = nullptr;
    if ( useStagingBuffer )
    {
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size, D3D12_RESOURCE_FLAG_NONE, 0);

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = DX12EnumConverter::ConvertHeapType(HeapType::CPU);

        HRESULT hr =
            m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &m_allocation, IID_PPV_ARGS(&stagingBuffer));
        DX_CHECK_RESULT(hr);

        void *mappedMemory = nullptr;
        hr = stagingBuffer->Map(0, NULL, &mappedMemory);
        DX_CHECK_RESULT(hr);
        memcpy(mappedMemory, data, m_size);
        stagingBuffer->Unmap(0, NULL);
    }

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size, D3D12_RESOURCE_FLAG_NONE);

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = DX12EnumConverter::ConvertHeapType(m_createInfo.HeapType);

    D3D12_RESOURCE_STATES start_state = D3D12_RESOURCE_STATE_COMMON;
    if ( m_createInfo.HeapType == HeapType::CPU_GPU || m_createInfo.HeapType == HeapType::CPU )
    {
        start_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    else if ( useStagingBuffer )
    {
        start_state = D3D12_RESOURCE_STATE_COPY_DEST;
    }

    HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, start_state, NULL, &m_allocation, IID_PPV_ARGS(&m_resource));
    DX_CHECK_RESULT(hr);

    if ( useStagingBuffer )
    {
        m_context->CopyCommandList->CopyBufferRegion(m_resource, 0, stagingBuffer, 0, m_size);
        stagingBuffer->Release();
    }
    else
    {
        if ( m_createInfo.KeepMemoryMapped && m_mappedMemory == nullptr )
        {
            hr = m_resource->Map(0, NULL, &m_mappedMemory);
            DX_CHECK_RESULT(hr);
        }
        memcpy(m_mappedMemory, data, m_size);
        if ( !m_createInfo.KeepMemoryMapped )
        {
            m_resource->Unmap(0, NULL);
            m_mappedMemory = nullptr;
        }
    }

    CreateBufferView();
}

void DX12BufferResource::CreateBufferView()
{
    std::unique_ptr<DX12DescriptorHeap> &heap = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

    if ( m_createInfo.Usage.UniformBuffer )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.BufferLocation = m_resource->GetGPUVirtualAddress();
        desc.SizeInBytes = m_size;
        m_context->D3DDevice->CreateConstantBufferView(&desc, heap->GetNextCPUHandleOffset(1));
    }
    else
    {
        uint32_t stride = m_createInfo.BufferView.Stride;

        if ( !m_createInfo.Usage.ReadWrite )
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.Format = DX12EnumConverter::ConvertImageFormat(m_createInfo.Format);
            desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Buffer.FirstElement = m_createInfo.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements = m_size / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            m_context->D3DDevice->CreateShaderResourceView(m_resource, &desc, heap->GetNextCPUHandleOffset(1));
        }
        else
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
            desc.Format = DX12EnumConverter::ConvertImageFormat(m_createInfo.Format);
            desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement = m_createInfo.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements = m_size / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.CounterOffsetInBytes = 0;
            desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            m_context->D3DDevice->CreateUnorderedAccessView(m_resource, nullptr, &desc, heap->GetNextCPUHandleOffset(1));
        }
    }
}

void DX12BufferResource::Deallocate()
{
    if ( m_mappedMemory != nullptr && m_createInfo.KeepMemoryMapped )
    {
        m_resource->Unmap(0, NULL);
        m_mappedMemory = nullptr;
    }

    m_allocation->Release();
    m_resource->Release();
}

DX12BufferResource::~DX12BufferResource()
{
    if ( m_resource != nullptr )
    {
        Deallocate();
    }
}
