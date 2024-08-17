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
    m_commandBuffer = [m_context->CommandQueue commandBuffer];
    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
        m_activeEncoderType = MetalEncoderType::Blit;
        m_blitEncoder       = [m_commandBuffer blitCommandEncoder];
        break;
    case QueueType::Compute:
        m_activeEncoderType = MetalEncoderType::Compute;
        m_computeEncoder    = [m_commandBuffer computeCommandEncoder];
        break;
    case QueueType::Graphics:
        // Initialized in BeginRendering
        break;
    }
}

void MetalCommandList::BeginRendering( const RenderingDesc &renderingInfo )
{
    if ( m_blitEncoder || m_computeEncoder )
    {
        LOG( ERROR ) << "Expected null blit or compute encoder, make sure the CommandList order is correct.";
    }

    m_activeEncoderType = MetalEncoderType::Render;
    // Begin rendering with the provided rendering information
    auto passDesc = MTLRenderPassDescriptor.renderPassDescriptor;
    for ( auto i = 0; i < renderingInfo.RTAttachments.size( ); i++ )
    {
        const RenderingAttachmentDesc &attachment      = renderingInfo.RTAttachments[ i ];
        MetalTextureResource          *metalRtResource = static_cast<MetalTextureResource *>( attachment.Resource );
        passDesc.colorAttachments[ i ].texture         = metalRtResource->Instance( );
        passDesc.colorAttachments[ i ].loadAction      = MTLLoadActionClear;
        passDesc.colorAttachments[ i ].storeAction     = MTLStoreActionStore;
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
}

void MetalCommandList::Execute( const ExecuteDesc &executeInfo )
{
    MetalFence *fence      = static_cast<MetalFence *>( executeInfo.Notify );
    bool        hasEncoder = false;
    if ( m_blitEncoder )
    {
        [m_blitEncoder endEncoding];
        m_blitEncoder = nil;
    }
    else if ( m_computeEncoder )
    {
        [m_computeEncoder endEncoding];
        m_computeEncoder = nil;
    }
    else if ( m_renderEncoder )
    {
        [m_renderEncoder endEncoding];
        m_renderEncoder = nil;
    }

    if ( executeInfo.Notify )
    {
        MetalFence *metalFence = static_cast<MetalFence *>( executeInfo.Notify );
        metalFence->NotifyOnCommandBufferCompletion( m_commandBuffer );
    }

    for ( ISemaphore *notifySemaphore : executeInfo.NotifySemaphores )
    {
        MetalSemaphore *metalSemaphore = static_cast<MetalSemaphore *>( notifySemaphore );
        metalSemaphore->NotifyOnCommandBufferCompletion( m_commandBuffer );
    }

    [m_commandBuffer commit];
}

void MetalCommandList::Present( ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks )
{
    MetalSwapChain *metalSwapChain = static_cast<MetalSwapChain *>( swapChain );
    metalSwapChain->Present( waitOnLocks );
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
    EnsureEncoder( MetalEncoderType::Render, "BindViewport called without a render encoder. Make sure to call BeginRendering" );
    MTLViewport viewport = { x, y, width, height, 0.0, 1.0 };
    [m_renderEncoder setViewport:viewport];
}

void MetalCommandList::BindScissorRect( float x, float y, float width, float height )
{
    EnsureEncoder( MetalEncoderType::Render, "BindScissorRect called without a render encoder. Make sure to call BeginRendering" );
    MTLScissorRect scissorRect = { static_cast<NSUInteger>( x ), static_cast<NSUInteger>( y ), static_cast<NSUInteger>( width ), static_cast<NSUInteger>( height ) };
    [m_renderEncoder setScissorRect:scissorRect];
}

void MetalCommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    MetalResourceBindGroup *metalBindGroup = static_cast<MetalResourceBindGroup *>( bindGroup );

    if ( m_desc.QueueType == QueueType::Copy )
    {
        LOG( WARNING ) << "BindResourceGroup is not supported for Copy queue";
        return;
    }

    if ( m_desc.QueueType == QueueType::Compute )
    {
        return;
    }
    else
    {
        return;
    }

    /*
     *
     * TODO METAL:
     * - Make sure descriptor tables are set correctly in resource bind group
     * - Make sure to useResource correctly below.
     * - Make sure that heaps are used correctly for read only resources
     */

    for ( const auto &buffer : metalBindGroup->Buffers( ) )
    {
        if ( m_desc.QueueType == QueueType::Compute )
        {
            [m_computeEncoder setBuffer:buffer.Resource->Instance( ) offset:0 atIndex:buffer.Location];
        }
        else
        {
            if ( ( buffer.ShaderStages & MTLRenderStageVertex ) == MTLRenderStageVertex )
            {
                [m_renderEncoder setVertexBuffer:buffer.Resource->Instance( ) offset:0 atIndex:buffer.Location];
            }
            if ( ( buffer.ShaderStages & MTLRenderStageFragment ) == MTLRenderStageFragment )
            {
                [m_renderEncoder setFragmentBuffer:buffer.Resource->Instance( ) offset:0 atIndex:buffer.Location];
            }
        }
    }

    for ( const auto &texture : metalBindGroup->Textures( ) )
    {
        if ( m_desc.QueueType == QueueType::Compute )
        {
            [m_computeEncoder setTexture:texture.Resource->Instance( ) atIndex:texture.Location];
        }
        else
        {
            [m_renderEncoder useResource:texture.Resource->Instance( ) usage:MTLResourceUsageSample];
            if ( ( texture.ShaderStages & MTLRenderStageVertex ) == MTLRenderStageVertex )
            {
                [m_renderEncoder setVertexTexture:texture.Resource->Instance( ) atIndex:texture.Location];
            }
            if ( ( texture.ShaderStages & MTLRenderStageFragment ) == MTLRenderStageFragment )
            {
                [m_renderEncoder setFragmentTexture:texture.Resource->Instance( ) atIndex:texture.Location];
            }
        }
    }

    for ( const auto &sampler : metalBindGroup->Samplers( ) )
    {
        if ( m_desc.QueueType == QueueType::Compute )
        {
            [m_computeEncoder setSamplerState:sampler.Resource->Instance( ) atIndex:sampler.Location];
        }
        else
        {
            if ( ( sampler.ShaderStages & MTLRenderStageVertex ) == MTLRenderStageVertex )
            {
                [m_renderEncoder setVertexSamplerState:sampler.Resource->Instance( ) atIndex:sampler.Location];
            }
            if ( ( sampler.ShaderStages & MTLRenderStageFragment ) == MTLRenderStageFragment )
            {
                [m_renderEncoder setFragmentSamplerState:sampler.Resource->Instance( ) atIndex:sampler.Location];
            }
        }
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
    EnsureEncoder( MetalEncoderType::Render, "DrawIndexed called without a render encoder. Make sure to call BeginRendering" );
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
    EnsureEncoder( MetalEncoderType::Render, "Draw called without a render encoder. Make sure to call BeginRendering" );
    [m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:firstVertex vertexCount:vertexCount instanceCount:instanceCount];
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    EnsureEncoder( MetalEncoderType::Compute, "Dispatch called without a compute encoder. Make sure to call Begin with QueueType::Compute" );

    MTLSize threadGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
    [m_computeEncoder dispatchThreadgroups:threadGroupsPerGrid threadsPerThreadgroup:MTLSizeMake( 1, 1, 1 )];
    [m_computeEncoder endEncoding];
}

void MetalCommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo )
{
    SwitchEncoder( MetalEncoderType::Blit );
    MetalBufferResource *srcBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionInfo.SrcBuffer );
    MetalBufferResource *dstBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionInfo.DstBuffer );
    [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                     sourceOffset:copyBufferRegionInfo.SrcOffset
                         toBuffer:dstBuffer->Instance( )
                destinationOffset:copyBufferRegionInfo.DstOffset
                             size:copyBufferRegionInfo.NumBytes];
}

void MetalCommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo )
{
    SwitchEncoder( MetalEncoderType::Blit );
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionInfo.SrcTexture );
    MetalTextureResource *dstTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionInfo.DstTexture );
    [m_blitEncoder copyFromTexture:srcTexture->Instance( )
                       sourceSlice:copyTextureRegionInfo.SrcArrayLayer
                       sourceLevel:copyTextureRegionInfo.SrcMipLevel
                      sourceOrigin:MTLOriginMake( copyTextureRegionInfo.SrcX, copyTextureRegionInfo.SrcY, copyTextureRegionInfo.SrcZ )
                        sourceSize:MTLSizeMake( copyTextureRegionInfo.Width, copyTextureRegionInfo.Height, copyTextureRegionInfo.Depth )
                         toTexture:dstTexture->Instance( )
                  destinationSlice:copyTextureRegionInfo.DstArrayLayer
                  destinationLevel:copyTextureRegionInfo.DstMipLevel
                 destinationOrigin:MTLOriginMake( copyTextureRegionInfo.DstX, copyTextureRegionInfo.DstY, copyTextureRegionInfo.DstZ )];
}

void MetalCommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture )
{
    SwitchEncoder( MetalEncoderType::Blit );

    MetalBufferResource  *srcBuffer  = dynamic_cast<MetalBufferResource *>( copyBufferToTexture.SrcBuffer );
    MetalTextureResource *dstTexture = dynamic_cast<MetalTextureResource *>( copyBufferToTexture.DstTexture );

    const uint32_t width  = std::max( 1u, dstTexture->GetWidth( ) >> copyBufferToTexture.MipLevel );
    const uint32_t height = std::max( 1u, dstTexture->GetHeight( ) >> copyBufferToTexture.MipLevel );
    const uint32_t depth  = std::max( 1u, dstTexture->GetDepth( ) >> copyBufferToTexture.MipLevel );

    const uint32_t formatSize      = FormatNumBytes( copyBufferToTexture.Format );
    const uint32_t blockSize       = FormatBlockSize( copyBufferToTexture.Format );
    const uint32_t rowPitch        = std::max( 1U, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
    const uint32_t numRows         = std::max( 1U, ( height + ( blockSize - 1 ) ) / blockSize );
    const uint32_t alignedRowPitch = Utilities::Align( rowPitch, m_context->SelectedDeviceInfo.Constants.BufferTextureRowAlignment );

    // TODO Calculate RowPitch and NumRows automatically if possible
    [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                     sourceOffset:copyBufferToTexture.SrcOffset
                sourceBytesPerRow:alignedRowPitch
              sourceBytesPerImage:alignedRowPitch * numRows
                       sourceSize:MTLSizeMake( width, height, depth )
                        toTexture:dstTexture->Instance( )
                 destinationSlice:copyBufferToTexture.ArrayLayer
                 destinationLevel:copyBufferToTexture.MipLevel
                destinationOrigin:MTLOriginMake( copyBufferToTexture.DstX, copyBufferToTexture.DstY, copyBufferToTexture.DstZ )];
}

void MetalCommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer )
{
    SwitchEncoder( MetalEncoderType::Blit );
    // TODO Calculate RowPitch and NumRows automatically if possible
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureToBuffer.SrcTexture );
    MetalBufferResource  *dstBuffer  = dynamic_cast<MetalBufferResource *>( copyTextureToBuffer.DstBuffer );
    [m_blitEncoder copyFromTexture:srcTexture->Instance( )
                       sourceSlice:copyTextureToBuffer.ArrayLayer
                       sourceLevel:copyTextureToBuffer.MipLevel
                      sourceOrigin:MTLOriginMake( copyTextureToBuffer.SrcX, copyTextureToBuffer.SrcY, copyTextureToBuffer.SrcZ )
                        sourceSize:MTLSizeMake( srcTexture->GetWidth( ), srcTexture->GetHeight( ), srcTexture->GetDepth( ) )
                          toBuffer:dstBuffer->Instance( )
                 destinationOffset:copyTextureToBuffer.DstOffset
            destinationBytesPerRow:copyTextureToBuffer.RowPitch
          destinationBytesPerImage:copyTextureToBuffer.RowPitch * copyTextureToBuffer.NumRows];
}

void MetalCommandList::EnsureEncoder( MetalEncoderType encoderType, std::string errorMessage )
{
    if ( m_activeEncoderType != encoderType )
    {
        LOG( ERROR ) << errorMessage;
        return;
    }

    switch ( encoderType )
    {
    case MetalEncoderType::Blit:
        if ( m_blitEncoder == nil )
        {
            LOG( ERROR ) << errorMessage;
        }
        break;
    case MetalEncoderType::Compute:
        if ( m_computeEncoder == nil )
        {
            LOG( ERROR ) << errorMessage;
        }
        break;
    case MetalEncoderType::Render:
        if ( m_renderEncoder == nil )
        {
            LOG( ERROR ) << errorMessage;
        }
        break;
    case MetalEncoderType::None:
        break;
    }
}

void MetalCommandList::SwitchEncoder( DenOfIz::MetalEncoderType encoderType )
{
    if ( m_activeEncoderType == encoderType )
    {
        return;
    }

    switch ( m_activeEncoderType )
    {
    case MetalEncoderType::Blit:
        [m_blitEncoder endEncoding];
        break;
    case MetalEncoderType::Compute:
        [m_computeEncoder endEncoding];
        break;
    case MetalEncoderType::Render:
        [m_renderEncoder endEncoding];
        break;
    case MetalEncoderType::None:
        break;
    }

    switch ( encoderType )
    {
    case MetalEncoderType::Blit:
        m_activeEncoderType = MetalEncoderType::Blit;
        m_blitEncoder       = [m_commandBuffer blitCommandEncoder];
        break;
    case MetalEncoderType::Compute:
        m_activeEncoderType = MetalEncoderType::Compute;
        m_computeEncoder    = [m_commandBuffer computeCommandEncoder];
        break;
    case MetalEncoderType::Render:
        LOG( ERROR ) << "Using metal, render encoder should be initialized in BeginRendering. This error means the order of your commands ";
        break;
    case MetalEncoderType::None:
        LOG( ERROR ) << "Invalid new encoder type, None should only be used after ending another encoder.";
        break;
    }
}
