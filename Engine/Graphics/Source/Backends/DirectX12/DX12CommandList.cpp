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

DX12CommandList::DX12CommandList(DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, wil::com_ptr<ID3D12GraphicsCommandList> commandList,
                                 CommandListDesc desc) : m_context(context), m_commandAllocator(commandAllocator), m_desc(desc)
{
    commandList->QueryInterface(IID_PPV_ARGS(m_commandList.put()));

    switch ( desc.QueueType )
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

#ifndef NDEBUG
    m_commandList->QueryInterface(IID_PPV_ARGS(m_debugCommandList.put()));
#endif
}

DX12CommandList::~DX12CommandList()
{
}

void DX12CommandList::Begin()
{
    THROW_IF_FAILED(m_commandAllocator->Reset());
    THROW_IF_FAILED(m_commandList->Reset(m_commandAllocator.get(), nullptr));

    m_currentRootSignature = nullptr;
    if ( m_desc.QueueType != QueueType::Copy )
    {
        m_commandList->SetDescriptorHeaps(m_heaps.size(), m_heaps.data());
    }
}

void DX12CommandList::BeginRendering(const RenderingDesc &renderingInfo)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets(renderingInfo.RTAttachments.size());
    for ( int i = 0; i < renderingInfo.RTAttachments.size(); i++ )
    {
        DX12TextureResource *pImageResource = reinterpret_cast<DX12TextureResource *>(renderingInfo.RTAttachments[ i ].Resource);
        renderTargets[ i ]                  = pImageResource->GetCpuHandle();

        m_commandList->ClearRenderTargetView(renderTargets[ i ], renderingInfo.RTAttachments[ i ].ClearColor.data(), 0, nullptr);
    }

    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc = {};
    m_commandList->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), FALSE, nullptr);
}

void DX12CommandList::EndRendering()
{
}

void DX12CommandList::Execute(const ExecuteDesc &executeInfo)
{
    THROW_IF_FAILED(m_commandList->Close());

    for ( uint32_t i = 0; i < executeInfo.WaitOnSemaphores.size(); ++i )
    {
        m_commandQueue->Wait(reinterpret_cast<DX12Semaphore *>(executeInfo.WaitOnSemaphores[ i ])->GetFence(), 1);
    }
    m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.addressof()));
    for ( uint32_t i = 0; i < executeInfo.NotifySemaphores.size(); ++i )
    {
        m_commandQueue->Signal(reinterpret_cast<DX12Semaphore *>(executeInfo.NotifySemaphores[ i ])->GetFence(), 1);
    }
    if ( executeInfo.Notify != nullptr )
    {
        m_commandQueue->Signal(reinterpret_cast<DX12Fence *>(executeInfo.Notify)->GetFence(), 1);
    }
}

void DX12CommandList::Present(ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks)
{
    DZ_NOT_NULL(swapChain);

    DX12SwapChain *dx12SwapChain = reinterpret_cast<DX12SwapChain *>(swapChain);
    uint32_t       flags         = 0;
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
    m_currentRootSignature     = dx12Pipeline->GetRootSignature();

    if ( m_desc.QueueType == QueueType::Graphics )
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
    vertexBufferView.BufferLocation           = pBuffer->GetResource()->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes            = 8 * sizeof(float); // pBuffer->GetStride();
    vertexBufferView.SizeInBytes              = pBuffer->GetSize();

    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void DX12CommandList::BindIndexBuffer(IBufferResource *buffer, const IndexType &indexType)
{
    DZ_NOT_NULL(buffer);

    DX12BufferResource *pBuffer = reinterpret_cast<DX12BufferResource *>(buffer);

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation          = pBuffer->GetResource()->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes             = pBuffer->GetSize();
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

void DX12CommandList::BindResourceGroup(IResourceBindGroup *bindGroup)
{
    DX12ResourceBindGroup *pTable = reinterpret_cast<DX12ResourceBindGroup *>(bindGroup);
    SetRootSignature(pTable->GetRootSignature());

    for ( const RootParameterHandle &handle : pTable->GetDescriptorTableHandles() )
    {
        AddDescriptorTable(handle);
    }

    for ( const RootParameterHandle &handle : pTable->GetSamplerHandles() )
    {
        AddDescriptorTable(handle);
    }
}

void DX12CommandList::AddDescriptorTable(const RootParameterHandle &handle)
{
    switch ( m_desc.QueueType )
    {
    case Graphics:
        m_commandList->SetGraphicsRootDescriptorTable(handle.Index, handle.GpuHandle);
        break;
    case Compute:
        m_commandList->SetComputeRootDescriptorTable(handle.Index, handle.GpuHandle);
        break;
    default:
        LOG(ERROR) << "`BindResourceGroup` is an invalid function for queue type";
        break;
    }
}

void DX12CommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor)
{ /*Move to pipeline state due to reduces support*/
}

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

void DX12CommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    m_commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void DX12CommandList::CopyBufferRegion(const CopyBufferRegionDesc &copyBufferRegionInfo)
{
    DZ_NOT_NULL(copyBufferRegionInfo.DstBuffer);
    DZ_NOT_NULL(copyBufferRegionInfo.SrcBuffer);

    DX12BufferResource *dstBuffer = reinterpret_cast<DX12BufferResource *>(copyBufferRegionInfo.DstBuffer);
    DX12BufferResource *srcBuffer = reinterpret_cast<DX12BufferResource *>(copyBufferRegionInfo.SrcBuffer);

    m_commandList->CopyBufferRegion(dstBuffer->GetResource(), copyBufferRegionInfo.DstOffset, srcBuffer->GetResource(), copyBufferRegionInfo.SrcOffset,
                                    copyBufferRegionInfo.NumBytes);
}

void DX12CommandList::CopyTextureRegion(const CopyTextureRegionDesc &copyTextureRegionInfo)
{
    DZ_NOT_NULL(copyTextureRegionInfo.DstTexture);
    DZ_NOT_NULL(copyTextureRegionInfo.SrcTexture);

    DX12TextureResource *dstTexture = reinterpret_cast<DX12TextureResource *>(copyTextureRegionInfo.DstTexture);
    DX12TextureResource *srcTexture = reinterpret_cast<DX12TextureResource *>(copyTextureRegionInfo.SrcTexture);

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource                   = dstTexture->GetResource();
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyTextureRegionInfo.DstMipLevel;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource                   = srcTexture->GetResource();
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex            = copyTextureRegionInfo.SrcMipLevel;

    D3D12_BOX box = {};
    box.left      = copyTextureRegionInfo.SrcX;
    box.top       = copyTextureRegionInfo.SrcY;
    box.front     = copyTextureRegionInfo.SrcZ;
    box.right     = copyTextureRegionInfo.SrcX + copyTextureRegionInfo.Width;
    box.bottom    = copyTextureRegionInfo.SrcY + copyTextureRegionInfo.Height;
    box.back      = copyTextureRegionInfo.SrcZ + copyTextureRegionInfo.Depth;

    m_commandList->CopyTextureRegion(&dst, copyTextureRegionInfo.DstX, copyTextureRegionInfo.DstY, copyTextureRegionInfo.DstZ, &src, &box);
}

void DX12CommandList::CopyBufferToTexture(const CopyBufferToTextureDesc &copyBufferToTexture)
{
    DZ_NOT_NULL(copyBufferToTexture.DstTexture);
    DZ_NOT_NULL(copyBufferToTexture.SrcBuffer);

    DX12TextureResource *dstTexture = reinterpret_cast<DX12TextureResource *>(copyBufferToTexture.DstTexture);
    DX12BufferResource  *srcBuffer  = reinterpret_cast<DX12BufferResource *>(copyBufferToTexture.SrcBuffer);

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource                   = dstTexture->GetResource();
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyBufferToTexture.MipLevel;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource                   = srcBuffer->GetResource();
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Offset      = copyBufferToTexture.SrcOffset;
    src.PlacedFootprint.Footprint.Format   = DX12EnumConverter::ConvertFormat(copyBufferToTexture.Format);
    src.PlacedFootprint.Footprint.Width    = copyBufferToTexture.Width;
    src.PlacedFootprint.Footprint.Height   = copyBufferToTexture.Height;
    src.PlacedFootprint.Footprint.Depth    = copyBufferToTexture.Depth;
    src.PlacedFootprint.Footprint.RowPitch = DX12DescriptorHeap::RoundUp(copyBufferToTexture.Width * sizeof(DWORD), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

    uint32_t subresource = copyBufferToTexture.ArrayLayer * dstTexture->GetDesc().MipLevels + copyBufferToTexture.MipLevel;
    m_context->D3DDevice->GetCopyableFootprints(&dstTexture->GetResourceDesc(), subresource, 1, copyBufferToTexture.SrcOffset, &src.PlacedFootprint, NULL, NULL, NULL);

    m_commandList->CopyTextureRegion(&dst, copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ, &src, nullptr);
}

void DX12CommandList::CompatibilityPipelineBarrier(const PipelineBarrier &barrier)
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    for ( const TextureBarrierDesc &imageBarrier : barrier.GetTextureBarriers() )
    {
        ID3D12Resource        *pResource       = reinterpret_cast<DX12TextureResource *>(imageBarrier.Resource)->GetResource();
        D3D12_RESOURCE_STATES  before          = DX12EnumConverter::ConvertResourceState(imageBarrier.OldState);
        D3D12_RESOURCE_STATES  after           = DX12EnumConverter::ConvertResourceState(imageBarrier.NewState);
        D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after);

        if ( before != after )
        {
            resourceBarriers.push_back(resourceBarrier);
        }
    }

    for ( const BufferBarrierDesc &bufferBarrier : barrier.GetBufferBarriers() )
    {
        ID3D12Resource        *pResource       = reinterpret_cast<DX12TextureResource *>(bufferBarrier.Resource)->GetResource();
        D3D12_RESOURCE_STATES  before          = DX12EnumConverter::ConvertResourceState(bufferBarrier.OldState);
        D3D12_RESOURCE_STATES  after           = DX12EnumConverter::ConvertResourceState(bufferBarrier.NewState);
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

    std::vector<D3D12_GLOBAL_BARRIER>  dxGlobalBarriers  = {};
    std::vector<D3D12_BUFFER_BARRIER>  dxBufferBarriers  = {};
    std::vector<D3D12_TEXTURE_BARRIER> dxTextureBarriers = {};

    for ( const TextureBarrierDesc &textureBarrier : barrier.GetTextureBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12TextureResource *>(textureBarrier.Resource)->GetResource();

        D3D12_TEXTURE_BARRIER dxTextureBarrier = dxTextureBarriers.emplace_back(D3D12_TEXTURE_BARRIER{});
        dxTextureBarrier.pResource             = pResource;
        dxTextureBarrier.LayoutBefore          = DX12EnumConverter::ConvertResourceStateToBarrierLayout(textureBarrier.OldState, m_desc.QueueType);
        dxTextureBarrier.LayoutAfter           = DX12EnumConverter::ConvertResourceStateToBarrierLayout(textureBarrier.NewState, m_desc.QueueType);
        dxTextureBarrier.AccessBefore          = DX12EnumConverter::ConvertResourceStateToBarrierAccess(textureBarrier.OldState);
        dxTextureBarrier.AccessAfter           = DX12EnumConverter::ConvertResourceStateToBarrierAccess(textureBarrier.NewState);
        // Todo dxTextureBarrier.Subresource, dxTextureBarrier.SyncBefore and dxTextureBarrier.SyncAfter

        if ( dxTextureBarrier.LayoutAfter != dxTextureBarrier.LayoutBefore || dxTextureBarrier.AccessAfter != dxTextureBarrier.AccessBefore )
        {
            dxTextureBarriers.push_back(dxTextureBarrier);
        }
    }

    for ( const BufferBarrierDesc &bufferBarrier : barrier.GetBufferBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12TextureResource *>(bufferBarrier.Resource)->GetResource();

        D3D12_BUFFER_BARRIER dxBufferBarrier = dxBufferBarriers.emplace_back(D3D12_BUFFER_BARRIER{});
        dxBufferBarrier.pResource            = pResource;
        dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess(bufferBarrier.OldState);
        dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess(bufferBarrier.NewState);
        dxBufferBarrier.Offset               = 0;
        dxBufferBarrier.Size                 = pResource->GetDesc().Width;

        if ( dxBufferBarrier.AccessAfter != dxBufferBarrier.AccessBefore )
        {
            dxBufferBarriers.push_back(dxBufferBarrier);
        }
    }

    D3D12_BARRIER_GROUP textureBarrierGroup = resourceBarriers.emplace_back(D3D12_BARRIER_GROUP{});
    textureBarrierGroup.Type                = D3D12_BARRIER_TYPE_TEXTURE;
    textureBarrierGroup.NumBarriers         = dxTextureBarriers.size();
    textureBarrierGroup.pTextureBarriers    = dxTextureBarriers.data();

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
        LOG(WARNING) << "Root signature is set to a different value, it is not expected to overwrite this value.";
    }

    m_currentRootSignature = rootSignature;
    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        m_commandList->SetGraphicsRootSignature(rootSignature);
        break;
    case QueueType::Compute:
        m_commandList->SetComputeRootSignature(rootSignature);
        break;
    default:
        LOG(ERROR) << "SetRootSignature is an invalid function for queue type";
        break;
    }
}
