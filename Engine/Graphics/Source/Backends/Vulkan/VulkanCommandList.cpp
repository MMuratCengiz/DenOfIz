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

VulkanCommandList::VulkanCommandList(VulkanContext *context, CommandListDesc desc) : m_context(context), m_desc(std::move(desc))
{
    auto commandPool = m_context->GraphicsQueueCommandPool;
    switch ( m_desc.QueueType )
    {
    case QueueType::Presentation:
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

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = commandPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary; // Todo
    allocInfo.commandBufferCount = 1;

    m_commandBuffer = m_context->LogicalDevice.allocateCommandBuffers(allocInfo)[ 0 ];
}

void VulkanCommandList::Begin()
{
    m_commandBuffer.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = {};
    m_commandBuffer.begin(beginInfo);
}

void VulkanCommandList::BeginRendering(const RenderingDesc &renderingInfo)
{
    vk::RenderingInfo renderInfo{};

    GraphicsWindowSurface surface = m_context->Window->GetSurface();

    float widthAdapted  = renderingInfo.RenderAreaWidth == 0 ? static_cast<float>(surface.Width) : static_cast<float>(renderingInfo.RenderAreaWidth);
    float heightAdapted = renderingInfo.RenderAreaHeight == 0 ? static_cast<float>(surface.Height) : static_cast<float>(renderingInfo.RenderAreaHeight);

    renderInfo.renderArea.extent = vk::Extent2D(widthAdapted, heightAdapted);
    renderInfo.renderArea.offset = vk::Offset2D(renderingInfo.RenderAreaOffsetX, renderingInfo.RenderAreaOffsetY);
    renderInfo.layerCount        = renderingInfo.LayerCount;
    renderInfo.viewMask          = 0;

    std::vector<vk::RenderingAttachmentInfo> colorAttachments;

    for ( const auto &colorAttachment : renderingInfo.RTAttachments )
    {
        VulkanTextureResource *vkColorAttachmentResource = dynamic_cast<VulkanTextureResource *>(colorAttachment.Resource);

        vk::RenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.imageView        = vkColorAttachmentResource->GetImageView();
        colorAttachmentInfo.imageLayout      = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachmentInfo.loadOp           = VulkanEnumConverter::ConvertLoadOp(colorAttachment.LoadOp);
        colorAttachmentInfo.storeOp          = VulkanEnumConverter::ConvertStoreOp(colorAttachment.StoreOp);
        colorAttachmentInfo.clearValue.color = colorAttachment.ClearColor;

        colorAttachments.push_back(colorAttachmentInfo);
    }

    renderInfo.setColorAttachments(colorAttachments);

    // Todo these need to be fixed.
    if ( renderingInfo.DepthAttachment.Resource != nullptr )
    {
        VulkanTextureResource *vkDepthStencilResource = dynamic_cast<VulkanTextureResource *>(renderingInfo.DepthAttachment.Resource);

        vk::RenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.imageView               = vkDepthStencilResource->GetImageView();
        depthAttachmentInfo.imageLayout             = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthAttachmentInfo.loadOp                  = VulkanEnumConverter::ConvertLoadOp(renderingInfo.DepthAttachment.LoadOp);
        depthAttachmentInfo.storeOp                 = VulkanEnumConverter::ConvertStoreOp(renderingInfo.DepthAttachment.StoreOp);
        depthAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue(renderingInfo.DepthAttachment.ClearDepth[ 0 ], renderingInfo.DepthAttachment.ClearDepth[ 1 ]);

        renderInfo.setPDepthAttachment(&depthAttachmentInfo);
    }

    if ( renderingInfo.StencilAttachment.Resource != nullptr )
    {
        VulkanTextureResource *vkDepthStencilResource = dynamic_cast<VulkanTextureResource *>(renderingInfo.StencilAttachment.Resource);

        vk::RenderingAttachmentInfo stencilAttachmentInfo{};
        stencilAttachmentInfo.imageView               = vkDepthStencilResource->GetImageView();
        stencilAttachmentInfo.imageLayout             = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        stencilAttachmentInfo.loadOp                  = VulkanEnumConverter::ConvertLoadOp(renderingInfo.StencilAttachment.LoadOp);
        stencilAttachmentInfo.storeOp                 = VulkanEnumConverter::ConvertStoreOp(renderingInfo.StencilAttachment.StoreOp);
        stencilAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue(renderingInfo.StencilAttachment.ClearDepth[ 0 ], renderingInfo.DepthAttachment.ClearDepth[ 1 ]);

        renderInfo.setPStencilAttachment(&stencilAttachmentInfo);
    }

    m_commandBuffer.beginRendering(renderInfo);
}

void VulkanCommandList::EndRendering()
{
    m_commandBuffer.endRendering();
}

void VulkanCommandList::Execute(const ExecuteDesc &executeInfo)
{
    m_commandBuffer.end();

    vk::SubmitInfo vkSubmitInfo{};

    std::vector<vk::PipelineStageFlags> waitStages;
    std::vector<vk::Semaphore>          waitOnSemaphores;
    for ( ISemaphore *waitOn : executeInfo.WaitOnSemaphores )
    {
        waitOnSemaphores.push_back(reinterpret_cast<VulkanSemaphore *>(waitOn)->GetSemaphore());
        waitStages.push_back(vk::PipelineStageFlagBits::eAllCommands);
    }
    std::vector<vk::Semaphore> signalSemaphores;
    for ( ISemaphore *signal : executeInfo.NotifySemaphores )
    {
        signalSemaphores.push_back(reinterpret_cast<VulkanSemaphore *>(signal)->GetSemaphore());
    }

    vkSubmitInfo.waitSemaphoreCount   = waitOnSemaphores.size();
    vkSubmitInfo.pWaitSemaphores      = waitOnSemaphores.data();
    vkSubmitInfo.pWaitDstStageMask    = waitStages.data();
    vkSubmitInfo.pWaitDstStageMask    = waitStages.data();
    vkSubmitInfo.commandBufferCount   = 1;
    vkSubmitInfo.pCommandBuffers      = &m_commandBuffer;
    vkSubmitInfo.signalSemaphoreCount = signalSemaphores.size();
    vkSubmitInfo.pSignalSemaphores    = signalSemaphores.data();

    vk::Fence vkNotifyFence = VK_NULL_HANDLE;
    if ( executeInfo.Notify != nullptr )
    {
        VulkanFence *notify = reinterpret_cast<VulkanFence *>(executeInfo.Notify);
        notify->Reset();
        vkNotifyFence = notify->GetFence();
    }
    const auto submitResult = m_context->Queues[ m_desc.QueueType ].submit(1, &vkSubmitInfo, vkNotifyFence);

    VK_CHECK_RESULT(submitResult);
}

void VulkanCommandList::BindPipeline(IPipeline *pipeline)
{
    m_boundPipeline = dynamic_cast<VulkanPipeline *>(pipeline);
    m_commandBuffer.bindPipeline(m_boundPipeline->BindPoint, m_boundPipeline->Instance);
}

void VulkanCommandList::BindVertexBuffer(IBufferResource *buffer)
{
    const auto               bufferResource = static_cast<VulkanBufferResource *>(buffer);
    constexpr vk::DeviceSize offset         = 0;
    m_commandBuffer.bindVertexBuffers(0, 1, &bufferResource->GetBuffer(), &offset);
}

void VulkanCommandList::BindIndexBuffer(IBufferResource *buffer, const IndexType &indexType)
{
    const auto               bufferResource = static_cast<VulkanBufferResource *>(buffer);
    constexpr vk::DeviceSize offset         = 0;

    switch ( indexType )
    {
    case IndexType::Uint16:
        m_commandBuffer.bindIndexBuffer(bufferResource->GetBuffer(), offset, vk::IndexType::eUint16);
        break;
    case IndexType::Uint32:
        m_commandBuffer.bindIndexBuffer(bufferResource->GetBuffer(), offset, vk::IndexType::eUint32);
        break;
    }
}

void VulkanCommandList::BindViewport(float offsetX, float offsetY, float width, float height)
{
    DZ_RETURN_IF(width == 0 || height == 0);
    m_viewport.x = offsetX;
    // Vulkan has inverted y-axis
    m_viewport.y      = height;
    m_viewport.height = -static_cast<float>(height);
    // --
    m_viewport.width    = width;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_commandBuffer.setViewportWithCount(1, &m_viewport);
}

void VulkanCommandList::BindScissorRect(float offsetX, float offsetY, float width, float height)
{
    m_scissorRect.offset = vk::Offset2D(offsetX, offsetY);
    m_scissorRect.extent = vk::Extent2D(width, height);
    m_commandBuffer.setScissorWithCount(1, &m_scissorRect);
}

void VulkanCommandList::BindResourceGroup(IResourceBindGroup *bindGroup)
{
    DZ_ASSERTM(m_boundPipeline != VK_NULL_HANDLE, "Pipeline must be bound before binding descriptor table.");

    VulkanResourceBindGroup *vkTable = dynamic_cast<VulkanResourceBindGroup *>(bindGroup);

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets = vkTable->GetWriteDescriptorSets();
    vk::PipelineBindPoint               bindPoint           = m_desc.QueueType == QueueType::Graphics ? vk::PipelineBindPoint::eGraphics : vk::PipelineBindPoint::eCompute;
    m_commandBuffer.pushDescriptorSetKHR(bindPoint, m_boundPipeline->Layout, 0, writeDescriptorSets.size(), writeDescriptorSets.data());
}

void VulkanCommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor)
{
    m_commandBuffer.setDepthBias(constantFactor, clamp, slopeFactor);
}

void VulkanCommandList::SetPipelineBarrier(const PipelineBarrier &barrier)
{
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier(m_context, m_commandBuffer, m_desc.QueueType, barrier);
}

void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    m_commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandList::CopyBufferRegion(const CopyBufferRegionDesc &copyBufferRegionDesc)
{
    VulkanBufferResource *srcBuffer = dynamic_cast<VulkanBufferResource *>(copyBufferRegionDesc.SrcBuffer);
    VulkanBufferResource *dstBuffer = dynamic_cast<VulkanBufferResource *>(copyBufferRegionDesc.DstBuffer);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = copyBufferRegionDesc.SrcOffset;
    copyRegion.dstOffset = copyBufferRegionDesc.DstOffset;
    copyRegion.size      = copyBufferRegionDesc.NumBytes;

    m_commandBuffer.copyBuffer(srcBuffer->GetBuffer(), dstBuffer->GetBuffer(), 1, &copyRegion);
}

void VulkanCommandList::CopyTextureRegion(const CopyTextureRegionDesc &copyTextureRegionDesc)
{
    VulkanTextureResource *srcTex = dynamic_cast<VulkanTextureResource *>(copyTextureRegionDesc.SrcTexture);
    VulkanTextureResource *dstTex = dynamic_cast<VulkanTextureResource *>(copyTextureRegionDesc.DstTexture);

    vk::ImageCopy copyRegion{};
    // TODO
}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    m_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    DZ_ASSERTM(m_desc.QueueType == QueueType::Compute, "Dispatch can only be called on compute queues.");
}

void VulkanCommandList::Present(ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks)
{
    DZ_ASSERTM(m_desc.QueueType == QueueType::Graphics || m_desc.QueueType == QueueType::Presentation, "Present can only be called on presentation queues.");

    vk::PresentInfoKHR presentInfo{};

    std::vector<vk::Semaphore> vkWaitOnSemaphores(waitOnLocks.size());
    for ( int i = 0; i < waitOnLocks.size(); i++ )
    {
        vkWaitOnSemaphores[ i ] = reinterpret_cast<VulkanSemaphore *>(waitOnLocks[ i ])->GetSemaphore();
    }

    presentInfo.waitSemaphoreCount = vkWaitOnSemaphores.size();
    presentInfo.pWaitSemaphores    = vkWaitOnSemaphores.data();
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = reinterpret_cast<VulkanSwapChain *>(swapChain)->GetSwapChain();
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    const auto presentResult = m_context->Queues[ QueueType::Presentation ].presentKHR(presentInfo);

    if ( presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR )
    {
        // TODO
    }

    VK_CHECK_RESULT(presentResult);
}
