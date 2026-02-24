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

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandList.h>
#include <map>
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h"
#include "WebGPUBindGroup.h"
#include "WebGPUContext.h"
#include "WebGPUPipeline.h"

namespace DenOfIz
{
    class WebGPUCommandList final : public ICommandList
    {
        WebGPUContext          *m_context;
        ICommandQueue          *m_commandQueue;
        DenOfIz_CommandListDesc m_desc;
        WGPUCommandEncoder      m_commandEncoder     = nullptr;
        WGPURenderPassEncoder   m_renderPassEncoder  = nullptr;
        WGPUComputePassEncoder  m_computePassEncoder = nullptr;

        WebGPUPipeline *m_currentPipeline = nullptr;
        struct DeferredVertexBuffer
        {
            WGPUBuffer Buffer = nullptr;
            uint64_t   Offset = 0;
            uint32_t   Stride = 0;
            uint32_t   Slot   = 0;
        };
        std::vector<DeferredVertexBuffer> m_deferredVertexBuffers;
        WGPUBuffer                        m_currentIndexBuffer = nullptr;
        WGPUIndexFormat                   m_currentIndexFormat = WGPUIndexFormat_Undefined;
        uint64_t                          m_indexBufferOffset  = 0;

        std::vector<std::pair<uint32_t, const WebGPUBindGroup *>> m_pendingBindGroups{ };

        struct QueuedRootConstant
        {
            uint32_t             Binding;
            std::vector<uint8_t> Data;
        };
        std::vector<QueuedRootConstant> m_queuedRootConstants;
        std::map<uint32_t, WGPUBuffer>  m_rootConstantBuffers;
        WGPUBindGroup                   m_rootConstantBindGroup = nullptr;
        bool                            m_rootConstantsDirty    = false;

        bool     m_isInsideRenderPass = false;
        float    m_viewportX          = 0.0f;
        float    m_viewportY          = 0.0f;
        float    m_viewportWidth      = 0.0f;
        float    m_viewportHeight     = 0.0f;
        uint32_t m_scissorX           = 0;
        uint32_t m_scissorY           = 0;
        uint32_t m_scissorWidth       = 0;
        uint32_t m_scissorHeight      = 0;

    public:
        WebGPUCommandList( WebGPUContext *context, ICommandQueue *commandQueue, const DenOfIz_CommandListDesc &desc );
        ~WebGPUCommandList( ) override;

        WGPUCommandBuffer Finish( );

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
        void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0 ) override;
        void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0 ) override;
        void CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc ) override;
        void CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc ) override;
        void CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture ) override;
        void CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer ) override;
        void UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc ) override;
        void BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc ) override;
        void BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc ) override;
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

    private:
        void BeginComputePass( );
        void EndCurrentPass( );
        void FlushPendingBindGroups( );
        void ApplyViewportAndScissor( ) const;
    };

} // namespace DenOfIz
