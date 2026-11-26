/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandList.h"
#include "VulkanBindGroup.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanSemaphore.h"

namespace DenOfIz
{

    class VulkanCommandList final : public ICommandList
    {
        DenOfIz_CommandListDesc m_desc;
        VulkanContext          *m_context = nullptr;

        VulkanPipeline                      *m_currentPipeline = nullptr;
        VkCommandBuffer                      m_commandBuffer{ };
        VkViewport                           m_viewport{ };
        VkRect2D                             m_scissorRect{ };
        VkRect2D m_requestedScissor{ };
        VkRect2D m_renderArea{ };
        bool     m_insideRendering       = false;
        bool     m_warnedAttachmentSizes = false;
        bool                                 m_viewportDirty = true;
        bool                                 m_scissorDirty  = true;
        struct BoundGroup
        {
            uint32_t               RegisterSpace;
            const VulkanBindGroup *Group;
            bool                   Dirty;
        };
        struct BoundRootConstant
        {
            uint32_t             Binding;
            std::vector<uint8_t> Data;
            bool                 Dirty;
        };
        std::vector<BoundGroup>        m_boundGroups;
        std::vector<BoundRootConstant> m_boundRootConstants;
        VkPipelineLayout               m_boundLayout    = VK_NULL_HANDLE;
        VkPipelineBindPoint            m_boundBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        const VkCommandPool             m_commandPool;
        DenOfIz_QueueType               m_queueType;

        void ApplyScissor( );

    public:
        VulkanCommandList( VulkanContext *context, DenOfIz_CommandListDesc desc, VkCommandPool commandPool );

        void Begin( ) override;
        void BeginRendering( const DenOfIz_RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override;
        void End( ) override;
        void BindPipeline( IPipeline *pipeline ) override;
        void BindVertexBuffer( IBuffer *buffer, uint64_t offset = 0, uint32_t stride = 0, uint32_t slot = 0 ) override;
        void BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, uint64_t offset = 0 ) override;
        void BindViewport( float offsetX, float offsetY, float width, float height ) override;
        void BindScissorRect( float offsetX, float offsetY, float width, float height ) override;
        void BindResourceGroup( IBindGroup *bindGroup ) override;
        void SetRootConstants( uint32_t binding, const DenOfIz_ByteArray &data ) override;
        void PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier ) override;
        void CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc ) override;
        void CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc ) override;
        void CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTextureDesc ) override;
        void CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBufferDesc ) override;
        void BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
        void UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc ) override;
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc ) override;
        void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
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

        const DenOfIz_QueueType GetQueueType( ) override;
        VkCommandBuffer        &GetCommandBuffer( );

    private:
        void ProcessBindGroups( );
    };

} // namespace DenOfIz
