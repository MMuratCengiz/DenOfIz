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
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12TextureResource.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void DX12BarrierHelper::ExecuteResourceBarrier( const DX12Context *context, ID3D12GraphicsCommandList7 *commandList, const QueueType &queueType,
                                                const PipelineBarrierDesc &barrier )
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
    return before == ResourceUsage::UnorderedAccess && after == ResourceUsage::UnorderedAccess ||
           before == ResourceUsage::AccelerationStructureWrite && after == ResourceUsage::AccelerationStructureRead ||
           before == ResourceUsage::AccelerationStructureRead && after == ResourceUsage::AccelerationStructureWrite;
}

UINT CalcSubresourceIndex( const uint32_t mipLevel, const uint32_t layer, const uint32_t depth, const uint32_t mipLevels, const uint32_t depthOrArraySize )
{
    return mipLevel + layer * mipLevels + depth * mipLevels * depthOrArraySize;
}

D3D12_BARRIER_SYNC GetSyncFlagsForState( const uint32_t &state )
{
    D3D12_BARRIER_SYNC syncFlags = D3D12_BARRIER_SYNC_NONE;

    if ( state & ResourceUsage::RenderTarget )
    {
        syncFlags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    }
    if ( state & ResourceUsage::UnorderedAccess )
    {
        syncFlags |= D3D12_BARRIER_SYNC_ALL;
    }
    if ( state & ResourceUsage::DepthWrite )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state & ResourceUsage::DepthRead )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state & ResourceUsage::CopyDst || state & ResourceUsage::CopySrc )
    {
        syncFlags |= D3D12_BARRIER_SYNC_COPY;
    }
    if ( state & ResourceUsage::AccelerationStructureWrite || state & ResourceUsage::AccelerationStructureRead )
    {
        syncFlags |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    return syncFlags != D3D12_BARRIER_SYNC_NONE ? syncFlags : D3D12_BARRIER_SYNC_ALL;
}

bool DX12BarrierHelper::NeedsGlobalUavSync( const PipelineBarrierDesc &barrier )
{
    for ( int i = 0; i < barrier.GetMemoryBarriers( ).NumElements( ); i++ )
    {
        const auto &memoryBarrier = barrier.GetMemoryBarriers( ).GetElement( i );
        bool        isGlobalUav   = memoryBarrier.OldState == ResourceUsage::UnorderedAccess || memoryBarrier.NewState == ResourceUsage::UnorderedAccess;
        isGlobalUav               = isGlobalUav || memoryBarrier.BufferResource == nullptr;
        isGlobalUav               = isGlobalUav || memoryBarrier.TextureResource == nullptr;
        isGlobalUav               = isGlobalUav || memoryBarrier.TopLevelAS == nullptr;
        isGlobalUav               = isGlobalUav || memoryBarrier.BottomLevelAS == nullptr;

        if ( isGlobalUav )
        {
            return true;
        }
    }
    return false;
}

void DX12BarrierHelper::ExecuteEnhancedResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const QueueType &queueType, const PipelineBarrierDesc &barrier )
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

    for ( int i = 0; i < barrier.GetBufferBarriers( ).NumElements( ); i++ )
    {
        const auto     &bufferBarrier = barrier.GetBufferBarriers( ).GetElement( i );
        ID3D12Resource *pResource     = dynamic_cast<DX12BufferResource *>( bufferBarrier.Resource )->Resource( );

        D3D12_BUFFER_BARRIER dxBufferBarrier = { };
        dxBufferBarrier.pResource            = pResource;
        dxBufferBarrier.Offset               = 0;
        dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;
        dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.OldState, queueType );
        dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.NewState, queueType );
        dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( bufferBarrier.OldState );
        dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( bufferBarrier.NewState );

        bufferBarriers.push_back( dxBufferBarrier );
    }

    for ( int i = 0; i < barrier.GetTextureBarriers( ).NumElements( ); i++ )
    {
        const auto     &textureBarrier = barrier.GetTextureBarriers( ).GetElement( i );
        ID3D12Resource *pResource      = dynamic_cast<DX12TextureResource *>( textureBarrier.Resource )->Resource( );

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
            dxTextureBarrier.LayoutBefore = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.OldState, textureBarrier.SourceQueue, true );
            dxTextureBarrier.LayoutAfter  = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.NewState, textureBarrier.DestinationQueue, true );
            dxTextureBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.OldState, textureBarrier.SourceQueue );
            dxTextureBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.NewState, textureBarrier.DestinationQueue );
            dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( textureBarrier.OldState );
            dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( textureBarrier.NewState );
        }
        else
        {
            dxTextureBarrier.LayoutBefore = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.OldState, queueType, true );
            dxTextureBarrier.LayoutAfter  = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.NewState, queueType, true );
            dxTextureBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.OldState, queueType );
            dxTextureBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.NewState, queueType );
            dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( textureBarrier.OldState );
            dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( textureBarrier.NewState );
        }
        textureBarriers.push_back( dxTextureBarrier );
    }

    for ( int i = 0; i < barrier.GetMemoryBarriers( ).NumElements( ); i++ )
    {
        const auto &memoryBarrier = barrier.GetMemoryBarriers( ).GetElement( i );
        if ( memoryBarrier.BufferResource != nullptr )
        {
            ID3D12Resource *pResource = dynamic_cast<DX12BufferResource *>( memoryBarrier.BufferResource )->Resource( );

            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = pResource;
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( memoryBarrier.OldState );
            dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( memoryBarrier.NewState );

            bufferBarriers.push_back( dxBufferBarrier );
        }
        if ( memoryBarrier.TextureResource != nullptr )
        {
            ID3D12Resource *pResource = dynamic_cast<DX12TextureResource *>( memoryBarrier.TextureResource )->Resource( );

            D3D12_TEXTURE_BARRIER dxTextureBarrier             = { };
            dxTextureBarrier.pResource                         = pResource;
            dxTextureBarrier.Subresources.IndexOrFirstMipLevel = 0;
            dxTextureBarrier.Subresources.NumMipLevels         = pResource->GetDesc( ).MipLevels;
            dxTextureBarrier.Subresources.FirstArraySlice      = 0;
            dxTextureBarrier.Subresources.NumArraySlices       = pResource->GetDesc( ).DepthOrArraySize;
            dxTextureBarrier.Subresources.FirstPlane           = 0;
            dxTextureBarrier.Subresources.NumPlanes            = 1;
            dxTextureBarrier.LayoutBefore                      = DX12EnumConverter::ConvertResourceStateToBarrierLayout( memoryBarrier.OldState, queueType, true );
            dxTextureBarrier.LayoutAfter                       = DX12EnumConverter::ConvertResourceStateToBarrierLayout( memoryBarrier.NewState, queueType, true );
            dxTextureBarrier.AccessBefore                      = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxTextureBarrier.AccessAfter                       = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxTextureBarrier.SyncBefore                        = GetSyncFlagsForState( memoryBarrier.OldState );
            dxTextureBarrier.SyncAfter                         = GetSyncFlagsForState( memoryBarrier.NewState );

            textureBarriers.push_back( dxTextureBarrier );
        }

        if ( memoryBarrier.BottomLevelAS != nullptr )
        {
            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = dynamic_cast<DX12BottomLevelAS *>( memoryBarrier.BottomLevelAS )->Buffer( )->Resource( );
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = dxBufferBarrier.pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
            dxBufferBarrier.SyncBefore           = GetSyncFlagsForState( memoryBarrier.OldState );
            dxBufferBarrier.SyncAfter            = GetSyncFlagsForState( memoryBarrier.NewState );

            bufferBarriers.push_back( dxBufferBarrier );
        }

        if ( memoryBarrier.TopLevelAS != nullptr )
        {
            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = dynamic_cast<DX12TopLevelAS *>( memoryBarrier.TopLevelAS )->Buffer( )->Resource( );
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = dxBufferBarrier.pResource->GetDesc( ).Width;
            dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
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

void DX12BarrierHelper::ExecuteLegacyResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const PipelineBarrierDesc &barrier )
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    if ( NeedsGlobalUavSync( barrier ) )
    {
        resourceBarriers.push_back( CD3DX12_RESOURCE_BARRIER::UAV( nullptr ) );
    }

    for ( int i = 0; i < barrier.GetTextureBarriers( ).NumElements( ); i++ )
    {
        const auto     &textureBarrier = barrier.GetTextureBarriers( ).GetElement( i );
        ID3D12Resource *pResource      = dynamic_cast<DX12TextureResource *>( textureBarrier.Resource )->Resource( );

        if ( textureBarrier.OldState & ResourceUsage::UnorderedAccess && textureBarrier.NewState & ResourceUsage::UnorderedAccess )
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
        transition.Transition.StateBefore = DX12EnumConverter::ConvertResourceUsage( textureBarrier.OldState );
        transition.Transition.StateAfter  = DX12EnumConverter::ConvertResourceUsage( textureBarrier.NewState );

        resourceBarriers.push_back( transition );
    }

    for ( int i = 0; i < barrier.GetBufferBarriers( ).NumElements( ); i++ )
    {
        const auto     &bufferBarrier = barrier.GetBufferBarriers( ).GetElement( i );
        ID3D12Resource *pResource     = dynamic_cast<DX12BufferResource *>( bufferBarrier.Resource )->Resource( );

        if ( bufferBarrier.OldState & ResourceUsage::UnorderedAccess && bufferBarrier.NewState & ResourceUsage::UnorderedAccess )
        {
            D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
            resourceBarriers.push_back( uavBarrier );
            continue;
        }

        const D3D12_RESOURCE_STATES before = DX12EnumConverter::ConvertResourceUsage( bufferBarrier.OldState );
        const D3D12_RESOURCE_STATES after  = DX12EnumConverter::ConvertResourceUsage( bufferBarrier.NewState );

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

    for ( int i = 0; i < barrier.GetMemoryBarriers( ).NumElements( ); i++ )
    {
        const auto &memoryBarrier = barrier.GetMemoryBarriers( ).GetElement( i );
        bool        isUavBarrier  = IsUavBarrier( memoryBarrier.OldState, memoryBarrier.NewState );

        if ( memoryBarrier.BufferResource != nullptr )
        {
            ID3D12Resource *pResource = dynamic_cast<DX12BufferResource *>( memoryBarrier.BufferResource )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition( pResource, DX12EnumConverter::ConvertResourceUsage( memoryBarrier.OldState ),
                                                                                          DX12EnumConverter::ConvertResourceUsage( memoryBarrier.NewState ) );
                resourceBarriers.push_back( transition );
            }
        }

        if ( memoryBarrier.TextureResource != nullptr )
        {
            ID3D12Resource *pResource = dynamic_cast<DX12TextureResource *>( memoryBarrier.TextureResource )->Resource( );

            if ( isUavBarrier )
            {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
                resourceBarriers.push_back( uavBarrier );
            }
            else
            {
                D3D12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition( pResource, DX12EnumConverter::ConvertResourceUsage( memoryBarrier.OldState ),
                                                                                          DX12EnumConverter::ConvertResourceUsage( memoryBarrier.NewState ) );
                resourceBarriers.push_back( transition );
            }
        }

        if ( memoryBarrier.BottomLevelAS != nullptr )
        {
            auto pResource = dynamic_cast<DX12BottomLevelAS *>( memoryBarrier.BottomLevelAS )->Buffer( )->Resource( );

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
                transition.Transition.StateBefore = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.OldState );
                transition.Transition.StateAfter  = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.NewState );
                resourceBarriers.push_back( transition );
            }
        }

        if ( memoryBarrier.TopLevelAS != nullptr )
        {
            auto pResource = dynamic_cast<DX12TopLevelAS *>( memoryBarrier.TopLevelAS )->Buffer( )->Resource( );

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
                transition.Transition.StateBefore = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.OldState );
                transition.Transition.StateAfter  = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.NewState );
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
