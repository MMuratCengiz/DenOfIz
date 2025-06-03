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

#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "MetalArgumentBuffer.h"
#include "MetalBufferResource.h"
#include "MetalContext.h"
#include "MetalFence.h"
#include "MetalPipeline.h"
#include "MetalResourceBindGroup.h"
#include "MetalSemaphore.h"
#include "MetalSwapChain.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{

    enum class MetalEncoderType
    {
        Render,
        Compute,
        Blit,
        AccelerationStructure,
        None
    };

    class MetalCommandList final : public ICommandList
    {
        CommandListDesc                             m_desc;
        MetalContext                               *m_context;
        id<MTLCommandBuffer>                        m_commandBuffer;
        id<MTLRenderCommandEncoder>                 m_renderEncoder;
        id<MTLComputeCommandEncoder>                m_computeEncoder;
        id<MTLBlitCommandEncoder>                   m_blitEncoder;
        id<MTLAccelerationStructureCommandEncoder>  m_accelerationStructureEncoder;
        bool                                        m_waitForQueueFence = false;
        id<MTLFence>                                m_queueFence;
        MetalEncoderType                            m_activeEncoderType = MetalEncoderType::None;
        std::vector<const MetalResourceBindGroup *> m_queuedBindGroups;

        // States:
        id<MTLBuffer>                        m_indexBuffer;
        MTLIndexType                         m_indexType;
        uint64_t                             m_currentBufferOffset = 0;
        uint64_t                             m_indexBufferOffset   = 0;
        std::unique_ptr<MetalArgumentBuffer> m_argumentBuffer;
        MetalRootSignature                  *m_rootSignature;
        MetalPipeline                       *m_pipeline;
        bool                                 m_computeTlasBound = false;
        bool                                 m_meshTlasBound    = false;
        bool                                 m_renderTlasBound  = false;
        // --

    public:
        MetalCommandList( MetalContext *context, CommandListDesc desc );
        ~MetalCommandList( ) override;

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
        void CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo ) override;
        void CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo ) override;
        void CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture ) override;
        void CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer ) override;
        void UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc ) override;
        void BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
        void DispatchRays( const DispatchRaysDesc &dispatchRaysDesc ) override;
        void DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;

        const QueueType            GetQueueType( ) override;
        const id<MTLCommandBuffer> GetCommandBuffer( ) const;

    private:
        void BindCommandResources( );
        void ProcessBindGroup( const MetalResourceBindGroup *metalBindGroup );
        void BindTopLevelArgumentBuffer( );
        void TopLevelArgumentBufferNextOffset( );
        void UseResource( const id<MTLResource> &resource, MTLResourceUsage usage = MTLResourceUsageRead, MTLRenderStages stages = MTLRenderStageVertex | MTLRenderStageFragment );
        // This is used because Vulkan+DX12 both support more operations in their graphics command list, so seamless transition is provided here
        void SwitchEncoder( MetalEncoderType encoderType, bool crossQueueBarrier = false );
    };

} // namespace DenOfIz
