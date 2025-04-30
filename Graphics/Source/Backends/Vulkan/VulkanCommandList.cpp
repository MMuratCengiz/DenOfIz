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

#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanCommandList.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanFence.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipelineBarrierHelper.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h>

using namespace DenOfIz;

VulkanCommandList::VulkanCommandList( VulkanContext *context, const CommandListDesc desc, const VkCommandPool commandPool ) :
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
        depthAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView   = vkDepthStencilResource->ImageView( );
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp      = VulkanEnumConverter::ConvertLoadOp( renderingDesc.DepthAttachment.LoadOp );
        depthAttachmentInfo.storeOp     = VulkanEnumConverter::ConvertStoreOp( renderingDesc.DepthAttachment.StoreOp );
        depthAttachmentInfo.clearValue.depthStencil =
            VkClearDepthStencilValue( renderingDesc.DepthAttachment.ClearDepthStencil[ 0 ], renderingDesc.DepthAttachment.ClearDepthStencil[ 1 ] );

        renderInfo.pDepthAttachment = &depthAttachmentInfo;
    }

    if ( renderingDesc.StencilAttachment.Resource != nullptr )
    {
        const auto *vkDepthStencilResource = dynamic_cast<VulkanTextureResource *>( renderingDesc.StencilAttachment.Resource );

        VkRenderingAttachmentInfo stencilAttachmentInfo{ };
        stencilAttachmentInfo.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        stencilAttachmentInfo.imageView   = vkDepthStencilResource->ImageView( );
        stencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        stencilAttachmentInfo.loadOp      = VulkanEnumConverter::ConvertLoadOp( renderingDesc.StencilAttachment.LoadOp );
        stencilAttachmentInfo.storeOp     = VulkanEnumConverter::ConvertStoreOp( renderingDesc.StencilAttachment.StoreOp );
        stencilAttachmentInfo.clearValue.depthStencil =
            VkClearDepthStencilValue( renderingDesc.StencilAttachment.ClearDepthStencil[ 0 ], renderingDesc.DepthAttachment.ClearDepthStencil[ 1 ] );

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
    m_currentPipeline = dynamic_cast<VulkanPipeline *>( pipeline );
    vkCmdBindPipeline( m_commandBuffer, m_currentPipeline->BindPoint( ), m_currentPipeline->Instance( ) );
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

void VulkanCommandList::BindViewport( const float offsetX, const float offsetY, const float width, const float height )
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
    m_queuedBindGroups.push_back( vkBindGroup );
}

void VulkanCommandList::PipelineBarrier( const PipelineBarrierDesc &barrier )
{
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, m_commandBuffer, m_queueType, barrier );
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
    copyRegion.imageOffset        = VkOffset3D( copyBufferToTextureDesc.DstX, copyBufferToTextureDesc.DstY, copyBufferToTextureDesc.DstZ );
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

void VulkanCommandList::BuildTopLevelAS( const BuildTopLevelASDesc &buildTopLevelASDesc )
{
    const VulkanTopLevelAS *topLevelAS = dynamic_cast<VulkanTopLevelAS *>( buildTopLevelASDesc.TopLevelAS );
    DZ_NOT_NULL( topLevelAS );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ };
    buildInfo.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                     = topLevelAS->Flags( );
    buildInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.dstAccelerationStructure  = topLevelAS->Instance( );
    buildInfo.geometryCount             = 1;
    buildInfo.pGeometries               = topLevelAS->GeometryDesc( );
    buildInfo.scratchData.deviceAddress = topLevelAS->Scratch( )->DeviceAddress( );

    vkCmdBuildAccelerationStructuresKHR( m_commandBuffer, 1, &buildInfo, topLevelAS->BuildRangeInfo( ) );
}

void VulkanCommandList::BuildBottomLevelAS( const BuildBottomLevelASDesc &buildBottomLevelASDesc )
{
    const auto *vkBottomLevelAS = dynamic_cast<VulkanBottomLevelAS *>( buildBottomLevelASDesc.BottomLevelAS );
    DZ_NOT_NULL( vkBottomLevelAS );

    const auto &geometryDescs = vkBottomLevelAS->GeometryDescs( );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ };
    buildInfo.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags                     = vkBottomLevelAS->Flags( );
    buildInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.dstAccelerationStructure  = vkBottomLevelAS->Instance( );
    buildInfo.geometryCount             = static_cast<uint32_t>( geometryDescs.size( ) );
    buildInfo.pGeometries               = geometryDescs.data( );
    buildInfo.scratchData.deviceAddress = vkBottomLevelAS->ScratchBuffer( )->DeviceAddress( );

    vkCmdBuildAccelerationStructuresKHR( m_commandBuffer, 1, &buildInfo, vkBottomLevelAS->BuildRangeInfos( ) );
}

void VulkanCommandList::UpdateTopLevelAS( const UpdateTopLevelASDesc &updateDesc )
{
    auto *vkTopLevelAS = dynamic_cast<VulkanTopLevelAS *>( updateDesc.TopLevelAS );
    DZ_NOT_NULL( vkTopLevelAS );

    UpdateTransformsDesc updateTransformDesc{ };
    updateTransformDesc.Transforms = updateDesc.Transforms;

    vkTopLevelAS->UpdateInstanceTransforms( updateTransformDesc );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { };
    buildInfo.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                                       = vkTopLevelAS->Flags( ) | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    buildInfo.mode                                        = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    buildInfo.srcAccelerationStructure                    = vkTopLevelAS->Instance( );
    buildInfo.dstAccelerationStructure                    = vkTopLevelAS->Instance( );
    buildInfo.geometryCount                               = 1;
    buildInfo.pGeometries                                 = vkTopLevelAS->GeometryDesc( );
    buildInfo.scratchData.deviceAddress                   = vkTopLevelAS->Scratch( )->DeviceAddress( );

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
    ProcessBindGroups( );
    vkCmdDrawIndexed( m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
}

void VulkanCommandList::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance )
{
    ProcessBindGroups( );
    vkCmdDraw( m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
}

void VulkanCommandList::DispatchRays( const DispatchRaysDesc &dispatchRaysDesc )
{
    ProcessBindGroups( );
    const VulkanShaderBindingTable *bindingTable = dynamic_cast<VulkanShaderBindingTable *>( dispatchRaysDesc.ShaderBindingTable );
    DZ_NOT_NULL( bindingTable );

    vkCmdTraceRaysKHR( m_commandBuffer, bindingTable->RayGenerationShaderRange( ), bindingTable->MissShaderRange( ), bindingTable->HitGroupShaderRange( ),
                       bindingTable->CallableShaderRange( ), dispatchRaysDesc.Width, dispatchRaysDesc.Height, dispatchRaysDesc.Depth );
}

void VulkanCommandList::Dispatch( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    ProcessBindGroups( );
    vkCmdDispatch( m_commandBuffer, groupCountX, groupCountY, groupCountZ );
}

void VulkanCommandList::DispatchMesh( const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ )
{
    ProcessBindGroups( );
    vkCmdDrawMeshTasksEXT( m_commandBuffer, groupCountX, groupCountY, groupCountZ );
}

const QueueType VulkanCommandList::GetQueueType( )
{
    return m_queueType;
}

VkCommandBuffer &VulkanCommandList::GetCommandBuffer( )
{
    return m_commandBuffer;
}

void VulkanCommandList::ProcessBindGroups( ) const
{
    for ( auto &vkBindGroup : m_queuedBindGroups )
    {
        if ( vkBindGroup->HasDescriptorSet( ) )
        {
            vkCmdBindDescriptorSets( m_commandBuffer, m_currentPipeline->BindPoint( ), vkBindGroup->RootSignature( )->PipelineLayout( ), vkBindGroup->RegisterSpace( ), 1,
                                     &vkBindGroup->GetDescriptorSet( ), 0, nullptr );
        }

        for ( const auto &rootConstant : vkBindGroup->RootConstants( ) )
        {
            vkCmdPushConstants( m_commandBuffer, rootConstant.PipelineLayout, rootConstant.ShaderStage, rootConstant.Offset, rootConstant.Size, rootConstant.Data );
        }
    }
}
