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
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandList.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <utility>

using namespace DenOfIz;

DX12CommandList::DX12CommandList( DX12Context *context, wil::com_ptr<ID3D12CommandAllocator> commandAllocator, const wil::com_ptr<ID3D12GraphicsCommandList> &commandList,
                                  const CommandListDesc desc ) : m_desc( desc ), m_context( context ), m_commandAllocator( std::move( commandAllocator ) )
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

    m_currentRootSignature = nullptr;
    m_currentPipeline      = nullptr;
    m_currentVertexBuffer  = nullptr;
}

void DX12CommandList::BeginRendering( const RenderingDesc &renderingDesc )
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets( renderingDesc.RTAttachments.NumElements );
    for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; i++ )
    {
        const auto &rtAttachment = renderingDesc.RTAttachments.Elements[ i ];
        if ( rtAttachment.Resource == nullptr )
        {
            spdlog::error( "BeginRendering called with null render target attachment at index {}", i );
            return;
        }
        auto *pImageResource = dynamic_cast<DX12TextureResource *>( rtAttachment.Resource );
        renderTargets[ i ]   = pImageResource->GetOrCreateRtvHandle( );
        if ( rtAttachment.LoadOp == LoadOp::Clear )
        {
            m_commandList->ClearRenderTargetView( renderTargets[ i ], rtAttachment.ClearColor, 0, nullptr );
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle       = { };
    bool                        hasDepthStencil = false;
    if ( renderingDesc.DepthAttachment.Resource != nullptr )
    {
        auto *pDepthResource = dynamic_cast<DX12TextureResource *>( renderingDesc.DepthAttachment.Resource );
        dsvHandle            = pDepthResource->GetOrCreateDsvHandle( );
        hasDepthStencil      = true;
        if ( renderingDesc.DepthAttachment.LoadOp == LoadOp::Clear )
        {
            D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
            if ( renderingDesc.StencilAttachment.Resource != nullptr || pDepthResource->GetFormat( ) == Format::D24UnormS8Uint )
            {
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
            }

            m_commandList->ClearDepthStencilView( dsvHandle, clearFlags, renderingDesc.DepthAttachment.ClearDepthStencil[ 0 ],
                                                  static_cast<UINT8>( renderingDesc.DepthAttachment.ClearDepthStencil[ 1 ] ), 0, nullptr );
        }
    }
    else if ( renderingDesc.StencilAttachment.Resource != nullptr )
    {
        auto *pStencilResource = dynamic_cast<DX12TextureResource *>( renderingDesc.StencilAttachment.Resource );
        dsvHandle              = pStencilResource->GetOrCreateDsvHandle( );
        hasDepthStencil        = true;

        if ( renderingDesc.StencilAttachment.LoadOp == LoadOp::Clear )
        {
            m_commandList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_STENCIL, 1.0f, static_cast<UINT8>( renderingDesc.StencilAttachment.ClearDepthStencil[ 1 ] ), 0,
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

    if ( m_currentVertexBuffer )
    {
        BindVertexBuffer( m_currentVertexBuffer );
        m_currentVertexBuffer = nullptr;
    }

    if ( m_currentPipeline->GetBindPoint( ) == BindPoint::RayTracing )
    {
        m_commandList->SetPipelineState1( m_currentPipeline->GetRayTracingSO( ) );
    }
    else
    {
        m_commandList->SetPipelineState( m_currentPipeline->GetPipeline( ) );
        m_commandList->IASetPrimitiveTopology( m_currentPipeline->GetTopology( ) );
    }
}

// TODO If BindVertex buffer is called before BindPipeline we might read the wrong stride
void DX12CommandList::BindVertexBuffer( IBufferResource *buffer, uint64_t offset )
{
    DZ_NOT_NULL( buffer );

    const auto pBuffer = dynamic_cast<DX12BufferResource *>( buffer );
    // We need the stride from the InputLayout, if we do not have it, wait until we have a pipeline then set the vertex buffers
    if ( !m_currentPipeline )
    {
        m_currentVertexBuffer = pBuffer;
        return;
    }

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
    vertexBufferView.BufferLocation           = pBuffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    vertexBufferView.StrideInBytes            = m_currentPipeline->GetIAStride( );
    vertexBufferView.SizeInBytes              = pBuffer->NumBytes( ) - offset;

    m_commandList->IASetVertexBuffers( 0, 1, &vertexBufferView );
}

void DX12CommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType, uint64_t offset )
{
    DZ_NOT_NULL( buffer );

    const DX12BufferResource *pBuffer = dynamic_cast<DX12BufferResource *>( buffer );

    D3D12_INDEX_BUFFER_VIEW indexBufferView = { };
    indexBufferView.BufferLocation          = pBuffer->Resource( )->GetGPUVirtualAddress( ) + offset;
    indexBufferView.SizeInBytes             = pBuffer->NumBytes( ) - offset;
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

void DX12CommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );

    const DX12ResourceBindGroup *dx12BindGroup = dynamic_cast<DX12ResourceBindGroup *>( bindGroup );
    m_queuedBindGroups.push_back( dx12BindGroup );
}

void DX12CommandList::ProcessBindGroups( )
{
    m_commandList->SetDescriptorHeaps( m_heaps.size( ), m_heaps.data( ) );

    for ( const auto &dx12BindGroup : m_queuedBindGroups )
    {
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
}

void DX12CommandList::BindResourceGroup( const uint32_t index, const D3D12_GPU_DESCRIPTOR_HANDLE &gpuHandle ) const
{
    switch ( m_currentPipeline->GetBindPoint( ) )
    {
    case BindPoint::Graphics:
    case BindPoint::Mesh:
        {
            this->m_commandList->SetGraphicsRootDescriptorTable( index, gpuHandle );
        }
        break;
    case BindPoint::RayTracing:
    case BindPoint::Compute:
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
    const bool isGraphicsBindPoint = m_currentPipeline->GetBindPoint( ) == BindPoint::Graphics || m_currentPipeline->GetBindPoint( ) == BindPoint::Mesh;
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
    case BindPoint::Graphics:
        m_commandList->SetGraphicsRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    case BindPoint::Compute:
        m_commandList->SetComputeRoot32BitConstants( rootConstant.Binding, rootConstant.NumBytes / 4, rootConstant.Data, 0 );
        break;
    default:
        spdlog::error( "`SetRootConstants` is an invalid function for queue type" );
        break;
    }
}

void DX12CommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
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

void DX12CommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionDesc )
{
    DZ_NOT_NULL( copyBufferRegionDesc.DstBuffer );
    DZ_NOT_NULL( copyBufferRegionDesc.SrcBuffer );

    const DX12BufferResource *dstBuffer = dynamic_cast<DX12BufferResource *>( copyBufferRegionDesc.DstBuffer );
    const DX12BufferResource *srcBuffer = dynamic_cast<DX12BufferResource *>( copyBufferRegionDesc.SrcBuffer );

    if ( copyBufferRegionDesc.NumBytes == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, CopyBufferRegion called with zero NumBytes" );
    }

    m_commandList->CopyBufferRegion( dstBuffer->Resource( ), copyBufferRegionDesc.DstOffset, srcBuffer->Resource( ), copyBufferRegionDesc.SrcOffset,
                                     copyBufferRegionDesc.NumBytes );
}

void DX12CommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionDesc )
{
    DZ_NOT_NULL( copyTextureRegionDesc.DstTexture );
    DZ_NOT_NULL( copyTextureRegionDesc.SrcTexture );

    const DX12TextureResource *dstTexture = dynamic_cast<DX12TextureResource *>( copyTextureRegionDesc.DstTexture );
    const DX12TextureResource *srcTexture = dynamic_cast<DX12TextureResource *>( copyTextureRegionDesc.SrcTexture );

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
    dst.pResource                   = dstTexture->Resource( );
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = copyBufferToTexture.MipLevel;
    m_commandList->CopyTextureRegion( &dst, copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ, &src, nullptr );
}

void DX12CommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer )
{
    DZ_NOT_NULL( copyTextureToBuffer.DstBuffer );
    DZ_NOT_NULL( copyTextureToBuffer.SrcTexture );

    const auto *dstBuffer  = dynamic_cast<DX12BufferResource *>( copyTextureToBuffer.DstBuffer );
    const auto *srcTexture = dynamic_cast<DX12TextureResource *>( copyTextureToBuffer.SrcTexture );

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

void DX12CommandList::UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc )
{
    const auto dx12TopLevelAS = dynamic_cast<DX12TopLevelAS *>( updateDesc.TopLevelAS );
    DZ_NOT_NULL( dx12TopLevelAS );

    UpdateTransformsDesc updateTransformsDesc = { };
    updateTransformsDesc.Transforms           = updateDesc.Transforms;

    dx12TopLevelAS->UpdateInstanceTransforms( updateTransformsDesc );

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = { };
    buildDesc.Inputs.DescsLayout                                 = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type                                        = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    buildDesc.Inputs.Flags                                       = dx12TopLevelAS->Flags( ) | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    buildDesc.Inputs.NumDescs                                    = dx12TopLevelAS->NumInstances( );
    buildDesc.Inputs.InstanceDescs                               = dx12TopLevelAS->InstanceBuffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.SourceAccelerationStructureData                    = dx12TopLevelAS->DX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.DestAccelerationStructureData                      = dx12TopLevelAS->DX12Buffer( )->Resource( )->GetGPUVirtualAddress( );
    buildDesc.ScratchAccelerationStructureData                   = dx12TopLevelAS->Scratch( )->Resource( )->GetGPUVirtualAddress( );

    m_commandList->BuildRaytracingAccelerationStructure( &buildDesc, 0, nullptr );
}

void DX12CommandList::DispatchRays( const DispatchRaysDesc &dispatchRaysDesc )
{
    DZ_NOT_NULL( dispatchRaysDesc.ShaderBindingTable );
    if ( dispatchRaysDesc.Width == 0 || dispatchRaysDesc.Height == 0 || dispatchRaysDesc.Depth == 0 )
    {
        spdlog::warn( "DispatchRays called with zero dimensions: width= {} , height= {} , depth={}", dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
    }

    ProcessBindGroups( );
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

void DX12CommandList::SetRootSignature( ID3D12RootSignature *rootSignature )
{
    DZ_RETURN_IF( rootSignature == nullptr );
    DZ_RETURN_IF( rootSignature == m_currentRootSignature );

    m_currentRootSignature = rootSignature;
    switch ( m_currentPipeline->GetBindPoint( ) )
    {
    case BindPoint::Graphics:
    case BindPoint::Mesh:
        m_commandList->SetGraphicsRootSignature( rootSignature );
        break;
    case BindPoint::Compute:
    case BindPoint::RayTracing:
        m_commandList->SetComputeRootSignature( rootSignature );
        break;
    default:
        spdlog::error( "SetRootSignature is an invalid function for queue type" );
        break;
    }
}

const QueueType DX12CommandList::GetQueueType( )
{
    return m_desc.QueueType;
}

ID3D12GraphicsCommandList7 *DX12CommandList::GetCommandList( ) const
{
    return m_commandList.get( );
}
