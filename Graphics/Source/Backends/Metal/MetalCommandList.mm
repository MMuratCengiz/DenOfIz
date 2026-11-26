/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <algorithm>
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandList.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalQueryPool.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalBottomLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderBindingTable.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalTopLevelAS.h"
#import "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalCommandList::MetalCommandList( MetalContext *context, id<MTLCommandQueue> queue, DenOfIz_CommandListDesc desc ) : m_context( context ), m_queue( queue ), m_desc( desc )
{
    @autoreleasepool
    {
        m_commandBuffer  = [m_context->CommandQueue commandBuffer];
        m_argumentBuffer = std::make_unique<MetalArgumentBuffer>( m_context, 256 * 1024 );
        m_queueFence     = [m_context->Device newFence];
    }
}

MetalCommandList::~MetalCommandList( ) = default;

void MetalCommandList::Begin( )
{
    @autoreleasepool
    {
        m_commandBuffer = [m_context->CommandQueue commandBuffer];
        m_pipeline = nullptr;
        m_boundGroups.clear( );
        m_boundRootConstants.clear( );
        switch ( m_desc.QueueType )
        {
        case DENOFIZ_QUEUE_TYPE_COPY:
            m_activeEncoderType = MetalEncoderType::Blit;
            m_blitEncoder       = [m_commandBuffer blitCommandEncoder];
            break;
        case DENOFIZ_QUEUE_TYPE_COMPUTE:
            m_activeEncoderType = MetalEncoderType::Compute;
            m_computeEncoder    = [m_commandBuffer computeCommandEncoder];
            break;
        case DENOFIZ_QUEUE_TYPE_GRAPHICS:
            break;
        }

        m_currentBufferOffset = 0;
        m_argumentBuffer->Reset( );
        m_boundArgumentSegment = 0;
        m_shaderLayout = nullptr;
        m_argumentsInUse = false;
        ResetEncoderBindings( );
    }
}

void MetalCommandList::BeginRendering( const DenOfIz_RenderingDesc &renderingDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::None, true );

        m_activeEncoderType = MetalEncoderType::Render;
        auto passDesc = MTLRenderPassDescriptor.renderPassDescriptor;
        passDesc.renderTargetArrayLength = renderingDesc.NumLayers > 0 ? renderingDesc.NumLayers : 1;
        for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; i++ )
        {
            const DenOfIz_RenderingAttachmentDesc &attachment = renderingDesc.RTAttachments.Elements[ i ];
            if ( !DENOFIZ_HANDLE_IS_VALID( attachment.Resource ) )
            {
                spdlog::error("BeginRendering called with null render target attachment at index {}", i);
                return;
            }
            MetalTexture *metalRtResource = static_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, attachment.Resource ) );
            DZ_NOT_NULL( metalRtResource );
            passDesc.colorAttachments[ i ].texture         = metalRtResource->Instance( );
            passDesc.colorAttachments[ i ].loadAction      = DenOfIz_MetalEnumConverter_ConvertLoadAction( attachment.LoadOp );
            passDesc.colorAttachments[ i ].storeAction     = DenOfIz_MetalEnumConverter_ConvertStoreAction( attachment.StoreOp );
            passDesc.colorAttachments[ i ].clearColor =
                MTLClearColorMake( attachment.ClearColor.X, attachment.ClearColor.Y, attachment.ClearColor.Z, attachment.ClearColor.W );
        }

        if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.DepthAttachment.Resource ) )
        {
            const DenOfIz_RenderingAttachmentDesc &attachment = renderingDesc.DepthAttachment;
            MetalTexture *depthResource = static_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, renderingDesc.DepthAttachment.Resource ) );
            DZ_NOT_NULL( depthResource );
            const DenOfIz_Format depthFormat = depthResource->GetFormat( );

            if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) && DenOfIz_Format_IsDepthOnly( depthFormat ) )
            {
                spdlog::warn( "MetalCommandList::BeginRendering: Separate stencil attachment provided with depth-only format ({}). "
                             "This configuration may not work as expected on Metal.", static_cast<int>( depthFormat ) );
            }

            passDesc.depthAttachment.texture          = depthResource->Instance( );
            passDesc.depthAttachment.loadAction       = DenOfIz_MetalEnumConverter_ConvertLoadAction( attachment.LoadOp );
            passDesc.depthAttachment.storeAction      = DenOfIz_MetalEnumConverter_ConvertStoreAction( attachment.StoreOp );
            passDesc.depthAttachment.clearDepth       = attachment.ClearDepthStencil.X;

            if ( DenOfIz_Format_HasStencilComponent( depthFormat ) && !DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) )
            {
                passDesc.stencilAttachment.texture      = depthResource->Instance( );
                passDesc.stencilAttachment.loadAction   = DenOfIz_MetalEnumConverter_ConvertLoadAction( attachment.LoadOp );
                passDesc.stencilAttachment.storeAction  = DenOfIz_MetalEnumConverter_ConvertStoreAction( attachment.StoreOp );
                passDesc.stencilAttachment.clearStencil = attachment.ClearDepthStencil.Y;
            }
        }

        if ( DENOFIZ_HANDLE_IS_VALID( renderingDesc.StencilAttachment.Resource ) )
        {
            const DenOfIz_RenderingAttachmentDesc &attachment = renderingDesc.StencilAttachment;
            MetalTexture *stencilResource = static_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, renderingDesc.StencilAttachment.Resource ) );
            DZ_NOT_NULL( stencilResource );
            const DenOfIz_Format stencilFormat = stencilResource->GetFormat( );

            if ( !DenOfIz_Format_HasStencilComponent( stencilFormat ) )
            {
                spdlog::error( "MetalCommandList::BeginRendering: Stencil attachment provided with format ({}) that has no stencil component.",
                              static_cast<int>( stencilFormat ) );
            }

            passDesc.stencilAttachment.texture        = stencilResource->Instance( );
            passDesc.stencilAttachment.loadAction     = DenOfIz_MetalEnumConverter_ConvertLoadAction( attachment.LoadOp );
            passDesc.stencilAttachment.storeAction    = DenOfIz_MetalEnumConverter_ConvertStoreAction( attachment.StoreOp );
            passDesc.stencilAttachment.clearStencil   = attachment.ClearDepthStencil.Y;
        }

        m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
        ResetEncoderBindings( );
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
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::None, true );
    }
}

void MetalCommandList::BindPipeline( IPipeline *pipeline )
{
    @autoreleasepool
    {
        DZ_NOT_NULL( pipeline );
        auto *newPipeline = static_cast<MetalPipeline *>( pipeline );
        if ( m_pipeline != newPipeline )
        {
            m_pipeline      = newPipeline;
            m_bindingsDirty = true;
        }
        switch ( m_pipeline->DenOfIz_BindPoint( ) )
        {
        case DENOFIZ_BIND_POINT_RAYTRACING:
        case DENOFIZ_BIND_POINT_COMPUTE:
            SwitchEncoder( MetalEncoderType::Compute );
            break;
        case DENOFIZ_BIND_POINT_GRAPHICS:
        case DENOFIZ_BIND_POINT_MESH:
            SwitchEncoder( MetalEncoderType::Render );
            break;
        }
        ApplyPipeline( );
    }
}

void MetalCommandList::ApplyPipeline( )
{
    if ( m_pipeline == nullptr || m_encoderPipeline == m_pipeline )
    {
        return;
    }
    m_encoderPipeline = m_pipeline;
    switch ( m_pipeline->DenOfIz_BindPoint( ) )
    {
    case DENOFIZ_BIND_POINT_RAYTRACING:
    case DENOFIZ_BIND_POINT_COMPUTE:
        [m_computeEncoder setComputePipelineState:m_pipeline->ComputePipelineState( )];
        break;
    case DENOFIZ_BIND_POINT_GRAPHICS:
    case DENOFIZ_BIND_POINT_MESH:
        [m_renderEncoder setRenderPipelineState:m_pipeline->GraphicsPipelineState( )];
        [m_renderEncoder setDepthStencilState:m_pipeline->DepthStencilState( )];
        [m_renderEncoder setCullMode:m_pipeline->CullMode( )];
        [m_renderEncoder setTriangleFillMode:m_pipeline->FillMode( )];
        [m_renderEncoder setFrontFacingWinding:m_pipeline->FrontFaceWinding( )];
        [m_renderEncoder setDepthBias:m_pipeline->DepthBias( ) slopeScale:m_pipeline->DepthBiasSlopeScale( ) clamp:m_pipeline->DepthBiasClamp( )];
        break;
    }
}

void MetalCommandList::BindVertexBuffer( IBuffer *buffer, uint64_t offset, uint32_t stride, uint32_t slot )
{
    @autoreleasepool
    {
        DZ_NOT_NULL( buffer );
        id<MTLBuffer> vertexBuffer = static_cast<MetalBuffer *>( buffer )->Instance( );

        switch ( m_desc.QueueType )
        {
        case DENOFIZ_QUEUE_TYPE_COPY:
        case DENOFIZ_QUEUE_TYPE_COMPUTE:
            break;
        case DENOFIZ_QUEUE_TYPE_GRAPHICS:
            [m_renderEncoder setVertexBuffer:vertexBuffer offset:offset atIndex:slot];
            break;
        }
    }
}

void MetalCommandList::BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, uint64_t offset )
{
    @autoreleasepool
    {
        DZ_NOT_NULL( buffer );
        switch ( indexType )
        {
        case DENOFIZ_INDEX_TYPE_UINT16:
            m_indexType = MTLIndexTypeUInt16;
            break;
        case DENOFIZ_INDEX_TYPE_UINT32:
            m_indexType = MTLIndexTypeUInt32;
            break;
        }

        m_indexBuffer       = static_cast<MetalBuffer *>( buffer )->Instance( );
        m_indexBufferOffset = offset;
    }
}

void MetalCommandList::BindViewport( float x, float y, float width, float height )
{
    @autoreleasepool
    {
        if ( width <= 0.0f || height <= 0.0f )
        {
            spdlog::error("Invalid viewport dimensions: width= {} , height={}", width, height);
            return;
        }
        SwitchEncoder( MetalEncoderType::Render );
        MTLViewport viewport = { x, y, width, height, 0.0, 1.0 };
        [m_renderEncoder setViewport:viewport];
    }
}

void MetalCommandList::BindScissorRect( float x, float y, float width, float height )
{
    @autoreleasepool
    {
        if ( width <= 0.0f || height <= 0.0f )
        {
            spdlog::error("Invalid scissor rect dimensions: width= {} , height={}", width, height);
            return;
        }
        SwitchEncoder( MetalEncoderType::Render );
        MTLScissorRect scissorRect = { static_cast<NSUInteger>( x ), static_cast<NSUInteger>( y ), static_cast<NSUInteger>( width ), static_cast<NSUInteger>( height ) };
        [m_renderEncoder setScissorRect:scissorRect];
    }
}

void MetalCommandList::BindResourceGroup( IBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );
    MetalBindGroup *metalBindGroup = static_cast<MetalBindGroup *>( bindGroup );

    const uint32_t registerSpace = metalBindGroup->RegisterSpace( );
    for ( BoundGroup &bound : m_boundGroups )
    {
        if ( bound.Group->RegisterSpace( ) == registerSpace )
        {
            if ( bound.Group != metalBindGroup )
            {
                bound           = BoundGroup{ metalBindGroup, metalBindGroup->Version( ) };
                m_bindingsDirty = true;
            }
            return;
        }
    }
    m_boundGroups.push_back( BoundGroup{ metalBindGroup, metalBindGroup->Version( ) } );
    m_bindingsDirty = true;
}

void MetalCommandList::SetRootConstants( const uint32_t binding, const DenOfIz_ByteArray &data )
{
    if ( data.Elements == nullptr || data.NumElements == 0 )
    {
        return;
    }
    const auto *bytes = static_cast<const uint8_t *>( data.Elements );
    m_bindingsDirty   = true;
    for ( BoundRootConstant &bound : m_boundRootConstants )
    {
        if ( bound.Binding == binding )
        {
            bound.Data.assign( bytes, bytes + data.NumElements );
            return;
        }
    }
    BoundRootConstant &bound = m_boundRootConstants.emplace_back( );
    bound.Binding            = binding;
    bound.Data.assign( bytes, bytes + data.NumElements );
}

void MetalCommandList::BindCommandResources( )
{
    if ( m_pipeline == nullptr )
    {
        return;
    }
    ApplyPipeline( );

    const MetalShaderLayout &shaderLayout = m_pipeline->ShaderLayout( );
    bool                     dirty        = m_bindingsDirty || m_shaderLayout != &shaderLayout;
    for ( const BoundGroup &bound : m_boundGroups )
    {
        dirty = dirty || bound.Version != bound.Group->Version( );
    }
    if ( !dirty )
    {
        return;
    }

    if ( m_shaderLayout != &shaderLayout )
    {
        m_shaderLayout        = &shaderLayout;
        m_currentBufferOffset = m_argumentBuffer->Reserve( m_shaderLayout->NumBytes( ) ).second;
    }
    else if ( m_argumentsInUse )
    {
        m_currentBufferOffset = m_argumentBuffer->Duplicate( m_shaderLayout->NumBytes( ) ).second;
    }
    m_argumentsInUse = true;
    m_bindingsDirty  = false;

    for ( BoundGroup &bound : m_boundGroups )
    {
        ProcessBindGroup( bound.Group );
        bound.Version = bound.Group->Version( );
    }

    for ( const BoundRootConstant &bound : m_boundRootConstants )
    {
        const MetalRootConstantLayout *rootConstant = m_shaderLayout->RootConstant( DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE, bound.Binding );
        if ( rootConstant == nullptr )
        {
            continue;
        }
        const uint32_t numBytes = static_cast<uint32_t>( bound.Data.size( ) );
        if ( numBytes > rootConstant->NumBytes )
        {
            spdlog::error( "Root constant data for binding {} is {} bytes but the shader expects at most {} bytes. Data truncated.", bound.Binding, numBytes,
                           rootConstant->NumBytes );
        }
        m_argumentBuffer->EncodeRootConstant( m_currentBufferOffset + rootConstant->Offset, std::min( numBytes, rootConstant->NumBytes ),
                                              static_cast<const Byte *>( bound.Data.data( ) ) );
    }

    BindTopLevelArgumentBuffer( );
    FlushBatchedResources( );
}

void MetalCommandList::ProcessBindGroup( const MetalBindGroup *metalBindGroup )
{
    if ( m_activeEncoderType == MetalEncoderType::Blit)
    {
        return;
    }

    const MetalSpaceLayout *spaceLayout = m_shaderLayout->Space( metalBindGroup->RegisterSpace( ) );
    if ( spaceLayout == nullptr )
    {
        return;
    }

    const MetalBindGroupTables *tables = metalBindGroup->TablesFor( *spaceLayout );
    for ( const auto &rootParameter : tables->RootParameters )
    {
        m_argumentBuffer->EncodeAddress( m_currentBufferOffset + rootParameter.OffsetBytes, rootParameter.Buffer.gpuAddress );
    }
    const bool cbvSrvUavTable = tables->CbvSrvUavTable && spaceLayout->CbvSrvUavTableOffset != UINT32_MAX;
    const bool samplerTable   = tables->SamplerTable && spaceLayout->SamplerTableOffset != UINT32_MAX;
    if ( cbvSrvUavTable )
    {
        m_argumentBuffer->EncodeAddress( m_currentBufferOffset + spaceLayout->CbvSrvUavTableOffset, tables->CbvSrvUavTable->Buffer( ).gpuAddress );
    }
    if ( samplerTable )
    {
        m_argumentBuffer->EncodeAddress( m_currentBufferOffset + spaceLayout->SamplerTableOffset, tables->SamplerTable->Buffer( ).gpuAddress );
    }

    const uint32_t version  = metalBindGroup->Version( );
    const auto     resident = [ & ]( const ResidentGroup &r ) { return r.Group == metalBindGroup && r.Version == version && r.Tables == tables; };
    if ( std::any_of( m_residentGroups.begin( ), m_residentGroups.end( ), resident ) )
    {
        return;
    }
    m_residentGroups.push_back( ResidentGroup{ metalBindGroup, version, tables } );
    for ( const auto &rootParameter : tables->RootParameters )
    {
        BatchResource( rootParameter.Buffer );
    }
    if ( cbvSrvUavTable )
    {
        BatchResource( tables->CbvSrvUavTable->Buffer( ) );
    }
    if ( samplerTable )
    {
        BatchResource( tables->SamplerTable->Buffer( ) );
    }

    for ( const auto &resource : metalBindGroup->IndirectResources( ) )
    {
        BatchResource( resource );
    }

    for ( const auto &buffer : metalBindGroup->Buffers( ) )
    {
        BatchResource( buffer.Resource->Instance( ), buffer.Usage, buffer.ShaderStages );
    }

    for ( const auto &texture : metalBindGroup->Textures( ) )
    {
        BatchResource( texture.Resource->Instance( ), texture.Usage, texture.ShaderStages );
    }
}

void MetalCommandList::PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier )
{
}

void MetalCommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance )
{
    @autoreleasepool
    {
        if ( indexCount == 0 || instanceCount == 0 )
        {
            spdlog::warn("Possible unintentional behavior, DrawIndexed called with zero count: indexCount= {} , instanceCount={}", indexCount, instanceCount);
        }
        SwitchEncoder( MetalEncoderType::Render );
        BindCommandResources( );

        uint64_t indexSize = (m_indexType == MTLIndexTypeUInt16) ? 2 : 4;
        uint64_t totalByteOffset = m_indexBufferOffset + (firstIndex * indexSize);

        IRRuntimeDrawIndexedPrimitives( m_renderEncoder, m_pipeline->PrimitiveType( ), indexCount, m_indexType, m_indexBuffer, totalByteOffset, instanceCount, vertexOffset, firstInstance );
    }
}

void MetalCommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
{
    @autoreleasepool
    {
        if ( vertexCount == 0 || instanceCount == 0 )
        {
            spdlog::warn("Possible unintentional behavior, Draw called with zero count: vertexCount= {} , instanceCount={}", vertexCount, instanceCount);
        }
        SwitchEncoder( MetalEncoderType::Render );
        BindCommandResources( );
        IRRuntimeDrawPrimitives( m_renderEncoder, m_pipeline->PrimitiveType( ), firstVertex, vertexCount, instanceCount );
    }
}

void MetalCommandList::Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ )
{
    @autoreleasepool
    {
        if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
        {
            spdlog::warn("Possible unintentional behavior, Dispatch called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ);
        }
        SwitchEncoder( MetalEncoderType::Compute );
        BindCommandResources( );
        MTLSize threadGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
        MTLSize threadsPerThreadgroup = m_pipeline->ComputeThreadsPerThreadgroup();
        [m_computeEncoder dispatchThreadgroups:threadGroupsPerGrid threadsPerThreadgroup:threadsPerThreadgroup];
    }
}

void MetalCommandList::DispatchMesh( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    @autoreleasepool
    {
        if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
        {
            spdlog::warn("Possible unintentional behavior, DispatchMesh called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ);
        }
        SwitchEncoder( MetalEncoderType::Render );
        BindCommandResources( );
        MTLSize meshGroupsPerGrid = MTLSizeMake( groupCountX, groupCountY, groupCountZ );
        MTLSize objectThreads = m_pipeline->ObjectThreadsPerThreadgroup();
        MTLSize meshThreads = m_pipeline->MeshThreadsPerThreadgroup();
        [m_renderEncoder drawMeshThreadgroups:meshGroupsPerGrid threadsPerObjectThreadgroup:objectThreads threadsPerMeshThreadgroup:meshThreads];
    }
}

void MetalCommandList::DrawIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride )
{
    @autoreleasepool
    {
        if ( !buffer )
        {
            spdlog::error( "MetalCommandList::DrawIndirect: buffer is null" );
            return;
        }

        auto *metalBuffer = static_cast<MetalBuffer *>( buffer );

        SwitchEncoder( MetalEncoderType::Render );
        BindCommandResources( );
        BatchResource( metalBuffer->Instance( ) );
        FlushBatchedResources( );

        for ( uint32_t i = 0; i < drawCount; ++i )
        {
            IRRuntimeDrawPrimitives( m_renderEncoder, m_pipeline->PrimitiveType( ), metalBuffer->Instance( ), offset + i * stride );
        }
    }
}

void MetalCommandList::DrawIndexedIndirect( IBuffer *buffer, uint64_t offset, uint32_t drawCount, uint32_t stride )
{
    @autoreleasepool
    {
        if ( !buffer )
        {
            spdlog::error( "MetalCommandList::DrawIndexedIndirect: buffer is null" );
            return;
        }

        if ( !m_indexBuffer )
        {
            spdlog::error( "MetalCommandList::DrawIndexedIndirect: No index buffer bound" );
            return;
        }

        auto *metalBuffer = static_cast<MetalBuffer *>( buffer );

        SwitchEncoder( MetalEncoderType::Render );
        BindCommandResources( );
        BatchResource( metalBuffer->Instance( ) );
        FlushBatchedResources( );

        for ( uint32_t i = 0; i < drawCount; ++i )
        {
            IRRuntimeDrawIndexedPrimitives(
                m_renderEncoder,
                m_pipeline->PrimitiveType( ),
                m_indexType,
                m_indexBuffer,
                m_indexBufferOffset,
                metalBuffer->Instance( ),
                offset + i * stride
            );
        }
    }
}

void MetalCommandList::DispatchIndirect( IBuffer *buffer, uint64_t offset )
{
    @autoreleasepool
    {
        if ( !buffer )
        {
            spdlog::error( "MetalCommandList::DispatchIndirect: buffer is null" );
            return;
        }

        auto *metalBuffer = static_cast<MetalBuffer *>( buffer );
    
        SwitchEncoder( MetalEncoderType::Compute );
        BindCommandResources( );
    
        [m_computeEncoder dispatchThreadgroupsWithIndirectBuffer:metalBuffer->Instance( )
                                             indirectBufferOffset:offset
                                            threadsPerThreadgroup:m_pipeline->ComputeThreadsPerThreadgroup( )];
    }
}

void MetalCommandList::CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::Blit );

        if ( copyBufferRegionDesc.NumBytes == 0 )
        {
            spdlog::warn("Possible unintentional behavior, CopyBufferRegion called with zero NumBytes");
        }

        MetalBuffer *srcBuffer = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.SrcBuffer ) );
        MetalBuffer *dstBuffer = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.DstBuffer ) );
        DZ_NOT_NULL( srcBuffer );
        DZ_NOT_NULL( dstBuffer );

        [m_blitEncoder copyFromBuffer:srcBuffer->Instance( )
                         sourceOffset:copyBufferRegionDesc.SrcOffset
                             toBuffer:dstBuffer->Instance( )
                    destinationOffset:copyBufferRegionDesc.DstOffset
                                 size:copyBufferRegionDesc.NumBytes];
    }
}

void MetalCommandList::CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::Blit );

        if ( copyTextureRegionDesc.Width == 0 || copyTextureRegionDesc.Height == 0 )
        {
            spdlog::warn("Possible unintentional behavior, CopyTextureRegion called with zero dimensions: Width= {} , Height={}", copyTextureRegionDesc.Width, copyTextureRegionDesc.Height);
        }

        MetalTexture *srcTexture = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.SrcTexture ) );
        MetalTexture *dstTexture = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.DstTexture ) );
        DZ_NOT_NULL( srcTexture );
        DZ_NOT_NULL( dstTexture );

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
}

void MetalCommandList::CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTexture )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::Blit );

        MetalBuffer  *srcBuffer  = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferToTexture.SrcBuffer ) );
        MetalTexture *dstTexture = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyBufferToTexture.DstTexture ) );
        DZ_NOT_NULL( srcBuffer );
        DZ_NOT_NULL( dstTexture );

        const uint32_t width  = std::max( 1u, dstTexture->GetWidth( ) >> copyBufferToTexture.MipLevel );
        const uint32_t height = std::max( 1u, dstTexture->GetHeight( ) >> copyBufferToTexture.MipLevel );
        const uint32_t depth  = std::max( 1u, dstTexture->GetDepth( ) >> copyBufferToTexture.MipLevel );

        const uint32_t formatSize      = DenOfIz_Format_NumBytes( copyBufferToTexture.Format );
        const uint32_t blockSize       = DenOfIz_Format_BlockSize( copyBufferToTexture.Format );
        const uint32_t rowPitch        = std::max( 1U, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
        const uint32_t numRows         = std::max( 1U, ( height + ( blockSize - 1 ) ) / blockSize );
        const uint32_t alignedRowPitch = Utilities::Align( rowPitch, m_context->SelectedDeviceInfo.Constants.BufferTextureRowAlignment );

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
}

void MetalCommandList::CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBuffer )
{
    @autoreleasepool
    {
        MetalTexture *srcTexture = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureToBuffer.SrcTexture ) );
        MetalBuffer  *dstBuffer  = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyTextureToBuffer.DstBuffer ) );
        DZ_NOT_NULL( srcTexture );
        DZ_NOT_NULL( dstBuffer );
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
}

void MetalCommandList::UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::AccelerationStructure );

        MetalTopLevelAS *metalTopLevelAS = static_cast<MetalTopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, updateDesc.TopLevelAS ) );
        MetalBuffer *scratchBuffer = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, updateDesc.ScratchBuffer ) );
        if ( !metalTopLevelAS )
        {
            spdlog::error("Invalid top level acceleration structure.");
            return;
        }
        if ( !scratchBuffer )
        {
            spdlog::error("Invalid scratch buffer.");
            return;
        }

        BatchResource( metalTopLevelAS->AccelerationStructure( ), MTLResourceUsageRead );
        BatchResource( scratchBuffer->Instance( ), MTLResourceUsageWrite );
        BatchResource( metalTopLevelAS->HeaderBuffer( ), MTLResourceUsageWrite );
        BatchResource( metalTopLevelAS->InstanceBuffer( ), MTLResourceUsageRead );
        FlushBatchedResources( );

        [m_accelerationStructureEncoder buildAccelerationStructure:metalTopLevelAS->AccelerationStructure( )
                                                        descriptor:metalTopLevelAS->Descriptor( )
                                                     scratchBuffer:scratchBuffer->Instance( )
                                               scratchBufferOffset:updateDesc.ScratchBufferOffset];
    }
}

void MetalCommandList::BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::AccelerationStructure );

        MetalTopLevelAS *metalTopLevelAS = static_cast<MetalTopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, buildTopLevelASDesc.TopLevelAS ) );
        MetalBuffer *scratchBuffer = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildTopLevelASDesc.ScratchBuffer ) );
        if ( !metalTopLevelAS )
        {
            spdlog::error("Invalid top level acceleration structure.");
            return;
        }
        if ( !scratchBuffer )
        {
            spdlog::error("Invalid scratch buffer.");
            return;
        }

        BatchResource( metalTopLevelAS->AccelerationStructure( ), MTLResourceUsageRead );
        BatchResource( scratchBuffer->Instance( ), MTLResourceUsageWrite );
        BatchResource( metalTopLevelAS->HeaderBuffer( ), MTLResourceUsageWrite );
        BatchResource( metalTopLevelAS->InstanceBuffer( ), MTLResourceUsageRead );
        FlushBatchedResources( );

        [m_accelerationStructureEncoder buildAccelerationStructure:metalTopLevelAS->AccelerationStructure( )
                                                        descriptor:metalTopLevelAS->Descriptor( )
                                                     scratchBuffer:scratchBuffer->Instance( )
                                               scratchBufferOffset:buildTopLevelASDesc.ScratchBufferOffset];
    }
}

void MetalCommandList::BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    @autoreleasepool
    {
        SwitchEncoder( MetalEncoderType::AccelerationStructure );

        MetalBottomLevelAS *metalBottomLevelAS = static_cast<MetalBottomLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBottomLevelAS, buildBottomLevelASDesc.BottomLevelAS ) );
        MetalBuffer *scratchBuffer = dynamic_cast<MetalBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildBottomLevelASDesc.ScratchBuffer ) );
        if ( !metalBottomLevelAS )
        {
            spdlog::error("Invalid bottom level acceleration structure.");
            return;
        }
        if ( !scratchBuffer )
        {
            spdlog::error("Invalid scratch buffer.");
            return;
        }

        BatchResource( metalBottomLevelAS->AccelerationStructure( ), MTLResourceUsageRead );
        BatchResource( scratchBuffer->Instance( ), MTLResourceUsageWrite );
        FlushBatchedResources( );

        [m_accelerationStructureEncoder buildAccelerationStructure:metalBottomLevelAS->AccelerationStructure( )
                                                        descriptor:metalBottomLevelAS->Descriptor( )
                                                     scratchBuffer:scratchBuffer->Instance( )
                                               scratchBufferOffset:buildBottomLevelASDesc.ScratchBufferOffset];
    }
}

void MetalCommandList::DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc )
{
    @autoreleasepool
    {
        DZ_NOT_NULL( dispatchRaysDesc.ShaderBindingTable );
        if ( dispatchRaysDesc.Width == 0 || dispatchRaysDesc.Height == 0 || dispatchRaysDesc.Depth == 0 )
        {
            spdlog::warn("DispatchRays called with zero dimensions: width= {} , height= {} , depth={}", dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth);
        }
        SwitchEncoder( MetalEncoderType::Compute );
        BindCommandResources( );

        MetalShaderBindingTable *shaderBindingTable = static_cast<MetalShaderBindingTable *>( DENOFIZ_FROM_HANDLE( DenOfIz::IShaderBindingTable, dispatchRaysDesc.ShaderBindingTable ) );

        for ( const auto &missShader : shaderBindingTable->UsedResources( ) )
        {
            BatchResource( missShader );
        }
        BatchResource( m_pipeline->VisibleFunctionTable( ) );
        BatchResource( m_pipeline->IntersectionFunctionTable( ) );
        BatchResource( m_argumentBuffer->Buffer( ) );
        FlushBatchedResources( );

        IRDispatchRaysDescriptor irDispatchRaysDesc;
        irDispatchRaysDesc.RayGenerationShaderRecord = shaderBindingTable->RayGenerationShaderRange( );
        irDispatchRaysDesc.HitGroupTable             = shaderBindingTable->HitGroupShaderRange( );
        irDispatchRaysDesc.MissShaderTable           = shaderBindingTable->MissShaderRange( );
        irDispatchRaysDesc.CallableShaderTable       = { .StartAddress = 0, .SizeInBytes = 0, .StrideInBytes = 0 };
        irDispatchRaysDesc.Width                     = dispatchRaysDesc.Width;
        irDispatchRaysDesc.Height                    = dispatchRaysDesc.Height;
        irDispatchRaysDesc.Depth                     = dispatchRaysDesc.Depth;

        IRDispatchRaysArgument dispatchRaysArgs = {};
        dispatchRaysArgs.DispatchRaysDesc          = irDispatchRaysDesc;
        dispatchRaysArgs.GRS                       = m_argumentBuffer->Buffer( ).gpuAddress + m_currentBufferOffset;

        if ( getenv( "DZ_DEBUG_DISPATCH" ) )
        {
            static int dumpCount = 0;
            if ( dumpCount++ < 3 )
            {
                const uint64_t *grsCpu = reinterpret_cast<const uint64_t *>( static_cast<const uint8_t *>( m_argumentBuffer->Buffer( ).contents ) + m_currentBufferOffset );
                spdlog::warn( "DispatchRays: GRS gpuAddr={:#x} offset={} qword0(tablePtr)={:#x}", m_argumentBuffer->Buffer( ).gpuAddress, m_currentBufferOffset, grsCpu[ 0 ] );
                spdlog::warn( "DispatchRays: raygen SBT addr={:#x} size={} hitGroup addr={:#x} stride={} miss addr={:#x}", irDispatchRaysDesc.RayGenerationShaderRecord.StartAddress,
                              irDispatchRaysDesc.RayGenerationShaderRecord.SizeInBytes, irDispatchRaysDesc.HitGroupTable.StartAddress, irDispatchRaysDesc.HitGroupTable.StrideInBytes,
                              irDispatchRaysDesc.MissShaderTable.StartAddress );
            }
        }
        dispatchRaysArgs.ResDescHeap               = 0;
        dispatchRaysArgs.SmpDescHeap               = 0;
        dispatchRaysArgs.VisibleFunctionTable      = m_pipeline->VisibleFunctionTable( ).gpuResourceID;
        dispatchRaysArgs.IntersectionFunctionTable = m_pipeline->IntersectionFunctionTable( ).gpuResourceID;

        [m_computeEncoder setBytes:&dispatchRaysArgs length:sizeof( IRDispatchRaysArgument ) atIndex:kIRRayDispatchArgumentsBindPoint];

        MTLSize threadGroupSize = MTLSizeMake( 8, 8, 1 );
        MTLSize gridSize = MTLSizeMake( dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
        [m_computeEncoder dispatchThreads:gridSize threadsPerThreadgroup:threadGroupSize];
    }
}

void MetalCommandList::BindTopLevelArgumentBuffer( )
{
    id<MTLBuffer> buffer = m_argumentBuffer->Buffer( );
    uint64_t      offset = m_currentBufferOffset;

    if ( m_argumentBuffer->Segment( ) != m_boundArgumentSegment )
    {
        m_boundArgumentSegment = m_argumentBuffer->Segment( );
        m_computeTlasBound     = false;
        m_renderTlasBound      = false;
        m_meshTlasBound        = false;
    }

    if ( m_activeEncoderType == MetalEncoderType::Compute )
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
        if ( m_pipeline && m_pipeline->DenOfIz_BindPoint() == DENOFIZ_BIND_POINT_MESH )
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

void MetalCommandList::ResetEncoderBindings( )
{
    m_encoderPipeline = nullptr;
    m_residentGroups.clear( );
    m_bindingsDirty    = true;
    m_computeTlasBound = false;
    m_renderTlasBound  = false;
    m_meshTlasBound    = false;
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
        ResetEncoderBindings( );
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
            spdlog::error("Using metal, render encoder should be initialized in BeginRendering. This error means the order of your commands ");
            break;
        case MetalEncoderType::None:
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

void MetalCommandList::BatchResource( const id<MTLResource> &resource, MTLResourceUsage usage, MTLRenderStages stages )
{
    if ( resource )
    {
        m_batchedResources.push_back( { resource, usage, stages } );
    }
}

void MetalCommandList::FlushBatchedResources( )
{
    if ( m_batchedResources.empty( ) )
    {
        return;
    }

    @autoreleasepool
    {
        const bool render  = m_activeEncoderType == MetalEncoderType::Render;
        const auto keyOf   = [ render ]( const BatchedResource &r ) { return static_cast<uint64_t>( r.Usage ) << 32 | ( render ? static_cast<uint64_t>( r.Stages ) : 0 ); };
        std::sort( m_batchedResources.begin( ), m_batchedResources.end( ), [ &keyOf ]( const BatchedResource &a, const BatchedResource &b ) { return keyOf( a ) < keyOf( b ); } );
        for ( size_t first = 0; first < m_batchedResources.size( ); )
        {
            const BatchedResource &head = m_batchedResources[ first ];
            m_usedResources.clear( );
            size_t last = first;
            for ( ; last < m_batchedResources.size( ) && keyOf( m_batchedResources[ last ] ) == keyOf( head ); ++last )
            {
                m_usedResources.push_back( m_batchedResources[ last ].Resource );
            }
            switch ( m_activeEncoderType )
            {
            case MetalEncoderType::Render:
                [m_renderEncoder useResources:m_usedResources.data( ) count:m_usedResources.size( ) usage:head.Usage stages:head.Stages];
                break;
            case MetalEncoderType::Compute:
                [m_computeEncoder useResources:m_usedResources.data( ) count:m_usedResources.size( ) usage:head.Usage];
                break;
            case MetalEncoderType::AccelerationStructure:
                [m_accelerationStructureEncoder useResources:m_usedResources.data( ) count:m_usedResources.size( ) usage:head.Usage];
                break;
            case MetalEncoderType::Blit:
            case MetalEncoderType::None:
                break;
            }
            first = last;
        }
        m_usedResources.clear( );
    }

    m_batchedResources.clear( );
}

const DenOfIz_QueueType MetalCommandList::GetQueueType( )
{
    return m_desc.QueueType;
}

const id<MTLCommandBuffer> MetalCommandList::GetCommandBuffer( ) const
{
    return m_commandBuffer;
}

void MetalCommandList::BeginDebugMarker( float r, float g, float b, DenOfIz_StringView name )
{
    @autoreleasepool
    {
        std::string markerName( name.Chars, name.NumChars );
        NSString *nsName = [NSString stringWithUTF8String:markerName.c_str( ) ];

        if ( m_activeEncoderType == MetalEncoderType::Render && m_renderEncoder )
        {
            [m_renderEncoder pushDebugGroup:nsName];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Compute && m_computeEncoder )
        {
            [m_computeEncoder pushDebugGroup:nsName];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Blit && m_blitEncoder )
        {
            [m_blitEncoder pushDebugGroup:nsName];
        }
        else if ( m_commandBuffer )
        {
            [m_commandBuffer pushDebugGroup:nsName];
        }
    }
}

void MetalCommandList::EndDebugMarker( )
{
    @autoreleasepool
    {
        if ( m_activeEncoderType == MetalEncoderType::Render && m_renderEncoder )
        {
            [m_renderEncoder popDebugGroup];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Compute && m_computeEncoder )
        {
            [m_computeEncoder popDebugGroup];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Blit && m_blitEncoder )
        {
            [m_blitEncoder popDebugGroup];
        }
        else if ( m_commandBuffer )
        {
            [m_commandBuffer popDebugGroup];
        }
    }
}

void MetalCommandList::InsertDebugMarker( float r, float g, float b, DenOfIz_StringView name )
{
    @autoreleasepool
    {
        NSString *nsName = [NSString stringWithUTF8String:name.Chars];
        if ( m_activeEncoderType == MetalEncoderType::Render && m_renderEncoder )
        {
            [m_renderEncoder insertDebugSignpost:nsName];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Compute && m_computeEncoder )
        {
            [m_computeEncoder insertDebugSignpost:nsName];
        }
        else if ( m_activeEncoderType == MetalEncoderType::Blit && m_blitEncoder )
        {
            [m_blitEncoder insertDebugSignpost:nsName];
        }
    }
}

void MetalCommandList::BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    @autoreleasepool
    {
        auto *metalQueryPool = dynamic_cast<MetalQueryPool *>( queryPool );
        if ( !metalQueryPool )
        {
            spdlog::error( "MetalCommandList::BeginQuery: Invalid query pool" );
            return;
        }

        if ( metalQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION )
        {
            if ( m_activeEncoderType == MetalEncoderType::Render && m_renderEncoder )
            {
                [m_renderEncoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:queryDesc.Index * sizeof( uint64_t )];
            }
        }
        else if ( metalQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
        {
            if ( metalQueryPool->GetCounterSampleBuffer( ) )
            {
                uint32_t actualIndex = queryDesc.Index * 2;
                if ( m_renderEncoder )
                {
                    [m_renderEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                            atSampleIndex:actualIndex 
                                              withBarrier:NO];
                }
                else if ( m_computeEncoder )
                {
                    [m_computeEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                             atSampleIndex:actualIndex 
                                               withBarrier:NO];
                }
                else if ( m_blitEncoder )
                {
                    [m_blitEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                          atSampleIndex:actualIndex 
                                            withBarrier:NO];
                }
            }
        }
    }
}

void MetalCommandList::EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    @autoreleasepool
    {
        auto *metalQueryPool = dynamic_cast<MetalQueryPool *>( queryPool );
        if ( !metalQueryPool )
        {
            spdlog::error( "MetalCommandList::EndQuery: Invalid query pool" );
            return;
        }

        if ( metalQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION )
        {
            if ( m_activeEncoderType == MetalEncoderType::Render && m_renderEncoder )
            {
                [m_renderEncoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
            }
        }
        else if ( metalQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
        {
            if ( metalQueryPool->GetCounterSampleBuffer( ) )
            {
                uint32_t actualIndex = queryDesc.Index * 2 + 1;
                if ( m_renderEncoder )
                {
                    [m_renderEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                            atSampleIndex:actualIndex 
                                              withBarrier:NO];
                }
                else if ( m_computeEncoder )
                {
                    [m_computeEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                             atSampleIndex:actualIndex 
                                               withBarrier:NO];
                }
                else if ( m_blitEncoder )
                {
                    [m_blitEncoder sampleCountersInBuffer:metalQueryPool->GetCounterSampleBuffer( ) 
                                          atSampleIndex:actualIndex 
                                            withBarrier:NO];
                }
            }
        }
    }
}

void MetalCommandList::ResolveQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount )
{
}

void MetalCommandList::ResetQuery( IQueryPool *queryPool, uint32_t startQuery, uint32_t queryCount )
{
}

