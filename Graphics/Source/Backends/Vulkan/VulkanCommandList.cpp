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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanCommandList.h>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanResourceBindGroup.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h"

using namespace DenOfIz;

VulkanCommandList::VulkanCommandList( VulkanContext *context, const CommandListDesc desc ) : m_desc( desc ), m_context( context )
{
    auto commandPool = m_context->GraphicsQueueCommandPool;
    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        commandPool = m_context->GraphicsQueueCommandPool;
        break;
    case QueueType::Compute:
        commandPool = m_context->ComputeQueueCommandPool;
        break;
    case QueueType::Copy:
        commandPool = m_context->TransferQueueCommandPool;
        break;
    }

    VkCommandBufferAllocateInfo allocInfo{ };
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Todo
    allocInfo.commandBufferCount = 1;

    VK_CHECK_RESULT( vkAllocateCommandBuffers( m_context->LogicalDevice, &allocInfo, &m_commandBuffer ) );
}

void VulkanCommandList::Begin( )
{
    VK_CHECK_RESULT( vkResetCommandBuffer( m_commandBuffer, 0 ) );

    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = { };

    VK_CHECK_RESULT( vkBeginCommandBuffer( m_commandBuffer, &beginInfo ) );
}

// Todo !IMPROVEMENT! this function may not need to exist.
void VulkanCommandList::BeginRendering( const RenderingDesc &renderingDesc )
{
    VkRenderingInfo renderInfo{ };
    renderInfo.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea.extent = VkExtent2D( renderingDesc.RenderAreaWidth, renderingDesc.RenderAreaHeight );
    renderInfo.renderArea.offset = VkOffset2D( renderingDesc.RenderAreaOffsetX, renderingDesc.RenderAreaOffsetY );
    renderInfo.layerCount        = renderingDesc.LayerCount;
    renderInfo.viewMask          = 0;

    std::vector<VkRenderingAttachmentInfo> colorAttachments;

    for ( int i = 0; i < renderingDesc.RTAttachments.NumElements( ); ++i )
    {
        const auto &colorAttachment           = renderingDesc.RTAttachments.GetElement( i );
        auto       *vkColorAttachmentResource = dynamic_cast<VulkanTextureResource *>( colorAttachment.Resource );

        VkRenderingAttachmentInfo colorAttachmentInfo{ };
        colorAttachmentInfo.sType                         = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo.imageView                     = vkColorAttachmentResource->ImageView( );
        colorAttachmentInfo.imageLayout                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo.loadOp                        = VulkanEnumConverter::ConvertLoadOp( colorAttachment.LoadOp );
        colorAttachmentInfo.storeOp                       = VulkanEnumConverter::ConvertStoreOp( colorAttachment.StoreOp );
        colorAttachmentInfo.clearValue.color.float32[ 0 ] = colorAttachment.ClearColor[ 0 ];
        colorAttachmentInfo.clearValue.color.float32[ 1 ] = colorAttachment.ClearColor[ 1 ];
        colorAttachmentInfo.clearValue.color.float32[ 2 ] = colorAttachment.ClearColor[ 2 ];
        colorAttachmentInfo.clearValue.color.float32[ 3 ] = colorAttachment.ClearColor[ 3 ];

        if ( renderInfo.renderArea.extent.height == 0 )
        {
            renderInfo.renderArea.extent.width  = vkColorAttachmentResource->GetWidth( );
            renderInfo.renderArea.extent.height = vkColorAttachmentResource->GetHeight( );
        }

        colorAttachments.push_back( colorAttachmentInfo );
    }

    renderInfo.colorAttachmentCount = colorAttachments.size( );
    renderInfo.pColorAttachments    = colorAttachments.data( );

    // Todo these need to be fixed.
    if ( renderingDesc.DepthAttachment.Resource != nullptr )
    {
        const auto *vkDepthStencilResource = dynamic_cast<VulkanTextureResource *>( renderingDesc.DepthAttachment.Resource );

        VkRenderingAttachmentInfo depthAttachmentInfo{ };
        depthAttachmentInfo.sType                   = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView               = vkDepthStencilResource->ImageView( );
        depthAttachmentInfo.imageLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp                  = VulkanEnumConverter::ConvertLoadOp( renderingDesc.DepthAttachment.LoadOp );
        depthAttachmentInfo.storeOp                 = VulkanEnumConverter::ConvertStoreOp( renderingDesc.DepthAttachment.StoreOp );
        depthAttachmentInfo.clearValue.depthStencil = VkClearDepthStencilValue( renderingDesc.DepthAttachment.ClearDepthStencil[ 0 ], renderingDesc.DepthAttachment.ClearDepthStencil[ 1 ] );

        renderInfo.pDepthAttachment = &depthAttachmentInfo;
    }

    if ( renderingDesc.StencilAttachment.Resource != nullptr )
    {
        const auto *vkDepthStencilResource = dynamic_cast<VulkanTextureResource *>( renderingDesc.StencilAttachment.Resource );

        VkRenderingAttachmentInfo stencilAttachmentInfo{ };
        stencilAttachmentInfo.sType                   = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        stencilAttachmentInfo.imageView               = vkDepthStencilResource->ImageView( );
        stencilAttachmentInfo.imageLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        stencilAttachmentInfo.loadOp                  = VulkanEnumConverter::ConvertLoadOp( renderingDesc.StencilAttachment.LoadOp );
        stencilAttachmentInfo.storeOp                 = VulkanEnumConverter::ConvertStoreOp( renderingDesc.StencilAttachment.StoreOp );
        stencilAttachmentInfo.clearValue.depthStencil = VkClearDepthStencilValue( renderingDesc.StencilAttachment.ClearDepthStencil[ 0 ], renderingDesc.DepthAttachment.ClearDepthStencil[ 1 ] );

        renderInfo.pStencilAttachment = &stencilAttachmentInfo;
    }

    vkCmdBeginRendering( m_commandBuffer, &renderInfo );
}

void VulkanCommandList::EndRendering( )
{
    vkCmdEndRendering( m_commandBuffer );
}

void VulkanCommandList::Execute( const ExecuteDesc &executeDesc )
{
    VK_CHECK_RESULT( vkEndCommandBuffer( m_commandBuffer ) );

    VkSubmitInfo vkSubmitInfo{ };
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore>          waitOnSemaphores;
    for ( int i = 0; i < executeDesc.WaitOnSemaphores.NumElements( ); i++ )
    {
        auto *waitOn = dynamic_cast<VulkanSemaphore *>( executeDesc.WaitOnSemaphores.GetElement( i ) );
        waitOnSemaphores.push_back( waitOn->GetSemaphore( ) );
        waitStages.push_back( VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
    }

    std::vector<VkSemaphore> signalSemaphores;
    for ( int i = 0; i < executeDesc.NotifySemaphores.NumElements( ); i++ )
    {
        auto *signal = dynamic_cast<VulkanSemaphore *>( executeDesc.NotifySemaphores.GetElement( i ) );
        signalSemaphores.push_back( signal->GetSemaphore( ) );
    }

    vkSubmitInfo.waitSemaphoreCount   = waitOnSemaphores.size( );
    vkSubmitInfo.pWaitSemaphores      = waitOnSemaphores.data( );
    vkSubmitInfo.pWaitDstStageMask    = waitStages.data( );
    vkSubmitInfo.pWaitDstStageMask    = waitStages.data( );
    vkSubmitInfo.commandBufferCount   = 1;
    vkSubmitInfo.pCommandBuffers      = &m_commandBuffer;
    vkSubmitInfo.signalSemaphoreCount = signalSemaphores.size( );
    vkSubmitInfo.pSignalSemaphores    = signalSemaphores.data( );

    VkFence vkNotifyFence = nullptr;
    if ( executeDesc.Notify != nullptr )
    {
        auto *notify = reinterpret_cast<VulkanFence *>( executeDesc.Notify );
        notify->Reset( );
        vkNotifyFence = notify->GetFence( );
    }

    VulkanQueueType queueType = VulkanQueueType::Graphics;
    switch ( m_desc.QueueType )
    {
    case QueueType::Graphics:
        queueType = VulkanQueueType::Graphics;
        break;
    case QueueType::Compute:
        queueType = VulkanQueueType::Compute;
        break;
    case QueueType::Copy:
        queueType = VulkanQueueType::Copy;
        break;
    }

    VK_CHECK_RESULT( vkQueueSubmit( m_context->Queues[ queueType ], 1, &vkSubmitInfo, vkNotifyFence ) );
}

void VulkanCommandList::BindPipeline( IPipeline *pipeline )
{
    const auto vkPipeline = dynamic_cast<VulkanPipeline *>( pipeline );
    vkCmdBindPipeline( m_commandBuffer, vkPipeline->BindPoint( ), vkPipeline->Instance( ) );
}

void VulkanCommandList::BindVertexBuffer( IBufferResource *buffer )
{
    const auto             bufferResource = dynamic_cast<VulkanBufferResource *>( buffer );
    constexpr VkDeviceSize offset         = 0;
    vkCmdBindVertexBuffers( m_commandBuffer, 0, 1, &bufferResource->Instance( ), &offset );
}

void VulkanCommandList::BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType )
{
    const auto             bufferResource = dynamic_cast<VulkanBufferResource *>( buffer );
    constexpr VkDeviceSize offset         = 0;

    switch ( indexType )
    {
    case IndexType::Uint16:
        vkCmdBindIndexBuffer( m_commandBuffer, bufferResource->Instance( ), offset, VK_INDEX_TYPE_UINT16 );
        break;
    case IndexType::Uint32:
        vkCmdBindIndexBuffer( m_commandBuffer, bufferResource->Instance( ), offset, VK_INDEX_TYPE_UINT32 );
        break;
    }
}

void VulkanCommandList::BindViewport( const float offsetX, float offsetY, const float width, const float height )
{
    DZ_RETURN_IF( width == 0 || height == 0 );
    m_viewport.x = offsetX;
    // Vulkan has inverted y-axis
    m_viewport.y      = offsetY + height;
    m_viewport.height = -height;
    // --
    m_viewport.width    = width;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    vkCmdSetViewportWithCount( m_commandBuffer, 1, &m_viewport );
}

void VulkanCommandList::BindScissorRect( const float offsetX, const float offsetY, const float width, const float height )
{
    m_scissorRect               = VkRect2D( );
    m_scissorRect.offset.x      = offsetX;
    m_scissorRect.offset.y      = offsetY;
    m_scissorRect.extent.width  = width;
    m_scissorRect.extent.height = height;
    vkCmdSetScissorWithCount( m_commandBuffer, 1, &m_scissorRect );
}

void VulkanCommandList::BindResourceGroup( IResourceBindGroup *bindGroup )
{
    const auto *vkBindGroup = dynamic_cast<VulkanResourceBindGroup *>( bindGroup );

    // Remember more bind points will be added in the future.
    const VkPipelineBindPoint bindPoint = m_desc.QueueType == QueueType::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

    if ( vkBindGroup->HasDescriptorSet( ) )
    {
        vkCmdBindDescriptorSets( m_commandBuffer, bindPoint, vkBindGroup->RootSignature( )->PipelineLayout( ), vkBindGroup->RegisterSpace( ), 1, &vkBindGroup->GetDescriptorSet( ),
                                 0, nullptr );
    }

    for ( const auto &rootConstant : vkBindGroup->RootConstants( ) )
    {
        vkCmdPushConstants( m_commandBuffer, rootConstant.PipelineLayout, rootConstant.ShaderStage, rootConstant.Offset, rootConstant.Size, rootConstant.Data );
    }
}

void VulkanCommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, m_commandBuffer, m_desc.QueueType, barrier );
}

void VulkanCommandList::DrawIndexed( const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const uint32_t vertexOffset, const uint32_t firstInstance )
{
    vkCmdDrawIndexed( m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

void VulkanCommandList::CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionDesc )
{
    const auto *srcBuffer = dynamic_cast<VulkanBufferResource *>( copyBufferRegionDesc.SrcBuffer );
    const auto *dstBuffer = dynamic_cast<VulkanBufferResource *>( copyBufferRegionDesc.DstBuffer );

    VkBufferCopy copyRegion{ };
    copyRegion.srcOffset = copyBufferRegionDesc.SrcOffset;
    copyRegion.dstOffset = copyBufferRegionDesc.DstOffset;
    copyRegion.size      = copyBufferRegionDesc.NumBytes;

    vkCmdCopyBuffer( m_commandBuffer, srcBuffer->Instance( ), dstBuffer->Instance( ), 1, &copyRegion );
}

void VulkanCommandList::CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionDesc )
{
    const auto *srcTex = dynamic_cast<VulkanTextureResource *>( copyTextureRegionDesc.SrcTexture );
    const auto *dstTex = dynamic_cast<VulkanTextureResource *>( copyTextureRegionDesc.DstTexture );

    VkImageCopy copyRegion{ };
    copyRegion.srcOffset      = VkOffset3D( copyTextureRegionDesc.SrcX, copyTextureRegionDesc.SrcY, copyTextureRegionDesc.SrcZ );
    copyRegion.srcSubresource = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureRegionDesc.SrcMipLevel, copyTextureRegionDesc.SrcArrayLayer, 1 );
    copyRegion.dstOffset      = VkOffset3D( copyTextureRegionDesc.DstX, copyTextureRegionDesc.DstY, copyTextureRegionDesc.DstZ );
    copyRegion.dstSubresource = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureRegionDesc.DstMipLevel, copyTextureRegionDesc.DstArrayLayer, 1 );
    copyRegion.extent         = VkExtent3D( copyTextureRegionDesc.Width, copyTextureRegionDesc.Height, copyTextureRegionDesc.Depth );

    vkCmdCopyImage( m_commandBuffer, srcTex->Image( ), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTex->Image( ), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion );
}

void VulkanCommandList::CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTextureDesc )
{
    const auto *srcBuffer = dynamic_cast<VulkanBufferResource *>( copyBufferToTextureDesc.SrcBuffer );
    const auto *dstTex    = dynamic_cast<VulkanTextureResource *>( copyBufferToTextureDesc.DstTexture );

    const uint32_t width  = std::max( 1u, dstTex->GetWidth( ) >> copyBufferToTextureDesc.MipLevel );
    const uint32_t height = std::max( 1u, dstTex->GetHeight( ) >> copyBufferToTextureDesc.MipLevel );
    const uint32_t depth  = std::max( 1u, dstTex->GetDepth( ) >> copyBufferToTextureDesc.MipLevel );

    const uint32_t formatSize      = FormatNumBytes( copyBufferToTextureDesc.Format );
    const uint32_t blockSize       = FormatBlockSize( copyBufferToTextureDesc.Format );
    const uint32_t rowPitch        = std::max( 1U, ( width + ( blockSize - 1 ) ) / blockSize ) * formatSize;
    const uint32_t numRows         = std::max( 1U, ( height + ( blockSize - 1 ) ) / blockSize );
    const uint32_t alignedRowPitch = Utilities::Align( rowPitch, m_context->SelectedDeviceInfo.Constants.BufferTextureRowAlignment );

    VkBufferImageCopy copyRegion{ };
    copyRegion.bufferOffset       = copyBufferToTextureDesc.SrcOffset;
    copyRegion.bufferRowLength    = alignedRowPitch / formatSize * blockSize;
    copyRegion.bufferImageHeight  = numRows * blockSize;
    copyRegion.imageSubresource   = VkImageSubresourceLayers( dstTex->Aspect( ), copyBufferToTextureDesc.MipLevel, copyBufferToTextureDesc.ArrayLayer, 1 );
    copyRegion.imageOffset        = VkOffset3D( 0, 0, 0 );
    copyRegion.imageExtent.width  = width;
    copyRegion.imageExtent.height = height;
    copyRegion.imageExtent.depth  = depth;

    vkCmdCopyBufferToImage( m_commandBuffer, srcBuffer->Instance( ), dstTex->Image( ), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion );
}

void VulkanCommandList::CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBufferDesc )
{
    const auto *dstBuffer = dynamic_cast<VulkanBufferResource *>( copyTextureToBufferDesc.DstBuffer );
    const auto *srcTex    = dynamic_cast<VulkanTextureResource *>( copyTextureToBufferDesc.SrcTexture );

    VkBufferImageCopy copyRegion{ };
    copyRegion.bufferOffset      = copyTextureToBufferDesc.DstOffset;
    copyRegion.bufferRowLength   = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource  = VkImageSubresourceLayers( VK_IMAGE_ASPECT_COLOR_BIT, copyTextureToBufferDesc.MipLevel, copyTextureToBufferDesc.ArrayLayer, 1 );
    copyRegion.imageOffset       = VkOffset3D( copyTextureToBufferDesc.SrcX, copyTextureToBufferDesc.SrcY, copyTextureToBufferDesc.SrcZ );
    copyRegion.imageExtent       = VkExtent3D( srcTex->GetWidth( ), srcTex->GetHeight( ), 1 );

    vkCmdCopyImageToBuffer( m_commandBuffer, srcTex->Image( ), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->Instance( ), 1, &copyRegion );
}

void VulkanCommandList::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    vkCmdDraw( m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
}

void VulkanCommandList::Dispatch( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    DZ_ASSERTM( m_desc.QueueType == QueueType::Compute, "Dispatch can only be called on compute queues." );
    vkCmdDispatch( m_commandBuffer, groupCountX, groupCountY, groupCountZ );
}

void VulkanCommandList::Present( ISwapChain *swapChain, const uint32_t imageIndex, const InteropArray<ISemaphore *> & waitOnLocks )
{
    DZ_ASSERTM( m_desc.QueueType == QueueType::Graphics, "Present can only be called on graphics queue." );

    VkPresentInfoKHR presentInfo{ };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    std::vector<VkSemaphore> vkWaitOnSemaphores( waitOnLocks.NumElements( ) );
    for ( int i = 0; i < waitOnLocks.NumElements( ); i++ )
    {
        vkWaitOnSemaphores[ i ] = reinterpret_cast<VulkanSemaphore *>( waitOnLocks.GetElement( i ) )->GetSemaphore( );
    }

    presentInfo.waitSemaphoreCount = vkWaitOnSemaphores.size( );
    presentInfo.pWaitSemaphores    = vkWaitOnSemaphores.data( );
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = reinterpret_cast<VulkanSwapChain *>( swapChain )->GetSwapChain( );
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    const VkResult presentResult = vkQueuePresentKHR( m_context->Queues[ VulkanQueueType::Presentation ], &presentInfo );

    if ( presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR )
    {
        // TODO
    }

    VK_CHECK_RESULT( presentResult );
}
