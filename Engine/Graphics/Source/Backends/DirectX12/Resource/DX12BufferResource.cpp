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
#include "DenOfIzGraphics/Backends/DirectX12/Resource/DX12Fence.h"
#include "directxtk12/BufferHelpers.h"

using namespace DenOfIz;

DX12BufferResource::DX12BufferResource(DX12Context *context, const BufferCreateInfo &createInfo) : m_context(context), m_createInfo(createInfo)
{
    m_stride = GetImageFormatSize(m_createInfo.Format);
}

void DX12BufferResource::Allocate(const void *data)
{
    if ( m_createInfo.KeepMemoryMapped && allocated )
    {
        DZ_NOT_NULL(m_mappedMemory);
        memcpy(m_mappedMemory, data, m_size);
        return;
    }

    bool useStaging = m_createInfo.HeapType == HeapType::GPU_CPU || m_createInfo.HeapType == HeapType::GPU;
    ID3D12Resource2 *stagingBuffer = nullptr;
    D3D12MA::Allocation *stagingAllocation = nullptr;

    if ( useStaging )
    {
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size, D3D12_RESOURCE_FLAG_NONE, 0);

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = DX12EnumConverter::ConvertHeapType(HeapType::CPU);
        // Remove the following line to once dependency to The Forge is removed !TF!
        allocationDesc.CreationNodeMask = 1;
        allocationDesc.VisibleNodeMask = 1;
        // --

        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, &stagingAllocation,
                                                                    IID_PPV_ARGS(&stagingBuffer));
        THROW_IF_FAILED(hr);

        void *mappedMemory = nullptr;
        hr = stagingBuffer->Map(0, NULL, &mappedMemory);
        THROW_IF_FAILED(hr);
        memcpy(mappedMemory, data, m_size);
        stagingBuffer->Unmap(0, NULL);
    }

    if ( !allocated )
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if ( m_createInfo.Usage.ReadWrite )
        {
            flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        if ( m_createInfo.Usage.AccelerationStructureScratch || m_createInfo.Usage.BottomLevelAccelerationStructureInput || m_createInfo.Usage.TopLevelAccelerationStructureInput )
        {
            flags = D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(DX12DescriptorHeap::RoundUp(m_size), flags);

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = DX12EnumConverter::ConvertHeapType(m_createInfo.HeapType);

        D3D12_RESOURCE_STATES start_state = D3D12_RESOURCE_STATE_COMMON;
        if ( m_createInfo.HeapType == HeapType::CPU_GPU || m_createInfo.HeapType == HeapType::CPU )
        {
            start_state = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else if ( m_createInfo.HeapType == HeapType::GPU_CPU || m_createInfo.HeapType == HeapType::GPU )
        {
            start_state = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        // Remove the following line to once dependency to The Forge is removed !TF!
        allocationDesc.CreationNodeMask = 1;
        allocationDesc.VisibleNodeMask = 1;
        // --

        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, start_state, NULL, &m_allocation, IID_PPV_ARGS(&m_resource));
        THROW_IF_FAILED(hr);
        std::wstring name = std::wstring(Name.begin(), Name.end());
        m_resource->SetName(name.c_str());
    }

    if ( useStaging )
    { // !IMPROVEMENT! Consider removing abstraction in future
        //        const CD3DX12_RESOURCE_BARRIER &toGenericRead = CD3DX12_RESOURCE_BARRIER::Transition(m_resource, D3D12_RESOURCE_STATE_COPY_DEST,
        //        D3D12_RESOURCE_STATE_GENERIC_READ); m_context->CopyCommandList->ResourceBarrier(1, &toGenericRead);
        DX12Fence fence(m_context);
        m_context->CopyCommandListAllocator->Reset();
        m_context->CopyCommandList->Reset(m_context->CopyCommandListAllocator.get(), nullptr);
        m_context->CopyCommandList->CopyBufferRegion(m_resource.get(), 0, stagingBuffer, 0, m_size);
        m_context->CopyCommandList->Close();
        m_context->CopyCommandQueue->ExecuteCommandLists(1, CommandListCast(m_context->CopyCommandList.addressof()));
        m_context->CopyCommandQueue->Signal(fence.GetFence(), 1);
        fence.Wait();

        stagingBuffer->Release();
        stagingAllocation->Release();
    }
    else
    {
        THROW_IF_FAILED(m_resource->Map(0, NULL, &m_mappedMemory));
        memcpy(m_mappedMemory, data, m_size);
        if ( !m_createInfo.KeepMemoryMapped )
        {
            m_resource->Unmap(0, NULL);
            m_mappedMemory = nullptr;
        }
    }

    if ( !allocated )
    {
        CreateBufferView();
        allocated = true;
    }
}

void DX12BufferResource::CreateBufferView()
{
    std::unique_ptr<DX12DescriptorHeap> &heap = m_createInfo.HeapType == HeapType::CPU_GPU || m_createInfo.HeapType == HeapType::GPU
        ? m_context->ShaderVisibleCbvSrvUavDescriptorHeap
        : m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

    // The views here are set explicitly in CommandList::BindVertexBuffer and CommandList::BindIndexBuffer
    DZ_RETURN_IF(m_createInfo.Usage.VertexBuffer || m_createInfo.Usage.IndexBuffer);

    if ( m_createInfo.Usage.UniformBuffer )
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
        desc.BufferLocation = m_allocation->GetResource()->GetGPUVirtualAddress();
        desc.SizeInBytes = DX12DescriptorHeap::RoundUp(m_size);
        m_context->D3DDevice->CreateConstantBufferView(&desc, heap->GetCPUStartHandle());
    }
    else
    {
        uint64_t stride = m_createInfo.BufferView.Stride;

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
            m_context->D3DDevice->CreateShaderResourceView(m_resource.get(), &desc, heap->GetCPUStartHandle());
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
            m_context->D3DDevice->CreateUnorderedAccessView(m_resource.get(), nullptr, &desc, heap->GetCPUStartHandle());
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

    if ( allocated )
    {
        //        m_resource = nullptr;
        //        m_allocation = nullptr;
        //        m_resource->Release();
        //        m_allocation->Release();
    }
}

DX12BufferResource::~DX12BufferResource()
{
    if ( m_resource != nullptr )
    {
        Deallocate();
    }
}
