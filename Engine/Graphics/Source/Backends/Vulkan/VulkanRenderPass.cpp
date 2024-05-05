/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/VulkanRenderPass.h>

using namespace DenOfIz;

VulkanRenderPass::VulkanRenderPass( VulkanContext *context, const RenderPassCreateInfo &createInfo ) :
    m_Context( context ), m_CreateInfo( createInfo )
{
    m_SwapChainImageAvailable = std::make_unique<VulkanLock>( this->m_Context, LockType::Semaphore );
    m_SwapChainImageRendered = std::make_unique<VulkanLock>( this->m_Context, LockType::Semaphore );

    vk::CommandBufferAllocateInfo bufferAllocateInfo{};
    bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    bufferAllocateInfo.commandPool = context->GraphicsQueueCommandPool;
    bufferAllocateInfo.commandBufferCount = 1;

    m_CommandBuffer = context->LogicalDevice.allocateCommandBuffers( bufferAllocateInfo )[ 0 ];

    m_ViewportExtent.width = m_ViewportExtent.width == 0 ? context->SurfaceExtent.width : m_ViewportExtent.width;
    m_ViewportExtent.height = m_ViewportExtent.height == 0 ? context->SurfaceExtent.height : m_ViewportExtent.height;
    UpdateViewport( m_ViewportExtent.width, m_ViewportExtent.height );
    CreateRenderTarget();
}

void VulkanRenderPass::CreateRenderTarget()
{
    RenderTargetAttachment &attachment = m_RenderTargetAttachments.emplace_back( RenderTargetAttachment{} );
    std::vector<vk::ImageView> bufferImageViews;

    if ( m_CreateInfo.RenderToSwapChain )
    {
        attachment.IsSwapChain = true;
        attachment.Image.ImageView = m_Context->SwapChainImageViews[ m_CreateInfo.SwapChainImageIndex ];
        attachment.Image.Instance = m_Context->SwapChainImages[ m_CreateInfo.SwapChainImageIndex ];
        bufferImageViews.push_back( attachment.Image.ImageView );
    }
    else
    {
        // Possible to be swap chain image, then no need to create a new attachment just attach to it.
        // Merge render target and render pass as it seems closely related.
        auto aspectFlags = VulkanEnumConverter::GetOutputImageVkAspect( m_CreateInfo.RenderTargetType );
        auto usageFlags = VulkanEnumConverter::GetVkUsageFlags( m_CreateInfo.RenderTargetType );
        auto imageFormat = VulkanEnumConverter::ConvertImageFormat( m_CreateInfo.Format );

        attachment.Image = CreateAttachment( imageFormat, usageFlags, aspectFlags );

        if ( m_CreateInfo.MSAASampleCount != MSAASampleCount::_0 && m_CreateInfo.RenderTargetType == RenderTargetType::Color )
        {
            RenderTargetAttachment &msaaAttachment = m_RenderTargetAttachments.emplace_back( RenderTargetAttachment{} );
            usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;

            msaaAttachment.Image = CreateAttachment( imageFormat, usageFlags, aspectFlags );
            bufferImageViews.push_back( attachment.Image.ImageView );
        }
    }

    m_RenderTarget = attachment.Image.Instance;
    m_RenderTargetImageView = attachment.Image.ImageView;
}

void VulkanRenderPass::AcquireNextImage()
{
    if ( !m_CreateInfo.RenderToSwapChain )
    {
        return;
    }

    auto image = m_Context->LogicalDevice.acquireNextImageKHR( m_Context->SwapChain, UINT64_MAX, m_SwapChainImageAvailable->GetVkSemaphore(), nullptr );
    if ( image.result == vk::Result::eErrorOutOfDateKHR )
    {
        throw std::runtime_error( "failed to acquire swap chain image!" );
    }
    if ( image.result != vk::Result::eSuccess && image.result != vk::Result::eSuboptimalKHR )
    {
        throw std::runtime_error( "failed to acquire swap chain image!" );
    }
    m_SwapChainIndex = image.value;
}

void VulkanRenderPass::Begin( const std::array<float, 4> clearColor )
{
    m_CurrentResources = std::vector<vk::WriteDescriptorSet>();
    m_HasIndexData = false;

    if ( m_CreateInfo.RenderToSwapChain )
    {
        AcquireNextImage();
    }

    vk::RenderingAttachmentInfo colorAttachmentInfo{};
    colorAttachmentInfo.imageView = m_RenderTargetImageView;
    colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentInfo.clearValue.color = clearColor;

    vk::RenderingInfo renderInfo{};
    renderInfo.renderArea.extent = m_ViewportExtent;
    renderInfo.renderArea.offset = m_ViewportOffset;
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = {};

    vk::ImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite, imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined, imageMemoryBarrier.newLayout =
        vk::ImageLayout::eColorAttachmentOptimal, imageMemoryBarrier.image = m_RenderTarget, imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;

    m_CommandBuffer.reset();
    m_CommandBuffer.begin( beginInfo );
    m_CommandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags{}, 0, nullptr, 0,
                                   nullptr, 1, &imageMemoryBarrier );

    m_CommandBuffer.beginRendering( &renderInfo );
}

void VulkanRenderPass::BindPipeline( VulkanPipeline *pipeline )
{
    m_BoundPipeline = pipeline;
    m_CommandBuffer.bindPipeline( pipeline->BindPoint, pipeline->Instance );
}

void VulkanRenderPass::BindResource( IResource *resource )
{
    vk::WriteDescriptorSet writeDescriptorSet = m_BoundPipeline->GetWriteDescriptorSet( resource->Name );
    switch ( resource->Type() )
    {
    case ResourceType::Buffer:
    {
        const auto bufferResource = static_cast<VulkanBufferResource *>(resource);
        writeDescriptorSet.pBufferInfo = &bufferResource->DescriptorInfo;
        break;
    }
    case ResourceType::Sampler:
    case ResourceType::CubeMap:
    {
        const auto samplerResource = static_cast<VulkanSamplerResource *>(resource);
        writeDescriptorSet.pImageInfo = &samplerResource->DescriptorInfo;
        break;
    }
    }

    writeDescriptorSet.descriptorCount = 1;
    m_CurrentResources.push_back( std::move( writeDescriptorSet ) );
}

void VulkanRenderPass::BindIndexBuffer( IBufferResource *resource )
{
    const auto bufferResource = static_cast<VulkanBufferResource *>(resource);
    constexpr vk::DeviceSize offset = 0;
    m_CommandBuffer.bindIndexBuffer( bufferResource->Instance, offset, vk::IndexType::eUint32 );
    m_HasIndexData = true;
}

void VulkanRenderPass::BindVertexBuffer( IBufferResource *resource ) const
{
    const auto bufferResource = static_cast<VulkanBufferResource *>(resource);
    constexpr vk::DeviceSize offset = 0;
    m_CommandBuffer.bindVertexBuffers( 0, 1, &bufferResource->Instance, &offset );
}

void VulkanRenderPass::SetDepthBias( const float constant, const float clamp, const float slope ) const
{
    m_CommandBuffer.setDepthBias( constant, clamp, slope );
}

/* Type of Resources:
 * - InstanceGeometryResource
 * - GeometryResource
 * - Sampler2DResource
 * - Sampler3DResource
 * -
 */

void VulkanRenderPass::Draw( const uint32_t &instanceCount, const uint32_t &vertexCount ) const
{
    m_CommandBuffer.setViewportWithCount( 1, &m_Viewport );
    m_CommandBuffer.setScissorWithCount( 1, &m_ViewScissor );

    const std::vector<vk::WriteDescriptorSet> &writeDescriptors = m_CurrentResources;
    if ( !writeDescriptors.empty() )
    {
        m_CommandBuffer.pushDescriptorSetKHR( m_BoundPipeline->BindPoint, m_BoundPipeline->Layout, 0, writeDescriptors );
    }

    //	for (const auto& pushConstantBinding : boundPipeline->getPushConstantBindings(frameIndex))
    //	{
    //		commandBuffers.pushConstants(boundPipeline->Layout, pushConstantBinding.stage, 0, pushConstantBinding.totalSize, pushConstantBinding.Data);
    //	}

    if ( m_HasIndexData )
    {
        m_CommandBuffer.drawIndexed( vertexCount, instanceCount, 0, 0, 0 );
    }
    else
    {
        m_CommandBuffer.draw( vertexCount, instanceCount, 0, 0 );
    }
}

SubmitResult VulkanRenderPass::Submit( const std::vector<std::shared_ptr<VulkanLock>> &waitOnLock, VulkanLock *notifyFence )
{
    m_CommandBuffer.endRendering();
    if ( m_CreateInfo.RenderToSwapChain )
    {
        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.dstAccessMask = {}, imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal, imageMemoryBarrier.newLayout =
            vk::ImageLayout::ePresentSrcKHR, imageMemoryBarrier.image = m_RenderTarget, imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        m_CommandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, 0, nullptr, 0, nullptr,
                                       1, &imageMemoryBarrier );
    }

    m_CommandBuffer.end();

    vk::SubmitInfo submitInfo{};

    std::vector<vk::Semaphore> semaphores;

    for ( auto &waitOn : waitOnLock )
    {
        semaphores.push_back( std::dynamic_pointer_cast<VulkanLock>( waitOn )->GetVkSemaphore() );
    }

    if ( m_CreateInfo.RenderToSwapChain )
    {
        semaphores.push_back( m_SwapChainImageAvailable->GetVkSemaphore() );
    }

    vk::PipelineStageFlags waitStages[ ] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    submitInfo.waitSemaphoreCount = semaphores.size();
    submitInfo.pWaitSemaphores = semaphores.data();
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffer;

    if ( m_CreateInfo.RenderToSwapChain )
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_SwapChainImageRendered->GetVkSemaphore();
    }
    else
    {
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
    }

    notifyFence->Reset();
    const auto submitResult = m_Context->Queues[ QueueType::Graphics ].submit( 1, &submitInfo, notifyFence->GetVkFence() );

    VK_CHECK_RESULT( submitResult );
    if ( m_CreateInfo.RenderToSwapChain )
    {
        return PresentPassToSwapChain();
    }

    return SubmitResult::Success;
}

SubmitResult VulkanRenderPass::PresentPassToSwapChain() const
{
    vk::PresentInfoKHR presentInfo{};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_SwapChainImageRendered->GetVkSemaphore();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_Context->SwapChain;
    presentInfo.pImageIndices = &m_SwapChainIndex;
    presentInfo.pResults = nullptr;

    const auto presentResult = m_Context->Queues[ QueueType::Presentation ].presentKHR( presentInfo );
    if ( presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR )
    {
        return SubmitResult::SwapChainInvalidated;
    }
    if ( presentResult != vk::Result::eSuccess )
    {
        return SubmitResult::OtherError;
    }

    return SubmitResult::Success;
}

void VulkanRenderPass::UpdateViewport( const uint32_t &width, const uint32_t &height )
{
    ReturnIf( width == 0 || height == 0 );
    m_Viewport.x = m_ViewportOffset.x;
    m_Viewport.y = m_ViewportOffset.x;
    m_Viewport.width = width;
    m_Viewport.height = static_cast<float>(height);
    m_Viewport.minDepth = 0.0f;
    m_Viewport.maxDepth = 1.0f;

    m_ViewScissor.offset = vk::Offset2D( m_Viewport.x, m_Viewport.y );
    m_ViewScissor.extent = vk::Extent2D( width, height );

    m_ViewportExtent.width = width;
    m_ViewportExtent.height = height;
}

VulkanImage VulkanRenderPass::CreateAttachment( const vk::Format &format, const vk::ImageUsageFlags &usage, const vk::ImageAspectFlags &aspect ) const
{
    VulkanImage newAttachment{};
    vk::ImageCreateInfo imageCreateInfo{};

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.extent.width = m_CreateInfo.Width == 0 ? m_Context->SurfaceExtent.width : m_CreateInfo.Width;
    imageCreateInfo.extent.height = m_CreateInfo.Height == 0 ? m_Context->SurfaceExtent.height : m_CreateInfo.Height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.samples = VulkanEnumConverter::ConverSampleCount( m_CreateInfo.MSAASampleCount );
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vk::Image image;
    VmaAllocation allocation;

    vmaCreateImage( m_Context->Vma, reinterpret_cast<VkImageCreateInfo *>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage *>(&image), &allocation, nullptr );

    newAttachment.Instance = image;
    newAttachment.Allocation = allocation;

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.subresourceRange.aspectMask = aspect;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    newAttachment.ImageView = m_Context->LogicalDevice.createImageView( imageViewCreateInfo );

    if ( usage & vk::ImageUsageFlagBits::eSampled )
    {
        vk::SamplerCreateInfo samplerCreateInfo{};

        samplerCreateInfo.magFilter = vk::Filter::eNearest;
        samplerCreateInfo.minFilter = vk::Filter::eNearest;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;

        newAttachment.Sampler = m_Context->LogicalDevice.createSampler( samplerCreateInfo );
    }

    return newAttachment;
}

VulkanRenderPass::~VulkanRenderPass()
{
    m_Context->LogicalDevice.resetCommandPool( m_Context->GraphicsQueueCommandPool );

    for ( auto &attachment : m_RenderTargetAttachments )
    {
        if ( !attachment.IsSwapChain )
        {
            attachment.Image.Dispose( m_Context );
        }
    }
}

#endif
