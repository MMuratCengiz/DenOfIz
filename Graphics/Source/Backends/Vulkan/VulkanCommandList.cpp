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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandList.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipelineBarrierHelper.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanQueryPool.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanSwapChain.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanCommandList::VulkanCommandList( VulkanContext *context, const DenOfIz_CommandListDesc desc, const VkCommandPool commandPool ) :
    m_desc( desc ), m_context( context ), m_commandPool( commandPool )
{
    m_queueType = desc.QueueType;

    VkCommandBufferAllocateInfo allocInfo{ };
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Todo
    allocInfo.commandBufferCount = 1;

    VK_CHECK_RESULT( vkAllocateCommandBuffers( m_context->LogicalDevice, &allocInfo, &m_commandBuffer ) );
}

void VulkanCommandList::Begin( )
{
    VK_CHECK_RESULT( vkResetCommandBuffer( m_commandBuffer, 0 ) );
    m_queuedBindGroups.clear( );
    m_queuedRootConstants.clear( );
    m_currentPipeline = nullptr;
    m_viewportDirty   = true;
    m_scissorDirty    = true;

    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = { };

    VK_CHECK_RESULT( vkBeginCommandBuffer( m_commandBuffer, &beginInfo ) );
}

// Todo !IMPROVEMENT! this function may not need to exist.
void VulkanCommandList::BeginRendering( const DenOfIz_RenderingDesc &renderingDesc )
{
    for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; ++i )
    {
        if ( renderingDesc.RTAttachments.Elements[ i ].Resource != DENOFIZ_NULL_HANDLE )
        {
            const auto colorAttachmentResource = DENOFIZ_FROM_HANDLE( ITexture, renderingDesc.RTAttachments.Elements[ i ].Resource );
            m_context->AutoSync->BeginRendering( colorAttachmentResource );
        }
    }

    VkRenderingInfo renderInfo{ };
    renderInfo.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea.extent = VkExtent2D( renderingDesc.RenderAreaWidth, renderingDesc.RenderAreaHeight );
    renderInfo.renderArea.offset = VkOffset2D( renderingDesc.RenderAreaOffsetX, renderingDesc.RenderAreaOffsetY );
    renderInfo.layerCount        = renderingDesc.NumLayers;
    renderInfo.viewMask          = 0;

    std::vector<VkRenderingAttachmentInfo> colorAttachments;

    for ( uint32_t i = 0; i < renderingDesc.RTAttachments.NumElements; ++i )
    {
        const auto &colorAttachment = renderingDesc.RTAttachments.Elements[ i ];
        if ( colorAttachment.Resource == DENOFIZ_NULL_HANDLE )
        {
            spdlog::error( "BeginRendering called with null render target attachment at index {}", i );
            return;
        }

        const auto colorAttachmentResource   = DENOFIZ_FROM_HANDLE( ITexture, colorAttachment.Resource );
        auto      *vkColorAttachmentResource = dynamic_cast<VulkanTexture *>( colorAttachmentResource );

        VkRenderingAttachmentInfo colorAttachmentInfo{ };
        colorAttachmentInfo.sType                         = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageView                     = vkColorAttachmentResource->ImageView( );
        colorAttachmentInfo.imageLayout                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp                        = DenOfIz_VulkanEnumConverter_ConvertLoadOp( colorAttachment.LoadOp );
        colorAttachmentInfo.storeOp                       = DenOfIz_VulkanEnumConverter_ConvertStoreOp( colorAttachment.StoreOp );
        colorAttachmentInfo.clearValue.color.float32[ 0 ] = colorAttachment.ClearColor.X;
        colorAttachmentInfo.clearValue.color.float32[ 1 ] = colorAttachment.ClearColor.Y;
        colorAttachmentInfo.clearValue.color.float32[ 2 ] = colorAttachment.ClearColor.Z;
        colorAttachmentInfo.clearValue.color.float32[ 3 ] = colorAttachment.ClearColor.W;

        if ( renderInfo.renderArea.extent.height == 0 )
        {
            renderInfo.renderArea.extent.width  = vkColorAttachmentResource->GetWidth( );
            renderInfo.renderArea.extent.height = vkColorAttachmentResource->GetHeight( );
        }

        colorAttachments.push_back( colorAttachmentInfo );
    }

    renderInfo.colorAttachmentCount = colorAttachments.size( );
    renderInfo.pColorAttachments    = colorAttachments.data( );

    if ( renderingDesc.DepthAttachment.Resource != DENOFIZ_NULL_HANDLE )
    {

        const auto           depthStencilResource   = DENOFIZ_FROM_HANDLE( ITexture, renderingDesc.DepthAttachment.Resource );
        const auto          *vkDepthStencilResource = dynamic_cast<VulkanTexture *>( depthStencilResource );
        const DenOfIz_Format depthFormat            = vkDepthStencilResource->GetFormat( );

        if ( renderingDesc.StencilAttachment.Resource != DENOFIZ_NULL_HANDLE && DenOfIz_Format_IsDepthOnly( depthFormat ) )
        {
            spdlog::error( "VulkanCommandList::BeginRendering: Separate stencil attachment provided with depth-only format ({}). "
                           "This is not supported in Vulkan.",
                           static_cast<int>( depthFormat ) );
        }

        if ( renderInfo.renderArea.extent.height == 0 )
        {
            renderInfo.renderArea.extent.width  = vkDepthStencilResource->GetWidth( );
            renderInfo.renderArea.extent.height = vkDepthStencilResource->GetHeight( );
        }

        VkRenderingAttachmentInfo depthAttachmentInfo{ };
        depthAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView   = vkDepthStencilResource->ImageView( );
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp      = DenOfIz_VulkanEnumConverter_ConvertLoadOp( renderingDesc.DepthAttachment.LoadOp );
        depthAttachmentInfo.storeOp     = DenOfIz_VulkanEnumConverter_ConvertStoreOp( renderingDesc.DepthAttachment.StoreOp );
        depthAttachmentInfo.clearValue.depthStencil =
            VkClearDepthStencilValue( renderingDesc.DepthAttachment.ClearDepthStencil.X, renderingDesc.DepthAttachment.ClearDepthStencil.Y );

        renderInfo.pDepthAttachment = &depthAttachmentInfo;

        if ( DenOfIz_Format_HasStencilComponent( depthFormat ) && renderingDesc.StencilAttachment.Resource == DENOFIZ_NULL_HANDLE )
        {
            renderInfo.pStencilAttachment = &depthAttachmentInfo;
        }
    }

    if ( renderingDesc.StencilAttachment.Resource != DENOFIZ_NULL_HANDLE )
    {
        const auto depthStencilResource = DENOFIZ_FROM_HANDLE( ITexture, renderingDesc.DepthAttachment.Resource );

        const auto          *vkDepthStencilResource = dynamic_cast<VulkanTexture *>( depthStencilResource );
        const DenOfIz_Format stencilFormat          = vkDepthStencilResource->GetFormat( );

        if ( !DenOfIz_Format_HasStencilComponent( stencilFormat ) )
        {
            spdlog::error( "VulkanCommandList::BeginRendering: Stencil attachment provided with format ({}) that has no stencil component.", static_cast<int>( stencilFormat ) );
        }

        VkRenderingAttachmentInfo stencilAttachmentInfo{ };
        stencilAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        stencilAttachmentInfo.imageView   = vkDepthStencilResource->ImageView( );
        stencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        stencilAttachmentInfo.loadOp      = DenOfIz_VulkanEnumConverter_ConvertLoadOp( renderingDesc.StencilAttachment.LoadOp );
        stencilAttachmentInfo.storeOp     = DenOfIz_VulkanEnumConverter_ConvertStoreOp( renderingDesc.StencilAttachment.StoreOp );
        stencilAttachmentInfo.clearValue.depthStencil =
            VkClearDepthStencilValue( renderingDesc.StencilAttachment.ClearDepthStencil.X, renderingDesc.DepthAttachment.ClearDepthStencil.Y );

        renderInfo.pStencilAttachment = &stencilAttachmentInfo;
    }

    vkCmdBeginRendering( m_commandBuffer, &renderInfo );
}

void VulkanCommandList::EndRendering( )
{
    vkCmdEndRendering( m_commandBuffer );
}

void VulkanCommandList::End( )
{
    VK_CHECK_RESULT( vkEndCommandBuffer( m_commandBuffer ) );
}

void VulkanCommandList::BindPipeline( IPipeline *pipeline )
{
    DZ_NOT_NULL( pipeline );
    auto *newPipeline = dynamic_cast<VulkanPipeline *>( pipeline );
    if ( m_currentPipeline == newPipeline )
    {
        return;
    }

    m_currentPipeline = newPipeline;
    vkCmdBindPipeline( m_commandBuffer, m_currentPipeline->DenOfIz_BindPoint( ), m_currentPipeline->Instance( ) );
}

void VulkanCommandList::BindVertexBuffer( IBuffer *buffer, const uint64_t offset, uint32_t stride, const uint32_t slot )
{
    DZ_NOT_NULL( buffer );
    const auto         bufferResource = dynamic_cast<VulkanBuffer *>( buffer );
    const VkDeviceSize vkOffset       = offset;
    vkCmdBindVertexBuffers( m_commandBuffer, slot, 1, &bufferResource->Instance( ), &vkOffset );
}

void VulkanCommandList::BindIndexBuffer( IBuffer *buffer, const DenOfIz_IndexType &indexType, const uint64_t offset )
{
    DZ_NOT_NULL( buffer );
    const auto         bufferResource = dynamic_cast<VulkanBuffer *>( buffer );
    const VkDeviceSize vkOffset       = offset;

    switch ( indexType )
    {
    case DENOFIZ_INDEX_TYPE_UINT16:
        vkCmdBindIndexBuffer( m_commandBuffer, bufferResource->Instance( ), vkOffset, VK_INDEX_TYPE_UINT16 );
        break;
    case DENOFIZ_INDEX_TYPE_UINT32:
        vkCmdBindIndexBuffer( m_commandBuffer, bufferResource->Instance( ), vkOffset, VK_INDEX_TYPE_UINT32 );
        break;
    }
}

void VulkanCommandList::BindViewport( const float offsetX, const float offsetY, const float width, const float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid viewport dimensions: width= {} , height={}", width, height );
        return;
    }
    const float newY      = offsetY + height;
    const float newHeight = -height;
    if ( m_viewport.x == offsetX && m_viewport.y == newY && m_viewport.width == width && m_viewport.height == newHeight && !m_viewportDirty )
    {
        return;
    }

    m_viewport.x = offsetX;
    // Vulkan has inverted y-axis
    m_viewport.y      = newY;
    m_viewport.height = newHeight;
    // --
    m_viewport.width    = width;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
    m_viewportDirty     = false;

    vkCmdSetViewportWithCount( m_commandBuffer, 1, &m_viewport );
}

void VulkanCommandList::BindScissorRect( const float offsetX, const float offsetY, const float width, const float height )
{
    if ( width <= 0.0f || height <= 0.0f )
    {
        spdlog::error( "Invalid scissor rect dimensions: width= {} , height={}", width, height );
        return;
    }
    if ( m_scissorRect.offset.x == static_cast<int32_t>( offsetX ) && m_scissorRect.offset.y == static_cast<int32_t>( offsetY ) &&
         m_scissorRect.extent.width == static_cast<uint32_t>( width ) && m_scissorRect.extent.height == static_cast<uint32_t>( height ) && !m_scissorDirty )
    {
        return;
    }

    m_scissorRect               = VkRect2D( );
    m_scissorRect.offset.x      = offsetX;
    m_scissorRect.offset.y      = offsetY;
    m_scissorRect.extent.width  = width;
    m_scissorRect.extent.height = height;
    m_scissorDirty              = false;
    vkCmdSetScissorWithCount( m_commandBuffer, 1, &m_scissorRect );
}

void VulkanCommandList::BindResourceGroup( IBindGroup *bindGroup )
{
    DZ_NOT_NULL( bindGroup );
    const auto *vkBindGroup = dynamic_cast<VulkanBindGroup *>( bindGroup );
    m_queuedBindGroups.push_back( vkBindGroup );
}

void VulkanCommandList::SetRootConstants( const uint32_t binding, const DenOfIz_ByteArray &data )
{
    QueuedRootConstant rootConstant;
    rootConstant.Binding = binding;
    rootConstant.Data    = data.Elements;
    rootConstant.Size    = static_cast<uint32_t>( data.NumElements );
    m_queuedRootConstants.push_back( rootConstant );
}

void VulkanCommandList::PipelineBarrier( const DenOfIz_PipelineBarrierDesc *barrier )
{
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, m_commandBuffer, m_queueType, barrier );
}

void VulkanCommandList::CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyBufferRegionDesc )
{
    const auto *srcBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.SrcBuffer ) );
    const auto *dstBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferRegionDesc.DstBuffer ) );
    DZ_NOT_NULL( srcBuffer );
    DZ_NOT_NULL( dstBuffer );

    if ( copyBufferRegionDesc.NumBytes == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, CopyBufferRegion called with zero NumBytes" );
    }

    VkBufferCopy copyRegion{ };
    copyRegion.srcOffset = copyBufferRegionDesc.SrcOffset;
    copyRegion.dstOffset = copyBufferRegionDesc.DstOffset;
    copyRegion.size      = copyBufferRegionDesc.NumBytes;

    vkCmdCopyBuffer( m_commandBuffer, srcBuffer->Instance( ), dstBuffer->Instance( ), 1, &copyRegion );
}

void VulkanCommandList::CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyTextureRegionDesc )
{
    const auto *srcTex = dynamic_cast<VulkanTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.SrcTexture ) );
    const auto *dstTex = dynamic_cast<VulkanTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureRegionDesc.DstTexture ) );
    DZ_NOT_NULL( srcTex );
    DZ_NOT_NULL( dstTex );

    if ( copyTextureRegionDesc.Width == 0 || copyTextureRegionDesc.Height == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, CopyTextureRegion called with zero dimensions: Width= {} , Height={}", copyTextureRegionDesc.Width,
                      copyTextureRegionDesc.Height );
    }

    VkImageCopy copyRegion{ };
    copyRegion.srcOffset      = VkOffset3D( copyTextureRegionDesc.SrcX, copyTextureRegionDesc.SrcY, copyTextureRegionDesc.SrcZ );
    copyRegion.srcSubresource = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureRegionDesc.SrcMipLevel, copyTextureRegionDesc.SrcArrayLayer, 1 );
    copyRegion.dstOffset      = VkOffset3D( copyTextureRegionDesc.DstX, copyTextureRegionDesc.DstY, copyTextureRegionDesc.DstZ );
    copyRegion.dstSubresource = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureRegionDesc.DstMipLevel, copyTextureRegionDesc.DstArrayLayer, 1 );
    copyRegion.extent         = VkExtent3D( copyTextureRegionDesc.Width, copyTextureRegionDesc.Height, copyTextureRegionDesc.Depth );

    vkCmdCopyImage( m_commandBuffer, srcTex->Image( ), srcTex->Layout( ), dstTex->Image( ), dstTex->Layout( ), 1, &copyRegion );
}

void VulkanCommandList::CopyBufferToTexture( const DenOfIz_CopyBufferToTextureDesc &copyBufferToTextureDesc )
{
    const auto *srcBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyBufferToTextureDesc.SrcBuffer ) );
    const auto *dstTex    = dynamic_cast<VulkanTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyBufferToTextureDesc.DstTexture ) );
    DZ_NOT_NULL( srcBuffer );
    DZ_NOT_NULL( dstTex );

    const uint32_t width  = std::max( 1u, dstTex->GetWidth( ) >> copyBufferToTextureDesc.MipLevel );
    const uint32_t height = std::max( 1u, dstTex->GetHeight( ) >> copyBufferToTextureDesc.MipLevel );
    const uint32_t depth  = std::max( 1u, dstTex->GetDepth( ) >> copyBufferToTextureDesc.MipLevel );

    const uint32_t formatSize      = DenOfIz_Format_NumBytes( copyBufferToTextureDesc.Format );
    const uint32_t blockSize       = DenOfIz_Format_BlockSize( copyBufferToTextureDesc.Format );
    const uint32_t rowPitch        = std::max( 1U, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
    const uint32_t numRows         = std::max( 1U, ( height + ( blockSize - 1 ) ) / blockSize );
    const uint32_t alignedRowPitch = Utilities::Align( rowPitch, m_context->SelectedDeviceInfo.Constants.BufferTextureRowAlignment );

    VkBufferImageCopy copyRegion{ };
    copyRegion.bufferOffset       = copyBufferToTextureDesc.SrcOffset;
    copyRegion.bufferRowLength    = alignedRowPitch / formatSize * blockSize;
    copyRegion.bufferImageHeight  = numRows * blockSize;
    copyRegion.imageSubresource   = VkImageSubresourceLayers( dstTex->Aspect( ), copyBufferToTextureDesc.MipLevel, copyBufferToTextureDesc.ArrayLayer, 1 );
    copyRegion.imageOffset        = VkOffset3D( copyBufferToTextureDesc.DstX, copyBufferToTextureDesc.DstY, copyBufferToTextureDesc.DstZ );
    copyRegion.imageExtent.width  = width;
    copyRegion.imageExtent.height = height;
    copyRegion.imageExtent.depth  = depth;

    vkCmdCopyBufferToImage( m_commandBuffer, srcBuffer->Instance( ), dstTex->Image( ), dstTex->Layout( ), 1, &copyRegion );
}

void VulkanCommandList::CopyTextureToBuffer( const DenOfIz_CopyTextureToBufferDesc &copyTextureToBufferDesc )
{
    const auto *dstBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, copyTextureToBufferDesc.DstBuffer ) );
    const auto *srcTex    = dynamic_cast<VulkanTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, copyTextureToBufferDesc.SrcTexture ) );
    DZ_NOT_NULL( dstBuffer );
    DZ_NOT_NULL( srcTex );

    VkBufferImageCopy copyRegion{ };
    copyRegion.bufferOffset      = copyTextureToBufferDesc.DstOffset;
    copyRegion.bufferRowLength   = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource  = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureToBufferDesc.MipLevel, copyTextureToBufferDesc.ArrayLayer, 1 );
    copyRegion.imageOffset       = VkOffset3D( copyTextureToBufferDesc.SrcX, copyTextureToBufferDesc.SrcY, copyTextureToBufferDesc.SrcZ );
    copyRegion.imageExtent       = VkExtent3D( srcTex->GetWidth( ), srcTex->GetHeight( ), 1 );

    vkCmdCopyImageToBuffer( m_commandBuffer, srcTex->Image( ), srcTex->Layout( ), dstBuffer->Instance( ), 1, &copyRegion );
}

void VulkanCommandList::BuildTopLevelAS( const DenOfIz_BuildTopLevelASDesc &buildTopLevelASDesc )
{
    const VulkanTopLevelAS *topLevelAS    = dynamic_cast<VulkanTopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, buildTopLevelASDesc.TopLevelAS ) );
    const VulkanBuffer     *scratchBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildTopLevelASDesc.ScratchBuffer ) );
    DZ_NOT_NULL( topLevelAS );
    DZ_NOT_NULL( scratchBuffer );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ };
    buildInfo.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                     = topLevelAS->Flags( );
    buildInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.dstAccelerationStructure  = topLevelAS->Instance( );
    buildInfo.geometryCount             = 1;
    buildInfo.pGeometries               = topLevelAS->GeometryDesc( );
    buildInfo.scratchData.deviceAddress = scratchBuffer->DeviceAddress( ) + buildTopLevelASDesc.ScratchBufferOffset;

    vkCmdBuildAccelerationStructuresKHR( m_commandBuffer, 1, &buildInfo, topLevelAS->BuildRangeInfo( ) );
}

void VulkanCommandList::BuildBottomLevelAS( const DenOfIz_BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    const auto         *vkBottomLevelAS = dynamic_cast<VulkanBottomLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBottomLevelAS, buildBottomLevelASDesc.BottomLevelAS ) );
    const VulkanBuffer *scratchBuffer   = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, buildBottomLevelASDesc.ScratchBuffer ) );
    DZ_NOT_NULL( vkBottomLevelAS );
    DZ_NOT_NULL( scratchBuffer );

    const auto &geometryDescs = vkBottomLevelAS->GeometryDescs( );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ };
    buildInfo.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags                     = vkBottomLevelAS->Flags( );
    buildInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.dstAccelerationStructure  = vkBottomLevelAS->Instance( );
    buildInfo.geometryCount             = static_cast<uint32_t>( geometryDescs.size( ) );
    buildInfo.pGeometries               = geometryDescs.data( );
    buildInfo.scratchData.deviceAddress = scratchBuffer->DeviceAddress( ) + buildBottomLevelASDesc.ScratchBufferOffset;

    vkCmdBuildAccelerationStructuresKHR( m_commandBuffer, 1, &buildInfo, vkBottomLevelAS->BuildRangeInfos( ) );
}

void VulkanCommandList::UpdateTopLevelAS( const DenOfIz_UpdateTopLevelASDesc &updateDesc )
{
    auto               *vkTopLevelAS  = dynamic_cast<VulkanTopLevelAS *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, updateDesc.TopLevelAS ) );
    const VulkanBuffer *scratchBuffer = dynamic_cast<VulkanBuffer *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, updateDesc.ScratchBuffer ) );
    DZ_NOT_NULL( vkTopLevelAS );
    DZ_NOT_NULL( scratchBuffer );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { };
    buildInfo.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                                       = vkTopLevelAS->Flags( ) | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    buildInfo.mode                                        = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    buildInfo.srcAccelerationStructure                    = vkTopLevelAS->Instance( );
    buildInfo.dstAccelerationStructure                    = vkTopLevelAS->Instance( );
    buildInfo.geometryCount                               = 1;
    buildInfo.pGeometries                                 = vkTopLevelAS->GeometryDesc( );
    buildInfo.scratchData.deviceAddress                   = scratchBuffer->DeviceAddress( ) + updateDesc.ScratchBufferOffset;

    vkCmdBuildAccelerationStructuresKHR( m_commandBuffer, 1, &buildInfo, vkTopLevelAS->BuildRangeInfo( ) );

    VkMemoryBarrier barrier = { };
    barrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask   = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstAccessMask   = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    vkCmdPipelineBarrier( m_commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0,
                          nullptr );
}

void VulkanCommandList::DrawIndexed( const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance )
{
    if ( indexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, DrawIndexed called with zero count: indexCount= {} , instanceCount={}", indexCount, instanceCount );
    }
    ProcessBindGroups( );
    vkCmdDrawIndexed( m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

void VulkanCommandList::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    if ( vertexCount == 0 || instanceCount == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, Draw called with zero count: vertexCount= {} , instanceCount={}", vertexCount, instanceCount );
    }
    ProcessBindGroups( );
    vkCmdDraw( m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
}

void VulkanCommandList::DispatchRays( const DenOfIz_DispatchRaysDesc &dispatchRaysDesc )
{
    if ( dispatchRaysDesc.Width == 0 || dispatchRaysDesc.Height == 0 || dispatchRaysDesc.Depth == 0 )
    {
        spdlog::warn( "DispatchRays called with zero dimensions: width= {} , height= {} , depth={}", dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
    }

    ProcessBindGroups( );
    const VulkanShaderBindingTable *bindingTable =
        dynamic_cast<VulkanShaderBindingTable *>( DENOFIZ_FROM_HANDLE( DenOfIz::IShaderBindingTable, dispatchRaysDesc.ShaderBindingTable ) );
    DZ_NOT_NULL( bindingTable );

    vkCmdTraceRaysKHR( m_commandBuffer, bindingTable->RayGenerationShaderRange( ), bindingTable->MissShaderRange( ), bindingTable->HitGroupShaderRange( ),
                       bindingTable->CallableShaderRange( ), dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
}

void VulkanCommandList::Dispatch( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, Dispatch called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ );
    }

    ProcessBindGroups( );
    vkCmdDispatch( m_commandBuffer, groupCountX, groupCountY, groupCountZ );
}

void VulkanCommandList::DispatchMesh( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    if ( groupCountX == 0 || groupCountY == 0 || groupCountZ == 0 )
    {
        spdlog::warn( "Possible unintentional behavior, DispatchMesh called with zero group count: x= {} , y= {} , z={}", groupCountX, groupCountY, groupCountZ );
    }

    ProcessBindGroups( );
    vkCmdDrawMeshTasksEXT( m_commandBuffer, groupCountX, groupCountY, groupCountZ );
}

void VulkanCommandList::DrawIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, uint32_t stride )
{
    if ( !buffer )
    {
        spdlog::error( "VulkanCommandList::DrawIndirect: buffer is null" );
        return;
    }

    const auto *vulkanBuffer = static_cast<VulkanBuffer *>( buffer );

    ProcessBindGroups( );

    if ( stride == 0 )
    {
        stride = sizeof( VkDrawIndirectCommand );
    }

    vkCmdDrawIndirect( m_commandBuffer, vulkanBuffer->Instance( ), offset, drawCount, stride );
}

void VulkanCommandList::DrawIndexedIndirect( IBuffer *buffer, const uint64_t offset, const uint32_t drawCount, uint32_t stride )
{
    if ( !buffer )
    {
        spdlog::error( "VulkanCommandList::DrawIndexedIndirect: buffer is null" );
        return;
    }

    const auto *vulkanBuffer = static_cast<VulkanBuffer *>( buffer );
    ProcessBindGroups( );

    if ( stride == 0 )
    {
        stride = sizeof( VkDrawIndexedIndirectCommand );
    }

    vkCmdDrawIndexedIndirect( m_commandBuffer, vulkanBuffer->Instance( ), offset, drawCount, stride );
}

void VulkanCommandList::DispatchIndirect( IBuffer *buffer, const uint64_t offset )
{
    if ( !buffer )
    {
        spdlog::error( "VulkanCommandList::DispatchIndirect: buffer is null" );
        return;
    }

    const auto *vulkanBuffer = static_cast<VulkanBuffer *>( buffer );
    ProcessBindGroups( );
    vkCmdDispatchIndirect( m_commandBuffer, vulkanBuffer->Instance( ), offset );
}

const DenOfIz_QueueType VulkanCommandList::GetQueueType( )
{
    return m_queueType;
}

VkCommandBuffer &VulkanCommandList::GetCommandBuffer( )
{
    return m_commandBuffer;
}

void VulkanCommandList::ProcessBindGroups( ) const
{
    if ( m_currentPipeline == nullptr )
    {
        return;
    }

    VulkanRootSignature *rootSignature = m_currentPipeline->RootSignature( );
    if ( rootSignature == nullptr )
    {
        return;
    }

    for ( auto &vkBindGroup : m_queuedBindGroups )
    {
        if ( vkBindGroup->HasDescriptorSet( ) )
        {
            vkCmdBindDescriptorSets( m_commandBuffer, m_currentPipeline->DenOfIz_BindPoint( ), rootSignature->PipelineLayout( ), vkBindGroup->RegisterSpace( ), 1,
                                     &vkBindGroup->GetDescriptorSet( ), 0, nullptr );
        }
    }

    for ( const auto &rootConstant : m_queuedRootConstants )
    {
        VkPushConstantRange range = rootSignature->PushConstantRange( rootConstant.Binding );
        vkCmdPushConstants( m_commandBuffer, rootSignature->PipelineLayout( ), range.stageFlags, range.offset, rootConstant.Size, rootConstant.Data );
    }
}

void VulkanCommandList::BeginDebugMarker( const float r, const float g, const float b, const DenOfIz_StringView name )
{
    if ( m_context->DebugUtilsEnabled && m_context->vkCmdBeginDebugUtilsLabelEXT )
    {
        VkDebugUtilsLabelEXT label{ };
        label.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.Chars;
        label.color[ 0 ] = r;
        label.color[ 1 ] = g;
        label.color[ 2 ] = b;
        label.color[ 3 ] = 1.0f;
        m_context->vkCmdBeginDebugUtilsLabelEXT( m_commandBuffer, &label );
    }
}

void VulkanCommandList::EndDebugMarker( )
{
    if ( m_context->DebugUtilsEnabled && m_context->vkCmdEndDebugUtilsLabelEXT )
    {
        m_context->vkCmdEndDebugUtilsLabelEXT( m_commandBuffer );
    }
}

void VulkanCommandList::InsertDebugMarker( const float r, const float g, const float b, const DenOfIz_StringView name )
{
    if ( m_context->DebugUtilsEnabled && m_context->vkCmdInsertDebugUtilsLabelEXT )
    {
        VkDebugUtilsLabelEXT label{ };
        label.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.Chars;
        label.color[ 0 ] = r;
        label.color[ 1 ] = g;
        label.color[ 2 ] = b;
        label.color[ 3 ] = 1.0f;
        m_context->vkCmdInsertDebugUtilsLabelEXT( m_commandBuffer, &label );
    }
}

void VulkanCommandList::BeginQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *vkQueryPool = dynamic_cast<VulkanQueryPool *>( queryPool );
    if ( !vkQueryPool )
    {
        spdlog::error( "VulkanCommandList::BeginQuery: Invalid query pool" );
        return;
    }

    if ( vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        // Following The Forge's approach: Timestamp queries use vkCmdWriteTimestamp, not vkCmdBeginQuery
        // Write begin timestamp using even index
        const uint32_t actualIndex = queryDesc.Index * 2;
        vkCmdWriteTimestamp( m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vkQueryPool->GetVkQueryPool( ), actualIndex );
    }
    else if ( vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION || vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        // Occlusion and pipeline statistics queries use vkCmdBeginQuery/vkCmdEndQuery pattern
        // Remove precise flag to avoid validation errors unless precision is specifically needed
        VkQueryControlFlags flags = 0;
        // Only use precise bit if it's specifically needed and enabled
        // flags = VK_QUERY_CONTROL_PRECISE_BIT; // Commented out to avoid validation errors

        vkCmdBeginQuery( m_commandBuffer, vkQueryPool->GetVkQueryPool( ), queryDesc.Index, flags );
    }
}

void VulkanCommandList::EndQuery( IQueryPool *queryPool, const DenOfIz_QueryDesc &queryDesc )
{
    const auto *vkQueryPool = dynamic_cast<VulkanQueryPool *>( queryPool );
    if ( !vkQueryPool )
    {
        spdlog::error( "VulkanCommandList::EndQuery: Invalid query pool" );
        return;
    }

    if ( vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        const uint32_t actualIndex = queryDesc.Index * 2 + 1;
        vkCmdWriteTimestamp( m_commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vkQueryPool->GetVkQueryPool( ), actualIndex );
    }
    else if ( vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_OCCLUSION || vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_PIPELINE_STATISTICS )
    {
        vkCmdEndQuery( m_commandBuffer, vkQueryPool->GetVkQueryPool( ), queryDesc.Index );
    }
}

void VulkanCommandList::ResolveQuery( IQueryPool *queryPool, const uint32_t startQuery, const uint32_t queryCount )
{
    const auto *vkQueryPool = dynamic_cast<VulkanQueryPool *>( queryPool );
    if ( !vkQueryPool )
    {
        spdlog::error( "VulkanCommandList::ResolveQuery: Invalid query pool" );
    }
    // Noop
}

void VulkanCommandList::ResetQuery( IQueryPool *queryPool, const uint32_t startQuery, const uint32_t queryCount )
{
    const auto *vkQueryPool = dynamic_cast<VulkanQueryPool *>( queryPool );
    if ( !vkQueryPool )
    {
        spdlog::error( "VulkanCommandList::ResetQuery: Invalid query pool" );
        return;
    }
    uint32_t actualStartQuery = startQuery;
    uint32_t actualQueryCount = queryCount;

    if ( vkQueryPool->GetType( ) == DENOFIZ_QUERY_TYPE_TIMESTAMP )
    {
        actualStartQuery = startQuery * 2;
        actualQueryCount = queryCount * 2;
    }

    vkCmdResetQueryPool( m_commandBuffer, vkQueryPool->GetVkQueryPool( ), actualStartQuery, actualQueryCount );
}
