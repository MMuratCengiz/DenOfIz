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
        bool                                 m_viewportDirty = true;
        bool                                 m_scissorDirty  = true;
        std::vector<const VulkanBindGroup *> m_queuedBindGroups;
        struct QueuedRootConstant
        {
            uint32_t Binding;
            void    *Data;
            uint32_t Size;
        };
        std::vector<QueuedRootConstant> m_queuedRootConstants;
        const VkCommandPool             m_commandPool;
        DenOfIz_QueueType               m_queueType;

    public:
        VulkanCommandList( VulkanContext *context, DenOfIz_CommandListDesc desc, VkCommandPool commandPool );

        void Begin( ) override;
        void BeginRendering( const DenOfIz_RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override; // TODO remove
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
        void ProcessBindGroups( ) const;
    };

} // namespace DenOfIz
