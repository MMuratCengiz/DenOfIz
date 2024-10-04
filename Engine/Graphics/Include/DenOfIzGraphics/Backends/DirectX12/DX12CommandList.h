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

#pragma once

#include <DenOfIzCore/Cast.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include "DX12BufferResource.h"
#include "DX12Context.h"
#include "DX12EnumConverter.h"
#include "DX12Fence.h"
#include "DX12Pipeline.h"
#include "DX12ResourceBindGroup.h"
#include "DX12Semaphore.h"
#include "DX12SwapChain.h"
#include "DX12TextureResource.h"

namespace DenOfIz
{

    class DX12CommandList final : public ICommandList
    {
        CommandListDesc m_desc;
        DX12Context    *m_context;

        wil::com_ptr<ID3D12CommandAllocator>     m_commandAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList7> m_commandList;
        wil::com_ptr<ID3D12DebugCommandList>     m_debugCommandList;
        ID3D12RootSignature                     *m_currentRootSignature = nullptr;

        CD3DX12_RECT                        m_scissor{ };
        D3D12_VIEWPORT                      m_viewport{ };
        ID3D12CommandQueue                 *m_commandQueue;
        std::vector<ID3D12DescriptorHeap *> m_heaps = { m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetHeap( ), m_context->ShaderVisibleSamplerDescriptorHeap->GetHeap( ) };

    public:
         DX12CommandList( DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, const wil::com_ptr<ID3D12GraphicsCommandList> &m_commandList,
                          CommandListDesc desc );
        ~DX12CommandList( ) override = default;

        void Begin( ) override;
        void BeginRendering( const RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override;
        void Execute( const ExecuteDesc &executeDesc ) override;
        void Present( ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks ) override;
        void BindPipeline( IPipeline *pipeline ) override;
        void BindVertexBuffer( IBufferResource *buffer ) override;
        void BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType ) override;
        void BindViewport( float x, float y, float width, float height ) override;
        void BindScissorRect( float x, float y, float width, float height ) override;
        void BindResourceGroup( IResourceBindGroup *bindGroup ) override;
        void PipelineBarrier( const PipelineBarrierDesc &barrier ) override;
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        void CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo ) override;
        void CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo ) override;
        void CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture ) override;
        void CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer ) override;

    private:
        void CompatibilityPipelineBarrier( const PipelineBarrierDesc &barrier ) const;
        void EnhancedPipelineBarrier( const PipelineBarrierDesc &barrier ) const;
        void SetRootSignature( ID3D12RootSignature *rootSignature );
        void BindResourceGroup( uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle ) const;
    };

} // namespace DenOfIz
