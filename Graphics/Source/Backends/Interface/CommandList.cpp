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

#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandList.h"

#define COMMANDLIST_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ICommandList, handle )

extern "C"
{

    void DenOfIz_CommandList_Begin( DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->Begin( );
    }

    void DenOfIz_CommandList_BeginRendering( DenOfIz_CommandList commandList, const DenOfIz_RenderingDesc *renderingDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || renderingDesc == NULL )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BeginRendering( *renderingDesc );
    }

    void DenOfIz_CommandList_EndRendering( DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->EndRendering( );
    }

    void DenOfIz_CommandList_End( DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->End( );
    }

    void DenOfIz_CommandList_BindPipeline( DenOfIz_CommandList commandList, DenOfIz_Pipeline pipeline )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( pipeline ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindPipeline( DENOFIZ_FROM_HANDLE( DenOfIz::IPipeline, pipeline ) );
    }

    void DenOfIz_CommandList_BindVertexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t stride, uint32_t slot )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindVertexBuffer( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buffer ), offset, stride, slot );
    }

    void DenOfIz_CommandList_BindIndexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, const DenOfIz_IndexType indexType, uint64_t offset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindIndexBuffer( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buffer ), indexType, offset );
    }

    void DenOfIz_CommandList_BindViewport( DenOfIz_CommandList commandList, float x, float y, float width, float height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindViewport( x, y, width, height );
    }

    void DenOfIz_CommandList_BindScissorRect( DenOfIz_CommandList commandList, float x, float y, float width, float height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindScissorRect( x, y, width, height );
    }

    void DenOfIz_CommandList_BindGroup( DenOfIz_CommandList commandList, DenOfIz_BindGroup bindGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BindResourceGroup( DENOFIZ_FROM_HANDLE( DenOfIz::IBindGroup, bindGroup ) );
    }

    void DenOfIz_CommandList_SetRootConstants( DenOfIz_CommandList commandList, uint32_t binding, const DenOfIz_ByteArray *data )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || data == NULL )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->SetRootConstants( binding, *data );
    }

    void DenOfIz_CommandList_PipelineBarrier( DenOfIz_CommandList commandList, const DenOfIz_PipelineBarrierDesc *barrier )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || barrier == NULL )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->PipelineBarrier( barrier );
    }

    void DenOfIz_CommandList_DrawIndexed( DenOfIz_CommandList commandList, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset,
                                          uint32_t firstInstance )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->DrawIndexed( indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
    }

    void DenOfIz_CommandList_Draw( DenOfIz_CommandList commandList, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->Draw( vertexCount, instanceCount, firstVertex, firstInstance );
    }

    void DenOfIz_CommandList_CopyBufferRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferRegionDesc *copyBufferRegionDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || copyBufferRegionDesc == NULL )
        {
            return;
        }
        DenOfIz_CopyBufferRegionDesc desc = *copyBufferRegionDesc;
        COMMANDLIST_IMPL( commandList )->CopyBufferRegion( desc );
    }

    void DenOfIz_CommandList_CopyTextureRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureRegionDesc *copyTextureRegionDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || copyTextureRegionDesc == NULL )
        {
            return;
        }
        DenOfIz_CopyTextureRegionDesc desc = *copyTextureRegionDesc;
        COMMANDLIST_IMPL( commandList )->CopyTextureRegion( desc );
    }

    void DenOfIz_CommandList_CopyBufferToTexture( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferToTextureDesc *copyBufferToTexture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || copyBufferToTexture == NULL )
        {
            return;
        }
        DenOfIz_CopyBufferToTextureDesc desc = *copyBufferToTexture;
        COMMANDLIST_IMPL( commandList )->CopyBufferToTexture( desc );
    }

    void DenOfIz_CommandList_CopyTextureToBuffer( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureToBufferDesc *copyTextureToBuffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || copyTextureToBuffer == NULL )
        {
            return;
        }
        DenOfIz_CopyTextureToBufferDesc desc = *copyTextureToBuffer;
        COMMANDLIST_IMPL( commandList )->CopyTextureToBuffer( desc );
    }

    void DenOfIz_CommandList_UpdateTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_UpdateTopLevelASDesc *updateDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || updateDesc == NULL )
        {
            return;
        }
        DenOfIz_UpdateTopLevelASDesc desc = *updateDesc;
        COMMANDLIST_IMPL( commandList )->UpdateTopLevelAS( desc );
    }

    void DenOfIz_CommandList_BuildTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildTopLevelASDesc *buildTopLevelASDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || buildTopLevelASDesc == NULL )
        {
            return;
        }
        DenOfIz_BuildTopLevelASDesc desc = *buildTopLevelASDesc;
        COMMANDLIST_IMPL( commandList )->BuildTopLevelAS( desc );
    }

    void DenOfIz_CommandList_BuildBottomLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildBottomLevelASDesc *buildBottomLevelASDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || buildBottomLevelASDesc == NULL )
        {
            return;
        }
        DenOfIz_BuildBottomLevelASDesc desc = *buildBottomLevelASDesc;
        COMMANDLIST_IMPL( commandList )->BuildBottomLevelAS( desc );
    }

    void DenOfIz_CommandList_DispatchRays( DenOfIz_CommandList commandList, const DenOfIz_DispatchRaysDesc *dispatchRaysDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || dispatchRaysDesc == NULL )
        {
            return;
        }
        DenOfIz_DispatchRaysDesc desc = *dispatchRaysDesc;
        COMMANDLIST_IMPL( commandList )->DispatchRays( desc );
    }

    void DenOfIz_CommandList_Dispatch( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->Dispatch( groupCountX, groupCountY, groupCountZ );
    }

    void DenOfIz_CommandList_DispatchMesh( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->DispatchMesh( groupCountX, groupCountY, groupCountZ );
    }

    void DenOfIz_CommandList_DrawIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->DrawIndirect( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buffer ), offset, drawCount, stride );
    }

    void DenOfIz_CommandList_DrawIndexedIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->DrawIndexedIndirect( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buffer ), offset, drawCount, stride );
    }

    void DenOfIz_CommandList_DispatchIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->DispatchIndirect( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buffer ), offset );
    }

    void DenOfIz_CommandList_BeginDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BeginDebugMarker( r, g, b, name );
    }

    void DenOfIz_CommandList_EndDebugMarker( DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->EndDebugMarker( );
    }

    void DenOfIz_CommandList_InsertDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->InsertDebugMarker( r, g, b, name );
    }

    void DenOfIz_CommandList_BeginQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( queryPool ) || queryDesc == NULL )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->BeginQuery( DENOFIZ_FROM_HANDLE( DenOfIz::IQueryPool, queryPool ), *queryDesc );
    }

    void DenOfIz_CommandList_EndQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( queryPool ) || queryDesc == NULL )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->EndQuery( DENOFIZ_FROM_HANDLE( DenOfIz::IQueryPool, queryPool ), *queryDesc );
    }

    void DenOfIz_CommandList_ResolveQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( queryPool ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->ResolveQuery( DENOFIZ_FROM_HANDLE( DenOfIz::IQueryPool, queryPool ), startQuery, queryCount );
    }

    void DenOfIz_CommandList_ResetQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( queryPool ) )
        {
            return;
        }
        COMMANDLIST_IMPL( commandList )->ResetQuery( DENOFIZ_FROM_HANDLE( DenOfIz::IQueryPool, queryPool ), startQuery, queryCount );
    }

    void DenOfIz_CommandList_GetQueueType( DenOfIz_CommandList commandList, DenOfIz_QueueType *outQueueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) || outQueueType == NULL )
        {
            return;
        }
        *outQueueType = COMMANDLIST_IMPL( commandList )->GetQueueType( );
    }

    void DenOfIz_CommandList_Destroy( DenOfIz_CommandList commandList )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( commandList ) )
        {
            return;
        }
        delete COMMANDLIST_IMPL( commandList );
    }
}
