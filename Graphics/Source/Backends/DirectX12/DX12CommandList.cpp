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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandList.h"
#include <utility>
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BarrierHelper.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12IndirectSignatureCache.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12QueryPool.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h"
#ifdef USE_PIX
#include <pix3.h>
#endif
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12CommandList::DX12CommandList( DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, const wil::com_ptr<ID3D12GraphicsCommandList> &commandList,
                                  const DenOfIz_CommandListDesc desc ) : m_desc( desc ), m_context( context ), m_commandAllocator( std::move( commandAllocator ) )
{
    DX_CHECK_RESULT( commandList->QueryInterface( IID_PPV_ARGS( m_commandList.put( ) ) ) );

#if not defined( NDEBUG ) && not defined( NSIGHT_ENABLE )
    DX_CHECK_RESULT( m_commandList->QueryInterface( IID_PPV_ARGS( m_debugCommandList.put( ) ) ) );
#endif
}

void DX12CommandList::Begin( )
{
    DX_CHECK_RESULT( m_commandAllocator->Reset( ) );
    DX_CHECK_RESULT( m_commandList->Reset( m_commandAllocator.get( ), nullptr ) );
    m_queuedBindGroups.clear( );
    m_queuedRootConstants.clear( );

    m_currentRootSignature = nullptr;
    m_currentPipeline      = nullptr;
    m_deferredVertexBuffers.clear( );
}

void DX12CommandList::BeginRendering( const DenOfIz_RenderingDesc &renderingDesc )
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets( renderingDesc.RTAttachments.NumElements );
    for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; i++ )
    {
        const auto &rtAttachment = renderingDesc.RTAttachments.Elements[ i ];
        if ( !DENOFIZ_HANDLE_IS_VALID( rtAttachment.Resource ) )
        {
            spdlog::error( "BeginRendering called with null render target attachment at index {}", i );
            return;
        }
        auto *pImageResource = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( ITexture, rtAttachment.Resource ) );
        renderTargets[ i ]   = pImageResource->GetOrCreateRtvHandle( );
        if ( rtAttachment.LoadOp == DENOFIZ_LOAD_OP_CLEAR )
        {
            const DenOfIz_Float4 clearColor           = rtAttachment.ClearColor;
            const float          clearColorArray[ 4 ] = { clearColor.X, clearColor.Y, clearColor.Z, clearColor.W };
            m_commandList->ClearRenderTargetView( renderTargets[ i ], clearColorArray, 0, nullptr );
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle       = { };
    bool                        hasDepthStencil = false;
    if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.DepthAttachment.Resource ) )
    {
        auto *pDepthResource = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( ITexture, renderingDesc.DepthAttachment.Resource ) );
        dsvHandle            = pDepthResource->GetOrCreateDsvHandle( );
        hasDepthStencil      = true;

        const DenOfIz_Format depthFormat = pDepthResource->GetFormat( );
        if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) && DenOfIz_Format_IsDepthOnly( depthFormat ) )
        {
            spdlog::warn( "DX12CommandList::BeginRendering: Separate stencil attachment provided with depth-only format ({}). "
                          "This configuration may not work as expected.",
                          static_cast<int>( depthFormat ) );
        }

        if ( renderingDesc.DepthAttachment.LoadOp == DENOFIZ_LOAD_OP_CLEAR )
        {
            D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
            if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) || DenOfIz_Format_HasStencilComponent( depthFormat ) )
            {
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
            }

            m_commandList->ClearDepthStencilView( dsvHandle, clearFlags, renderingDesc.DepthAttachment.ClearDepthStencil.X,
                                                  static_cast<UINT8>( renderingDesc.DepthAttachment.ClearDepthStencil.Y ), 0, nullptr );
        }
    }
    else if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) )
    {
        auto *pStencilResource = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( ITexture, renderingDesc.StencilAttachment.Resource ) );
        dsvHandle              = pStencilResource->GetOrCreateDsvHandle( );
        hasDepthStencil        = true;

        const DenOfIz_Format stencilFormat = pStencilResource->GetFormat( );
        if ( !DenOfIz_Format_HasStencilComponent( stencilFormat ) )
        {
            spdlog::error( "DX12CommandList::BeginRendering: Stencil attachment provided with format ({}) that has no stencil component.", static_cast<int>( stencilFormat ) );
        }

        if ( renderingDesc.StencilAttachment.LoadOp == DENOFIZ_LOAD_OP_CLEAR )
        {
            m_commandList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_STENCIL, 1.0f, static_cast<UINT8>( renderingDesc.StencilAttachment.ClearDepthStencil.Y ), 0,
                                                  nullptr );
        }
    }

    m_commandList->OMSetRenderTargets( static_cast<UINT>( renderTargets.size( ) ), renderTargets.empty( ) ? nullptr : renderTargets.data( ), FALSE,
                                       hasDepthStencil ? &dsvHandle : nullptr );
}

void DX12CommandList::EndRendering( )
{
}

void DX12CommandList::End( )
{
    DX_CHECK_RESULT( m_commandList->Close( ) );
}

void DX12CommandList::BindPipeline( IPipeline *pipeline )
{
    DZ_NOT_NULL( pipeline );

    m_currentPipeline = dynamic_cast<DX12Pipeline *>( pipeline );
    SetRootSignature( m_currentPipeline->GetRootSignature( ) );

    if ( !m_deferredVertexBuffers.empty( ) )
    {
        for ( const auto &deferred : m_deferredVertexBuffers )
        {
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
            vertexBufferView.BufferLocation           = deferred.Buffer->Resource( )->GetGPUVirtualAddress( ) + deferred.Offset;
            vertexBufferView.SizeInBytes              = deferred.Buffer->NumBytes( ) - deferred.Offset;

            if ( deferred.Stride > 0 )
            {
                vertexBufferView.StrideInBytes = deferred.Stride;
            }
            else
            {
                vertexBufferView.StrideInBytes = m_currentPipeline->GetIAStride( );
            }

            m_commandList->IASetVertexBuffers( deferred.Slot, 1, &vertexBufferView );
        }
        m_deferredVertexBuffers.clear( );
    }

    if ( m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_RAYTRACING )
    {
        m_commandList->SetPipelineState1( m_currentPipeline->GetRayTracingSO( ) );
    }
    else
    {
        m_commandList->SetPipelineState( m_currentPipeline->GetPipeline( ) );
        if ( m_currentPipeline->GetBindPoint( ) != DENOFIZ_BIND_POINT_COMPUTE )
        {
            m_commandList->IASetPrimitiveTopology( m_currentPipeline->GetTopology( ) );
        }
    }
}

void DX12CommandList::BindVertexBuffer( IBuffer *buffer, const uint64_t offset, const uint32_t stride, const uint32_t slot )
{
    DZ_NOT_NULL( buffer );

    const auto pBuffer = dynamic_cast<DX12Buffer *>( buffer );

    if ( !m_currentPipeline )
    {
        DeferredVertexBuffer deferred;
        deferred.Buffer = pBuffer;
        deferred.Offset = offset;
        deferred.Stride = stride;
        deferred.Slot   = slot;
        m_deferredVertexBuffers.push_back( deferred );
        return;
    }

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
    vertexBufferView.BufferLocation           = pBuffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    vertexBufferView.SizeInBytes              = pBuffer->NumBytes( ) - offset;

    if ( stride > 0 )
    {
        vertexBufferView.StrideInBytes = stride;
    }
    else
    {
        vertexBufferView.StrideInBytes = m_currentPipeline->GetIAStride( );
    }

    m_commandList->IASetVertexBuffers( slot, 1, &vertexBufferView );
}

void DX12CommandList::BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, const uint64_t offset )
{
    DZ_NOT_NULL( buffer );

    const DX12Buffer *pBuffer = dynamic_cast<DX12Buffer *>( buffer );

    D3D12_INDEX_BUFFER_VIEW indexBufferView = { };
    indexBufferView.BufferLocation          = pBuffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    indexBufferView.SizeInBytes             = pBuffer->NumBytes( ) - offset;
    switch ( indexType )
    {
    case DENOFIZ_INDEX_TYPE_UINT16:
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        break;
    case DENOFIZ_INDEX_TYPE_UINT32:
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        break;
    }
    m_commandList->IASetIndexBuffer( &indexBufferView );
}

void DX12CommandList::BindViewport( const float x, const float y, const float width, const float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid viewport dimensions: width= {} , height={}", width, height );
        return;
    }

    m_viewport = CD3DX12_VIEWPORT( x, y, width, height );
    m_commandList->RSSetViewports( 1, &m_viewport );
}

void DX12CommandList::BindScissorRect( const float x, const float y, const float width, const float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid scissor rect dimensions: width= {} , height={}", width, height );
        return;
    }

    m_scissor = CD3DX12_RECT( x, y, x + width, y + height );
    m_commandList->RSSetScissorRects( 1, &m_scissor );
}

void DX12CommandList::BindResourceGroup( IBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );

    const DX12BindGroup *dx12BindGroup = dynamic_cast<DX12BindGroup *>( bindGroup );
    m_queuedBindGroups.push_back( dx12BindGroup );
}

void DX12CommandList::SetRootConstants( const uint32_t binding, const DenOfIz_ByteArray &data )
{
    DX12RootConstant rootConstant;
    rootConstant.Binding  = binding;
    rootConstant.Data     = data.Elements;
    rootConstant.NumBytes = static_cast<uint32_t>( data.NumElements );
    m_queuedRootConstants.push_back( rootConstant );
}

void DX12CommandList::ProcessBindGroups( )
{
    if ( m_currentPipeline == nullptr )
    {
        return;
    }

    DX12RootSignature *rootSignature = m_currentPipeline->RootSignature( );
    if ( rootSignature == nullptr )
    {
        return;
    }

    m_heaps.clear( );

    ID3D12DescriptorHeap *cbvSrvUavHeap = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetActiveHeap( );
    ID3D12DescriptorHeap *samplerHeap   = m_context->ShaderVisibleSamplerDescriptorHeap->GetActiveHeap( );

    if ( cbvSrvUavHeap )
    {
        m_heaps.push_back( cbvSrvUavHeap );
    }
    if ( samplerHeap )
    {
        m_heaps.push_back( samplerHeap );
    }

    if ( !m_heaps.empty( ) )
    {
        m_commandList->SetDescriptorHeaps( static_cast<UINT>( m_heaps.size( ) ), m_heaps.data( ) );
    }

    SetRootSignature( rootSignature->Instance( ) );

    for ( const auto &dx12BindGroup : m_queuedBindGroups )
    {
        uint32_t index = 0;
        if ( dx12BindGroup->CbvSrvUavCount( ) > 0 )
        {
            BindResourceGroup( rootSignature->RegisterSpaceOffset( dx12BindGroup->RegisterSpace( ) ) + index++, dx12BindGroup->CbvSrvUavHandle( ).Gpu );
        }

        if ( dx12BindGroup->SamplerCount( ) > 0 )
        {
            BindResourceGroup( rootSignature->RegisterSpaceOffset( dx12BindGroup->RegisterSpace( ) ) + index, dx12BindGroup->SamplerHandle( ).Gpu );
        }

        for ( const auto &rootDescriptor : dx12BindGroup->RootDescriptors( ) )
        {
            BindRootDescriptors( rootDescriptor );
        }
    }

    for ( const auto &rootConstant : m_queuedRootConstants )
    {
        SetRootConstants( rootConstant );
    }
}

void DX12CommandList::BindResourceGroup( const uint32_t index, const D3D12_GPU_DESCRIPTOR_HANDLE &gpuHandle ) const
{
    switch ( m_currentPipeline->GetBindPoint( ) )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
    case DENOFIZ_BIND_POINT_MESH:
        {
            this->m_commandList->SetGraphicsRootDescriptorTable( index, gpuHandle );
        }
        break;
    case DENOFIZ_BIND_POINT_RAYTRACING:
    case DENOFIZ_BIND_POINT_COMPUTE:
        {
            this->m_commandList->SetComputeRootDescriptorTable( index, gpuHandle );
        }
        break;
    default:
        spdlog::error( "`BindResourceGroup` is an invalid function for queue type" );
        break;
    }
}

void DX12CommandList::BindRootDescriptors( const DX12RootDescriptor &rootDescriptor ) const
{
    const bool isGraphicsBindPoint = m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_GRAPHICS || m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_MESH;
    switch ( rootDescriptor.ParameterType )
    {
    case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
    case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
        break;
    case D3D12_ROOT_PARAMETER_TYPE_CBV:
        if ( isGraphicsBindPoint )
        {
            m_commandList->SetGraphicsRootConstantBufferView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        else
        {
            m_commandList->SetComputeRootConstantBufferView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        break;
    case D3D12_ROOT_PARAMETER_TYPE_SRV:
        if ( isGraphicsBindPoint )
        {
            m_commandList->SetGraphicsRootShaderResourceView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        else
        {
            m_commandList->SetComputeRootShaderResourceView( rootDescriptor.RootParameterIndex, rootDescriptor.GpuAddress );
        }
        break;
    case D3D12_ROOT_PARAMETER_TYPE_UAV:
        if ( isGraphicsBindPoint )
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
    switch ( m_currentPipeline->GetBindPoint( ) )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
        m_commandList->SetGraphicsRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    case DENOFIZ_BIND_POINT_COMPUTE:
        m_commandList->SetComputeRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    default:
        spdlog::error( "`SetRootConstants` is an invalid function for queue type" );
        break;
    }
}

void DX12CommandList::PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier )
{
    DX12BarrierHelper::ExecuteResourceBarrier( m_context, m_commandList.get( ), m_desc.QueueType, barrier );
}

void DX12CommandList::DrawIndexed( const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance )
{
    if ( indexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, DrawIndexed called with zero count: indexCount= {} , instanceCount={}", indexCount, instanceCount );
    }

    ProcessBindGroups( );
    m_commandList->DrawIndexedInstanced( indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

void DX12CommandList::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    if ( vertexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, Draw called with zero count: vertexCount= {} , instanceCount={}", vertexCount, instanceCount );
    }

    ProcessBindGroups( );
    m_commandList->DrawInstanced( vertexCount, instanceCount, firstVertex, firstInstance );
}

void DX12CommandList::Dispatch( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, Dispatch called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ );
    }

    ProcessBindGroups( );
    m_commandList->Dispatch( groupCountX, groupCountY, groupCountZ );
}

void DX12CommandList::DispatchMesh( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, DispatchMesh called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ );
    }

    ProcessBindGroups( );
    m_commandList->DispatchMesh( groupCountX, groupCountY, groupCountZ );
}

void DX12CommandList::DrawIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, const uint32_t stride )
{
    if ( !buffer )
    {
        spdlog::error( "DX12CommandList::DrawIndirect: buffer is null" );
        return;
    }

    const auto *dx12Buffer = static_cast<DX12Buffer *>( buffer );

    ProcessBindGroups( );

    ID3D12CommandSignature *signature = m_context->IndirectSignatureCache->GetDrawSignature( stride );
    m_commandList->ExecuteIndirect( signature, drawCount, dx12Buffer->Resource( ), offset, nullptr, 0 );
}

void DX12CommandList::DrawIndexedIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, const uint32_t stride )
{
    if ( !buffer )
    {
        spdlog::error( "DX12CommandList::DrawIndexedIndirect: buffer is null" );
        return;
    }

    const auto *dx12Buffer = static_cast<DX12Buffer *>( buffer );

    ProcessBindGroups( );

    ID3D12CommandSignature *signature = m_context->IndirectSignatureCache->GetDrawIndexedSignature( stride );
    m_commandList->ExecuteIndirect( signature, drawCount, dx12Buffer->Resource( ), offset, nullptr, 0 );
}

void DX12CommandList::DispatchIndirect( IBuffer *buffer, const uint64_t offset )
{
    if ( !buffer )
    {
        spdlog::error( "DX12CommandList::DispatchIndirect: buffer is null" );
        return;
    }

    const auto *dx12Buffer = static_cast<DX12Buffer *>( buffer );

    ProcessBindGroups( );

    ID3D12CommandSignature *signature = m_context->IndirectSignatureCache->GetDispatchSignature( );
    m_commandList->ExecuteIndirect( signature, 1, dx12Buffer->Resource( ), offset, nullptr, 0 );
}

void DX12CommandList::CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc )
{
    const DX12Buffer *dstBuffer = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.DstBuffer ) );
    const DX12Buffer *srcBuffer = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.SrcBuffer ) );
    DZ_NOT_NULL( dstBuffer );
    DZ_NOT_NULL( srcBuffer );

    if ( copyBufferRegionDesc.NumBytes == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, CopyBufferRegion called with zero NumBytes" );
    }

    m_commandList->CopyBufferRegion( dstBuffer->Resource( ), copyBufferRegionDesc.DstOffset, srcBuffer->Resource( ), copyBufferRegionDesc.SrcOffset,
                                     copyBufferRegionDesc.NumBytes );
}

void DX12CommandList::CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc )
{
    const DX12Texture *dstTexture = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.DstTexture ) );
    const DX12Texture *srcTexture = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.SrcTexture ) );
    DZ_NOT_NULL( dstTexture );
    DZ_NOT_NULL( srcTexture );

    if ( copyTextureRegionDesc.Width == 0 || copyTextureRegionDesc.Height == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, CopyTextureRegion called with zero dimensions: Width= {} , Height={}", copyTextureRegionDesc.Width,
                      copyTextureRegionDesc.Height );
    }

    D3D12_TEXTURE_COPY_LOCATION src = { };
    src.pResource                   = srcTexture->Resource( );
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex            = copyTextureRegionDesc.SrcMipLevel;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstTexture->Resource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyTextureRegionDesc.DstMipLevel;

    D3D12_BOX box = { };
    box.left      = copyTextureRegionDesc.SrcX;
    box.top       = copyTextureRegionDesc.SrcY;
    box.front     = copyTextureRegionDesc.SrcZ;
    box.right     = copyTextureRegionDesc.SrcX + copyTextureRegionDesc.Width;
    box.bottom    = copyTextureRegionDesc.SrcY + copyTextureRegionDesc.Height;
    box.back      = copyTextureRegionDesc.SrcZ + copyTextureRegionDesc.Depth;

    m_commandList->CopyTextureRegion( &dst, copyTextureRegionDesc.DstX, copyTextureRegionDesc.DstY, copyTextureRegionDesc.DstZ, &src, &box );
}

void DX12CommandList::CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture )
{
    const DX12Texture *dstTexture = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyBufferToTexture.DstTexture ) );
    const DX12Buffer  *srcBuffer  = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferToTexture.SrcBuffer ) );
    DZ_NOT_NULL( dstTexture );
    DZ_NOT_NULL( srcBuffer );

    const DenOfIz_TextureDesc   dstDesc = dstTexture->GetDesc( );
    D3D12_TEXTURE_COPY_LOCATION src     = { };
    src.pResource                       = srcBuffer->Resource( );
    src.Type                            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    const uint32_t subresource          = D3D12CalcSubresource( copyBufferToTexture.MipLevel, copyBufferToTexture.ArrayLayer, 0, dstDesc.MipLevels, dstDesc.ArraySize );
    m_context->D3DDevice->GetCopyableFootprints( &dstTexture->GetResourceDesc( ), subresource, 1, copyBufferToTexture.SrcOffset, &src.PlacedFootprint, nullptr, nullptr, nullptr );
    src.PlacedFootprint.Offset = copyBufferToTexture.SrcOffset;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstTexture->Resource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyBufferToTexture.MipLevel;
    m_commandList->CopyTextureRegion( &dst, copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ, &src, nullptr );
}

void DX12CommandList::CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer )
{
    const auto *dstBuffer  = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyTextureToBuffer.DstBuffer ) );
    const auto *srcTexture = dynamic_cast<DX12Texture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureToBuffer.SrcTexture ) );
    DZ_NOT_NULL( dstBuffer );
    DZ_NOT_NULL( srcTexture );

    D3D12_TEXTURE_COPY_LOCATION src = { };
    src.pResource                   = srcTexture->Resource( );
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex            = copyTextureToBuffer.MipLevel;

    D3D12_TEXTURE_COPY_LOCATION dst = { };
    dst.pResource                   = dstBuffer->Resource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    const uint32_t subresource      = copyTextureToBuffer.ArrayLayer * srcTexture->GetDesc( ).MipLevels + copyTextureToBuffer.MipLevel;
    m_context->D3DDevice->GetCopyableFootprints( &srcTexture->GetResourceDesc( ), subresource, 1, copyTextureToBuffer.DstOffset, &dst.PlacedFootprint, nullptr, nullptr, nullptr );

    m_commandList->CopyTextureRegion( &dst, copyTextureToBuffer.DstOffset, 0, 0, &src, nullptr );
}

void DX12CommandList::BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc )
{
    const auto dx12TopLevelAS    = dynamic_cast<DX12TopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, buildTopLevelASDesc.TopLevelAS ) );
    const auto dx12ScratchBuffer = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildTopLevelASDesc.ScratchBuffer ) );
    DZ_NOT_NULL( dx12TopLevelAS );
    DZ_NOT_NULL( dx12ScratchBuffer );

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = { };
    buildDesc.DestAccelerationStructureData                      = dx12TopLevelAS->GetDX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.Inputs.DescsLayout                                 = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                        = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    buildDesc.Inputs.Flags                                       = dx12TopLevelAS->Flags( );
    buildDesc.Inputs.NumDescs                                    = dx12TopLevelAS->NumInstances( );
    buildDesc.Inputs.InstanceDescs                               = dx12TopLevelAS->InstanceBuffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                   = dx12ScratchBuffer->Resource( )->GetGPUVirtualAddress( ) + buildTopLevelASDesc.ScratchBufferOffset;

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    const auto dx12BottomLevelAS = dynamic_cast<DX12BottomLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBottomLevelAS, buildBottomLevelASDesc.BottomLevelAS ) );
    const auto dx12ScratchBuffer = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildBottomLevelASDesc.ScratchBuffer ) );
    DZ_NOT_NULL( dx12BottomLevelAS );
    DZ_NOT_NULL( dx12ScratchBuffer );

    const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> &geometryDescs = dx12BottomLevelAS->GeometryDescs( );
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc     = { };
    buildDesc.Inputs.DescsLayout                                     = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                            = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    buildDesc.Inputs.Flags                                           = dx12BottomLevelAS->Flags( );
    buildDesc.Inputs.NumDescs                                        = geometryDescs.size( );
    buildDesc.Inputs.pGeometryDescs                                  = geometryDescs.data( );
    buildDesc.DestAccelerationStructureData                          = dx12BottomLevelAS->Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                       = dx12ScratchBuffer->Resource( )->GetGPUVirtualAddress( ) + buildBottomLevelASDesc.ScratchBufferOffset;

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc )
{
    const auto dx12TopLevelAS    = dynamic_cast<DX12TopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, updateDesc.TopLevelAS ) );
    const auto dx12ScratchBuffer = dynamic_cast<DX12Buffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, updateDesc.ScratchBuffer ) );
    DZ_NOT_NULL( dx12TopLevelAS );
    DZ_NOT_NULL( dx12ScratchBuffer );

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = { };
    buildDesc.Inputs.DescsLayout                                 = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                        = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    buildDesc.Inputs.Flags                                       = dx12TopLevelAS->Flags( ) | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    buildDesc.Inputs.NumDescs                                    = dx12TopLevelAS->NumInstances( );
    buildDesc.Inputs.InstanceDescs                               = dx12TopLevelAS->InstanceBuffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.SourceAccelerationStructureData                    = dx12TopLevelAS->GetDX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.DestAccelerationStructureData                      = dx12TopLevelAS->GetDX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                   = dx12ScratchBuffer->Resource( )->GetGPUVirtualAddress( ) + updateDesc.ScratchBufferOffset;

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc )
{
    if ( dispatchRaysDesc.Width == 0 || dispatchRaysDesc.Height == 0 || dispatchRaysDesc.Depth == 0 )
    {
        spdlog::warn( "DispatchRays called with zero dimensions: width= {} , height= {} , depth={}", dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
    }

    ProcessBindGroups( );
    const DX12ShaderBindingTable *sbt = dynamic_cast<DX12ShaderBindingTable *>( DENOFIZ_FROM_HANDLE( DenOfIz::IShaderBindingTable, dispatchRaysDesc.ShaderBindingTable ) );
    DZ_NOT_NULL( sbt );
    D3D12_DISPATCH_RAYS_DESC desc  = { };
    desc.RayGenerationShaderRecord = sbt->RayGenerationShaderRecord( );
    desc.MissShaderTable           = sbt->MissShaderRange( );
    desc.HitGroupTable             = sbt->HitGroupShaderRange( );

    desc.Width  = dispatchRaysDesc.Width;
    desc.Height = dispatchRaysDesc.Height;
    desc.Depth  = dispatchRaysDesc.Depth;
    m_commandList->DispatchRays( &desc );
}

void DX12CommandList::SetRootSignature( ID3D12RootSignature *rootSignature )
{
    DZ_RETURN_IF( rootSignature == nullptr );
    DZ_RETURN_IF( rootSignature == m_currentRootSignature );

    m_currentRootSignature = rootSignature;
    switch ( m_currentPipeline->GetBindPoint( ) )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
    case DENOFIZ_BIND_POINT_MESH:
        m_commandList->SetGraphicsRootSignature( rootSignature );
        break;
    case DENOFIZ_BIND_POINT_COMPUTE:
    case DENOFIZ_BIND_POINT_RAYTRACING:
        m_commandList->SetComputeRootSignature( rootSignature );
        break;
    default:
        spdlog::error( "SetRootSignature is an invalid function for queue type" );
        break;
    }
}

const DenOfIz_QueueType DX12CommandList::GetQueueType( )
{
    return m_desc.QueueType;
}

ID3D12GraphicsCommandList7 *DX12CommandList::GetCommandList( ) const
{
    return m_commandList.get( );
}

void DX12CommandList::BeginDebugMarker( const float r, const float g, const float b, const DenOfIz_StringView name )
{
    if ( m_debugCommandList )
    {
#ifdef USE_PIX
        const size_t nameLen  = name.NumChars + 1;
        const auto   wideName = new wchar_t[ nameLen ];
        mbstowcs_s( nullptr, wideName, nameLen, name.Chars, nameLen - 1 );

        const UINT64 color = 0xFF000000 | static_cast<UINT32>( r * 255.0f ) << 16 | static_cast<UINT32>( g * 255.0f ) << 8 | static_cast<UINT32>( b * 255.0f );

        PIXBeginEvent( m_commandList.get( ), color, wideName );
        delete[] wideName;
#endif
    }
}

void DX12CommandList::EndDebugMarker( )
{
    if ( m_debugCommandList )
    {
#ifdef USE_PIX
        PIXEndEvent( m_commandList.get( ) );
#endif
    }
}

void DX12CommandList::InsertDebugMarker( const float r, const float g, const float b, const DenOfIz_StringView name )
{
    if ( m_debugCommandList )
    {
#ifdef USE_PIX
        const size_t nameLen  = name.NumChars + 1;
        const auto   wideName = new wchar_t[ nameLen ];
        mbstowcs_s( nullptr, wideName, nameLen, name.Chars, nameLen - 1 );

        const UINT64 color = 0xFF000000 | static_cast<UINT32>( r * 255.0f ) << 16 | static_cast<UINT32>( g * 255.0f ) << 8 | static_cast<UINT32>( b * 255.0f );

        PIXSetMarker( m_commandList.get( ), color, wideName );
        delete[] wideName;
#endif
    }
}

void DX12CommandList::BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *dx12QueryPool = dynamic_cast<DX12QueryPool *>( queryPool );
    if ( !dx12QueryPool )
    {
        spdlog::error( "DX12CommandList::BeginQuery: Invalid query pool" );
        return;
    }
    if ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        const uint32_t actualIndex = queryDesc.Index * 2;
        m_commandList->EndQuery( dx12QueryPool->GetQueryHeap( ), dx12QueryPool->GetD3D12QueryType( ), actualIndex );
    }
    else if ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION || dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        m_commandList->BeginQuery( dx12QueryPool->GetQueryHeap( ), dx12QueryPool->GetD3D12QueryType( ), queryDesc.Index );
    }
}

void DX12CommandList::EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *dx12QueryPool = dynamic_cast<DX12QueryPool *>( queryPool );
    if ( !dx12QueryPool )
    {
        spdlog::error( "DX12CommandList::EndQuery: Invalid query pool" );
        return;
    }

    if ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        const uint32_t actualIndex = queryDesc.Index * 2 + 1;
        m_commandList->EndQuery( dx12QueryPool->GetQueryHeap( ), dx12QueryPool->GetD3D12QueryType( ), actualIndex );
    }
    else if ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION || dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        m_commandList->EndQuery( dx12QueryPool->GetQueryHeap( ), dx12QueryPool->GetD3D12QueryType( ), queryDesc.Index );
    }
}

void DX12CommandList::ResolveQuery( IQueryPool *queryPool, const uint32_t startQuery, const uint32_t queryCount )
{
    const auto *dx12QueryPool = dynamic_cast<DX12QueryPool *>( queryPool );
    if ( !dx12QueryPool )
    {
        spdlog::error( "DX12CommandList::ResolveQuery: Invalid query pool" );
        return;
    }

    const uint32_t internalQueryCount = ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP ? 2 : 1 );
    size_t         stride             = sizeof( uint64_t );
    if ( dx12QueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        stride = sizeof( D3D12_QUERY_DATA_PIPELINE_STATISTICS );
    }

    const uint32_t internalStartQuery = startQuery * internalQueryCount;
    const uint32_t internalCount      = queryCount * internalQueryCount;
    const uint64_t offset             = startQuery * internalQueryCount * stride;

    m_commandList->ResolveQueryData( dx12QueryPool->GetQueryHeap( ), dx12QueryPool->GetD3D12QueryType( ), internalStartQuery, internalCount, dx12QueryPool->GetReadbackBuffer( ),
                                     offset );
}

void DX12CommandList::ResetQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount )
{
}
