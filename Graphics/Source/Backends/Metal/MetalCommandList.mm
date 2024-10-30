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
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalBottomLevelAS.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderBindingTable.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalTopLevelAS.h>

using namespace DenOfIz;

MetalCommandList::MetalCommandList( MetalContext *context, CommandListDesc desc ) : m_context( context ), m_desc( desc )
{
    m_commandBuffer  = [m_context->CommandQueue commandBuffer];
    m_argumentBuffer = std::make_unique<MetalArgumentBuffer>( m_context, 256 * 1024 );
}

MetalCommandList::~MetalCommandList( ) = default;

void MetalCommandList::Begin( )
{
    @autoreleasepool
    {
        m_commandBuffer = [m_context->CommandQueue commandBuffer];
        switch ( m_desc.QueueType )
        {
        case QueueType::Copy:
            m_activeEncoderType = MetalEncoderType::Blit;
            m_blitEncoder       = [m_commandBuffer blitCommandEncoder];
            break;
        case QueueType::RayTracing: // Validate
        case QueueType::Compute:
            m_activeEncoderType = MetalEncoderType::Compute;
            m_computeEncoder    = [m_commandBuffer computeCommandEncoder];
            break;
        case QueueType::Graphics:
            // Initialized in BeginRendering
            break;
        }
    }

    m_currentBufferOffset = 0;
    m_argumentBuffer->Reset( );
    m_rootSignature = nullptr;
}

void MetalCommandList::BeginRendering( const RenderingDesc &renderingDesc )
{
    if ( m_blitEncoder || m_computeEncoder )
    {
        LOG( ERROR ) << "Expected null blit or compute encoder, make sure the CommandList order is correct.";
    }

    m_activeEncoderType = MetalEncoderType::Render;
    // Begin rendering with the provided rendering information
    auto passDesc = MTLRenderPassDescriptor.renderPassDescriptor;
    @autoreleasepool
    {
        for ( auto i = 0; i < renderingDesc.RTAttachments.NumElements( ); i++ )
        {
            const RenderingAttachmentDesc &attachment      = renderingDesc.RTAttachments.GetElement( i );
            MetalTextureResource          *metalRtResource = static_cast<MetalTextureResource *>( attachment.Resource );
            passDesc.colorAttachments[ i ].texture         = metalRtResource->Instance( );
            passDesc.colorAttachments[ i ].loadAction      = MetalEnumConverter::ConvertLoadAction( attachment.LoadOp );
            passDesc.colorAttachments[ i ].storeAction     = MetalEnumConverter::ConvertStoreAction( attachment.StoreOp );
            passDesc.colorAttachments[ i ].clearColor =
                MTLClearColorMake( attachment.ClearColor[ 0 ], attachment.ClearColor[ 1 ], attachment.ClearColor[ 2 ], attachment.ClearColor[ 3 ] );
        }

        if ( renderingDesc.DepthAttachment.Resource )
        {
            const RenderingAttachmentDesc &attachment = renderingDesc.DepthAttachment;
            passDesc.depthAttachment.texture          = static_cast<MetalTextureResource *>( renderingDesc.DepthAttachment.Resource )->Instance( );
            passDesc.depthAttachment.loadAction       = MetalEnumConverter::ConvertLoadAction( attachment.LoadOp );
            passDesc.depthAttachment.storeAction      = MetalEnumConverter::ConvertStoreAction( attachment.StoreOp );
            passDesc.depthAttachment.clearDepth       = attachment.ClearDepthStencil[ 0 ]; // Validate
        }

        if ( renderingDesc.StencilAttachment.Resource )
        {
            const RenderingAttachmentDesc &attachment = renderingDesc.StencilAttachment;
            passDesc.stencilAttachment.texture        = static_cast<MetalTextureResource *>( renderingDesc.StencilAttachment.Resource )->Instance( );
            passDesc.stencilAttachment.loadAction     = MetalEnumConverter::ConvertLoadAction( attachment.LoadOp );
            passDesc.stencilAttachment.storeAction    = MetalEnumConverter::ConvertStoreAction( attachment.StoreOp );
            passDesc.stencilAttachment.clearStencil   = attachment.ClearDepthStencil[ 1 ]; // Validate
        }

        m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    }
}

void MetalCommandList::EndRendering( )
{
}

void MetalCommandList::Execute( const ExecuteDesc &executeDesc )
{
    @autoreleasepool
    {
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
        else if ( m_accelerationStructureEncoder )
        {
            [m_accelerationStructureEncoder endEncoding];
            m_accelerationStructureEncoder = nil;
        }

        if ( executeDesc.Notify )
        {
            MetalFence *metalFence = static_cast<MetalFence *>( executeDesc.Notify );
            metalFence->NotifyOnCommandBufferCompletion( m_commandBuffer );
        }

        for ( int i = 0; i < executeDesc.NotifySemaphores.NumElements( ); ++i )
        {
            MetalSemaphore *metalSemaphore = static_cast<MetalSemaphore *>( executeDesc.NotifySemaphores.GetElement( i ) );
            metalSemaphore->NotifyOnCommandBufferCompletion( m_commandBuffer );
        }

        [m_commandBuffer commit];
    }
}

void MetalCommandList::Present( ISwapChain *swapChain, uint32_t imageIndex, const InteropArray<ISemaphore *> &waitOnLocks )
{
    MetalSwapChain *metalSwapChain = static_cast<MetalSwapChain *>( swapChain );
    metalSwapChain->Present( waitOnLocks );
}

void MetalCommandList::BindPipeline( IPipeline *pipeline )
{
    m_pipeline = static_cast<MetalPipeline *>( pipeline );
    @autoreleasepool
    {
        switch ( m_desc.QueueType )
        {
        case QueueType::Copy:
            break;
        case QueueType::Compute:
        case QueueType::RayTracing:
            [m_computeEncoder setComputePipelineState:m_pipeline->ComputePipelineState( )];
            break;
        case QueueType::Graphics:
            [m_renderEncoder setRenderPipelineState:m_pipeline->GraphicsPipelineState( )];
            [m_renderEncoder setDepthStencilState:m_pipeline->DepthStencilState( )];
            [m_renderEncoder setCullMode:m_pipeline->CullMode( )];
            [m_renderEncoder setFrontFacingWinding:MTLWindingClockwise];
            break;
        }
    }
}

void MetalCommandList::BindVertexBuffer( IBufferResource *buffer )
{
    id<MTLBuffer> vertexBuffer = static_cast<MetalBufferResource *>( buffer )->Instance( );

    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
    case QueueType::Compute:
    case QueueType::RayTracing:
        LOG( WARNING ) << "BindVertexBuffer is not supported for Copy and Compute queues";
        break;
    case QueueType::Graphics:
        {
            [m_renderEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        }
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

    if ( m_rootSignature == nullptr && m_rootSignature != metalBindGroup->RootSignature( ) )
    {
        m_rootSignature       = metalBindGroup->RootSignature( );
        m_currentBufferOffset = m_argumentBuffer->Reserve( m_rootSignature->NumTLABAddresses( ), m_rootSignature->NumRootConstantBytes( ) ).second;
    }

    const auto &rootConstant = metalBindGroup->RootConstant( );
    if ( !rootConstant.empty( ) )
    {
        m_argumentBuffer->EncodeRootConstant( m_currentBufferOffset, m_rootSignature->NumRootConstantBytes( ), rootConstant.data( ) );
    }

    uint64_t addressesOffset = m_currentBufferOffset + m_rootSignature->NumRootConstantBytes( );
    for ( const auto &rootParameter : metalBindGroup->RootParameters( ) )
    {
        m_argumentBuffer->EncodeAddress( addressesOffset, rootParameter.TLABOffset, rootParameter.Buffer.gpuAddress );
        UseResource( rootParameter.Buffer );
    }

    const MetalDescriptorTableBinding *cbvSrvUavTable = metalBindGroup->CbvSrvUavTable( );
    if ( cbvSrvUavTable != nullptr && cbvSrvUavTable->NumEntries > 0 )
    {
        m_argumentBuffer->EncodeAddress( addressesOffset, cbvSrvUavTable->TLABOffset, cbvSrvUavTable->Table.Buffer( ).gpuAddress );
        UseResource( cbvSrvUavTable->Table.Buffer( ) );
    }
    const MetalDescriptorTableBinding *samplerTable = metalBindGroup->SamplerTable( );
    if ( samplerTable != nullptr && samplerTable->NumEntries > 0 )
    {
        m_argumentBuffer->EncodeAddress( addressesOffset, samplerTable->TLABOffset, samplerTable->Table.Buffer( ).gpuAddress );
        UseResource( samplerTable->Table.Buffer( ) );
    }

    for ( const auto &resource : metalBindGroup->IndirectResources( ) )
    {
        UseResource( resource );
    }

    for ( const auto &buffer : metalBindGroup->Buffers( ) )
    {
        UseResource( buffer.Resource->Instance( ), buffer.Usage, buffer.ShaderStages );
    }

    for ( const auto &texture : metalBindGroup->Textures( ) )
    {
        UseResource( texture.Resource->Instance( ), texture.Usage, texture.ShaderStages );
    }
}

void MetalCommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
}

void MetalCommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance )
{
    EnsureEncoder( MetalEncoderType::Render, "DrawIndexed called without a render encoder. Make sure to call BeginRendering" );
    BindTopLevelArgumentBuffer( );
    IRRuntimeDrawIndexedPrimitives( m_renderEncoder, MTLPrimitiveTypeTriangle, indexCount, m_indexType, m_indexBuffer, firstIndex, instanceCount, vertexOffset, firstInstance );
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
{
    EnsureEncoder( MetalEncoderType::Render, "Draw called without a render encoder. Make sure to call BeginRendering" );
    BindTopLevelArgumentBuffer( );
    IRRuntimeDrawPrimitives( m_renderEncoder, MTLPrimitiveTypeTriangle, firstVertex, vertexCount, instanceCount );
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    EnsureEncoder( MetalEncoderType::Compute, "Dispatch called without a compute encoder. Make sure to call Begin with QueueType::Compute" );
    BindTopLevelArgumentBuffer( );
    MTLSize threadGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
    [m_computeEncoder dispatchThreadgroups:threadGroupsPerGrid threadsPerThreadgroup:MTLSizeMake( 1, 1, 1 )];
    TopLevelArgumentBufferNextOffset( );
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

void MetalCommandList::BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc )
{
    SwitchEncoder( MetalEncoderType::AccelerationStructure );

    MetalTopLevelAS *metalTopLevelAS = static_cast<MetalTopLevelAS *>( buildTopLevelASDesc.TopLevelAS );
    if ( !metalTopLevelAS )
    {
        LOG( ERROR ) << "Invalid top level acceleration structure.";
        return;
    }

    [m_accelerationStructureEncoder buildAccelerationStructure:metalTopLevelAS->AccelerationStructure( )
                                                    descriptor:metalTopLevelAS->Descriptor( )
                                                 scratchBuffer:metalTopLevelAS->Scratch( )->Instance( )
                                           scratchBufferOffset:0];
}

void MetalCommandList::BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    SwitchEncoder( MetalEncoderType::AccelerationStructure );

    MetalBottomLevelAS *metalBottomLevelAS = static_cast<MetalBottomLevelAS *>( buildBottomLevelASDesc.BottomLevelAS );
    if ( !metalBottomLevelAS )
    {
        LOG( ERROR ) << "Invalid bottom level acceleration structure.";
        return;
    }

    [m_accelerationStructureEncoder buildAccelerationStructure:metalBottomLevelAS->AccelerationStructure( )
                                                    descriptor:metalBottomLevelAS->Descriptor( )
                                                 scratchBuffer:metalBottomLevelAS->Scratch( )->Instance( )
                                           scratchBufferOffset:0];
}

void MetalCommandList::DispatchRays( const DispatchRaysDesc &dispatchRaysDesc )
{
    SwitchEncoder( MetalEncoderType::Compute );

    MetalShaderBindingTable *shaderBindingTable = static_cast<MetalShaderBindingTable *>( dispatchRaysDesc.ShaderBindingTable );

    if ( shaderBindingTable == nullptr )
    {
        LOG( ERROR ) << "DispatchRaysDesc.ShaderBindingTable == null";
        return;
    }

    UseResource( shaderBindingTable->MetalBuffer( ) );
    UseResource( m_pipeline->VisibleFunctionTable( ) );
    UseResource( m_pipeline->IntersectionFunctionTable( ) );
    UseResource( m_argumentBuffer->Buffer( ) );

    IRDispatchRaysDescriptor irDispatchRaysDesc;
    irDispatchRaysDesc.RayGenerationShaderRecord = shaderBindingTable->RayGenerationShaderRange( );
    irDispatchRaysDesc.HitGroupTable             = shaderBindingTable->HitGroupShaderRange( );
    irDispatchRaysDesc.MissShaderTable           = shaderBindingTable->MissShaderRange( );
    irDispatchRaysDesc.CallableShaderTable       = { .StartAddress = 0, .SizeInBytes = 0, .StrideInBytes = 0 };
    irDispatchRaysDesc.Width                     = dispatchRaysDesc.Width;
    irDispatchRaysDesc.Height                    = dispatchRaysDesc.Height;
    irDispatchRaysDesc.Depth                     = dispatchRaysDesc.Depth;

    IRDispatchRaysArgument dispatchRaysArgs;
    dispatchRaysArgs.DispatchRaysDesc          = irDispatchRaysDesc;
    dispatchRaysArgs.GRS                       = m_argumentBuffer->Buffer( ).gpuAddress + m_currentBufferOffset;
    dispatchRaysArgs.ResDescHeap               = 0;
    dispatchRaysArgs.SmpDescHeap               = 0;
    dispatchRaysArgs.VisibleFunctionTable      = m_pipeline->VisibleFunctionTable( ).gpuResourceID;
    dispatchRaysArgs.IntersectionFunctionTable = m_pipeline->IntersectionFunctionTable( ).gpuResourceID;

    [m_computeEncoder setBytes:&dispatchRaysArgs length:sizeof( IRDispatchRaysArgument ) atIndex:kIRRayDispatchArgumentsBindPoint];

    NSUInteger threadGroupSizeX = [m_pipeline->ComputePipelineState( ) maxTotalThreadsPerThreadgroup];
    MTLSize    threadGroupSize  = ( MTLSize ){ threadGroupSizeX, 1, 1 };

    MTLSize gridSize = MTLSize( dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
    [m_computeEncoder dispatchThreadgroups:gridSize threadsPerThreadgroup:threadGroupSize];
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::BindTopLevelArgumentBuffer( )
{
    id<MTLBuffer> buffer = m_argumentBuffer->Buffer( );
    uint64_t      offset = m_currentBufferOffset;
    if ( m_currentBufferOffset == 0 )
    {
        if ( m_desc.QueueType == QueueType::Compute || m_desc.QueueType == QueueType::RayTracing )
        {
            [m_computeEncoder setBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
        }
        else
        {
            [m_renderEncoder setVertexBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
            [m_renderEncoder setFragmentBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
        }
    }
    else
    {
        if ( m_desc.QueueType == QueueType::Compute || m_desc.QueueType == QueueType::RayTracing )
        {
            [m_computeEncoder setBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
        }
        else
        {
            [m_renderEncoder setVertexBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
            [m_renderEncoder setFragmentBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
        }
    }
}

void MetalCommandList::TopLevelArgumentBufferNextOffset( )
{
    m_currentBufferOffset = m_argumentBuffer->Duplicate( m_rootSignature->NumTLABAddresses( ), m_rootSignature->NumRootConstantBytes( ) ).second;
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
    case MetalEncoderType::AccelerationStructure:
        if ( m_accelerationStructureEncoder == nil )
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
    @autoreleasepool
    {
        if ( m_activeEncoderType == encoderType )
        {
            return;
        }

        switch ( m_activeEncoderType )
        {
        case MetalEncoderType::Blit:
            [m_blitEncoder endEncoding];
            m_blitEncoder = nil;
            break;
        case MetalEncoderType::Compute:
            [m_computeEncoder endEncoding];
            m_computeEncoder = nil;
            break;
        case MetalEncoderType::AccelerationStructure:
            [m_accelerationStructureEncoder endEncoding];
            m_computeEncoder = nil;
        case MetalEncoderType::Render:
            [m_renderEncoder endEncoding];
            m_renderEncoder = nil;
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
        case MetalEncoderType::AccelerationStructure:
            m_activeEncoderType            = MetalEncoderType::AccelerationStructure;
            m_accelerationStructureEncoder = [m_commandBuffer accelerationStructureCommandEncoder];
            break;
        case MetalEncoderType::Render:
            LOG( ERROR ) << "Using metal, render encoder should be initialized in BeginRendering. This error means the order of your commands ";
            break;
        case MetalEncoderType::None:
            LOG( ERROR ) << "Invalid new encoder type, None should only be used after ending another encoder.";
            break;
        }
    }
}
