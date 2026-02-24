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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandList.h"
#include <algorithm>
#include <cstring>
#include <string>
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUQueryPool.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPURootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUTexture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

static bool g_warnedAboutFirstInstance = false;

WebGPUCommandList::WebGPUCommandList( WebGPUContext *context, ICommandQueue *commandQueue, const DenOfIz_CommandListDesc &desc ) :
    m_context( context ), m_commandQueue( commandQueue ), m_desc( desc )
{
}

WebGPUCommandList::~WebGPUCommandList( )
{
    EndCurrentPass( );

    if ( m_commandEncoder )
    {
        wgpuCommandEncoderRelease( m_commandEncoder );
    }

    for ( auto &[ binding, buffer ] : m_rootConstantBuffers )
    {
        if ( buffer )
        {
            wgpuBufferRelease( buffer );
        }
    }
    m_rootConstantBuffers.clear( );

    if ( m_rootConstantBindGroup )
    {
        wgpuBindGroupRelease( m_rootConstantBindGroup );
        m_rootConstantBindGroup = nullptr;
    }
}

WGPUCommandBuffer WebGPUCommandList::Finish( )
{
    if ( !m_commandEncoder )
    {
        return nullptr;
    }

    EndCurrentPass( );

    WGPUCommandBufferDescriptor cmdBufferDesc = { };
    cmdBufferDesc.label                       = DZ_WEBGPU_STRING( "Command Buffer" );

    const WGPUCommandBuffer &commandBuffer = wgpuCommandEncoderFinish( m_commandEncoder, &cmdBufferDesc );
    wgpuCommandEncoderRelease( m_commandEncoder );
    m_commandEncoder = nullptr;
    return commandBuffer;
}

void WebGPUCommandList::Begin( )
{
    if ( m_commandEncoder )
    {
        spdlog::critical( "WebGPUCommandList::Begin: Command encoder already exists" );
        return;
    }

    WGPUCommandEncoderDescriptor encoderDesc = { };
    encoderDesc.label                        = DZ_WEBGPU_STRING( "Command Encoder" );

    m_commandEncoder = wgpuDeviceCreateCommandEncoder( m_context->Device, &encoderDesc );

    m_currentPipeline = nullptr;
    m_deferredVertexBuffers.clear( );
    m_currentIndexBuffer = nullptr;
    m_pendingBindGroups.clear( );
    m_queuedRootConstants.clear( );
    m_rootConstantsDirty = false;
    m_isInsideRenderPass = false;
}

void WebGPUCommandList::BeginRendering( const DenOfIz_RenderingDesc &renderingDesc )
{
    if ( !m_commandEncoder )
    {
        spdlog::critical( "WebGPUCommandList::BeginRendering: No command encoder" );
        return;
    }

    if ( m_isInsideRenderPass )
    {
        spdlog::critical( "WebGPUCommandList::BeginRendering: Already inside a render pass" );
        return;
    }

    EndCurrentPass( );

    std::vector<WGPURenderPassColorAttachment> colorAttachments;
    colorAttachments.reserve( renderingDesc.RTAttachments.NumElements );

    for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; i++ )
    {
        const auto &rtAttachment = renderingDesc.RTAttachments.Elements[ i ];
        if ( !DENOFIZ_HANDLE_IS_VALID( rtAttachment.Resource ) )
        {
            spdlog::error( "BeginRendering called with null render target attachment at index {}", i );
            continue;
        }

        const auto *textureResource = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, rtAttachment.Resource ) );
        DZ_NOT_NULL( textureResource );

        WGPURenderPassColorAttachment colorAttachment = { };
        colorAttachment.view                          = textureResource->GetTextureView( );
        colorAttachment.loadOp                        = DenOfIz_WebGPUEnumConverter_ConvertLoadOp( rtAttachment.LoadOp );
        colorAttachment.storeOp                       = DenOfIz_WebGPUEnumConverter_ConvertStoreOp( rtAttachment.StoreOp );
        colorAttachment.clearValue                    = { rtAttachment.ClearColor.X, rtAttachment.ClearColor.Y, rtAttachment.ClearColor.Z, rtAttachment.ClearColor.W };
        colorAttachment.depthSlice                    = WGPU_DEPTH_SLICE_UNDEFINED;

        colorAttachments.push_back( colorAttachment );
    }

    WGPURenderPassDepthStencilAttachment depthStencilAttachment = { };
    bool                                 hasDepthStencil        = false;

    if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.DepthAttachment.Resource ) )
    {
        const auto *depthResource = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, renderingDesc.DepthAttachment.Resource ) );
        DZ_NOT_NULL( depthResource );
        const DenOfIz_Format depthFormat = depthResource->GetFormat( );

        if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) && DenOfIz_Format_IsDepthOnly( depthFormat ) )
        {
            spdlog::error( "WebGPUCommandList::BeginRendering: Separate stencil attachment provided with depth-only format ({}). "
                           "WebGPU requires formats to match their usage.",
                           static_cast<int>( depthFormat ) );
        }

        depthStencilAttachment.view            = depthResource->GetTextureView( );
        depthStencilAttachment.depthLoadOp     = DenOfIz_WebGPUEnumConverter_ConvertLoadOp( renderingDesc.DepthAttachment.LoadOp );
        depthStencilAttachment.depthStoreOp    = DenOfIz_WebGPUEnumConverter_ConvertStoreOp( renderingDesc.DepthAttachment.StoreOp );
        depthStencilAttachment.depthClearValue = renderingDesc.DepthAttachment.ClearDepthStencil.X;
        depthStencilAttachment.depthReadOnly   = false;

        if ( DenOfIz_Format_HasStencilComponent( depthFormat ) || DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) )
        {
            depthStencilAttachment.stencilLoadOp = DenOfIz_WebGPUEnumConverter_ConvertLoadOp(
                DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) ? renderingDesc.StencilAttachment.LoadOp : DENOFIZ_LOAD_OP_DONT_CARE );
            depthStencilAttachment.stencilStoreOp = DenOfIz_WebGPUEnumConverter_ConvertStoreOp(
                DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) ? renderingDesc.StencilAttachment.StoreOp : DENOFIZ_STORE_OP_DONT_CARE );
            depthStencilAttachment.stencilClearValue = static_cast<uint32_t>( renderingDesc.DepthAttachment.ClearDepthStencil.Y );
            depthStencilAttachment.stencilReadOnly   = false;
        }

        hasDepthStencil = true;
    }
    else if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) )
    {
        const auto *stencilResource = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, renderingDesc.StencilAttachment.Resource ) );
        DZ_NOT_NULL( stencilResource );
        const DenOfIz_Format stencilFormat = stencilResource->GetFormat( );

        if ( !DenOfIz_Format_HasStencilComponent( stencilFormat ) )
        {
            spdlog::error( "WebGPUCommandList::BeginRendering: Stencil attachment provided with format ({}) that has no stencil component.", static_cast<int>( stencilFormat ) );
        }

        depthStencilAttachment.view              = stencilResource->GetTextureView( );
        depthStencilAttachment.stencilLoadOp     = DenOfIz_WebGPUEnumConverter_ConvertLoadOp( renderingDesc.StencilAttachment.LoadOp );
        depthStencilAttachment.stencilStoreOp    = DenOfIz_WebGPUEnumConverter_ConvertStoreOp( renderingDesc.StencilAttachment.StoreOp );
        depthStencilAttachment.stencilClearValue = static_cast<uint32_t>( renderingDesc.StencilAttachment.ClearDepthStencil.Y );
        depthStencilAttachment.stencilReadOnly   = false;

        hasDepthStencil = true;
    }

    WGPURenderPassDescriptor renderPassDesc = { };
    renderPassDesc.colorAttachmentCount     = static_cast<uint32_t>( colorAttachments.size( ) );
    renderPassDesc.colorAttachments         = colorAttachments.empty( ) ? nullptr : colorAttachments.data( );
    renderPassDesc.depthStencilAttachment   = hasDepthStencil ? &depthStencilAttachment : nullptr;

    m_renderPassEncoder  = wgpuCommandEncoderBeginRenderPass( m_commandEncoder, &renderPassDesc );
    m_isInsideRenderPass = true;

    if ( renderingDesc.RenderAreaWidth > 0 && renderingDesc.RenderAreaHeight > 0 )
    {
        BindViewport( renderingDesc.RenderAreaOffsetX, renderingDesc.RenderAreaOffsetY, renderingDesc.RenderAreaWidth, renderingDesc.RenderAreaHeight );
        BindScissorRect( renderingDesc.RenderAreaOffsetX, renderingDesc.RenderAreaOffsetY, renderingDesc.RenderAreaWidth, renderingDesc.RenderAreaHeight );
    }
}

void WebGPUCommandList::EndRendering( )
{
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderEnd( m_renderPassEncoder );
        wgpuRenderPassEncoderRelease( m_renderPassEncoder );
        m_renderPassEncoder  = nullptr;
        m_isInsideRenderPass = false;
    }
}

void WebGPUCommandList::End( )
{
    EndCurrentPass( );
}

void WebGPUCommandList::BindPipeline( IPipeline *pipeline )
{
    DZ_NOT_NULL( pipeline );
    auto *newPipeline = dynamic_cast<WebGPUPipeline *>( pipeline );
    if ( m_currentPipeline == newPipeline )
    {
        return;
    }

    m_currentPipeline = newPipeline;
    if ( m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_GRAPHICS || m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_MESH )
    {
        if ( !m_renderPassEncoder )
        {
            spdlog::critical( "WebGPUCommandList::BindPipeline: No render pass encoder for graphics pipeline" );
            return;
        }

        wgpuRenderPassEncoderSetPipeline( m_renderPassEncoder, m_currentPipeline->GetRenderPipeline( ) );

        if ( !m_deferredVertexBuffers.empty( ) )
        {
            for ( const auto &deferred : m_deferredVertexBuffers )
            {
                wgpuRenderPassEncoderSetVertexBuffer( m_renderPassEncoder, deferred.Slot, deferred.Buffer, deferred.Offset, WGPU_WHOLE_SIZE );
            }
            m_deferredVertexBuffers.clear( );
        }

        if ( m_currentIndexBuffer )
        {
            wgpuRenderPassEncoderSetIndexBuffer( m_renderPassEncoder, m_currentIndexBuffer, m_currentIndexFormat, m_indexBufferOffset, WGPU_WHOLE_SIZE );
        }

        ApplyViewportAndScissor( );
    }
    else if ( m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_COMPUTE )
    {
        BeginComputePass( );
        wgpuComputePassEncoderSetPipeline( m_computePassEncoder, m_currentPipeline->GetComputePipeline( ) );
    }
    else if ( m_currentPipeline->GetBindPoint( ) == DENOFIZ_BIND_POINT_RAYTRACING )
    {
        spdlog::critical( "WebGPUCommandList::BindPipeline: Ray tracing pipelines not supported in WebGPU" );
    }
}

void WebGPUCommandList::BindVertexBuffer( IBuffer *buffer, const uint64_t offset, const uint32_t stride, const uint32_t slot )
{
    DZ_NOT_NULL( buffer );

    const auto *webgpuBuffer = dynamic_cast<WebGPUBuffer *>( buffer );
    webgpuBuffer->SyncShadowBuffer( );

    if ( !m_renderPassEncoder || !m_currentPipeline )
    {
        DeferredVertexBuffer deferred;
        deferred.Buffer = webgpuBuffer->GetBuffer( );
        deferred.Offset = offset;
        deferred.Stride = stride;
        deferred.Slot   = slot;
        m_deferredVertexBuffers.push_back( deferred );
        return;
    }

    wgpuRenderPassEncoderSetVertexBuffer( m_renderPassEncoder, slot, webgpuBuffer->GetBuffer( ), offset, WGPU_WHOLE_SIZE );
}

void WebGPUCommandList::BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, const uint64_t offset )
{
    DZ_NOT_NULL( buffer );

    const auto *webgpuBuffer = dynamic_cast<WebGPUBuffer *>( buffer );
    webgpuBuffer->SyncShadowBuffer( );
    m_currentIndexBuffer = webgpuBuffer->GetBuffer( );
    m_currentIndexFormat = DenOfIz_WebGPUEnumConverter_ConvertIndexType( indexType );
    m_indexBufferOffset  = offset;

    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderSetIndexBuffer( m_renderPassEncoder, m_currentIndexBuffer, m_currentIndexFormat, offset, WGPU_WHOLE_SIZE );
    }
}

void WebGPUCommandList::BindViewport( const float x, const float y, float width, float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid viewport dimensions: width={}, height={}", width, height );
        return;
    }

    m_viewportX      = x;
    m_viewportY      = y;
    m_viewportWidth  = width;
    m_viewportHeight = height;

    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderSetViewport( m_renderPassEncoder, x, y, width, height, 0.0f, 1.0f );
    }
}

void WebGPUCommandList::BindScissorRect( const float x, const float y, float width, float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid scissor rect dimensions: width={}, height={}", width, height );
        return;
    }

    m_scissorX      = static_cast<uint32_t>( std::round( x ) );
    m_scissorY      = static_cast<uint32_t>( std::round( y ) );
    m_scissorWidth  = static_cast<uint32_t>( std::round( width ) );
    m_scissorHeight = static_cast<uint32_t>( std::round( height ) );

    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderSetScissorRect( m_renderPassEncoder, m_scissorX, m_scissorY, m_scissorWidth, m_scissorHeight );
    }
}

void WebGPUCommandList::BindResourceGroup( IBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );

    auto    *webgpuBindGroup = dynamic_cast<WebGPUBindGroup *>( bindGroup );
    uint32_t bindGroupIndex  = webgpuBindGroup->GetRegisterSpace( );
    m_pendingBindGroups.push_back( { bindGroupIndex, webgpuBindGroup } );
}

void WebGPUCommandList::SetRootConstants( const uint32_t binding, const DenOfIz_ByteArray &data )
{
    QueuedRootConstant rootConstant;
    rootConstant.Binding = binding;
    rootConstant.Data.resize( data.NumElements );
    std::memcpy( rootConstant.Data.data( ), data.Elements, data.NumElements );

    auto it = std::find_if( m_queuedRootConstants.begin( ), m_queuedRootConstants.end( ), [ binding ]( const QueuedRootConstant &rc ) { return rc.Binding == binding; } );
    if ( it != m_queuedRootConstants.end( ) )
    {
        *it = std::move( rootConstant );
    }
    else
    {
        m_queuedRootConstants.push_back( std::move( rootConstant ) );
    }
    m_rootConstantsDirty = true;
}

void WebGPUCommandList::PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier )
{
    // WebGPU handles barriers automatically through usage tracking
}

void WebGPUCommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance )
{
    if ( indexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "DrawIndexed called with zero count: indexCount={}, instanceCount={}", indexCount, instanceCount );
    }

    if ( firstInstance != 0 && !g_warnedAboutFirstInstance )
    {
        g_warnedAboutFirstInstance = true;
        spdlog::warn( "WebGPU: Non-zero firstInstance ({}) behaves differently than other backends. "
                      "In WebGPU, SV_InstanceID/instance_index includes firstInstance offset, while in DX12/Vulkan it starts at 0. "
                      "For consistent cross-platform behavior, use firstInstance=0 and pass the offset via constant buffer.",
                      firstInstance );
    }

    if ( !m_renderPassEncoder )
    {
        spdlog::critical( "WebGPUCommandList::DrawIndexed: No render pass encoder" );
        return;
    }

    FlushPendingBindGroups( );

    if ( vertexOffset != 0 )
    {
        wgpuRenderPassEncoderDrawIndexed( m_renderPassEncoder, indexCount, instanceCount, firstIndex, static_cast<int32_t>( vertexOffset ), firstInstance );
    }
    else
    {
        wgpuRenderPassEncoderDrawIndexed( m_renderPassEncoder, indexCount, instanceCount, firstIndex, 0, firstInstance );
    }
}

void WebGPUCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    if ( vertexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "Draw called with zero count: vertexCount={}, instanceCount={}", vertexCount, instanceCount );
    }

    if ( firstInstance != 0 && !g_warnedAboutFirstInstance )
    {
        g_warnedAboutFirstInstance = true;
        spdlog::warn( "WebGPU: Non-zero firstInstance ({}) behaves differently than other backends. "
                      "In WebGPU, SV_InstanceID/instance_index includes firstInstance offset, while in DX12/Vulkan it starts at 0. "
                      "For consistent cross-platform behavior, use firstInstance=0 and pass the offset via constant buffer.",
                      firstInstance );
    }

    if ( !m_renderPassEncoder )
    {
        spdlog::critical( "WebGPUCommandList::Draw: No render pass encoder" );
        return;
    }

    FlushPendingBindGroups( );
    wgpuRenderPassEncoderDraw( m_renderPassEncoder, vertexCount, instanceCount, firstVertex, firstInstance );
}

void WebGPUCommandList::CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc )
{
    if ( copyBufferRegionDesc.NumBytes == 0 )
    {
        spdlog::warn( "CopyBufferRegion called with zero NumBytes" );
    }

    EndCurrentPass( );

    const auto *srcBuffer = dynamic_cast<WebGPUBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.SrcBuffer ) );
    const auto *dstBuffer = dynamic_cast<WebGPUBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.DstBuffer ) );
    DZ_NOT_NULL( srcBuffer );
    DZ_NOT_NULL( dstBuffer );

    srcBuffer->SyncShadowBuffer( );
    dstBuffer->SyncShadowBuffer( );

    wgpuCommandEncoderCopyBufferToBuffer( m_commandEncoder, srcBuffer->GetBuffer( ), copyBufferRegionDesc.SrcOffset, dstBuffer->GetBuffer( ), copyBufferRegionDesc.DstOffset,
                                          copyBufferRegionDesc.NumBytes );
}

void WebGPUCommandList::CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc )
{
    if ( copyTextureRegionDesc.Width == 0 || copyTextureRegionDesc.Height == 0 )
    {
        spdlog::warn( "CopyTextureRegion called with zero dimensions: Width={}, Height={}", copyTextureRegionDesc.Width, copyTextureRegionDesc.Height );
    }

    EndCurrentPass( );

    const auto *srcTexture = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.SrcTexture ) );
    const auto *dstTexture = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.DstTexture ) );
    DZ_NOT_NULL( srcTexture );
    DZ_NOT_NULL( dstTexture );

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyTextureInfo srcCopy = { };
#else
    WGPUImageCopyTexture srcCopy = { };
#endif
    srcCopy.texture  = srcTexture->GetTexture( );
    srcCopy.mipLevel = copyTextureRegionDesc.SrcMipLevel;
    srcCopy.origin   = { copyTextureRegionDesc.SrcX, copyTextureRegionDesc.SrcY, copyTextureRegionDesc.SrcZ };
    srcCopy.aspect   = WGPUTextureAspect_All;

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyTextureInfo dstCopy = { };
#else
    WGPUImageCopyTexture dstCopy = { };
#endif
    dstCopy.texture  = dstTexture->GetTexture( );
    dstCopy.mipLevel = copyTextureRegionDesc.DstMipLevel;
    dstCopy.origin   = { copyTextureRegionDesc.DstX, copyTextureRegionDesc.DstY, copyTextureRegionDesc.DstZ };
    dstCopy.aspect   = WGPUTextureAspect_All;

    WGPUExtent3D copySize       = { };
    copySize.width              = copyTextureRegionDesc.Width;
    copySize.height             = copyTextureRegionDesc.Height;
    copySize.depthOrArrayLayers = copyTextureRegionDesc.Depth > 0 ? copyTextureRegionDesc.Depth : 1;

    wgpuCommandEncoderCopyTextureToTexture( m_commandEncoder, &srcCopy, &dstCopy, &copySize );
}

void WebGPUCommandList::CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture )
{
    EndCurrentPass( );

    const auto *srcBuffer  = dynamic_cast<WebGPUBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferToTexture.SrcBuffer ) );
    const auto *dstTexture = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyBufferToTexture.DstTexture ) );
    DZ_NOT_NULL( srcBuffer );
    DZ_NOT_NULL( dstTexture );

    srcBuffer->SyncShadowBuffer( );

    const DenOfIz_TextureDesc &textureDesc = dstTexture->GetDesc( );

    const uint32_t width  = std::max( 1u, textureDesc.Width >> copyBufferToTexture.MipLevel );
    const uint32_t height = std::max( 1u, textureDesc.Height >> copyBufferToTexture.MipLevel );
    const uint32_t depth  = std::max( 1u, textureDesc.Depth >> copyBufferToTexture.MipLevel );

    const uint32_t formatSize = DenOfIz_Format_NumBytes( copyBufferToTexture.Format );
    const uint32_t blockSize  = DenOfIz_Format_BlockSize( copyBufferToTexture.Format );

    uint32_t rowPitch = copyBufferToTexture.RowPitch;
    if ( rowPitch == 0 )
    {
        rowPitch = std::max( 1u, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
    }

    const uint32_t rowAlignment    = m_context->SelectedDevice.Constants.BufferTextureRowAlignment;
    const uint32_t alignedRowPitch = ( rowPitch + rowAlignment - 1 ) & ~( rowAlignment - 1 );

    uint32_t numRows = copyBufferToTexture.NumRows;
    if ( numRows == 0 )
    {
        numRows = std::max( 1u, ( height + ( blockSize - 1 ) ) / blockSize );
    }

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyBufferInfo bufferCopy = { };
    bufferCopy.buffer                  = srcBuffer->GetBuffer( );
    bufferCopy.layout.offset           = copyBufferToTexture.SrcOffset;
    bufferCopy.layout.bytesPerRow      = alignedRowPitch;
    bufferCopy.layout.rowsPerImage     = numRows;
#else
    WGPUImageCopyBuffer bufferCopy = { };
    bufferCopy.buffer              = srcBuffer->GetBuffer( );
    bufferCopy.layout.offset       = copyBufferToTexture.SrcOffset;
    bufferCopy.layout.bytesPerRow  = alignedRowPitch;
    bufferCopy.layout.rowsPerImage = numRows;
#endif

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyTextureInfo textureCopy = { };
#else
    WGPUImageCopyTexture textureCopy = { };
#endif
    textureCopy.texture  = dstTexture->GetTexture( );
    textureCopy.mipLevel = copyBufferToTexture.MipLevel;
    textureCopy.origin   = { copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ };
    textureCopy.aspect   = WGPUTextureAspect_All;

    WGPUExtent3D copySize       = { };
    copySize.width              = width;
    copySize.height             = height;
    copySize.depthOrArrayLayers = textureDesc.Depth > 1 ? depth : 1;

    wgpuCommandEncoderCopyBufferToTexture( m_commandEncoder, &bufferCopy, &textureCopy, &copySize );
}

void WebGPUCommandList::CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer )
{
    EndCurrentPass( );

    const auto *srcTexture = dynamic_cast<WebGPUTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureToBuffer.SrcTexture ) );
    const auto *dstBuffer  = dynamic_cast<WebGPUBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyTextureToBuffer.DstBuffer ) );
    DZ_NOT_NULL( srcTexture );
    DZ_NOT_NULL( dstBuffer );

    const DenOfIz_TextureDesc &textureDesc = srcTexture->GetDesc( );

    const uint32_t width  = std::max( 1u, textureDesc.Width >> copyTextureToBuffer.MipLevel );
    const uint32_t height = std::max( 1u, textureDesc.Height >> copyTextureToBuffer.MipLevel );
    const uint32_t depth  = std::max( 1u, textureDesc.Depth >> copyTextureToBuffer.MipLevel );

    const uint32_t formatSize = DenOfIz_Format_NumBytes( copyTextureToBuffer.Format );
    const uint32_t blockSize  = DenOfIz_Format_BlockSize( copyTextureToBuffer.Format );

    uint32_t rowPitch = copyTextureToBuffer.RowPitch;
    if ( rowPitch == 0 )
    {
        rowPitch = std::max( 1u, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
    }

    const uint32_t rowAlignment    = m_context->SelectedDevice.Constants.BufferTextureRowAlignment;
    const uint32_t alignedRowPitch = ( rowPitch + rowAlignment - 1 ) & ~( rowAlignment - 1 );

    uint32_t numRows = copyTextureToBuffer.NumRows;
    if ( numRows == 0 )
    {
        numRows = std::max( 1u, ( height + ( blockSize - 1 ) ) / blockSize );
    }

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyTextureInfo textureCopy = { };
#else
    WGPUImageCopyTexture textureCopy = { };
#endif
    textureCopy.texture  = srcTexture->GetTexture( );
    textureCopy.mipLevel = copyTextureToBuffer.MipLevel;
    textureCopy.origin   = { copyTextureToBuffer.SrcX, copyTextureToBuffer.SrcY, copyTextureToBuffer.SrcZ };
    textureCopy.aspect   = WGPUTextureAspect_All;

#if DZ_WEBGPU_USE_DAWN_API
    WGPUTexelCopyBufferInfo bufferCopy = { };
    bufferCopy.buffer                  = dstBuffer->GetBuffer( );
    bufferCopy.layout.offset           = copyTextureToBuffer.DstOffset;
    bufferCopy.layout.bytesPerRow      = alignedRowPitch;
    bufferCopy.layout.rowsPerImage     = numRows;
#else
    WGPUImageCopyBuffer bufferCopy = { };
    bufferCopy.buffer              = dstBuffer->GetBuffer( );
    bufferCopy.layout.offset       = copyTextureToBuffer.DstOffset;
    bufferCopy.layout.bytesPerRow  = alignedRowPitch;
    bufferCopy.layout.rowsPerImage = numRows;
#endif

    WGPUExtent3D copySize       = { };
    copySize.width              = width;
    copySize.height             = height;
    copySize.depthOrArrayLayers = textureDesc.Depth > 1 ? depth : 1;

    wgpuCommandEncoderCopyTextureToBuffer( m_commandEncoder, &textureCopy, &bufferCopy, &copySize );
}

void WebGPUCommandList::UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc )
{
    spdlog::critical( "WebGPUCommandList::UpdateTopLevelAS: Ray tracing not supported in WebGPU" );
}

void WebGPUCommandList::BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc )
{
    spdlog::critical( "WebGPUCommandList::BuildTopLevelAS: Ray tracing not supported in WebGPU" );
}

void WebGPUCommandList::BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    spdlog::critical( "WebGPUCommandList::BuildBottomLevelAS: Ray tracing not supported in WebGPU" );
}

void WebGPUCommandList::DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc )
{
    spdlog::critical( "WebGPUCommandList::DispatchRays: Ray tracing not supported in WebGPU" );
}

void WebGPUCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        spdlog::warn( "Dispatch called with zero group count: x={}, y={}, z={}", groupCountX, groupCountY, groupCountZ );
    }

    if ( !m_computePassEncoder )
    {
        BeginComputePass( );
    }

    FlushPendingBindGroups( );
    wgpuComputePassEncoderDispatchWorkgroups( m_computePassEncoder, groupCountX, groupCountY, groupCountZ );
}

void WebGPUCommandList::DispatchMesh( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    spdlog::critical( "WebGPUCommandList::DispatchMesh: Mesh shaders not supported in WebGPU" );
}

void WebGPUCommandList::DrawIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, uint32_t stride )
{
    if ( !m_renderPassEncoder )
    {
        spdlog::error( "WebGPUCommandList::DrawIndirect called outside of render pass" );
        return;
    }

    if ( !buffer )
    {
        spdlog::error( "WebGPUCommandList::DrawIndirect: buffer is null" );
        return;
    }

    const auto *webgpuBuffer = static_cast<WebGPUBuffer *>( buffer );

    if ( drawCount > 1 )
    {
        spdlog::warn( "WebGPUCommandList::DrawIndirect: WebGPU does not support multi-draw indirect, only first draw will be executed" );
    }

    FlushPendingBindGroups( );
    wgpuRenderPassEncoderDrawIndirect( m_renderPassEncoder, webgpuBuffer->GetBuffer( ), offset );
}

void WebGPUCommandList::DrawIndexedIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, uint32_t stride )
{
    if ( !m_renderPassEncoder )
    {
        spdlog::error( "WebGPUCommandList::DrawIndexedIndirect called outside of render pass" );
        return;
    }

    if ( !buffer )
    {
        spdlog::error( "WebGPUCommandList::DrawIndexedIndirect: buffer is null" );
        return;
    }

    const auto *webgpuBuffer = static_cast<WebGPUBuffer *>( buffer );

    if ( drawCount > 1 )
    {
        spdlog::warn( "WebGPUCommandList::DrawIndexedIndirect: WebGPU does not support multi-draw indirect, only first draw will be executed" );
    }

    FlushPendingBindGroups( );
    wgpuRenderPassEncoderDrawIndexedIndirect( m_renderPassEncoder, webgpuBuffer->GetBuffer( ), offset );
}

void WebGPUCommandList::DispatchIndirect( IBuffer *buffer, const uint64_t offset )
{
    if ( !buffer )
    {
        spdlog::error( "WebGPUCommandList::DispatchIndirect: buffer is null" );
        return;
    }

    if ( m_renderPassEncoder )
    {
        EndCurrentPass( );
    }

    if ( !m_computePassEncoder )
    {
        BeginComputePass( );
    }

    const auto *webgpuBuffer = static_cast<WebGPUBuffer *>( buffer );

    FlushPendingBindGroups( );
    wgpuComputePassEncoderDispatchWorkgroupsIndirect( m_computePassEncoder, webgpuBuffer->GetBuffer( ), offset );
}

const DenOfIz_QueueType WebGPUCommandList::GetQueueType( )
{
    return m_desc.QueueType;
}

void WebGPUCommandList::BeginComputePass( )
{
    EndCurrentPass( );

    WGPUComputePassDescriptor computePassDesc = { };
    computePassDesc.label                     = DZ_WEBGPU_STRING( "Compute Pass" );

    m_computePassEncoder = wgpuCommandEncoderBeginComputePass( m_commandEncoder, &computePassDesc );
}

void WebGPUCommandList::EndCurrentPass( )
{
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderEnd( m_renderPassEncoder );
        wgpuRenderPassEncoderRelease( m_renderPassEncoder );
        m_renderPassEncoder  = nullptr;
        m_isInsideRenderPass = false;
    }

    if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderEnd( m_computePassEncoder );
        wgpuComputePassEncoderRelease( m_computePassEncoder );
        m_computePassEncoder = nullptr;
    }
}

void WebGPUCommandList::FlushPendingBindGroups( )
{
    std::vector<bool> boundIndices;
    if ( m_currentPipeline != nullptr )
    {
        WebGPURootSignature *rootSignature = m_currentPipeline->RootSignature( );
        if ( rootSignature != nullptr )
        {
            boundIndices.resize( rootSignature->GetNumBindGroupSlots( ), false );
        }
    }

    for ( const auto &[ index, bindGroup ] : m_pendingBindGroups )
    {
        bindGroup->SyncBuffers( );

        const WGPUBindGroup wgpuBindGroup = bindGroup->GetBindGroup( );

        if ( m_renderPassEncoder )
        {
            wgpuRenderPassEncoderSetBindGroup( m_renderPassEncoder, index, wgpuBindGroup, 0, nullptr );
        }
        else if ( m_computePassEncoder )
        {
            wgpuComputePassEncoderSetBindGroup( m_computePassEncoder, index, wgpuBindGroup, 0, nullptr );
        }

        if ( index < boundIndices.size( ) )
        {
            boundIndices[ index ] = true;
        }
    }

    m_pendingBindGroups.clear( );

    if ( m_currentPipeline != nullptr )
    {
        WebGPURootSignature *rootSignature = m_currentPipeline->RootSignature( );
        if ( rootSignature != nullptr )
        {
            WGPUBindGroup                           emptyBindGroup = rootSignature->GetEmptyBindGroup( );
            const std::vector<WGPUBindGroupLayout> &layouts        = rootSignature->GetWGPUBindGroupLayouts( );
            for ( uint32_t i = 0; i < boundIndices.size( ); ++i )
            {
                if ( !boundIndices[ i ] && i < layouts.size( ) && layouts[ i ] == nullptr )
                {
                    if ( m_renderPassEncoder )
                    {
                        wgpuRenderPassEncoderSetBindGroup( m_renderPassEncoder, i, emptyBindGroup, 0, nullptr );
                    }
                    else if ( m_computePassEncoder )
                    {
                        wgpuComputePassEncoderSetBindGroup( m_computePassEncoder, i, emptyBindGroup, 0, nullptr );
                    }
                }
            }
        }
    }

    if ( m_rootConstantsDirty && m_currentPipeline != nullptr && !m_queuedRootConstants.empty( ) )
    {
        WebGPURootSignature *rootSignature = m_currentPipeline->RootSignature( );
        if ( rootSignature != nullptr && rootSignature->NumRootConstants( ) > 0 )
        {
            const auto &layouts = rootSignature->BindGroupLayouts( );
            if ( DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE < layouts.size( ) && layouts[ DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE ] != nullptr )
            {
                std::vector<WGPUBindGroupEntry> entries;
                entries.reserve( m_queuedRootConstants.size( ) );

                for ( const auto &queued : m_queuedRootConstants )
                {
                    auto bufferIt = m_rootConstantBuffers.find( queued.Binding );
                    if ( bufferIt == m_rootConstantBuffers.end( ) || bufferIt->second == nullptr )
                    {
                        WGPUBufferDescriptor bufferDesc{ };
                        bufferDesc.label            = DZ_WEBGPU_STRING( "Root Constant Buffer" );
                        bufferDesc.usage            = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
                        bufferDesc.size             = queued.Data.size( );
                        bufferDesc.mappedAtCreation = false;

                        m_rootConstantBuffers[ queued.Binding ] = wgpuDeviceCreateBuffer( m_context->Device, &bufferDesc );
                    }

                    WGPUBuffer buffer = m_rootConstantBuffers[ queued.Binding ];
                    wgpuQueueWriteBuffer( m_context->Queue, buffer, 0, queued.Data.data( ), queued.Data.size( ) );

                    WGPUBindGroupEntry entry{ };
                    entry.binding = queued.Binding;
                    entry.buffer  = buffer;
                    entry.offset  = 0;
                    entry.size    = queued.Data.size( );
                    entries.push_back( entry );
                }

                if ( m_rootConstantBindGroup )
                {
                    wgpuBindGroupRelease( m_rootConstantBindGroup );
                }

                WGPUBindGroupDescriptor bindGroupDesc{ };
                bindGroupDesc.label      = DZ_WEBGPU_STRING( "Root Constant Bind Group" );
                bindGroupDesc.layout     = layouts[ DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE ]->GetBindGroupLayout( );
                bindGroupDesc.entryCount = static_cast<uint32_t>( entries.size( ) );
                bindGroupDesc.entries    = entries.data( );

                m_rootConstantBindGroup = wgpuDeviceCreateBindGroup( m_context->Device, &bindGroupDesc );

                if ( m_renderPassEncoder )
                {
                    wgpuRenderPassEncoderSetBindGroup( m_renderPassEncoder, DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE, m_rootConstantBindGroup, 0, nullptr );
                }
                else if ( m_computePassEncoder )
                {
                    wgpuComputePassEncoderSetBindGroup( m_computePassEncoder, DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE, m_rootConstantBindGroup, 0, nullptr );
                }
            }
        }
        m_rootConstantsDirty = false;
    }
}

void WebGPUCommandList::ApplyViewportAndScissor( ) const
{
    if ( m_viewportWidth > 0 && m_viewportHeight > 0 )
    {
        wgpuRenderPassEncoderSetViewport( m_renderPassEncoder, m_viewportX, m_viewportY, m_viewportWidth, m_viewportHeight, 0.0f, 1.0f );
    }

    if ( m_scissorWidth > 0 && m_scissorHeight > 0 )
    {
        wgpuRenderPassEncoderSetScissorRect( m_renderPassEncoder, m_scissorX, m_scissorY, m_scissorWidth, m_scissorHeight );
    }
}

void WebGPUCommandList::BeginDebugMarker( float r, float g, float b, const DenOfIz_StringView name )
{
#if DZ_WEBGPU_USE_DAWN_API
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderPushDebugGroup( m_renderPassEncoder, { name.Chars, name.NumChars } );
    }
    else if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderPushDebugGroup( m_computePassEncoder, { name.Chars, name.NumChars } );
    }
    else if ( m_commandEncoder )
    {
        wgpuCommandEncoderPushDebugGroup( m_commandEncoder, { name.Chars, name.NumChars } );
    }
#else
    std::string nameStr( name.Chars, name.NumChars );
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderPushDebugGroup( m_renderPassEncoder, nameStr.c_str( ) );
    }
    else if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderPushDebugGroup( m_computePassEncoder, nameStr.c_str( ) );
    }
    else if ( m_commandEncoder )
    {
        wgpuCommandEncoderPushDebugGroup( m_commandEncoder, nameStr.c_str( ) );
    }
#endif
}

void WebGPUCommandList::EndDebugMarker( )
{
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderPopDebugGroup( m_renderPassEncoder );
    }
    else if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderPopDebugGroup( m_computePassEncoder );
    }
    else if ( m_commandEncoder )
    {
        wgpuCommandEncoderPopDebugGroup( m_commandEncoder );
    }
}

void WebGPUCommandList::InsertDebugMarker( float r, float g, float b, const DenOfIz_StringView name )
{
#if DZ_WEBGPU_USE_DAWN_API
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderInsertDebugMarker( m_renderPassEncoder, { name.Chars, name.NumChars } );
    }
    else if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderInsertDebugMarker( m_computePassEncoder, { name.Chars, name.NumChars } );
    }
    else if ( m_commandEncoder )
    {
        wgpuCommandEncoderInsertDebugMarker( m_commandEncoder, { name.Chars, name.NumChars } );
    }
#else
    std::string nameStr( name.Chars, name.NumChars );
    if ( m_renderPassEncoder )
    {
        wgpuRenderPassEncoderInsertDebugMarker( m_renderPassEncoder, nameStr.c_str( ) );
    }
    else if ( m_computePassEncoder )
    {
        wgpuComputePassEncoderInsertDebugMarker( m_computePassEncoder, nameStr.c_str( ) );
    }
    else if ( m_commandEncoder )
    {
        wgpuCommandEncoderInsertDebugMarker( m_commandEncoder, nameStr.c_str( ) );
    }
#endif
}

void WebGPUCommandList::BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *webgpuQueryPool = dynamic_cast<WebGPUQueryPool *>( queryPool );
    if ( !webgpuQueryPool )
    {
        spdlog::error( "WebGPUCommandList::BeginQuery: Invalid query pool" );
        return;
    }
    if ( webgpuQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION )
    {
        if ( m_renderPassEncoder )
        {
            wgpuRenderPassEncoderBeginOcclusionQuery( m_renderPassEncoder, queryDesc.Index );
        }
    }
    else if ( webgpuQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        const uint32_t actualIndex = queryDesc.Index * 2; // Begin timestamp uses even index
        wgpuCommandEncoderWriteTimestamp( m_commandEncoder, webgpuQueryPool->GetQuerySet( ), actualIndex );
    }
    // DenOfIz_QueryType::Pipeline not supported
}

void WebGPUCommandList::EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *webgpuQueryPool = dynamic_cast<WebGPUQueryPool *>( queryPool );
    if ( !webgpuQueryPool )
    {
        spdlog::error( "WebGPUCommandList::EndQuery: Invalid query pool" );
        return;
    }
    if ( webgpuQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION )
    {
        if ( m_renderPassEncoder )
        {
            wgpuRenderPassEncoderEndOcclusionQuery( m_renderPassEncoder );
        }
    }
    else if ( webgpuQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        const uint32_t actualIndex = queryDesc.Index * 2 + 1;
        wgpuCommandEncoderWriteTimestamp( m_commandEncoder, webgpuQueryPool->GetQuerySet( ), actualIndex );
    }
}

void WebGPUCommandList::ResolveQuery( IQueryPool *queryPool, const uint32_t startQuery, const uint32_t queryCount )
{
    const auto *webgpuQueryPool = dynamic_cast<WebGPUQueryPool *>( queryPool );
    if ( !webgpuQueryPool )
    {
        spdlog::error( "WebGPUCommandList::ResolveQuery: Invalid query pool" );
        return;
    }
    uint32_t actualStartQuery = startQuery;
    uint32_t actualQueryCount = queryCount;

    if ( webgpuQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        actualStartQuery = startQuery * 2;
        actualQueryCount = queryCount * 2;
    }

    wgpuCommandEncoderResolveQuerySet( m_commandEncoder, webgpuQueryPool->GetQuerySet( ), actualStartQuery, actualQueryCount, webgpuQueryPool->GetReadbackBuffer( ), 0 );
}

void WebGPUCommandList::ResetQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount )
{
}
