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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandList.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalBottomLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderBindingTable.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalTopLevelAS.h"
#import "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

MetalCommandList::MetalCommandList( MetalContext *context, CommandListDesc desc ) : m_context( context ), m_desc( desc )
{
    m_commandBuffer  = [m_context->CommandQueue commandBuffer];
    m_argumentBuffer = std::make_unique<MetalArgumentBuffer>( m_context, 256 * 1024 );
    m_queueFence     = [m_context->Device newFence];
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
    m_queuedBindGroups.clear( );
    m_rootSignature = nullptr;
    m_computeTlasBound = false;
    m_renderTlasBound = false;
    m_meshTlasBound = false;
}

void MetalCommandList::BeginRendering( const RenderingDesc &renderingDesc )
{
    SwitchEncoder( MetalEncoderType::None, true );

    m_activeEncoderType = MetalEncoderType::Render;
    // Begin rendering with the provided rendering information
    auto passDesc = MTLRenderPassDescriptor.renderPassDescriptor;
    @autoreleasepool
    {
        for ( auto i = 0; i < renderingDesc.RTAttachments.NumElements( ); i++ )
        {
            const RenderingAttachmentDesc &attachment = renderingDesc.RTAttachments.GetElement( i );
            if ( attachment.Resource == nullptr )
            {
                LOG( ERROR ) << "BeginRendering called with null render target attachment at index " << i;
                return;
            }
            MetalTextureResource *metalRtResource = static_cast<MetalTextureResource *>( attachment.Resource );
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
        if ( m_waitForQueueFence )
        {
            [m_renderEncoder waitForFence:m_queueFence beforeStages:MTLRenderStageVertex];
            m_waitForQueueFence = false;
        }
    }
}

void MetalCommandList::EndRendering( )
{
}

void MetalCommandList::End( )
{
    SwitchEncoder( MetalEncoderType::None, true );
}

void MetalCommandList::BindPipeline( IPipeline *pipeline )
{
    DZ_NOT_NULL( pipeline );
    m_pipeline = static_cast<MetalPipeline *>( pipeline );
    @autoreleasepool
    {
        switch ( m_pipeline->BindPoint( ) )
        {
        case BindPoint::RayTracing:
        case BindPoint::Compute:
            SwitchEncoder( MetalEncoderType::Compute );
            [m_computeEncoder setComputePipelineState:m_pipeline->ComputePipelineState( )];
            break;
        case BindPoint::Graphics:
        case BindPoint::Mesh:
            SwitchEncoder( MetalEncoderType::Render );
            [m_renderEncoder setRenderPipelineState:m_pipeline->GraphicsPipelineState( )];
            [m_renderEncoder setDepthStencilState:m_pipeline->DepthStencilState( )];
            [m_renderEncoder setCullMode:m_pipeline->CullMode( )];
            [m_renderEncoder setTriangleFillMode:m_pipeline->FillMode( )];
            [m_renderEncoder setFrontFacingWinding:MTLWindingClockwise];
            break;
        }
    }
}

void MetalCommandList::BindVertexBuffer( IBufferResource *buffer, uint64_t offset )
{
    DZ_NOT_NULL( buffer );
    id<MTLBuffer> vertexBuffer = static_cast<MetalBufferResource *>( buffer )->Instance( );

    switch ( m_desc.QueueType )
    {
    case QueueType::Copy:
    case QueueType::Compute:
        LOG( WARNING ) << "BindVertexBuffer is not supported for Copy and Compute queues";
        break;
    case QueueType::Graphics:
        {
            [m_renderEncoder setVertexBuffer:vertexBuffer offset:offset atIndex:0];
        }
        break;
    }
}

void MetalCommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType, uint64_t offset )
{
    DZ_NOT_NULL( buffer );
    switch ( indexType )
    {
    case IndexType::Uint16:
        m_indexType = MTLIndexTypeUInt16;
        break;
    case IndexType::Uint32:
        m_indexType = MTLIndexTypeUInt32;
        break;
    }

    m_indexBuffer       = static_cast<MetalBufferResource *>( buffer )->Instance( );
    m_indexBufferOffset = offset;
}

void MetalCommandList::BindViewport( float x, float y, float width, float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        LOG( ERROR ) << "Invalid viewport dimensions: width=" << width << ", height=" << height;
        return;
    }
    SwitchEncoder( MetalEncoderType::Render );
    MTLViewport viewport = { x, y, width, height, 0.0, 1.0 };
    [m_renderEncoder setViewport:viewport];
}

void MetalCommandList::BindScissorRect( float x, float y, float width, float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        LOG( ERROR ) << "Invalid scissor rect dimensions: width=" << width << ", height=" << height;
        return;
    }
    SwitchEncoder( MetalEncoderType::Render );
    MTLScissorRect scissorRect = { static_cast<NSUInteger>( x ), static_cast<NSUInteger>( y ), static_cast<NSUInteger>( width ), static_cast<NSUInteger>( height ) };
    [m_renderEncoder setScissorRect:scissorRect];
}

void MetalCommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );
    MetalResourceBindGroup *metalBindGroup = static_cast<MetalResourceBindGroup *>( bindGroup );
    m_queuedBindGroups.push_back( metalBindGroup );
}

void MetalCommandList::BindCommandResources( )
{
    for ( auto &bindGroup : m_queuedBindGroups )
    {
        ProcessBindGroup( bindGroup );
    }
    if ( m_rootSignature == nullptr && m_pipeline != nullptr )
    {
        m_rootSignature = m_pipeline->RootSignature( );
    }
    if ( m_rootSignature != nullptr )
    {
        BindTopLevelArgumentBuffer( );
    }
}

void MetalCommandList::ProcessBindGroup( const MetalResourceBindGroup *metalBindGroup )
{
    if ( m_activeEncoderType == MetalEncoderType::Blit)
    {
        return;
    }
    
    if ( m_rootSignature == nullptr || m_rootSignature != metalBindGroup->RootSignature( ) )
    {
        m_rootSignature       = metalBindGroup->RootSignature( );
        m_currentBufferOffset = m_argumentBuffer->Reserve( m_rootSignature->NumTLABAddresses( ), m_rootSignature->NumRootConstantBytes( ) ).second;
    }

    const auto &	rootConstant = metalBindGroup->RootConstant( );
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
    if ( cbvSrvUavTable != nullptr )
    {
        m_argumentBuffer->EncodeAddress( addressesOffset, cbvSrvUavTable->TLABOffset, cbvSrvUavTable->Table.Buffer( ).gpuAddress );
        UseResource( cbvSrvUavTable->Table.Buffer( ) );
    }
    const MetalDescriptorTableBinding *samplerTable = metalBindGroup->SamplerTable( );
    if ( samplerTable != nullptr )
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
    if ( indexCount == 0 || instanceCount == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, DrawIndexed called with zero count: indexCount=" << indexCount << ", instanceCount=" << instanceCount;
    }
    SwitchEncoder( MetalEncoderType::Render );
    BindCommandResources( );

    uint64_t indexSize = (m_indexType == MTLIndexTypeUInt16) ? 2 : 4;
    uint64_t totalByteOffset = m_indexBufferOffset + (firstIndex * indexSize);

    IRRuntimeDrawIndexedPrimitives( m_renderEncoder, MTLPrimitiveTypeTriangle, indexCount, m_indexType, m_indexBuffer, totalByteOffset, instanceCount, vertexOffset, firstInstance );
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
{
    if ( vertexCount == 0 || instanceCount == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, Draw called with zero count: vertexCount=" << vertexCount << ", instanceCount=" << instanceCount;
    }
    SwitchEncoder( MetalEncoderType::Render );
    BindCommandResources( );
    IRRuntimeDrawPrimitives( m_renderEncoder, MTLPrimitiveTypeTriangle, firstVertex, vertexCount, instanceCount );
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, Dispatch called with zero group count: x=" << groupCountX << ", y=" << groupCountY << ", z=" << groupCountZ;
    }
    SwitchEncoder( MetalEncoderType::Compute );
    BindCommandResources( );
    MTLSize threadGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
    MTLSize threadsPerThreadgroup = m_pipeline->ComputeThreadsPerThreadgroup();
    [m_computeEncoder dispatchThreadgroups:threadGroupsPerGrid threadsPerThreadgroup:threadsPerThreadgroup];
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::DispatchMesh( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, DispatchMesh called with zero group count: x=" << groupCountX << ", y=" << groupCountY << ", z=" << groupCountZ;
    }
    SwitchEncoder( MetalEncoderType::Render );
    BindCommandResources( );
    MTLSize meshGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
    MTLSize objectThreads = m_pipeline->ObjectThreadsPerThreadgroup();
    MTLSize meshThreads = m_pipeline->MeshThreadsPerThreadgroup();
    [m_renderEncoder drawMeshThreadgroups:meshGroupsPerGrid threadsPerObjectThreadgroup:objectThreads threadsPerMeshThreadgroup:meshThreads];
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionDesc )
{
    DZ_NOT_NULL( copyBufferRegionDesc.SrcBuffer );
    DZ_NOT_NULL( copyBufferRegionDesc.DstBuffer );
    SwitchEncoder( MetalEncoderType::Blit );
    
    if ( copyBufferRegionDesc.NumBytes == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, CopyBufferRegion called with zero NumBytes";
    }
    
    MetalBufferResource *srcBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionDesc.SrcBuffer );
    MetalBufferResource *dstBuffer = dynamic_cast<MetalBufferResource *>( copyBufferRegionDesc.DstBuffer );

    [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                     sourceOffset:copyBufferRegionDesc.SrcOffset
                         toBuffer:dstBuffer->Instance( )
                destinationOffset:copyBufferRegionDesc.DstOffset
                             size:copyBufferRegionDesc.NumBytes];
}

void MetalCommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionDesc )
{
    DZ_NOT_NULL( copyTextureRegionDesc.SrcTexture );
    DZ_NOT_NULL( copyTextureRegionDesc.DstTexture );
    SwitchEncoder( MetalEncoderType::Blit );

    if ( copyTextureRegionDesc.Width == 0 || copyTextureRegionDesc.Height == 0 )
    {
        LOG( WARNING ) << "Possible unintentional behavior, CopyTextureRegion called with zero dimensions: Width=" << copyTextureRegionDesc.Width
                       << ", Height=" << copyTextureRegionDesc.Height;
    }
    
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionDesc.SrcTexture );
    MetalTextureResource *dstTexture = dynamic_cast<MetalTextureResource *>( copyTextureRegionDesc.DstTexture );

    [m_blitEncoder copyFromTexture:srcTexture->Instance( )
                       sourceSlice:copyTextureRegionDesc.SrcArrayLayer
                       sourceLevel:copyTextureRegionDesc.SrcMipLevel
                      sourceOrigin:MTLOriginMake( copyTextureRegionDesc.SrcX, copyTextureRegionDesc.SrcY, copyTextureRegionDesc.SrcZ )
                        sourceSize:MTLSizeMake( copyTextureRegionDesc.Width, copyTextureRegionDesc.Height, copyTextureRegionDesc.Depth )
                         toTexture:dstTexture->Instance( )
                  destinationSlice:copyTextureRegionDesc.DstArrayLayer
                  destinationLevel:copyTextureRegionDesc.DstMipLevel
                 destinationOrigin:MTLOriginMake( copyTextureRegionDesc.DstX, copyTextureRegionDesc.DstY, copyTextureRegionDesc.DstZ )];
}

void MetalCommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture )
{
    DZ_NOT_NULL( copyBufferToTexture.SrcBuffer );
    DZ_NOT_NULL( copyBufferToTexture.DstTexture );
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
    DZ_NOT_NULL( copyTextureToBuffer.SrcTexture );
    DZ_NOT_NULL( copyTextureToBuffer.DstBuffer );
    
    MetalTextureResource *srcTexture = dynamic_cast<MetalTextureResource *>( copyTextureToBuffer.SrcTexture );
    MetalBufferResource  *dstBuffer  = dynamic_cast<MetalBufferResource *>( copyTextureToBuffer.DstBuffer );
    // TODO Calculate RowPitch and NumRows automatically if possible
    SwitchEncoder( MetalEncoderType::Blit );

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

void MetalCommandList::UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc )
{
    SwitchEncoder( MetalEncoderType::AccelerationStructure );

    MetalTopLevelAS *metalTopLevelAS = static_cast<MetalTopLevelAS *>( updateDesc.TopLevelAS );
    if ( !metalTopLevelAS )
    {
        LOG( ERROR ) << "Invalid top level acceleration structure.";
        return;
    }

    UpdateTransformsDesc updateTransformDesc{ };
    updateTransformDesc.Transforms = updateDesc.Transforms;
    metalTopLevelAS->UpdateInstanceTransforms( updateTransformDesc );

    [m_accelerationStructureEncoder useResource:metalTopLevelAS->AccelerationStructure( ) usage:MTLResourceUsageRead];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->Scratch( ) usage:MTLResourceUsageWrite];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->HeaderBuffer( ) usage:MTLResourceUsageWrite];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->InstanceBuffer( ) usage:MTLResourceUsageRead];

    [m_accelerationStructureEncoder buildAccelerationStructure:metalTopLevelAS->AccelerationStructure( )
                                                    descriptor:metalTopLevelAS->Descriptor( )
                                                 scratchBuffer:metalTopLevelAS->Scratch( )
                                           scratchBufferOffset:0];
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

    [m_accelerationStructureEncoder useResource:metalTopLevelAS->AccelerationStructure( ) usage:MTLResourceUsageRead];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->Scratch( ) usage:MTLResourceUsageWrite];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->HeaderBuffer( ) usage:MTLResourceUsageWrite];
    [m_accelerationStructureEncoder useResource:metalTopLevelAS->InstanceBuffer( ) usage:MTLResourceUsageRead];

    [m_accelerationStructureEncoder buildAccelerationStructure:metalTopLevelAS->AccelerationStructure( )
                                                    descriptor:metalTopLevelAS->Descriptor( )
                                                 scratchBuffer:metalTopLevelAS->Scratch( )
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

    [m_accelerationStructureEncoder useResource:metalBottomLevelAS->AccelerationStructure( ) usage:MTLResourceUsageRead];
    [m_accelerationStructureEncoder useResource:metalBottomLevelAS->Scratch( ) usage:MTLResourceUsageWrite];

    [m_accelerationStructureEncoder buildAccelerationStructure:metalBottomLevelAS->AccelerationStructure( )
                                                    descriptor:metalBottomLevelAS->Descriptor( )
                                                 scratchBuffer:metalBottomLevelAS->Scratch( )
                                           scratchBufferOffset:0];
}

void MetalCommandList::DispatchRays( const DispatchRaysDesc &dispatchRaysDesc )
{
    DZ_NOT_NULL( dispatchRaysDesc.ShaderBindingTable );
    if ( dispatchRaysDesc.Width == 0 || dispatchRaysDesc.Height == 0 || dispatchRaysDesc.Depth == 0 )
    {
        LOG( WARNING ) << "DispatchRays called with zero dimensions: width=" << dispatchRaysDesc.Width << ", height=" << dispatchRaysDesc.Height
                       << ", depth=" << dispatchRaysDesc.Depth;
    }
    SwitchEncoder( MetalEncoderType::Compute );
    BindCommandResources( );

    MetalShaderBindingTable *shaderBindingTable = static_cast<MetalShaderBindingTable *>( dispatchRaysDesc.ShaderBindingTable );

    for ( const auto &missShader : shaderBindingTable->UsedResources( ) )
    {
        UseResource( missShader );
    }
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
    MTLSize    threadGroupSize  = (MTLSize){ threadGroupSizeX, 1, 1 };

    MTLSize gridSize = MTLSizeMake( dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
    [m_computeEncoder dispatchThreadgroups:gridSize threadsPerThreadgroup:threadGroupSize];
    TopLevelArgumentBufferNextOffset( );
}

void MetalCommandList::BindTopLevelArgumentBuffer( )
{
    id<MTLBuffer> buffer = m_argumentBuffer->Buffer( );
    uint64_t      offset = m_currentBufferOffset;

    if ( m_desc.QueueType == QueueType::Compute )
    {
        if ( !m_computeTlasBound )
        {
            [m_computeEncoder setBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
            m_computeTlasBound = true;
        }
        else
        {
            [m_computeEncoder setBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
        }
    }
    else
    {
        if ( m_pipeline && m_pipeline->BindPoint() == BindPoint::Mesh )
        {
            const MeshShaderUsedStages& meshShaderUsedStages = m_pipeline->MeshShaderUsedStages( );
            if ( !m_meshTlasBound )
            {
                if ( meshShaderUsedStages.Task )
                {
                    [m_renderEncoder setObjectBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
                }
                if ( meshShaderUsedStages.Mesh )
                {
                    [m_renderEncoder setMeshBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
                }
                if ( meshShaderUsedStages.Pixel )
                {
                    [m_renderEncoder setFragmentBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
                }
                m_meshTlasBound = true;
            }
            else
            {
                if ( meshShaderUsedStages.Task )
                {
                    [m_renderEncoder setObjectBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
                }
                if ( meshShaderUsedStages.Mesh )
                {
                    [m_renderEncoder setMeshBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
                }
                if ( meshShaderUsedStages.Pixel )
                {
                    [m_renderEncoder setFragmentBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
                }
            }
        }
        else
        {
            if ( !m_renderTlasBound )
            {
                [m_renderEncoder setVertexBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
                [m_renderEncoder setFragmentBuffer:buffer offset:offset atIndex:kIRArgumentBufferBindPoint];
                m_renderTlasBound = true;
            }
            else
            {
                [m_renderEncoder setVertexBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
                [m_renderEncoder setFragmentBufferOffset:offset atIndex:kIRArgumentBufferBindPoint];
            }
        }
    }
}

void MetalCommandList::TopLevelArgumentBufferNextOffset( )
{
    m_currentBufferOffset = m_argumentBuffer->Duplicate( m_rootSignature->NumTLABAddresses( ), m_rootSignature->NumRootConstantBytes( ) ).second;
}

void MetalCommandList::SwitchEncoder( DenOfIz::MetalEncoderType encoderType, bool crossQueueBarrier )
{
    @autoreleasepool
    {
        if ( m_activeEncoderType == encoderType )
        {
            return;
        }

        if ( m_blitEncoder )
        {
            if ( crossQueueBarrier && encoderType != MetalEncoderType::Blit )
            {
                [m_blitEncoder updateFence:m_queueFence];
                m_waitForQueueFence = true;
            }
            [m_blitEncoder endEncoding];
            m_blitEncoder = nil;
        }

        if ( m_computeEncoder )
        {
            if ( crossQueueBarrier && encoderType != MetalEncoderType::Compute )
            {
                [m_computeEncoder updateFence:m_queueFence];
                m_waitForQueueFence = true;
            }
            [m_computeEncoder endEncoding];
            m_computeEncoder = nil;
        }
        if ( m_renderEncoder )
        {
            if ( crossQueueBarrier && encoderType != MetalEncoderType::Render )
            {
                [m_renderEncoder updateFence:m_queueFence afterStages:MTLRenderStageFragment];
                m_waitForQueueFence = true;
            }
            [m_renderEncoder endEncoding];
            m_renderEncoder = nil;
        }
        if ( m_accelerationStructureEncoder )
        {
            if ( crossQueueBarrier && encoderType != MetalEncoderType::AccelerationStructure )
            {
                [m_accelerationStructureEncoder updateFence:m_queueFence];
                m_waitForQueueFence = true;
            }
            [m_accelerationStructureEncoder endEncoding];
            m_accelerationStructureEncoder = nil;
        }

        m_activeEncoderType = encoderType;
        switch ( encoderType )
        {
        case MetalEncoderType::Blit:
            if ( m_blitEncoder == nil )
            {
                m_blitEncoder = [m_commandBuffer blitCommandEncoder];
            }
            if ( m_waitForQueueFence )
            {
                [m_blitEncoder updateFence:m_queueFence];
            }
            break;
        case MetalEncoderType::Compute:
            if ( m_computeEncoder == nil )
            {
                m_computeEncoder = [m_commandBuffer computeCommandEncoder];
            }
            if ( m_waitForQueueFence )
            {
                [m_computeEncoder updateFence:m_queueFence];
            }
            break;
        case MetalEncoderType::AccelerationStructure:
            if ( m_accelerationStructureEncoder == nil )
            {
                m_accelerationStructureEncoder = [m_commandBuffer accelerationStructureCommandEncoder];
            }
            if ( m_waitForQueueFence )
            {
                [m_accelerationStructureEncoder updateFence:m_queueFence];
            }
            break;
        case MetalEncoderType::Render:
            LOG( ERROR ) << "Using metal, render encoder should be initialized in BeginRendering. This error means the order of your commands ";
            break;
        case MetalEncoderType::None:
            // Simply end current encoder
            break;
        }
        m_waitForQueueFence = false;
    }
}

void MetalCommandList::UseResource( const id<MTLResource> &resource, MTLResourceUsage usage, MTLRenderStages stages )
{
    switch ( m_activeEncoderType )
    {
    case MetalEncoderType::Render:
        [m_renderEncoder useResource:resource usage:usage stages:stages];
        break;
    case MetalEncoderType::Compute:
        [m_computeEncoder useResource:resource usage:usage];
        break;
    case MetalEncoderType::Blit:
        break;
    case MetalEncoderType::AccelerationStructure:
        [m_accelerationStructureEncoder useResource:resource usage:usage];
        break;
    case MetalEncoderType::None:
        break;
    }
}

const QueueType MetalCommandList::GetQueueType( )
{
    return m_desc.QueueType;
}

const id<MTLCommandBuffer> MetalCommandList::GetCommandBuffer( ) const
{
    return m_commandBuffer;
}
