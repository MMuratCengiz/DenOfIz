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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BarrierHelper.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Buffer.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Texture.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define DX12_TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12Texture, handle )
#define DX12_BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12Buffer, handle )
#define DX12_BLAS_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12BottomLevelAS, handle )
#define DX12_TLAS_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::DX12TopLevelAS, handle )

using namespace DenOfIz;

void DX12BarrierHelper::ExecuteResourceBarrier( const DX12Context *context, ID3D12GraphicsCommandList7 *commandList, const DenOfIz_QueueType &queueType,
                                                const DenOfIz_PipelineBarrierDesc *barrier )
{
    if ( context->DX12Capabilities.EnhancedBarriers )
    {
        ExecuteEnhancedResourceBarrier( commandList, queueType, barrier );
    }
    else
    {
        ExecuteLegacyResourceBarrier( commandList, barrier );
    }
}

bool IsUavBarrier( const uint32_t &before, const uint32_t &after )
{
    return before == DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT && after == DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT ||
           before == DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT && after == DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT ||
           before == DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT && after == DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
}

UINT CalcSubresourceIndex( const uint32_t mipLevel, const uint32_t layer, const uint32_t depth, const uint32_t mipLevels, const uint32_t depthOrArraySize )
{
    return mipLevel + layer * mipLevels + depth * mipLevels * depthOrArraySize;
}

D3D12_BARRIER_SYNC GetSyncFlagsForState( const uint32_t &state )
{
    D3D12_BARRIER_SYNC syncFlags = D3D12_BARRIER_SYNC_NONE;

    if ( state & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_ALL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT || state & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_COPY;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT || state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT )
    {
        syncFlags |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    return syncFlags != D3D12_BARRIER_SYNC_NONE ? syncFlags : D3D12_BARRIER_SYNC_ALL;
}

bool DX12BarrierHelper::NeedsGlobalUavSync( const DenOfIz_PipelineBarrierDesc *barrier )
{
    for ( int i = 0; i < barrier->MemoryBarriers.NumElements; i++ )
    {
        const auto &memoryBarrier = barrier->MemoryBarriers.Elements[ i ];
        bool        isGlobalUav   = memoryBarrier.OldState == DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT || memoryBarrier.NewState == DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
        isGlobalUav               = isGlobalUav || !DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BufferResource );
        isGlobalUav               = isGlobalUav || !DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TextureResource );
        isGlobalUav               = isGlobalUav || !DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TopLevelAS );
        isGlobalUav               = isGlobalUav || !DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BottomLevelAS );

        if ( isGlobalUav )
        {
            return true;
        }
    }
    return false;
}

void DX12BarrierHelper::ExecuteEnhancedResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const DenOfIz_QueueType &queueType, const DenOfIz_PipelineBarrierDesc *barrier )
{
    std::vector<D3D12_BARRIER_GROUP> barrierGroups;

    std::vector<D3D12_GLOBAL_BARRIER>  globalBarriers;
    std::vector<D3D12_BUFFER_BARRIER>  bufferBarriers;
    std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;

    if ( NeedsGlobalUavSync( barrier ) )
    {
        D3D12_GLOBAL_BARRIER globalBarrier = { };
        globalBarrier.SyncBefore           = D3D12_BARRIER_SYNC_ALL;
        globalBarrier.SyncAfter            = D3D12_BARRIER_SYNC_ALL;
        globalBarrier.AccessBefore         = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        globalBarrier.AccessAfter          = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        globalBarriers.push_back( globalBarrier );
    }

    for ( int i = 0; i < barrier->BufferBarriers.NumElements; i++ )
    {
        const auto     &bufferBarrier = barrier->BufferBarriers.Elements[ i ];
        ID3D12Resource *pResource     = DX12_BUFFER_IMPL( bufferBarrier.Resource )->Resource( );

        D3D12_BUFFER_BARRIER dxBufferBarrier = { };
        dxBufferBarrier.pResource            = pResource;
        dxBufferBarrier.Offset               = 0;
        dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;
        dxBufferBarrier.AccessBefore         = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( bufferBarrier.OldState, queueType );
        dxBufferBarrier.AccessAfter          = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( bufferBarrier.NewState, queueType );
        dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( bufferBarrier.OldState );
        dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( bufferBarrier.NewState );

        bufferBarriers.push_back( dxBufferBarrier );
    }

    for ( int i = 0; i < barrier->TextureBarriers.NumElements; i++ )
    {
        const auto     &textureBarrier = barrier->TextureBarriers.Elements[ i ];
        ID3D12Resource *pResource      = DX12_TEXTURE_IMPL( textureBarrier.Resource )->Resource( );

        D3D12_TEXTURE_BARRIER dxTextureBarrier = { };
        dxTextureBarrier.pResource             = pResource;
        if ( textureBarrier.EnableSubresourceBarrier )
        {
            dxTextureBarrier.Subresources.IndexOrFirstMipLevel = textureBarrier.MipLevel;
            dxTextureBarrier.Subresources.NumMipLevels         = 1;
            dxTextureBarrier.Subresources.FirstArraySlice      = textureBarrier.ArrayLayer;
            dxTextureBarrier.Subresources.NumArraySlices       = 1;
        }
        else
        {
            dxTextureBarrier.Subresources.IndexOrFirstMipLevel = 0;
            dxTextureBarrier.Subresources.NumMipLevels         = pResource->GetDesc( ).MipLevels;
            dxTextureBarrier.Subresources.FirstArraySlice      = 0;
            dxTextureBarrier.Subresources.NumArraySlices       = pResource->GetDesc( ).DepthOrArraySize;
        }
        dxTextureBarrier.Subresources.FirstPlane = 0;
        dxTextureBarrier.Subresources.NumPlanes  = 1;

        if ( textureBarrier.EnableQueueBarrier && textureBarrier.SourceQueue != textureBarrier.DestinationQueue )
        {
            dxTextureBarrier.LayoutBefore = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( textureBarrier.OldState, textureBarrier.SourceQueue, true );
            dxTextureBarrier.LayoutAfter  = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( textureBarrier.NewState, textureBarrier.DestinationQueue, true );
            dxTextureBarrier.AccessBefore = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( textureBarrier.OldState, textureBarrier.SourceQueue );
            dxTextureBarrier.AccessAfter  = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( textureBarrier.NewState, textureBarrier.DestinationQueue );
            dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( textureBarrier.OldState );
            dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( textureBarrier.NewState );
        }
        else
        {
            dxTextureBarrier.LayoutBefore = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( textureBarrier.OldState, queueType, true );
            dxTextureBarrier.LayoutAfter  = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( textureBarrier.NewState, queueType, true );
            dxTextureBarrier.AccessBefore = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( textureBarrier.OldState, queueType );
            dxTextureBarrier.AccessAfter  = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( textureBarrier.NewState, queueType );
            dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( textureBarrier.OldState );
            dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( textureBarrier.NewState );
        }
        textureBarriers.push_back( dxTextureBarrier );
    }

    for ( int i = 0; i < barrier->MemoryBarriers.NumElements; i++ )
    {
        const auto &memoryBarrier = barrier->MemoryBarriers.Elements[ i ];
        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BufferResource ) )
        {
            ID3D12Resource *pResource = DX12_BUFFER_IMPL( memoryBarrier.BufferResource )->Resource( );

            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = pResource;
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( memoryBarrier.OldState );
            dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( memoryBarrier.NewState );

            bufferBarriers.push_back( dxBufferBarrier );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TextureResource ) )
        {
            ID3D12Resource *pResource = DX12_TEXTURE_IMPL( memoryBarrier.TextureResource )->Resource( );

            D3D12_TEXTURE_BARRIER dxTextureBarrier             = { };
            dxTextureBarrier.pResource                         = pResource;
            dxTextureBarrier.Subresources.IndexOrFirstMipLevel = 0;
            dxTextureBarrier.Subresources.NumMipLevels         = pResource->GetDesc( ).MipLevels;
            dxTextureBarrier.Subresources.FirstArraySlice      = 0;
            dxTextureBarrier.Subresources.NumArraySlices       = pResource->GetDesc( ).DepthOrArraySize;
            dxTextureBarrier.Subresources.FirstPlane           = 0;
            dxTextureBarrier.Subresources.NumPlanes            = 1;
            dxTextureBarrier.LayoutBefore                      = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( memoryBarrier.OldState, queueType, true );
            dxTextureBarrier.LayoutAfter                       = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierLayout( memoryBarrier.NewState, queueType, true );
            dxTextureBarrier.AccessBefore                      = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxTextureBarrier.AccessAfter                       = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxTextureBarrier.SyncBefore                        = GetSyncFlagsForState( memoryBarrier.OldState );
            dxTextureBarrier.SyncAfter                         = GetSyncFlagsForState( memoryBarrier.NewState );

            textureBarriers.push_back( dxTextureBarrier );
        }

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BottomLevelAS ) )
        {
            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = DX12_BLAS_IMPL( memoryBarrier.BottomLevelAS )->Buffer( )->Resource( );
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = dxBufferBarrier.pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( memoryBarrier.OldState );
            dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( memoryBarrier.NewState );

            bufferBarriers.push_back( dxBufferBarrier );
        }

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TopLevelAS ) )
        {
            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = DX12_TLAS_IMPL( memoryBarrier.TopLevelAS )->Buffer( )->Resource( );
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = dxBufferBarrier.pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DenOfIz_DX12EnumConverter_ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
        }
    }

    if ( !globalBarriers.empty( ) )
    {
        D3D12_BARRIER_GROUP &group = barrierGroups.emplace_back( );
        group.Type                 = D3D12_BARRIER_TYPE_GLOBAL;
        group.NumBarriers          = globalBarriers.size( );
        group.pGlobalBarriers      = globalBarriers.data( );
    }

    if ( !bufferBarriers.empty( ) )
    {
        D3D12_BARRIER_GROUP &group = barrierGroups.emplace_back( );
        group.Type                 = D3D12_BARRIER_TYPE_BUFFER;
        group.NumBarriers          = bufferBarriers.size( );
        group.pBufferBarriers      = bufferBarriers.data( );
    }

    if ( !textureBarriers.empty( ) )
    {
        D3D12_BARRIER_GROUP &group = barrierGroups.emplace_back( );
        group.Type                 = D3D12_BARRIER_TYPE_TEXTURE;
        group.NumBarriers          = textureBarriers.size( );
        group.pTextureBarriers     = textureBarriers.data( );
    }

    if ( !barrierGroups.empty( ) )
    {
        commandList->Barrier( barrierGroups.size( ), barrierGroups.data( ) );
    }
}

void DX12BarrierHelper::ExecuteLegacyResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const DenOfIz_PipelineBarrierDesc *barrier )
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    if ( NeedsGlobalUavSync( barrier ) )
    {
        resourceBarriers.push_back( CD3DX12_RESOURCE_BARRIER::UAV( nullptr ) );
    }

    for ( int i = 0; i < barrier->TextureBarriers.NumElements; i++ )
    {
        const auto     &textureBarrier = barrier->TextureBarriers.Elements[ i ];
        ID3D12Resource *pResource      = DX12_TEXTURE_IMPL( textureBarrier.Resource )->Resource( );

        if ( textureBarrier.OldState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT && textureBarrier.NewState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
        {
            D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
            resourceBarriers.push_back( uavBarrier );
        }
        UINT subresource = textureBarrier.EnableSubresourceBarrier ? CalcSubresourceIndex( textureBarrier.MipLevel, textureBarrier.ArrayLayer, 0, pResource->GetDesc( ).MipLevels,
                                                                                           pResource->GetDesc( ).DepthOrArraySize )
                                                                   : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        D3D12_RESOURCE_BARRIER transition = { };
        transition.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        transition.Transition.pResource   = pResource;
        transition.Transition.Subresource = subresource;
        transition.Transition.StateBefore = DenOfIz_DX12EnumConverter_ConvertResourceUsage( textureBarrier.OldState );
        transition.Transition.StateAfter  = DenOfIz_DX12EnumConverter_ConvertResourceUsage( textureBarrier.NewState );

        resourceBarriers.push_back( transition );
    }

    for ( int i = 0; i < barrier->BufferBarriers.NumElements; i++ )
    {
        const auto     &bufferBarrier = barrier->BufferBarriers.Elements[ i ];
        ID3D12Resource *pResource     = DX12_BUFFER_IMPL( bufferBarrier.Resource )->Resource( );

        if ( bufferBarrier.OldState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT && bufferBarrier.NewState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
        {
            D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
            resourceBarriers.push_back( uavBarrier );
            continue;
        }

        const D3D12_RESOURCE_STATES before = DenOfIz_DX12EnumConverter_ConvertResourceUsage( bufferBarrier.OldState );
        const D3D12_RESOURCE_STATES after  = DenOfIz_DX12EnumConverter_ConvertResourceUsage( bufferBarrier.NewState );

        if ( before != after )
        {
            D3D12_RESOURCE_BARRIER transition = { };
            transition.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            transition.Transition.pResource   = pResource;
            transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            transition.Transition.StateBefore = before;
            transition.Transition.StateAfter  = after;

            resourceBarriers.push_back( transition );
        }
    }

    for ( int i = 0; i < barrier->MemoryBarriers.NumElements; i++ )
    {
        const auto &memoryBarrier = barrier->MemoryBarriers.Elements[ i ];
        bool        isUavBarrier  = IsUavBarrier( memoryBarrier.OldState, memoryBarrier.NewState );

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BufferResource ) )
        {
            ID3D12Resource *pResource = DX12_BUFFER_IMPL( memoryBarrier.BufferResource )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition( pResource, DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.OldState ),
                                                                                          DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.NewState ) );
                resourceBarriers.push_back( transition );
            }
        }

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TextureResource ) )
        {
            ID3D12Resource *pResource = DX12_TEXTURE_IMPL( memoryBarrier.TextureResource )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition( pResource, DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.OldState ),
                                                                                          DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.NewState ) );
                resourceBarriers.push_back( transition );
            }
        }

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.BottomLevelAS ) )
        {
            auto pResource = DX12_BLAS_IMPL( memoryBarrier.BottomLevelAS )->Buffer( )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = { };
                transition.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                transition.Transition.pResource   = pResource;
                transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                transition.Transition.StateBefore = DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.OldState );
                transition.Transition.StateAfter  = DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.NewState );
                resourceBarriers.push_back( transition );
            }
        }

        if ( DENOFIZ_HANDLE_IS_VALID( memoryBarrier.TopLevelAS ) )
        {
            auto pResource = DX12_TLAS_IMPL( memoryBarrier.TopLevelAS )->Buffer( )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = { };
                transition.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                transition.Transition.pResource   = pResource;
                transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                transition.Transition.StateBefore = DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.OldState );
                transition.Transition.StateAfter  = DenOfIz_DX12EnumConverter_ConvertResourceUsage( memoryBarrier.NewState );
                resourceBarriers.push_back( transition );
            }
        }
    }

    if ( !resourceBarriers.empty( ) )
    {
        for ( int i = 0; i < resourceBarriers.size( ) - 1; i++ )
        {
            if ( resourceBarriers[ i ].Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
                 resourceBarriers[ i ].Transition.StateBefore == resourceBarriers[ i ].Transition.StateAfter )
            {
                spdlog::error( "State before and after are the same for resource barrier" );
            }
        }
        commandList->ResourceBarrier( static_cast<UINT>( resourceBarriers.size( ) ), resourceBarriers.data( ) );
    }
}
