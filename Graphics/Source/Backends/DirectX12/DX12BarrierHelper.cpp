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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BarrierHelper.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12EnumConverter.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12TextureResource.h>

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

bool IsUAVState( const ResourceUsage state )
{
    return state == ResourceUsage::UnorderedAccess || state == ResourceUsage::AccelerationStructureWrite || state == ResourceUsage::AccelerationStructureRead;
}

UINT CalcSubresourceIndex( const uint32_t mipLevel, const uint32_t layer, const uint32_t depth, const uint32_t mipLevels, const uint32_t depthOrArraySize )
{
    return mipLevel * layer * depth * mipLevels * depthOrArraySize;
}

D3D12_BARRIER_SYNC GetSyncFlagsForState( const BitSet<ResourceUsage> &state )
{
    D3D12_BARRIER_SYNC syncFlags = D3D12_BARRIER_SYNC_NONE;

    if ( state.IsSet( ResourceUsage::RenderTarget ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    }
    if ( state.IsSet( ResourceUsage::UnorderedAccess ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_ALL;
    }
    if ( state.IsSet( ResourceUsage::DepthWrite ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state.IsSet( ResourceUsage::DepthRead ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if ( state.IsSet( ResourceUsage::CopyDst ) || state.IsSet( ResourceUsage::CopySrc ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_COPY;
    }
    if ( state.IsSet( ResourceUsage::AccelerationStructureWrite ) || state.IsSet( ResourceUsage::AccelerationStructureRead ) )
    {
        syncFlags |= D3D12_BARRIER_SYNC_RAYTRACING;
    }

    return syncFlags != D3D12_BARRIER_SYNC_NONE ? syncFlags : D3D12_BARRIER_SYNC_ALL;
}

void DX12BarrierHelper::ExecuteEnhancedResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const QueueType &queueType, const PipelineBarrierDesc &barrier )
{
    std::vector<D3D12_BARRIER_GROUP> barrierGroups;

    std::vector<D3D12_GLOBAL_BARRIER>  globalBarriers;
    std::vector<D3D12_BUFFER_BARRIER>  bufferBarriers;
    std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;

    bool needsGlobalSync = false;
    for ( int i = 0; i < barrier.GetTextureBarriers( ).NumElements( ); i++ )
    {
        if ( const auto &textureBarrier = barrier.GetTextureBarriers( ).GetElement( i ); IsUAVState( textureBarrier.OldState ) || IsUAVState( textureBarrier.NewState ) )
        {
            needsGlobalSync = true;
            break;
        }
    }
    if ( needsGlobalSync )
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

        if ( bufferBarrier.OldState.IsSet( ResourceUsage::UnorderedAccess ) && bufferBarrier.NewState.IsSet( ResourceUsage::UnorderedAccess ) )
        {
            dxBufferBarrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
            dxBufferBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
            dxBufferBarrier.SyncBefore   = D3D12_BARRIER_SYNC_ALL;
            dxBufferBarrier.SyncAfter    = D3D12_BARRIER_SYNC_ALL;
        }
        else
        {
            dxBufferBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.OldState, queueType );
            dxBufferBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.NewState, queueType );
            dxBufferBarrier.SyncBefore   = GetSyncFlagsForState( bufferBarrier.OldState );
            dxBufferBarrier.SyncAfter    = GetSyncFlagsForState( bufferBarrier.NewState );
        }

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
            D3D12_TEXTURE_BARRIER toCommon = dxTextureBarrier;
            toCommon.LayoutBefore          = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.OldState, textureBarrier.SourceQueue, true );
            toCommon.LayoutAfter           = D3D12_BARRIER_LAYOUT_COMMON;
            toCommon.AccessBefore          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.OldState, textureBarrier.SourceQueue );
            toCommon.AccessAfter           = D3D12_BARRIER_ACCESS_COMMON;
            toCommon.SyncBefore            = GetSyncFlagsForState( textureBarrier.OldState );
            toCommon.SyncAfter             = D3D12_BARRIER_SYNC_ALL;
            textureBarriers.push_back( toCommon );

            D3D12_TEXTURE_BARRIER fromCommon = dxTextureBarrier;
            fromCommon.LayoutBefore          = D3D12_BARRIER_LAYOUT_COMMON;
            fromCommon.LayoutAfter           = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.NewState, textureBarrier.DestinationQueue, true );
            fromCommon.AccessBefore          = D3D12_BARRIER_ACCESS_COMMON;
            fromCommon.AccessAfter           = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.NewState, textureBarrier.DestinationQueue );
            fromCommon.SyncBefore            = D3D12_BARRIER_SYNC_ALL;
            fromCommon.SyncAfter             = GetSyncFlagsForState( textureBarrier.NewState );
            textureBarriers.push_back( fromCommon );
        }
        else
        {
            dxTextureBarrier.LayoutBefore = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.OldState, queueType, true );
            dxTextureBarrier.LayoutAfter  = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.NewState, queueType, true );
            dxTextureBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.OldState, queueType );
            dxTextureBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.NewState, queueType );
            dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( textureBarrier.OldState );
            dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( textureBarrier.NewState );
            textureBarriers.push_back( dxTextureBarrier );
        }
    }

    for ( int i = 0; i < barrier.GetMemoryBarriers( ).NumElements( ); i++ )
    {
        const auto &memoryBarrier = barrier.GetMemoryBarriers( ).GetElement( i );

        // Handle buffer memory barriers
        if ( memoryBarrier.BufferResource != nullptr )
        {
            ID3D12Resource *pResource = dynamic_cast<DX12BufferResource *>( memoryBarrier.BufferResource )->Resource( );

            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = pResource;
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;

            // Special case for UAV barriers
            if ( memoryBarrier.OldState.IsSet( ResourceUsage::UnorderedAccess ) && memoryBarrier.NewState.IsSet( ResourceUsage::UnorderedAccess ) )
            {
                dxBufferBarrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
                dxBufferBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
                dxBufferBarrier.SyncBefore   = D3D12_BARRIER_SYNC_ALL;
                dxBufferBarrier.SyncAfter    = D3D12_BARRIER_SYNC_ALL;
            }
            else
            {
                dxBufferBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
                dxBufferBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
                dxBufferBarrier.SyncBefore   = GetSyncFlagsForState( memoryBarrier.OldState );
                dxBufferBarrier.SyncAfter    = GetSyncFlagsForState( memoryBarrier.NewState );
            }

            bufferBarriers.push_back( dxBufferBarrier );
        }

        // Handle texture memory barriers
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

            // Special case for UAV memory barriers
            if ( memoryBarrier.OldState.IsSet( ResourceUsage::UnorderedAccess ) && memoryBarrier.NewState.IsSet( ResourceUsage::UnorderedAccess ) )
            {
                dxTextureBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
                dxTextureBarrier.LayoutAfter  = D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
                dxTextureBarrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
                dxTextureBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
                dxTextureBarrier.SyncBefore   = D3D12_BARRIER_SYNC_ALL;
                dxTextureBarrier.SyncAfter    = D3D12_BARRIER_SYNC_ALL;
            }
            else
            {
                dxTextureBarrier.LayoutBefore = DX12EnumConverter::ConvertResourceStateToBarrierLayout( memoryBarrier.OldState, queueType, true );
                dxTextureBarrier.LayoutAfter  = DX12EnumConverter::ConvertResourceStateToBarrierLayout( memoryBarrier.NewState, queueType, true );
                dxTextureBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
                dxTextureBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
                dxTextureBarrier.SyncBefore   = GetSyncFlagsForState( memoryBarrier.OldState );
                dxTextureBarrier.SyncAfter    = GetSyncFlagsForState( memoryBarrier.NewState );
            }

            textureBarriers.push_back( dxTextureBarrier );
        }

        if ( memoryBarrier.BottomLevelAS != nullptr )
        {
            D3D12_BUFFER_BARRIER dxBufferBarrier = { };
            dxBufferBarrier.pResource            = dynamic_cast<DX12BufferResource *>( memoryBarrier.BottomLevelAS )->Resource( );
            dxBufferBarrier.Offset               = 0;
            dxBufferBarrier.Size                 = dxBufferBarrier.pResource->GetDesc( ).Width;

            if ( memoryBarrier.OldState.IsSet( ResourceUsage::AccelerationStructureWrite ) && memoryBarrier.NewState.IsSet( ResourceUsage::AccelerationStructureRead ) )
            {
                dxBufferBarrier.AccessBefore = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
                dxBufferBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
                dxBufferBarrier.SyncBefore   = D3D12_BARRIER_SYNC_RAYTRACING;
                dxBufferBarrier.SyncAfter    = D3D12_BARRIER_SYNC_RAYTRACING;
            }
            else
            {
                dxBufferBarrier.AccessBefore = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.OldState, queueType );
                dxBufferBarrier.AccessAfter  = DX12EnumConverter::ConvertResourceStateToBarrierAccess( memoryBarrier.NewState, queueType );
                dxBufferBarrier.SyncBefore   = GetSyncFlagsForState( memoryBarrier.OldState );
                dxBufferBarrier.SyncAfter    = GetSyncFlagsForState( memoryBarrier.NewState );
            }

            bufferBarriers.push_back( dxBufferBarrier );
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
    for ( int i = 0; i < barrier.GetTextureBarriers( ).NumElements( ); i++ )
    {
        const auto     &textureBarrier = barrier.GetTextureBarriers( ).GetElement( i );
        ID3D12Resource *pResource      = dynamic_cast<DX12TextureResource *>( textureBarrier.Resource )->Resource( );

        if ( textureBarrier.OldState.IsSet( ResourceUsage::UnorderedAccess ) && textureBarrier.NewState.IsSet( ResourceUsage::UnorderedAccess ) )
        {
            D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV( pResource );
            resourceBarriers.push_back( uavBarrier );
            continue;
        }
        UINT subresource = textureBarrier.EnableSubresourceBarrier ? CalcSubresourceIndex( textureBarrier.MipLevel, textureBarrier.ArrayLayer, 0, pResource->GetDesc( ).MipLevels,
                                                                                           pResource->GetDesc( ).DepthOrArraySize )
                                                                   : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        if ( textureBarrier.EnableQueueBarrier && textureBarrier.SourceQueue != textureBarrier.DestinationQueue )
        {
            D3D12_RESOURCE_BARRIER toCommon =
                CD3DX12_RESOURCE_BARRIER::Transition( pResource, DX12EnumConverter::ConvertResourceUsage( textureBarrier.OldState ), D3D12_RESOURCE_STATE_COMMON );

            D3D12_RESOURCE_BARRIER fromCommon =
                CD3DX12_RESOURCE_BARRIER::Transition( pResource, D3D12_RESOURCE_STATE_COMMON, DX12EnumConverter::ConvertResourceUsage( textureBarrier.NewState ) );

            resourceBarriers.push_back( toCommon );
            resourceBarriers.push_back( fromCommon );
        }
        else
        {
            D3D12_RESOURCE_BARRIER transition = { };
            transition.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            transition.Transition.pResource   = pResource;
            transition.Transition.Subresource = subresource;
            transition.Transition.StateBefore = DX12EnumConverter::ConvertResourceUsage( textureBarrier.OldState );
            transition.Transition.StateAfter  = DX12EnumConverter::ConvertResourceUsage( textureBarrier.NewState );

            resourceBarriers.push_back( transition );
        }
    }

    for ( int i = 0; i < barrier.GetBufferBarriers( ).NumElements( ); i++ )
    {
        const auto     &bufferBarrier = barrier.GetBufferBarriers( ).GetElement( i );
        ID3D12Resource *pResource     = dynamic_cast<DX12BufferResource *>( bufferBarrier.Resource )->Resource( );

        if ( bufferBarrier.OldState.IsSet( ResourceUsage::UnorderedAccess ) && bufferBarrier.NewState.IsSet( ResourceUsage::UnorderedAccess ) )
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
        bool isUavBarrier = memoryBarrier.OldState.IsSet( ResourceUsage::AccelerationStructureWrite ) && memoryBarrier.NewState.IsSet( ResourceUsage::AccelerationStructureRead );
        isUavBarrier |= memoryBarrier.OldState.IsSet( ResourceUsage::DepthWrite ) && memoryBarrier.NewState.IsSet( ResourceUsage::DepthRead );

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
    }

    if ( !resourceBarriers.empty( ) )
    {
        commandList->ResourceBarrier( static_cast<UINT>( resourceBarriers.size( ) ), resourceBarriers.data( ) );
    }
}
