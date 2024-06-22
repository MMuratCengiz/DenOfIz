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

#include <DenOfIzGraphics/Backends/DirectX12/DX12CommandList.h>

using namespace DenOfIz;

DX12CommandList::DX12CommandList(DX12Context *context, CommandListCreateInfo createInfo) : m_context(context), m_createInfo(createInfo)
{
    switch ( createInfo.QueueType )
    {
    case QueueType::Presentation:
    case QueueType::Graphics:
        m_commandQueue = m_context->GraphicsCommandQueue.get();
        break;
    case QueueType::Compute:
        m_commandQueue = m_context->ComputeCommandQueue.get();
        break;
    case QueueType::Copy:
        m_commandQueue = m_context->CopyCommandQueue.get();
        break;
    }

    D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType(m_createInfo.QueueType);

    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(m_commandAllocator.put())));
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandList(0, commandListType, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(m_commandList.put())));
#ifndef NDEBUG
    m_commandList->QueryInterface(IID_PPV_ARGS(m_debugCommandList.put()));
#endif
    m_commandList->Close();
}

DX12CommandList::~DX12CommandList()
{
    //    DX_SAFE_RELEASE(m_commandList);
    //    DX_SAFE_RELEASE(m_commandAllocator);
}

void DX12CommandList::Begin()
{
    THROW_IF_FAILED(m_commandAllocator->Reset());
    THROW_IF_FAILED(m_commandList->Reset(m_commandAllocator.get(), nullptr));

    m_currentRootSignature = nullptr;
    m_commandList->SetDescriptorHeaps(m_heaps.size(), m_heaps.data());
}

void DX12CommandList::BeginRendering(const RenderingInfo &renderingInfo)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets(renderingInfo.RTAttachments.size());
    for ( int i = 0; i < renderingInfo.RTAttachments.size(); i++ )
    {
        DX12ImageResource *pImageResource = reinterpret_cast<DX12ImageResource *>(renderingInfo.RTAttachments[ i ].Resource);
        renderTargets[ i ] = pImageResource->GetCpuHandle();

        m_commandList->ClearRenderTargetView(renderTargets[ i ], renderingInfo.RTAttachments[ i ].ClearColor.data(), 0, nullptr);
    }

    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc = {};
    m_commandList->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), FALSE, nullptr);
}

void DX12CommandList::EndRendering() {}

void DX12CommandList::Execute(const ExecuteInfo &executeInfo)
{
    THROW_IF_FAILED(m_commandList->Close());

    for ( uint32_t i = 0; i < executeInfo.WaitOnLocks.size(); ++i )
    {
        m_commandQueue->Wait(reinterpret_cast<DX12Semaphore *>(executeInfo.WaitOnLocks[ i ])->GetFence(), 1);
    }
    m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.addressof()));
    for ( uint32_t i = 0; i < executeInfo.SignalLocks.size(); ++i )
    {
        m_commandQueue->Signal(reinterpret_cast<DX12Semaphore *>(executeInfo.SignalLocks[ i ])->GetFence(), 1);
    }
    m_commandQueue->Signal(reinterpret_cast<DX12Fence *>(executeInfo.Notify)->GetFence(), 1);
}

void DX12CommandList::Present(ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks)
{
    DZ_NOT_NULL(swapChain);

    DX12SwapChain *dx12SwapChain = reinterpret_cast<DX12SwapChain *>(swapChain);
    uint32_t flags = 0;
    if ( m_context->SelectedDeviceInfo.Capabilities.Tearing )
    {
        flags |= DXGI_PRESENT_ALLOW_TEARING;
    }
    dx12SwapChain->GetSwapChain()->Present(0, flags);
}

void DX12CommandList::BindPipeline(IPipeline *pipeline)
{
    DZ_NOT_NULL(pipeline);

    DX12Pipeline *dx12Pipeline = reinterpret_cast<DX12Pipeline *>(pipeline);
    m_currentRootSignature = dx12Pipeline->GetRootSignature();

    if ( m_createInfo.QueueType == QueueType::Graphics )
    {
        m_commandList->SetGraphicsRootSignature(dx12Pipeline->GetRootSignature());
        m_commandList->IASetPrimitiveTopology(dx12Pipeline->GetTopology());
        m_commandList->SetPipelineState(dx12Pipeline->GetPipeline());
    }
    else
    {
        m_commandList->SetComputeRootSignature(dx12Pipeline->GetRootSignature());
        m_commandList->SetPipelineState(dx12Pipeline->GetPipeline());
    }
}

void DX12CommandList::BindVertexBuffer(IBufferResource *buffer)
{
    DZ_NOT_NULL(buffer);

    DX12BufferResource *pBuffer = reinterpret_cast<DX12BufferResource *>(buffer);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = pBuffer->GetResource()->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = 8 * sizeof(float); // pBuffer->GetStride();
    vertexBufferView.SizeInBytes = pBuffer->GetSize();

    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void DX12CommandList::BindIndexBuffer(IBufferResource *buffer, const IndexType &indexType)
{
    DZ_NOT_NULL(buffer);

    DX12BufferResource *pBuffer = reinterpret_cast<DX12BufferResource *>(buffer);

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation = pBuffer->GetResource()->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = pBuffer->GetSize();
    switch ( indexType )
    {
    case IndexType::Uint16:
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        break;
    case IndexType::Uint32:
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        break;
    }
    m_commandList->IASetIndexBuffer(&indexBufferView);
}

void DX12CommandList::BindViewport(float x, float y, float width, float height)
{
    m_viewport = CD3DX12_VIEWPORT(x, y, width, height);
    m_commandList->RSSetViewports(1, &m_viewport);
}

void DX12CommandList::BindScissorRect(float x, float y, float width, float height)
{
    m_scissor = CD3DX12_RECT(x, y, x + width, y + height);
    m_commandList->RSSetScissorRects(1, &m_scissor);
}

void DX12CommandList::BindDescriptorTable(IDescriptorTable *table)
{
    DX12DescriptorTable *pTable = reinterpret_cast<DX12DescriptorTable *>(table);
    SetRootSignature(pTable->GetRootSignature());
}

void DX12CommandList::BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void *data) {}

void DX12CommandList::BindBufferResource(IBufferResource *resource) {}

void DX12CommandList::BindImageResource(ITextureResource *resource) {}

void DX12CommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor) { /*Move to pipeline state due to reduces support*/ }

void DX12CommandList::SetPipelineBarrier(const PipelineBarrier &barrier)
{
    if ( m_context->DX12Capabilities.EnhancedBarriers )
    {
        EnhancedPipelineBarrier(barrier);
    }
    else
    {
        CompatibilityPipelineBarrier(barrier);
    }
}

void DX12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    m_commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void DX12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void DX12CommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) { m_commandList->Dispatch(groupCountX, groupCountY, groupCountZ); }

void DX12CommandList::TransitionImageLayout(ITextureResource *image, ImageLayout oldLayout, ImageLayout newLayout) {}

void DX12CommandList::CompatibilityPipelineBarrier(const PipelineBarrier &barrier)
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    for ( const TextureBarrierInfo &imageBarrier : barrier.GetTextureBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(imageBarrier.Resource)->GetResource();
        D3D12_RESOURCE_STATES before = DX12EnumConverter::ConvertResourceState(imageBarrier.OldState);
        D3D12_RESOURCE_STATES after = DX12EnumConverter::ConvertResourceState(imageBarrier.NewState);
        D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after);

        if ( before != after )
        {
            resourceBarriers.push_back(resourceBarrier);
        }
    }

    for ( const BufferBarrierInfo &bufferBarrier : barrier.GetBufferBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(bufferBarrier.Resource)->GetResource();
        D3D12_RESOURCE_STATES before = DX12EnumConverter::ConvertResourceState(bufferBarrier.OldState);
        D3D12_RESOURCE_STATES after = DX12EnumConverter::ConvertResourceState(bufferBarrier.NewState);
        D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after);
        if ( before != after )
        {
            resourceBarriers.push_back(resourceBarrier);
        }
    }

    if ( resourceBarriers.size() > 0 )
    {
        m_commandList->ResourceBarrier(resourceBarriers.size(), resourceBarriers.data());
    }
}

void DX12CommandList::EnhancedPipelineBarrier(const PipelineBarrier &barrier)
{
    std::vector<D3D12_BARRIER_GROUP> resourceBarriers;

    std::vector<D3D12_GLOBAL_BARRIER> dxGlobalBarriers = {};
    std::vector<D3D12_BUFFER_BARRIER> dxBufferBarriers = {};
    std::vector<D3D12_TEXTURE_BARRIER> dxTextureBarriers = {};

    for ( const TextureBarrierInfo &textureBarrier : barrier.GetTextureBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(textureBarrier.Resource)->GetResource();

        D3D12_TEXTURE_BARRIER dxTextureBarrier = dxTextureBarriers.emplace_back(D3D12_TEXTURE_BARRIER{});
        dxTextureBarrier.pResource = pResource;
        dxTextureBarrier.LayoutBefore = DX12EnumConverter::ConvertResourceStateToBarrierLayout(textureBarrier.OldState, m_createInfo.QueueType);
        dxTextureBarrier.LayoutAfter = DX12EnumConverter::ConvertResourceStateToBarrierLayout(textureBarrier.NewState, m_createInfo.QueueType);
        dxTextureBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess(textureBarrier.OldState);
        dxTextureBarrier.AccessAfter = DX12EnumConverter::ConvertResourceStateToBarrierAccess(textureBarrier.NewState);
        // Todo dxTextureBarrier.Subresource, dxTextureBarrier.SyncBefore and dxTextureBarrier.SyncAfter

        if ( dxTextureBarrier.LayoutAfter != dxTextureBarrier.LayoutBefore || dxTextureBarrier.AccessAfter != dxTextureBarrier.AccessBefore )
        {
            dxTextureBarriers.push_back(dxTextureBarrier);
        }
    }

    for ( const BufferBarrierInfo &bufferBarrier : barrier.GetBufferBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(bufferBarrier.Resource)->GetResource();

        D3D12_BUFFER_BARRIER dxBufferBarrier = dxBufferBarriers.emplace_back(D3D12_BUFFER_BARRIER{});
        dxBufferBarrier.pResource = pResource;
        dxBufferBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess(bufferBarrier.OldState);
        dxBufferBarrier.AccessAfter = DX12EnumConverter::ConvertResourceStateToBarrierAccess(bufferBarrier.NewState);
        dxBufferBarrier.Offset = 0;
        dxBufferBarrier.Size = pResource->GetDesc().Width;

        if ( dxBufferBarrier.AccessAfter != dxBufferBarrier.AccessBefore )
        {
            dxBufferBarriers.push_back(dxBufferBarrier);
        }
    }

    D3D12_BARRIER_GROUP textureBarrierGroup = resourceBarriers.emplace_back(D3D12_BARRIER_GROUP{});
    textureBarrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
    textureBarrierGroup.NumBarriers = dxTextureBarriers.size();
    textureBarrierGroup.pTextureBarriers = dxTextureBarriers.data();

    if ( resourceBarriers.size() > 0 )
    {
        m_commandList->Barrier(resourceBarriers.size(), resourceBarriers.data());
    }
}
void DX12CommandList::SetRootSignature(ID3D12RootSignature *rootSignature)
{
    DZ_RETURN_IF(rootSignature == nullptr);

    if ( m_currentRootSignature != nullptr && rootSignature != m_currentRootSignature )
    {
        LOG(WARNING) << "DX12CommandList" << "Root signature is set to a different value, it is not expected to overwrite this value.";
    }
    m_currentRootSignature = rootSignature;
    if ( m_createInfo.QueueType == QueueType::Graphics )
    {
        m_commandList->SetGraphicsRootSignature(rootSignature);
        // TODO! Validate these for all cases
        m_commandList->SetGraphicsRootDescriptorTable(0, m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
    }
    else
    {
        m_commandList->SetComputeRootSignature(rootSignature);
        m_commandList->SetComputeRootDescriptorTable(0, m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart());
    }
}
