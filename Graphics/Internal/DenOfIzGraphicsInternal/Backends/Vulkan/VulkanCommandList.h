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
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanResourceBindGroup.h"
#include "VulkanSemaphore.h"

namespace DenOfIz
{

    class VulkanCommandList final : public ICommandList
    {
        CommandListDesc m_desc;
        VulkanContext  *m_context = nullptr;

        VulkanPipeline                              *m_currentPipeline = nullptr;
        VkCommandBuffer                              m_commandBuffer{ };
        VkViewport                                   m_viewport{ };
        VkRect2D                                     m_scissorRect{ };
        std::vector<const VulkanResourceBindGroup *> m_queuedBindGroups;
        const VkCommandPool                          m_commandPool;
        QueueType                                    m_queueType;

    public:
        VulkanCommandList( VulkanContext *context, CommandListDesc desc, VkCommandPool commandPool );

        void Begin( ) override;
        void BeginRendering( const RenderingDesc &renderingDesc ) override;
        void EndRendering( ) override; // TODO remove
        void End( ) override;
        void BindPipeline( IPipeline *pipeline ) override;
        void BindVertexBuffer( IBufferResource *buffer, uint64_t offset = 0 ) override;
        void BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType, uint64_t offset = 0 ) override;
        void BindViewport( float offsetX, float offsetY, float width, float height ) override;
        void BindScissorRect( float offsetX, float offsetY, float width, float height ) override;
        void BindResourceGroup( IResourceBindGroup *bindGroup ) override;
        void PipelineBarrier( const PipelineBarrierDesc &barrier ) override;
        void CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionDesc ) override;
        void CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionDesc ) override;
        void CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTextureDesc ) override;
        void CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBufferDesc ) override;
        void BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
        void UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc ) override;
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance ) override;
        void DispatchRays( const DispatchRaysDesc &dispatchRaysDesc ) override;
        void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;
        void DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) override;

        const QueueType  GetQueueType( ) override;
        VkCommandBuffer &GetCommandBuffer( );

    private:
        void ProcessBindGroups( ) const;
    };

} // namespace DenOfIz
