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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRenderPass.h>

using namespace DenOfIz;

VulkanRenderPass::VulkanRenderPass(VulkanContext* context, const RenderPassCreateInfo& createInfo)
		:
		m_context(context), m_createInfo(createInfo)
{
	m_swapChainImageAvailable = std::make_unique<VulkanLock>(this->m_context, LockType::Semaphore);
	m_swapChainImageRendered = std::make_unique<VulkanLock>(this->m_context, LockType::Semaphore);

	vk::CommandBufferAllocateInfo bufferAllocateInfo{};
	bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	bufferAllocateInfo.commandPool = context->GraphicsQueueCommandPool;
	bufferAllocateInfo.commandBufferCount = 1;

	m_commandBuffer = context->LogicalDevice.allocateCommandBuffers(bufferAllocateInfo)[0];

	m_viewportExtent.width = m_viewportExtent.width == 0 ? context->SurfaceExtent.width : m_viewportExtent.width;
	m_viewportExtent.height = m_viewportExtent.height == 0 ? context->SurfaceExtent.height : m_viewportExtent.height;
	UpdateViewport(m_viewportExtent.width, m_viewportExtent.height);
	CreateRenderTarget();
}

void VulkanRenderPass::CreateRenderTarget()
{
	RenderTargetAttachment& attachment = m_renderTargetAttachments.emplace_back(RenderTargetAttachment{});
	std::vector<vk::ImageView> bufferImageViews;

	if (m_createInfo.RenderToSwapChain)
	{
		attachment.IsSwapChain = true;
		attachment.Image.ImageView = m_context->SwapChainImageViews[m_createInfo.SwapChainImageIndex];
		attachment.Image.Instance = m_context->SwapChainImages[m_createInfo.SwapChainImageIndex];
		bufferImageViews.push_back(attachment.Image.ImageView);
	}
	else
	{
		// Possible to be swap chain image, then no need to create a new attachment just attach to it.
		// Merge render target and render pass as it seems closely related.
		auto aspectFlags = VulkanEnumConverter::GetOutputImageVkAspect(m_createInfo.RenderTargetType);
		auto usageFlags = VulkanEnumConverter::GetVkUsageFlags(m_createInfo.RenderTargetType);
		auto imageFormat = VulkanEnumConverter::ConvertImageFormat(m_createInfo.Format);

		attachment.Image = CreateAttachment(imageFormat, usageFlags, aspectFlags);

		if (m_createInfo.MSAASampleCount != MSAASampleCount::_0 && m_createInfo.RenderTargetType == RenderTargetType::Color)
		{
			RenderTargetAttachment& msaaAttachment = m_renderTargetAttachments.emplace_back(RenderTargetAttachment{});
			usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;

			msaaAttachment.Image = CreateAttachment(imageFormat, usageFlags, aspectFlags);
			bufferImageViews.push_back(attachment.Image.ImageView);
		}
	}

	m_renderTarget = attachment.Image.Instance;
	m_renderTargetImageView = attachment.Image.ImageView;
}

void VulkanRenderPass::AcquireNextImage()
{
	if (!m_createInfo.RenderToSwapChain)
	{
		return;
	}

	auto image = m_context->LogicalDevice.acquireNextImageKHR(m_context->SwapChain, UINT64_MAX, m_swapChainImageAvailable->GetVkSemaphore(), nullptr);
	if (image.result == vk::Result::eErrorOutOfDateKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	if (image.result != vk::Result::eSuccess && image.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	m_swapChainIndex = image.value;
}

void VulkanRenderPass::Begin(const std::array<float, 4> clearColor)
{
	m_currentResources = std::vector<vk::WriteDescriptorSet>();
	m_hasIndexData = false;

	if (m_createInfo.RenderToSwapChain)
	{
		AcquireNextImage();
	}

	vk::RenderingAttachmentInfo colorAttachmentInfo{};
	colorAttachmentInfo.imageView = m_renderTargetImageView;
	colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachmentInfo.clearValue.color = clearColor;

	vk::RenderingInfo renderInfo{};
	renderInfo.renderArea.extent = m_viewportExtent;
	renderInfo.renderArea.offset = m_viewportOffset;
	renderInfo.layerCount = 1;
	renderInfo.setColorAttachments(colorAttachmentInfo);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = {};

	vk::ImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
	imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imageMemoryBarrier.image = m_renderTarget;
	imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	m_commandBuffer.reset();
	m_commandBuffer.begin(beginInfo);
	m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags{}, 0, nullptr, 0,
			nullptr, 1, &imageMemoryBarrier);

	m_commandBuffer.beginRendering(&renderInfo);
}

void VulkanRenderPass::BindPipeline(IPipeline* pipeline)
{
	VulkanPipeline* vkPipeline = dynamic_cast<VulkanPipeline*>(pipeline);
	m_boundPipeline = vkPipeline;
	m_commandBuffer.bindPipeline(vkPipeline->BindPoint, vkPipeline->Instance);
}

void VulkanRenderPass::BindResource(IResource* resource)
{
	vk::WriteDescriptorSet writeDescriptorSet = m_boundPipeline->GetWriteDescriptorSet(resource->Name);
	switch (resource->Type())
	{
	case ResourceType::Buffer:
	{
		const auto bufferResource = dynamic_cast<VulkanBufferResource*>(resource);
		writeDescriptorSet.pBufferInfo = &bufferResource->DescriptorInfo;
		break;
	}
	case ResourceType::Texture:
	case ResourceType::CubeMap:
	{
		const auto samplerResource = dynamic_cast<VulkanSamplerResource*>(resource);
		writeDescriptorSet.pImageInfo = &samplerResource->DescriptorInfo;
		break;
	}
	}

	writeDescriptorSet.descriptorCount = 1;
	m_currentResources.push_back(std::move(writeDescriptorSet));
}

void VulkanRenderPass::BindIndexBuffer(IBufferResource* resource)
{
	const auto bufferResource = static_cast<VulkanBufferResource*>(resource);
	constexpr vk::DeviceSize offset = 0;
	m_commandBuffer.bindIndexBuffer(bufferResource->GetBuffer(), offset, vk::IndexType::eUint32);
	m_hasIndexData = true;
}

void VulkanRenderPass::BindVertexBuffer(IBufferResource* resource) const
{
	const auto bufferResource = static_cast<VulkanBufferResource*>(resource);
	constexpr vk::DeviceSize offset = 0;
	m_commandBuffer.bindVertexBuffers(0, 1, &bufferResource->GetBuffer(), &offset);
}

void VulkanRenderPass::SetDepthBias(const float constant, const float clamp, const float slope) const
{
	m_commandBuffer.setDepthBias(constant, clamp, slope);
}

/* Type of Resources:
 * - InstanceGeometryResource
 * - GeometryResource
 * - Sampler2DResource
 * - Sampler3DResource
 * -
 */

void VulkanRenderPass::Draw(const uint32_t& instanceCount, const uint32_t& vertexCount) const
{
	m_commandBuffer.setViewportWithCount(1, &m_viewport);
	m_commandBuffer.setScissorWithCount(1, &m_viewScissor);

	const std::vector<vk::WriteDescriptorSet>& writeDescriptors = m_currentResources;
	if (!writeDescriptors.empty())
	{
		m_commandBuffer.pushDescriptorSetKHR(m_boundPipeline->BindPoint, m_boundPipeline->Layout, 0, writeDescriptors);
	}

	//	for (const auto& pushConstantBinding : boundPipeline->getPushConstantBindings(frameIndex))
	//	{
	//		commandBuffers.pushConstants(boundPipeline->Layout, pushConstantBinding.stage, 0, pushConstantBinding.totalSize, pushConstantBinding.Data);
	//	}

	if (m_hasIndexData)
	{
		m_commandBuffer.drawIndexed(vertexCount, instanceCount, 0, 0, 0);
	}
	else
	{
		m_commandBuffer.draw(vertexCount, instanceCount, 0, 0);
	}
}

SubmitResult VulkanRenderPass::Submit(const std::vector<std::shared_ptr<ILock>>& waitOnLock, ILock* notifyFence)
{
	m_commandBuffer.endRendering();
	if (m_createInfo.RenderToSwapChain)
	{
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.dstAccessMask = {};
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		imageMemoryBarrier.image = m_renderTarget;
		imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, 0, nullptr, 0, nullptr,
				1, &imageMemoryBarrier);
	}

	m_commandBuffer.end();

	vk::SubmitInfo submitInfo{};

	std::vector<vk::Semaphore> semaphores(waitOnLock.size());
	for (auto& waitOn : waitOnLock)
	{
		semaphores.push_back(std::dynamic_pointer_cast<VulkanLock>(waitOn)->GetVkSemaphore());
	}

	if (m_createInfo.RenderToSwapChain)
	{
		semaphores.push_back(m_swapChainImageAvailable->GetVkSemaphore());
	}

	constexpr vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.waitSemaphoreCount = semaphores.size();
	submitInfo.pWaitSemaphores = semaphores.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffer;

	if (m_createInfo.RenderToSwapChain)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_swapChainImageRendered->GetVkSemaphore();
	}
	else
	{
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
	}

	notifyFence->Reset();
	const auto submitResult = m_context->Queues[QueueType::Graphics].submit(1, &submitInfo, dynamic_cast<VulkanLock*>(notifyFence)->GetVkFence());

	VK_CHECK_RESULT(submitResult);
	if (m_createInfo.RenderToSwapChain)
	{
		return PresentPassToSwapChain();
	}

	return SubmitResult::Success;
}

SubmitResult VulkanRenderPass::PresentPassToSwapChain() const
{
	vk::PresentInfoKHR presentInfo{};

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_swapChainImageRendered->GetVkSemaphore();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_context->SwapChain;
	presentInfo.pImageIndices = &m_swapChainIndex;
	presentInfo.pResults = nullptr;

	const auto presentResult = m_context->Queues[QueueType::Presentation].presentKHR(presentInfo);
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
	{
		return SubmitResult::SwapChainInvalidated;
	}
	if (presentResult != vk::Result::eSuccess)
	{
		return SubmitResult::OtherError;
	}

	return SubmitResult::Success;
}

void VulkanRenderPass::UpdateViewport(const uint32_t& width, const uint32_t& height)
{
	RETURN_IF(width == 0 || height == 0);
	m_viewport.x = m_viewportOffset.x;
	m_viewport.y = m_viewportOffset.x;
	m_viewport.width = width;
	m_viewport.height = static_cast<float>(height);
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_viewScissor.offset = vk::Offset2D(m_viewport.x, m_viewport.y);
	m_viewScissor.extent = vk::Extent2D(width, height);

	m_viewportExtent.width = width;
	m_viewportExtent.height = height;
}

VulkanImage VulkanRenderPass::CreateAttachment(const vk::Format& format, const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspect) const
{
	VulkanImage newAttachment{};
	vk::ImageCreateInfo imageCreateInfo{};

	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = m_createInfo.Width == 0 ? m_context->SurfaceExtent.width : m_createInfo.Width;
	imageCreateInfo.extent.height = m_createInfo.Height == 0 ? m_context->SurfaceExtent.height : m_createInfo.Height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = VulkanEnumConverter::ConvertSampleCount(m_createInfo.MSAASampleCount);
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vk::Image image;
	VmaAllocation allocation;

	vmaCreateImage(m_context->Vma, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage*>(&image), &allocation, nullptr);

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

	newAttachment.ImageView = m_context->LogicalDevice.createImageView(imageViewCreateInfo);

	if (usage & vk::ImageUsageFlagBits::eSampled)
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

		newAttachment.Sampler = m_context->LogicalDevice.createSampler(samplerCreateInfo);
	}

	return newAttachment;
}

VulkanRenderPass::~VulkanRenderPass()
{
	m_context->LogicalDevice.resetCommandPool(m_context->GraphicsQueueCommandPool);

	for (auto& attachment : m_renderTargetAttachments)
	{
		if (!attachment.IsSwapChain)
		{
			attachment.Image.Dispose(m_context);
		}
	}
}
