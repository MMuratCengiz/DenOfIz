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
    D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType(m_createInfo.QueueType);

    m_context->D3DDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&m_commandAllocator));
    m_context->D3DDevice->CreateCommandList(0, commandListType, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
}

void DX12CommandList::Begin()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);

    m_currentRootSignature = nullptr;
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

void DX12CommandList::EndRendering() { m_commandList->EndRenderPass(); }

void DX12CommandList::Execute(const ExecuteInfo &executeInfo)
{
    DX_CHECK_RESULT(m_commandList->Close());
    m_context->GraphicsCommandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));
}

void DX12CommandList::Present(ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks)
{
    DX12SwapChain *dx12SwapChain = reinterpret_cast<DX12SwapChain *>(swapChain);
    dx12SwapChain->GetSwapChain()->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}

void DX12CommandList::BindPipeline(IPipeline *pipeline)
{
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
    DX12BufferResource *pBuffer = reinterpret_cast<DX12BufferResource *>(buffer);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = pBuffer->GetResource()->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = pBuffer->GetSize();
    vertexBufferView.SizeInBytes = pBuffer->GetSize();

    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void DX12CommandList::BindIndexBuffer(IBufferResource *buffer, const IndexType &indexType)
{
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

void DX12CommandList::BindImageResource(IImageResource *resource) {}

void DX12CommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor) { m_commandList->RSSetDepthBias(constantFactor, clamp, slopeFactor); }

void DX12CommandList::SetPipelineBarrier(const PipelineBarrier &barrier)
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    for ( const ImageBarrierInfo &imageBarrier : barrier.GetImageBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(imageBarrier.Resource)->GetResource();
        D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, DX12EnumConverter::ConvertResourceState(imageBarrier.OldState),
                                                                                      DX12EnumConverter::ConvertResourceState(imageBarrier.NewState));
        resourceBarriers.push_back(resourceBarrier);
    }

    for ( const BufferBarrierInfo &bufferBarrier : barrier.GetBufferBarriers() )
    {
        ID3D12Resource *pResource = reinterpret_cast<DX12ImageResource *>(bufferBarrier.Resource)->GetResource();
        D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, DX12EnumConverter::ConvertResourceState(bufferBarrier.OldState),
                                                                                      DX12EnumConverter::ConvertResourceState(bufferBarrier.NewState));
        resourceBarriers.push_back(resourceBarrier);
    }

    m_commandList->ResourceBarrier(resourceBarriers.size(), resourceBarriers.data());
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

void DX12CommandList::TransitionImageLayout(IImageResource *image, ImageLayout oldLayout, ImageLayout newLayout) {}

void DX12CommandList::SetRootSignature(ID3D12RootSignature *rootSignature)
{
    RETURN_IF(rootSignature == nullptr);
    RETURN_IF(m_currentRootSignature == rootSignature);
    if ( m_currentRootSignature != nullptr && rootSignature != m_currentRootSignature )
    {
        LOG(Verbosity::Warning, "DX12CommandList", "Root signature is set to a different value, it is not expected to overwrite this value.");
    }

    if ( m_createInfo.QueueType == QueueType::Graphics )
    {
        m_commandList->SetGraphicsRootSignature(rootSignature);
    }
    else
    {
        m_commandList->SetComputeRootSignature(rootSignature);
    }
}
