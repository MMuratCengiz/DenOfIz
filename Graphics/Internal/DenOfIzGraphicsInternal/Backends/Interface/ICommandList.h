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

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/PipelineBarrierDesc.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "IBindGroup.h"
#include "IBuffer.h"
#include "IFence.h"
#include "IPipeline.h"
#include "IQueryPool.h"

namespace DenOfIz
{

    class ICommandList
    {
    public:
        virtual ~ICommandList( ) = default;

        virtual void Begin( )                                                                                                                                   = 0;
        virtual void BeginRendering( const DenOfIz_RenderingDesc &renderingDesc )                                                                               = 0;
        virtual void EndRendering( )                                                                                                                            = 0;
        virtual void End( )                                                                                                                                     = 0;
        virtual void BindPipeline( IPipeline *pipeline )                                                                                                        = 0;
        virtual void BindVertexBuffer( IBuffer *buffer, uint64_t offset = 0, uint32_t stride = 0, uint32_t slot = 0 )                                           = 0;
        virtual void BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, uint64_t offset = 0 )                                                = 0;
        virtual void BindViewport( float x, float y, float width, float height )                                                                                = 0;
        virtual void BindScissorRect( float x, float y, float width, float height )                                                                             = 0;
        virtual void BindResourceGroup( IBindGroup *bindGroup )                                                                                                 = 0;
        virtual void SetRootConstants( uint32_t binding, const DenOfIz_ByteArray &data )                                                                        = 0;
        virtual void PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier )                                                                              = 0;
        virtual void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0 ) = 0;
        virtual void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0 )                                 = 0;
        // List of copy commands
        virtual void CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc )      = 0;
        virtual void CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc )   = 0;
        virtual void CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture ) = 0;
        virtual void CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer ) = 0;
        // --
        virtual void UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc )                           = 0;
        virtual void BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc )                    = 0;
        virtual void BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc )           = 0;
        virtual void DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc )                             = 0;
        virtual void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )                 = 0;
        virtual void DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )             = 0;
        virtual void DrawIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride )        = 0;
        virtual void DrawIndexedIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride ) = 0;
        virtual void DispatchIndirect( IBuffer *buffer, uint64_t offset )                                         = 0;

        virtual void BeginDebugMarker( float r, float g, float b, DenOfIz_StringView name )  = 0;
        virtual void EndDebugMarker( )                                                       = 0;
        virtual void InsertDebugMarker( float r, float g, float b, DenOfIz_StringView name ) = 0;

        virtual void BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )         = 0;
        virtual void EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )           = 0;
        virtual void ResolveQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount ) = 0;
        virtual void ResetQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount )   = 0;

        virtual const DenOfIz_QueueType GetQueueType( ) = 0;
    };
} // namespace DenOfIz

typedef struct DenOfIz_ICommandListArray
{
    DenOfIz::ICommandList **Elements;
    size_t                  NumElements;
} DenOfIz_ICommandListArray;
