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

#include <DenOfIzGraphics/Backends/DirectX12/DX12CommandList.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12TopLevelAS.h>
#include <utility>

using namespace DenOfIz;

DX12CommandList::DX12CommandList( DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, const wil::com_ptr<ID3D12GraphicsCommandList> &commandList,
                                  const CommandListDesc desc ) : m_desc( desc ), m_context( context ), m_commandAllocator( std::move( commandAllocator ) )
{
    DX_CHECK_RESULT( commandList->QueryInterface( IID_PPV_ARGS( m_commandList.put( ) ) ) );

    switch ( desc.QueueType )
    {
    case QueueType::Graphics:
        m_commandQueue = m_context->GraphicsCommandQueue.get( );
        break;
    case QueueType::RayTracing:
    case QueueType::Compute:
        m_commandQueue = m_context->ComputeCommandQueue.get( );
        break;
    case QueueType::Copy:
        m_commandQueue = m_context->CopyCommandQueue.get( );
        break;
    }

#if not defined( NDEBUG ) && not defined( NSIGHT_ENABLE )
    DX_CHECK_RESULT( m_commandList->QueryInterface( IID_PPV_ARGS( m_debugCommandList.put( ) ) ) );
#endif
}

void DX12CommandList::Begin( )
{
    DX_CHECK_RESULT( m_commandAllocator->Reset( ) );
    DX_CHECK_RESULT( m_commandList->Reset( m_commandAllocator.get( ), nullptr ) );

    m_currentRootSignature = nullptr;
    if ( m_desc.QueueType != QueueType::Copy )
    {
        m_commandList->SetDescriptorHeaps( m_heaps.size( ), m_heaps.data( ) );
    }
}

void DX12CommandList::BeginRendering( const RenderingDesc &renderingDesc )
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets( renderingDesc.RTAttachments.NumElements( ) );
    for ( int i = 0; i < renderingDesc.RTAttachments.NumElements( ); i++ )
    {
        auto *pImageResource = dynamic_cast<DX12TextureResource *>( renderingDesc.RTAttachments.GetElement( i ).Resource );
        renderTargets[ i ]   = pImageResource->GetOrCreateRtvHandle( );

        m_commandList->ClearRenderTargetView( renderTargets[ i ], renderingDesc.RTAttachments.GetElement( i ).ClearColor, 0, nullptr );
    }

    m_commandList->OMSetRenderTargets( renderTargets.size( ), renderTargets.data( ), FALSE, nullptr );
}

void DX12CommandList::EndRendering( )
{
}

void DX12CommandList::Execute( const ExecuteDesc &executeDesc )
{
    DX_CHECK_RESULT( m_commandList->Close( ) );

    for ( int i = 0; i < executeDesc.WaitOnSemaphores.NumElements( ); i++ )
    {
        DX_CHECK_RESULT( m_commandQueue->Wait( dynamic_cast<DX12Semaphore *>( executeDesc.WaitOnSemaphores.GetElement( i ) )->GetFence( ), 1 ) );
    }
    m_commandQueue->ExecuteCommandLists( 1, CommandListCast( m_commandList.addressof( ) ) );

    for ( int i = 0; i < executeDesc.NotifySemaphores.NumElements( ); i++ )
    {
        const auto dx12Semaphore = dynamic_cast<DX12Semaphore *>( executeDesc.NotifySemaphores.GetElement( i ) );
        dx12Semaphore->NotifyCommandQueue( m_commandQueue );
    }
    if ( executeDesc.Notify != nullptr )
    {
        const auto dx12Fence = dynamic_cast<DX12Fence *>( executeDesc.Notify );
        dx12Fence->NotifyCommandQueue( m_commandQueue );
    }
}

void DX12CommandList::Present( ISwapChain *swapChain, uint32_t imageIndex, const InteropArray<ISemaphore *> &waitOnLocks )
{
    DZ_NOT_NULL( swapChain );

    const DX12SwapChain *dx12SwapChain = dynamic_cast<DX12SwapChain *>( swapChain );
    uint32_t             flags         = 0;
    if ( m_context->SelectedDeviceInfo.Capabilities.Tearing )
    {
        flags |= DXGI_PRESENT_ALLOW_TEARING;
    }
    DX_CHECK_RESULT( dx12SwapChain->GetSwapChain( )->Present( 0, flags ) );
}

void DX12CommandList::BindPipeline( IPipeline *pipeline )
{
    DZ_NOT_NULL( pipeline );

    const DX12Pipeline *dx12Pipeline = dynamic_cast<DX12Pipeline *>( pipeline );
    m_currentRootSignature           = dx12Pipeline->GetRootSignature( );

    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        m_commandList->SetGraphicsRootSignature( dx12Pipeline->GetRootSignature( ) );
        m_commandList->IASetPrimitiveTopology( dx12Pipeline->GetTopology( ) );
        m_commandList->SetPipelineState( dx12Pipeline->GetPipeline( ) );
        break;
    case QueueType::RayTracing:
        m_commandList->SetComputeRootSignature( dx12Pipeline->GetRootSignature( ) );
        m_commandList->SetPipelineState1( dx12Pipeline->GetRayTracingSO( ) );
        break;
    case QueueType::Compute:
        m_commandList->SetComputeRootSignature( dx12Pipeline->GetRootSignature( ) );
        m_commandList->SetPipelineState( dx12Pipeline->GetPipeline( ) );
    case QueueType::Copy:
        LOG( ERROR ) << "Copy queue type is not supported for `BindPipeline`";
        break;
    }
}

void DX12CommandList::BindVertexBuffer( IBufferResource *buffer )
{
    DZ_NOT_NULL( buffer );

    const DX12BufferResource *pBuffer = dynamic_cast<DX12BufferResource *>( buffer );

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
    vertexBufferView.BufferLocation           = pBuffer->Resource( )->GetGPUVirtualAddress( );
    vertexBufferView.StrideInBytes            = 8 * sizeof( float ); // pBuffer->GetStride();
    vertexBufferView.SizeInBytes              = pBuffer->NumBytes( );

    m_commandList->IASetVertexBuffers( 0, 1, &vertexBufferView );
}

void DX12CommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType )
{
    DZ_NOT_NULL( buffer );

    const DX12BufferResource *pBuffer = dynamic_cast<DX12BufferResource *>( buffer );

    D3D12_INDEX_BUFFER_VIEW indexBufferView = { };
    indexBufferView.BufferLocation          = pBuffer->Resource( )->GetGPUVirtualAddress( );
    indexBufferView.SizeInBytes             = pBuffer->NumBytes( );
    switch ( indexType )
    {
    case IndexType::Uint16:
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        break;
    case IndexType::Uint32:
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        break;
    }
    m_commandList->IASetIndexBuffer( &indexBufferView );
}

void DX12CommandList::BindViewport( const float x, const float y, const float width, const float height )
{
    m_viewport = CD3DX12_VIEWPORT( x, y, width, height );
    m_commandList->RSSetViewports( 1, &m_viewport );
}

void DX12CommandList::BindScissorRect( const float x, const float y, const float width, const float height )
{
    m_scissor = CD3DX12_RECT( x, y, x + width, y + height );
    m_commandList->RSSetScissorRects( 1, &m_scissor );
}

void DX12CommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    const DX12ResourceBindGroup *dx12BindGroup = dynamic_cast<DX12ResourceBindGroup *>( bindGroup );
    SetRootSignature( dx12BindGroup->RootSignature( )->Instance( ) );

    uint32_t index = 0;
    if ( dx12BindGroup->CbvSrvUavCount( ) > 0 )
    {
        BindResourceGroup( dx12BindGroup->RootSignature( )->RegisterSpaceOffset( dx12BindGroup->RegisterSpace( ) ) + index++, dx12BindGroup->CbvSrvUavHandle( ).Gpu );
    }

    if ( dx12BindGroup->SamplerCount( ) > 0 )
    {
        BindResourceGroup( dx12BindGroup->RootSignature( )->RegisterSpaceOffset( dx12BindGroup->RegisterSpace( ) ) + index, dx12BindGroup->SamplerHandle( ).Gpu );
    }

    for ( const auto &rootConstant : dx12BindGroup->RootConstants( ) )
    {
        SetRootConstants( rootConstant );
    }

    for ( const auto &rootDescriptor : dx12BindGroup->RootDescriptors( ) )
    {
        BindRootDescriptors( rootDescriptor );
    }
}

void DX12CommandList::BindResourceGroup( const uint32_t index, const D3D12_GPU_DESCRIPTOR_HANDLE &gpuHandle ) const
{
    switch ( this->m_desc.QueueType )
    {
    case QueueType::Graphics:
        {
            this->m_commandList->SetGraphicsRootDescriptorTable( index, gpuHandle );
        }
        break;
    case QueueType::RayTracing:
    case QueueType::Compute:
        {
            this->m_commandList->SetComputeRootDescriptorTable( index, gpuHandle );
        }
        break;
    default:
        LOG( ERROR ) << "`BindResourceGroup` is an invalid function for queue type";
        break;
    }
}

void DX12CommandList::BindRootDescriptors( const DX12RootDescriptor &rootDescriptor ) const
{
    switch ( rootDescriptor.ParameterType )
    {
    case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
    case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
        break;
    case D3D12_ROOT_PARAMETER_TYPE_CBV:
        if ( m_desc.QueueType == QueueType::Graphics )
        {
            m_commandList->SetGraphicsRootConstantBufferView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        else
        {
            m_commandList->SetComputeRootConstantBufferView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        break;
    case D3D12_ROOT_PARAMETER_TYPE_SRV:
        if ( m_desc.QueueType == QueueType::Graphics )
        {
            m_commandList->SetGraphicsRootShaderResourceView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        else
        {
            m_commandList->SetComputeRootShaderResourceView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        break;
    case D3D12_ROOT_PARAMETER_TYPE_UAV:
        if ( m_desc.QueueType == QueueType::Graphics )
        {
            m_commandList->SetGraphicsRootUnorderedAccessView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        else
        {
            m_commandList->SetComputeRootUnorderedAccessView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        break;
    }
}

void DX12CommandList::SetRootConstants( const DX12RootConstant &rootConstant ) const
{
    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        m_commandList->SetGraphicsRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    case QueueType::RayTracing:
    case QueueType::Compute:
        m_commandList->SetComputeRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    default:
        LOG( ERROR ) << "`SetRootConstants` is an invalid function for queue type";
        break;
    }
}

void DX12CommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
    if ( m_context->DX12Capabilities.EnhancedBarriers )
    {
        EnhancedPipelineBarrier( barrier );
    }
    else
    {
        CompatibilityPipelineBarrier( barrier );
    }
}

void DX12CommandList::DrawIndexed( const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance )
{
    m_commandList->DrawIndexedInstanced( indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

void DX12CommandList::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    m_commandList->DrawInstanced( vertexCount, instanceCount, firstVertex, firstInstance );
}

void DX12CommandList::Dispatch( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    m_commandList->Dispatch( groupCountX, groupCountY, groupCountZ );
}

void DX12CommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo )
{
    DZ_NOT_NULL( copyBufferRegionInfo.DstBuffer );
    DZ_NOT_NULL( copyBufferRegionInfo.SrcBuffer );

    const DX12BufferResource *dstBuffer = dynamic_cast<DX12BufferResource *>( copyBufferRegionInfo.DstBuffer );
    const DX12BufferResource *srcBuffer = dynamic_cast<DX12BufferResource *>( copyBufferRegionInfo.SrcBuffer );

    m_commandList->CopyBufferRegion( dstBuffer->Resource( ), copyBufferRegionInfo.DstOffset, srcBuffer->Resource( ), copyBufferRegionInfo.SrcOffset,
                                     copyBufferRegionInfo.NumBytes );
}

void DX12CommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo )
{
    DZ_NOT_NULL( copyTextureRegionInfo.DstTexture );
    DZ_NOT_NULL( copyTextureRegionInfo.SrcTexture );

    const DX12TextureResource *dstTexture = dynamic_cast<DX12TextureResource *>( copyTextureRegionInfo.DstTexture );
    const DX12TextureResource *srcTexture = dynamic_cast<DX12TextureResource *>( copyTextureRegionInfo.SrcTexture );

    D3D12_TEXTURE_COPY_LOCATION src = { };
    src.pResource                   = srcTexture->GetResource( );
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex            = copyTextureRegionInfo.SrcMipLevel;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstTexture->GetResource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyTextureRegionInfo.DstMipLevel;

    D3D12_BOX box = { };
    box.left      = copyTextureRegionInfo.SrcX;
    box.top       = copyTextureRegionInfo.SrcY;
    box.front     = copyTextureRegionInfo.SrcZ;
    box.right     = copyTextureRegionInfo.SrcX + copyTextureRegionInfo.Width;
    box.bottom    = copyTextureRegionInfo.SrcY + copyTextureRegionInfo.Height;
    box.back      = copyTextureRegionInfo.SrcZ + copyTextureRegionInfo.Depth;

    m_commandList->CopyTextureRegion( &dst, copyTextureRegionInfo.DstX, copyTextureRegionInfo.DstY, copyTextureRegionInfo.DstZ, &src, &box );
}

void DX12CommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture )
{
    DZ_NOT_NULL( copyBufferToTexture.DstTexture );
    DZ_NOT_NULL( copyBufferToTexture.SrcBuffer );

    const DX12TextureResource *dstTexture = dynamic_cast<DX12TextureResource *>( copyBufferToTexture.DstTexture );
    const DX12BufferResource  *srcBuffer  = dynamic_cast<DX12BufferResource *>( copyBufferToTexture.SrcBuffer );

    const TextureDesc           dstDesc = dstTexture->GetDesc( );
    D3D12_TEXTURE_COPY_LOCATION src     = { };
    src.pResource                       = srcBuffer->Resource( );
    src.Type                            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    const uint32_t subresource          = D3D12CalcSubresource( copyBufferToTexture.MipLevel, copyBufferToTexture.ArrayLayer, 0, dstDesc.MipLevels, dstDesc.ArraySize );
    m_context->D3DDevice->GetCopyableFootprints( &dstTexture->GetResourceDesc( ), subresource, 1, copyBufferToTexture.SrcOffset, &src.PlacedFootprint, nullptr, nullptr, nullptr );
    src.PlacedFootprint.Offset = copyBufferToTexture.SrcOffset;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstTexture->GetResource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyBufferToTexture.MipLevel;
    m_commandList->CopyTextureRegion( &dst, 0, 0, 0, &src, nullptr );
}

void DX12CommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer )
{
    DZ_NOT_NULL( copyTextureToBuffer.DstBuffer );
    DZ_NOT_NULL( copyTextureToBuffer.SrcTexture );

    const auto *dstBuffer  = dynamic_cast<DX12BufferResource *>( copyTextureToBuffer.DstBuffer );
    const auto *srcTexture = dynamic_cast<DX12TextureResource *>( copyTextureToBuffer.SrcTexture );

    D3D12_TEXTURE_COPY_LOCATION src = { };
    src.pResource                   = srcTexture->GetResource( );
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex            = copyTextureToBuffer.MipLevel;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstBuffer->Resource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    const uint32_t subresource      = copyTextureToBuffer.ArrayLayer * srcTexture->GetDesc( ).MipLevels + copyTextureToBuffer.MipLevel;
    m_context->D3DDevice->GetCopyableFootprints( &srcTexture->GetResourceDesc( ), subresource, 1, copyTextureToBuffer.DstOffset, &dst.PlacedFootprint, nullptr, nullptr, nullptr );

    m_commandList->CopyTextureRegion( &dst, copyTextureToBuffer.DstOffset, 0, 0, &src, nullptr );
}

void DX12CommandList::BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc )
{
    const auto dx12TopLevelAS = dynamic_cast<DX12TopLevelAS *>( buildTopLevelASDesc.TopLevelAS );
    DZ_NOT_NULL( dx12TopLevelAS );

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = { };
    buildDesc.DestAccelerationStructureData                      = dx12TopLevelAS->DX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.Inputs.DescsLayout                                 = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                        = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    buildDesc.Inputs.Flags                                       = dx12TopLevelAS->Flags( );
    buildDesc.Inputs.NumDescs                                    = dx12TopLevelAS->NumInstances( );
    buildDesc.Inputs.InstanceDescs                               = dx12TopLevelAS->InstanceBuffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                   = dx12TopLevelAS->Scratch( )->Resource( )->GetGPUVirtualAddress( );

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    const auto dx12BottomLevelAS = dynamic_cast<DX12BottomLevelAS *>( buildBottomLevelASDesc.BottomLevelAS );
    DZ_NOT_NULL( dx12BottomLevelAS );

    const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> &geometryDescs = dx12BottomLevelAS->GeometryDescs( );
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc     = { };
    buildDesc.Inputs.DescsLayout                                     = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                            = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    buildDesc.Inputs.Flags                                           = dx12BottomLevelAS->Flags( );
    buildDesc.Inputs.NumDescs                                        = geometryDescs.size( );
    buildDesc.Inputs.pGeometryDescs                                  = geometryDescs.data( );
    buildDesc.DestAccelerationStructureData                          = dx12BottomLevelAS->Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                       = dx12BottomLevelAS->Scratch( )->Resource( )->GetGPUVirtualAddress( );

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::DispatchRays( const DispatchRaysDesc &dispatchRaysDesc )
{
    const DX12ShaderBindingTable *sbt  = dynamic_cast<DX12ShaderBindingTable *>( dispatchRaysDesc.ShaderBindingTable );
    D3D12_DISPATCH_RAYS_DESC      desc = { };
    desc.RayGenerationShaderRecord     = sbt->RayGenerationShaderRecord( );
    desc.MissShaderTable               = sbt->MissShaderRange( );
    desc.HitGroupTable                 = sbt->HitGroupShaderRange( );

    desc.Width  = dispatchRaysDesc.Width;
    desc.Height = dispatchRaysDesc.Height;
    desc.Depth  = dispatchRaysDesc.Depth;
    m_commandList->DispatchRays( &desc );
}

void DX12CommandList::CompatibilityPipelineBarrier( const PipelineBarrierDesc &barrier ) const
{
    std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

    const InteropArray<TextureBarrierDesc> &textureBarriers = barrier.GetTextureBarriers( );
    const InteropArray<BufferBarrierDesc>  &bufferBarriers  = barrier.GetBufferBarriers( );

    for ( int i = 0; i < textureBarriers.NumElements( ); i++ )
    {
        const TextureBarrierDesc   &textureBarrier  = textureBarriers.GetElement( i );
        ID3D12Resource             *pResource       = dynamic_cast<DX12TextureResource *>( textureBarrier.Resource )->GetResource( );
        const D3D12_RESOURCE_STATES before          = DX12EnumConverter::ConvertResourceUsage( textureBarrier.OldState );
        const D3D12_RESOURCE_STATES after           = DX12EnumConverter::ConvertResourceUsage( textureBarrier.NewState );
        D3D12_RESOURCE_BARRIER      resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition( pResource, before, after );

        if ( before != after )
        {
            resourceBarriers.push_back( resourceBarrier );
        }
    }

    for ( int i = 0; i < bufferBarriers.NumElements( ); i++ )
    {
        const BufferBarrierDesc    &bufferBarrier   = bufferBarriers.GetElement( i );
        ID3D12Resource             *pResource       = dynamic_cast<DX12BufferResource *>( bufferBarrier.Resource )->Resource( );
        const D3D12_RESOURCE_STATES before          = DX12EnumConverter::ConvertResourceUsage( bufferBarrier.OldState );
        const D3D12_RESOURCE_STATES after           = DX12EnumConverter::ConvertResourceUsage( bufferBarrier.NewState );
        D3D12_RESOURCE_BARRIER      resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition( pResource, before, after );
        if ( before != after )
        {
            resourceBarriers.push_back( resourceBarrier );
        }
    }

    for ( int i = 0; i < barrier.GetMemoryBarriers( ).NumElements( ); i++ )
    {
        const MemoryBarrierDesc &memoryBarrier = barrier.GetMemoryBarriers( ).GetElement( i );

        // Special Cases, Uav Barrier:
        bool isUavBarrier = memoryBarrier.OldState.IsSet( ResourceUsage::AccelerationStructureWrite ) && memoryBarrier.NewState.IsSet( ResourceUsage::AccelerationStructureRead );
        isUavBarrier |= memoryBarrier.OldState.IsSet( ResourceUsage::DepthWrite ) && memoryBarrier.NewState.IsSet( ResourceUsage::DepthRead );

        ID3D12Resource *dx12Resource = nullptr;
        if ( memoryBarrier.BufferResource != nullptr )
        {
            const auto *bufferResource = dynamic_cast<DX12BufferResource *>( memoryBarrier.BufferResource );
            dx12Resource               = bufferResource->Resource( );
        }
        if ( memoryBarrier.TextureResource != nullptr )
        {
            const auto *textureResource = dynamic_cast<DX12TextureResource *>( memoryBarrier.TextureResource );
            dx12Resource                = textureResource->GetResource( );
        }
        if ( isUavBarrier )
        {
            D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::UAV( dx12Resource );
            resourceBarriers.push_back( resourceBarrier );
        }
        else
        {
            const D3D12_RESOURCE_STATES before = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.OldState );
            const D3D12_RESOURCE_STATES after  = DX12EnumConverter::ConvertResourceUsage( memoryBarrier.NewState );

            D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition( dx12Resource, before, after );
            resourceBarriers.push_back( resourceBarrier );
        }
    }

    if ( !resourceBarriers.empty( ) )
    {
        m_commandList->ResourceBarrier( resourceBarriers.size( ), resourceBarriers.data( ) );
    }
}

void DX12CommandList::EnhancedPipelineBarrier( const PipelineBarrierDesc &barrier ) const
{
    std::vector<D3D12_BARRIER_GROUP> resourceBarriers;

    std::vector<D3D12_GLOBAL_BARRIER>  dxGlobalBarriers  = { };
    std::vector<D3D12_BUFFER_BARRIER>  dxBufferBarriers  = { };
    std::vector<D3D12_TEXTURE_BARRIER> dxTextureBarriers = { };

    const InteropArray<TextureBarrierDesc> &textureBarriers = barrier.GetTextureBarriers( );
    const InteropArray<BufferBarrierDesc>  &bufferBarriers  = barrier.GetBufferBarriers( );

    for ( int i = 0; i < textureBarriers.NumElements( ); i++ )
    {
        const TextureBarrierDesc &textureBarrier = textureBarriers.GetElement( i );
        ID3D12Resource           *pResource      = dynamic_cast<DX12TextureResource *>( textureBarrier.Resource )->GetResource( );

        D3D12_TEXTURE_BARRIER dxTextureBarrier = dxTextureBarriers.emplace_back( D3D12_TEXTURE_BARRIER{ } );
        dxTextureBarrier.pResource             = pResource;
        dxTextureBarrier.LayoutBefore          = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.OldState, m_desc.QueueType );
        dxTextureBarrier.LayoutAfter           = DX12EnumConverter::ConvertResourceStateToBarrierLayout( textureBarrier.NewState, m_desc.QueueType );
        dxTextureBarrier.AccessBefore          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.OldState );
        dxTextureBarrier.AccessAfter           = DX12EnumConverter::ConvertResourceStateToBarrierAccess( textureBarrier.NewState );
        // Todo dxTextureBarrier.Subresource, dxTextureBarrier.SyncBefore and dxTextureBarrier.SyncAfter

        if ( dxTextureBarrier.LayoutAfter != dxTextureBarrier.LayoutBefore || dxTextureBarrier.AccessAfter != dxTextureBarrier.AccessBefore )
        {
            dxTextureBarriers.push_back( dxTextureBarrier );
        }
    }

    for ( int i = 0; i < bufferBarriers.NumElements( ); i++ )
    {
        const BufferBarrierDesc &bufferBarrier = bufferBarriers.GetElement( i );
        ID3D12Resource          *pResource     = dynamic_cast<DX12TextureResource *>( bufferBarrier.Resource )->GetResource( );

        D3D12_BUFFER_BARRIER dxBufferBarrier = dxBufferBarriers.emplace_back( D3D12_BUFFER_BARRIER{ } );
        dxBufferBarrier.pResource            = pResource;
        dxBufferBarrier.AccessBefore         = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.OldState );
        dxBufferBarrier.AccessAfter          = DX12EnumConverter::ConvertResourceStateToBarrierAccess( bufferBarrier.NewState );
        dxBufferBarrier.Offset               = 0;
        dxBufferBarrier.Size                 = pResource->GetDesc( ).Width;

        if ( dxBufferBarrier.AccessAfter != dxBufferBarrier.AccessBefore )
        {
            dxBufferBarriers.push_back( dxBufferBarrier );
        }
    }

    D3D12_BARRIER_GROUP textureBarrierGroup = resourceBarriers.emplace_back( D3D12_BARRIER_GROUP{ } );
    textureBarrierGroup.Type                = D3D12_BARRIER_TYPE_TEXTURE;
    textureBarrierGroup.NumBarriers         = dxTextureBarriers.size( );
    textureBarrierGroup.pTextureBarriers    = dxTextureBarriers.data( );

    if ( !resourceBarriers.empty( ) )
    {
        m_commandList->Barrier( resourceBarriers.size( ), resourceBarriers.data( ) );
    }
}

void DX12CommandList::SetRootSignature( ID3D12RootSignature *rootSignature )
{
    DZ_RETURN_IF( rootSignature == nullptr );
    DZ_RETURN_IF( rootSignature == m_currentRootSignature );

    if ( m_currentRootSignature != nullptr )
    {
        LOG( WARNING ) << "Root signature is set to a different value, it is not expected to overwrite this value.";
    }

    m_currentRootSignature = rootSignature;
    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        m_commandList->SetGraphicsRootSignature( rootSignature );
        break;
    case QueueType::Compute:
    case QueueType::RayTracing:
        m_commandList->SetComputeRootSignature( rootSignature );
        break;
    default:
        LOG( ERROR ) << "SetRootSignature is an invalid function for queue type";
        break;
    }
}
