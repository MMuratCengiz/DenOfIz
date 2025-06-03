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

#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include "DX12BufferResource.h"
#include "DX12Context.h"
#include "DX12Pipeline.h"
#include "DX12ResourceBindGroup.h"

namespace DenOfIz
{
    class DX12CommandList final : public ICommandList
    {
        CommandListDesc m_desc;
        DX12Context    *m_context;

        wil::com_ptr<ID3D12CommandAllocator>       m_commandAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList7>   m_commandList;
        wil::com_ptr<ID3D12DebugCommandList>       m_debugCommandList;
        DX12BufferResource                        *m_currentVertexBuffer  = nullptr;
        ID3D12RootSignature                       *m_currentRootSignature = nullptr;
        DX12Pipeline                              *m_currentPipeline      = nullptr;
        std::vector<const DX12ResourceBindGroup *> m_queuedBindGroups;

        CD3DX12_RECT                        m_scissor{ };
        D3D12_VIEWPORT                      m_viewport{ };
        std::vector<ID3D12DescriptorHeap *> m_heaps = { m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetHeap( ), m_context->ShaderVisibleSamplerDescriptorHeap->GetHeap( ) };

    public:
        DX12CommandList( DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, const wil::com_ptr<ID3D12GraphicsCommandList> &m_commandList,
                         CommandListDesc desc );
        ~DX12CommandList( ) override = default;

        void Begin( ) override;
        void BeginRendering( const RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override;
        void End( ) override;
        void BindPipeline( IPipeline *pipeline ) override;
        void BindVertexBuffer( IBufferResource *buffer, uint64_t offset = 0 ) override;
        void BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType, uint64_t offset = 0 ) override;
        void BindViewport( float x, float y, float width, float height ) override;
        void BindScissorRect( float x, float y, float width, float height ) override;
        void BindResourceGroup( IResourceBindGroup *bindGroup ) override;
        void PipelineBarrier( const PipelineBarrierDesc &barrier ) override;
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        void DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        // List of copy commands
        void CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionDesc ) override;
        void CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionDesc ) override;
        void CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture ) override;
        void CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer ) override;
        // Ray tracing commands
        void BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
        void UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc ) override;
        void DispatchRays( const DispatchRaysDesc &dispatchRaysDesc ) override;

        const QueueType             GetQueueType( ) override;
        ID3D12GraphicsCommandList7 *GetCommandList( ) const;

    private:
        void SetRootSignature( ID3D12RootSignature *rootSignature );
        void ProcessBindGroups( );
        void BindRootDescriptors( const DX12RootDescriptor &rootDescriptor ) const;
        void SetRootConstants( const DX12RootConstant &rootConstant ) const;
        void BindResourceGroup( uint32_t index, const D3D12_GPU_DESCRIPTOR_HANDLE &gpuHandle ) const;
    };

} // namespace DenOfIz
