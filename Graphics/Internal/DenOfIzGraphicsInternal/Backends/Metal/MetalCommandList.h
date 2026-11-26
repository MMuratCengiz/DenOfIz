/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandList.h"
#include "MetalArgumentBuffer.h"
#include "MetalBindGroup.h"
#include "MetalBuffer.h"
#include "MetalContext.h"
#include "MetalFence.h"
#include "MetalPipeline.h"
#include "MetalSemaphore.h"
#include "MetalTexture.h"

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
        DenOfIz_CommandListDesc                    m_desc;
        MetalContext                              *m_context;
        id<MTLCommandQueue>                        m_queue;
        id<MTLCommandBuffer>                       m_commandBuffer;
        id<MTLRenderCommandEncoder>                m_renderEncoder;
        id<MTLComputeCommandEncoder>               m_computeEncoder;
        id<MTLBlitCommandEncoder>                  m_blitEncoder;
        id<MTLAccelerationStructureCommandEncoder> m_accelerationStructureEncoder;
        bool                                       m_waitForQueueFence = false;
        id<MTLFence>                               m_queueFence;
        MetalEncoderType                           m_activeEncoderType = MetalEncoderType::None;
        struct BoundGroup
        {
            const MetalBindGroup *Group;
            uint32_t              Version;
        };
        std::vector<BoundGroup> m_boundGroups;
        struct ResidentGroup
        {
            const MetalBindGroup       *Group;
            uint32_t                    Version;
            const MetalBindGroupTables *Tables;
        };
        std::vector<ResidentGroup> m_residentGroups;
        bool                       m_bindingsDirty  = true;
        bool                       m_argumentsInUse = false;
        struct BoundRootConstant
        {
            uint32_t             Binding;
            std::vector<uint8_t> Data;
        };
        std::vector<BoundRootConstant> m_boundRootConstants;

        id<MTLBuffer>                        m_indexBuffer;
        MTLIndexType                         m_indexType;
        uint64_t                             m_currentBufferOffset = 0;
        uint64_t                             m_indexBufferOffset   = 0;
        std::unique_ptr<MetalArgumentBuffer> m_argumentBuffer;
        size_t                               m_boundArgumentSegment = 0;
        const MetalShaderLayout             *m_shaderLayout         = nullptr;
        MetalPipeline                       *m_pipeline;
        const MetalPipeline                 *m_encoderPipeline  = nullptr;
        bool                                 m_computeTlasBound = false;
        bool                                 m_meshTlasBound    = false;
        bool                                 m_renderTlasBound  = false;

    public:
        MetalCommandList( MetalContext *context, id<MTLCommandQueue> queue, DenOfIz_CommandListDesc desc );
        ~MetalCommandList( ) override;

        void Begin( ) override;
        void BeginRendering( const DenOfIz_RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override;
        void End( ) override;
        void BindPipeline( IPipeline *pipeline ) override;
        void BindVertexBuffer( IBuffer *buffer, uint64_t offset = 0, uint32_t stride = 0, uint32_t slot = 0 ) override;
        void BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, uint64_t offset = 0 ) override;
        void BindViewport( float x, float y, float width, float height ) override;
        void BindScissorRect( float x, float y, float width, float height ) override;
        void BindResourceGroup( IBindGroup *bindGroup ) override;
        void SetRootConstants( uint32_t binding, const DenOfIz_ByteArray &data ) override;
        void PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier ) override;
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        void CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionInfo ) override;
        void CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionInfo ) override;
        void CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture ) override;
        void CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer ) override;
        void UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc ) override;
        void BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
        void DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc ) override;
        void DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        void DrawIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride ) override;
        void DrawIndexedIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride ) override;
        void DispatchIndirect( IBuffer *buffer, uint64_t offset ) override;

        void BeginDebugMarker( float r, float g, float b, DenOfIz_StringView name ) override;
        void EndDebugMarker( ) override;
        void InsertDebugMarker( float r, float g, float b, DenOfIz_StringView name ) override;

        void BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc ) override;
        void EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc ) override;
        void ResolveQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount ) override;
        void ResetQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount ) override;

        const DenOfIz_QueueType    GetQueueType( ) override;
        const id<MTLCommandBuffer> GetCommandBuffer( ) const;

    private:
        struct BatchedResource
        {
            id<MTLResource>  Resource;
            MTLResourceUsage Usage;
            MTLRenderStages  Stages;
        };

        std::vector<BatchedResource> m_batchedResources;
        std::vector<id<MTLResource>> m_usedResources;

        void BindCommandResources( );
        void ProcessBindGroup( const MetalBindGroup *metalBindGroup );
        void BindTopLevelArgumentBuffer( );
        void ApplyPipeline( );
        void ResetEncoderBindings( );
        void UseResource( const id<MTLResource> &resource, MTLResourceUsage usage = MTLResourceUsageRead, MTLRenderStages stages = MTLRenderStageVertex | MTLRenderStageFragment );
        void BatchResource( const id<MTLResource> &resource, MTLResourceUsage usage = MTLResourceUsageRead,
                            MTLRenderStages stages = MTLRenderStageVertex | MTLRenderStageFragment );
        void FlushBatchedResources( );
        void SwitchEncoder( MetalEncoderType encoderType, bool crossQueueBarrier = false );
    };

} // namespace DenOfIz
