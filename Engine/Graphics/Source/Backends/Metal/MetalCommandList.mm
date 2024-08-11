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

#import <DenOfIzGraphics/Backends/Metal/MetalCommandList.h>

using namespace DenOfIz;

MetalCommandList::MetalCommandList( MetalContext *context, CommandListDesc desc ) : m_context( context ), m_desc( desc )
{
    m_commandBuffer = [m_context->CommandQueue commandBuffer];
}

MetalCommandList::~MetalCommandList( ) = default;

void MetalCommandList::Begin( )
{
    [m_commandBuffer enqueue];

    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
        m_blitEncoder = [m_commandBuffer blitCommandEncoder];
        break;
    case QueueType::Compute:
        m_computeEncoder = [m_commandBuffer computeCommandEncoder];
        break;
    case QueueType::Graphics:
        // Initialized in BeginRendering
        break;
    }
}

void MetalCommandList::BeginRendering( const RenderingDesc &renderingInfo )
{
    // Begin rendering with the provided rendering information
    auto passDesc = MTLRenderPassDescriptor.renderPassDescriptor;
    for ( auto i = 0; i < renderingInfo.RTAttachments.size( ); i++ )
    {
        const RenderingAttachmentDesc &attachment  = renderingInfo.RTAttachments[ i ];
        passDesc.colorAttachments[ i ].texture     = static_cast<MetalTextureResource *>( attachment.Resource )->Instance( );
        passDesc.colorAttachments[ i ].loadAction  = MTLLoadActionClear;
        passDesc.colorAttachments[ i ].storeAction = MTLStoreActionStore;
        passDesc.colorAttachments[ i ].clearColor =
            MTLClearColorMake( attachment.ClearColor[ 0 ], attachment.ClearColor[ 1 ], attachment.ClearColor[ 2 ], attachment.ClearColor[ 3 ] );
    }

    if ( renderingInfo.DepthAttachment.Resource )
    {
        const RenderingAttachmentDesc &attachment = renderingInfo.DepthAttachment;
        passDesc.depthAttachment.texture          = static_cast<MetalTextureResource *>( renderingInfo.DepthAttachment.Resource )->Instance( );
        passDesc.depthAttachment.loadAction       = MTLLoadActionClear;
        passDesc.depthAttachment.storeAction      = MTLStoreActionStore;
        passDesc.depthAttachment.clearDepth       = attachment.ClearDepth[ 0 ]; // Validate
    }

    if ( renderingInfo.StencilAttachment.Resource )
    {
        const RenderingAttachmentDesc &attachment = renderingInfo.StencilAttachment;
        passDesc.stencilAttachment.texture        = static_cast<MetalTextureResource *>( renderingInfo.StencilAttachment.Resource )->Instance( );
        passDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
        passDesc.stencilAttachment.storeAction    = MTLStoreActionStore;
        passDesc.stencilAttachment.clearStencil   = attachment.ClearDepth[ 0 ]; // Validate
    }

    m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
}

void MetalCommandList::EndRendering( )
{
    [m_renderEncoder endEncoding];
}

void MetalCommandList::Execute( const ExecuteDesc &executeInfo )
{

    [m_commandBuffer commit];
}

void MetalCommandList::Present( ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks )
{
    MetalSwapChain       *mtkSwapChain       = static_cast<MetalSwapChain *>( swapChain );
    MetalTextureResource *mtkTextureResource = static_cast<MetalTextureResource *>( mtkSwapChain->GetRenderTarget( imageIndex ) );
    [m_commandBuffer presentDrawable:mtkSwapChain->Drawable( )];
}

void MetalCommandList::BindPipeline( IPipeline *pipeline )
{
    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
        break;
    case QueueType::Compute:
        [m_computeEncoder setComputePipelineState:static_cast<MetalPipeline *>( pipeline )->ComputePipelineState( )];
        break;
    case QueueType::Graphics:
        [m_renderEncoder setRenderPipelineState:static_cast<MetalPipeline *>( pipeline )->GraphicsPipelineState( )];
        break;
    }
}

void MetalCommandList::BindVertexBuffer( IBufferResource *buffer )
{
    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
    case QueueType::Compute:
        LOG( WARNING ) << "BindVertexBuffer is not supported for Copy and Compute queues";
        break;
    case QueueType::Graphics:
        [m_renderEncoder setVertexBuffer:static_cast<MetalBufferResource *>( buffer )->Instance( ) offset:0 atIndex:0];
        break;
    }
}

void MetalCommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType )
{
    switch ( indexType )
    {
    case IndexType::Uint16:
        m_indexType = MTLIndexTypeUInt16;
        break;
    case IndexType::Uint32:
        m_indexType = MTLIndexTypeUInt32;
        break;
    }

    m_indexBuffer = static_cast<MetalBufferResource *>( buffer )->Instance( );
}

void MetalCommandList::BindViewport( float x, float y, float width, float height )
{
    MTLViewport viewport = { x, y, width, height, 0.0, 1.0 };
    [m_renderEncoder setViewport:viewport];
}

void MetalCommandList::BindScissorRect( float x, float y, float width, float height )
{
    MTLScissorRect scissorRect = { static_cast<NSUInteger>( x ), static_cast<NSUInteger>( y ), static_cast<NSUInteger>( width ), static_cast<NSUInteger>( height ) };
    [m_renderEncoder setScissorRect:scissorRect];
}

void MetalCommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    MetalResourceBindGroup *mtkBindGroup = static_cast<MetalResourceBindGroup *>( bindGroup );

    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
        break;
    case QueueType::Compute:

        [m_computeEncoder setBuffer:static_cast<MetalResourceBindGroup *>( bindGroup )->Buffers( )[ 0 ].Resource->Instance( ) offset:0 atIndex:0];
        break;
    case QueueType::Graphics:
        [m_renderEncoder setFragmentBuffer:static_cast<MetalResourceBindGroup *>( bindGroup )->Buffers( )[ 0 ].Resource->Instance( ) offset:0 atIndex:0];
        break;
    }
}

void MetalCommandList::SetDepthBias( float constantFactor, float clamp, float slopeFactor )
{
}

void MetalCommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
}

void MetalCommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance )
{
    [m_renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:indexCount
                                 indexType:m_indexType
                               indexBuffer:m_indexBuffer
                         indexBufferOffset:0
                             instanceCount:instanceCount
                                baseVertex:vertexOffset
                              baseInstance:firstInstance];
}

void MetalCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
{
    [m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:firstVertex vertexCount:vertexCount instanceCount:instanceCount];
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    MTLSize threadGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
    [m_computeEncoder dispatchThreadgroups:threadGroupsPerGrid threadsPerThreadgroup:MTLSizeMake( 1, 1, 1 )];
    [m_computeEncoder endEncoding];
}

void MetalCommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo )
{
    MetalBufferResource *srcBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionInfo.SrcBuffer );
    MetalBufferResource *dstBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionInfo.DstBuffer );
    m_blitEncoder                  = [m_commandBuffer blitCommandEncoder];
    [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                     sourceOffset:copyBufferRegionInfo.SrcOffset
                         toBuffer:dstBuffer->Instance( )
                destinationOffset:copyBufferRegionInfo.DstOffset
                             size:copyBufferRegionInfo.NumBytes];
    [m_blitEncoder endEncoding];
}

void MetalCommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo )
{
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionInfo.SrcTexture );
    MetalTextureResource *dstTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionInfo.DstTexture );
    m_blitEncoder                    = [m_commandBuffer blitCommandEncoder];
    [m_blitEncoder copyFromTexture:srcTexture->Instance( )
                       sourceSlice:copyTextureRegionInfo.SrcArrayLayer
                       sourceLevel:copyTextureRegionInfo.SrcMipLevel
                      sourceOrigin:MTLOriginMake( copyTextureRegionInfo.SrcX, copyTextureRegionInfo.SrcY, copyTextureRegionInfo.SrcZ )
                        sourceSize:MTLSizeMake( copyTextureRegionInfo.Width, copyTextureRegionInfo.Height, copyTextureRegionInfo.Depth )
                         toTexture:dstTexture->Instance( )
                  destinationSlice:copyTextureRegionInfo.DstArrayLayer
                  destinationLevel:copyTextureRegionInfo.DstMipLevel
                 destinationOrigin:MTLOriginMake( copyTextureRegionInfo.DstX, copyTextureRegionInfo.DstY, copyTextureRegionInfo.DstZ )];
    [m_blitEncoder endEncoding];
}

void MetalCommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture )
{
    // TODO Calculate RowPitch and NumRows automatically if possible
    MetalBufferResource  *srcBuffer  = dynamic_cast<MetalBufferResource *>( copyBufferToTexture.SrcBuffer );
    MetalTextureResource *dstTexture = dynamic_cast<MetalTextureResource *>( copyBufferToTexture.DstTexture );
    m_blitEncoder                    = [m_commandBuffer blitCommandEncoder];
    [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                     sourceOffset:copyBufferToTexture.SrcOffset
                sourceBytesPerRow:copyBufferToTexture.RowPitch
              sourceBytesPerImage:copyBufferToTexture.RowPitch * copyBufferToTexture.NumRows
                       sourceSize:MTLSizeMake( dstTexture->GetWidth(), dstTexture->GetHeight(), dstTexture->GetDepth( ) )
                        toTexture:dstTexture->Instance( )
                 destinationSlice:copyBufferToTexture.ArrayLayer
                 destinationLevel:copyBufferToTexture.MipLevel
                destinationOrigin:MTLOriginMake( copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ )];
    [m_blitEncoder endEncoding];
}

void MetalCommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer )
{
    // TODO Calculate RowPitch and NumRows automatically if possible
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureToBuffer.SrcTexture );
    MetalBufferResource  *dstBuffer  = dynamic_cast<MetalBufferResource *>( copyTextureToBuffer.DstBuffer );
    m_blitEncoder                    = [m_commandBuffer blitCommandEncoder];
    [m_blitEncoder copyFromTexture:srcTexture->Instance( )
                       sourceSlice:copyTextureToBuffer.ArrayLayer
                       sourceLevel:copyTextureToBuffer.MipLevel
                      sourceOrigin:MTLOriginMake( copyTextureToBuffer.SrcX, copyTextureToBuffer.SrcY, copyTextureToBuffer.SrcZ )
                        sourceSize:MTLSizeMake( srcTexture->GetWidth(), srcTexture->GetHeight(), srcTexture->GetDepth( )  )
                          toBuffer:dstBuffer->Instance( )
                 destinationOffset:copyTextureToBuffer.DstOffset
            destinationBytesPerRow:copyTextureToBuffer.RowPitch
          destinationBytesPerImage:copyTextureToBuffer.RowPitch * copyTextureToBuffer.NumRows];
    [m_blitEncoder endEncoding];
}
