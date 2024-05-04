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
		:context(context), createInfo(createInfo)
{
	swapChainImageAvailable.resize(this->context->SwapChainImages.size());
	swapChainImageRendered.resize(this->context->SwapChainImages.size());
	hasIndexData.resize(context->SwapChainImages.size());
	currentResources.resize(context->SwapChainImages.size());

	for (uint32_t i = 0; i < swapChainImageAvailable.size(); ++i)
	{
		swapChainImageAvailable[i] = std::make_unique<VulkanLock>(this->context, LockType::Semaphore);
		swapChainImageRendered[i] = std::make_unique<VulkanLock>(this->context, LockType::Semaphore);
	}
	uint32_t attachmentIndex = 0;

	vk::CommandBufferAllocateInfo bufferAllocateInfo{};
	bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	bufferAllocateInfo.commandPool = context->GraphicsQueueCommandPool;
	bufferAllocateInfo.commandBufferCount = context->SwapChainImages.size();

	commandBuffers = context->LogicalDevice.allocateCommandBuffers(bufferAllocateInfo);

	viewportExtent.width = viewportExtent.width == 0 ? context->SurfaceExtent.width : viewportExtent.width;
	viewportExtent.height = viewportExtent.height == 0 ? context->SurfaceExtent.height : viewportExtent.height;
	UpdateViewport(viewportExtent.width, viewportExtent.height);
	CreateRenderTarget();
}

void VulkanRenderPass::CreateRenderTarget()
{
	for (uint32_t swapChainImageIndex = 0; swapChainImageIndex < createInfo.SwapChainImageCount; ++swapChainImageIndex)
	{
		RenderTargetAttachment& attachment = renderTargetAttachments.emplace_back(RenderTargetAttachment{});
		std::vector<vk::ImageView> bufferImageViews;

		if (createInfo.RenderToSwapChain)
		{
			attachment.IsSwapChain = true;
			attachment.Instance.ImageView = context->SwapChainImageViews[swapChainImageIndex];
			attachment.Instance.Instance = context->SwapChainImages[swapChainImageIndex];
			bufferImageViews.push_back(attachment.Instance.ImageView);
		}
		else
		{
			// Possible to be swap chain image, then no need to create a new attachment just attach to it.
			// Merge render target and render pass as it seems closely related.
			auto aspectFlags = VulkanEnumConverter::GetOutputImageVkAspect(createInfo.RenderTargetType);
			auto usageFlags = VulkanEnumConverter::GetVkUsageFlags(createInfo.RenderTargetType);
			auto imageFormat = VulkanEnumConverter::ConvertImageFormat(createInfo.Format);

			attachment.Instance = CreateAttachment(imageFormat, usageFlags, aspectFlags);

			if (createInfo.MSAASampleCount != MSAASampleCount::_0 && createInfo.RenderTargetType == RenderTargetType::Color)
			{
				RenderTargetAttachment& msaaAttachment = renderTargetAttachments.emplace_back(RenderTargetAttachment{});
				usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment;

				msaaAttachment.Instance = CreateAttachment(imageFormat, usageFlags, aspectFlags);
				bufferImageViews.push_back(attachment.Instance.ImageView);
			}

		}
	}
}

void VulkanRenderPass::AcquireNextImage()
{
	if (!createInfo.RenderToSwapChain)
	{
		return;
	}

	auto image = context->LogicalDevice.acquireNextImageKHR(context->SwapChain, UINT64_MAX, swapChainImageAvailable[frameIndex]->GetVkSemaphore(), nullptr);
	if (image.result == vk::Result::eErrorOutOfDateKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	else if (image.result != vk::Result::eSuccess && image.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	swapChainIndex = image.value;
}

void VulkanRenderPass::Begin(uint32_t frameIdx, std::array<float, 4> clearColor)
{
	this->frameIndex = frameIdx;

	currentResources[frameIndex] = std::vector<vk::WriteDescriptorSet>();
	hasIndexData[frameIndex] = false;

	uint32_t renderIndex = frameIdx;
	if (createInfo.RenderToSwapChain)
	{
		AcquireNextImage();
		renderIndex = swapChainIndex;
	}

	vk::RenderingAttachmentInfo colorAttachmentInfo{};
	colorAttachmentInfo.imageView = renderTargetAttachments[renderIndex].Instance.ImageView;
	colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachmentInfo.clearValue.color = clearColor;

	vk::RenderingInfo renderInfo{};
	renderInfo.renderArea.extent = viewportExtent;
	renderInfo.renderArea.offset = viewportOffset;
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachmentInfo;

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = {};

	vk::ImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
	imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined,
	imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal,
	imageMemoryBarrier.image = renderTargetAttachments[renderIndex].Instance.Instance,
	imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	commandBuffers[frameIndex].reset();
	commandBuffers[frameIndex].begin(beginInfo);
	commandBuffers[frameIndex].pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::DependencyFlags{},
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageMemoryBarrier
	);

	commandBuffers[frameIndex].beginRendering(&renderInfo);
}

void VulkanRenderPass::BindPipeline(VulkanPipeline* pipeline)
{
	boundPipeline = pipeline;
	commandBuffers[frameIndex].bindPipeline(pipeline->BindPoint, pipeline->Instance);
}

void VulkanRenderPass::BindResource(IResource* resource)
{
	vk::WriteDescriptorSet writeDescriptorSet = boundPipeline->GetWriteDescriptorSet(resource->Name);
	switch (resource->Type())
	{
	case ResourceType::Buffer:
	{
		VulkanBufferResource* bufferResource = static_cast<VulkanBufferResource*>(resource);
		writeDescriptorSet.pBufferInfo = &bufferResource->DescriptorInfo;
		break;
	}
	case ResourceType::Sampler:
	case ResourceType::CubeMap:
	{
		VulkanSamplerResource* samplerResource = static_cast<VulkanSamplerResource*>(resource);
		writeDescriptorSet.pImageInfo = &samplerResource->DescriptorInfo;
		break;
	}
	}

	writeDescriptorSet.descriptorCount = 1;
	currentResources[frameIndex].push_back(std::move(writeDescriptorSet));
}

void VulkanRenderPass::BindIndexBuffer(IBufferResource* resource)
{
	VulkanBufferResource* bufferResource = (VulkanBufferResource*)resource;
	vk::DeviceSize offset = 0;
	commandBuffers[frameIndex].bindIndexBuffer(bufferResource->Instance, offset, vk::IndexType::eUint32);
	hasIndexData[frameIndex] = true;
}

void VulkanRenderPass::BindVertexBuffer(IBufferResource* resource)
{
	VulkanBufferResource* bufferResource = static_cast<VulkanBufferResource*>(resource);
	vk::DeviceSize offset = 0;
	commandBuffers[frameIndex].bindVertexBuffers(0, 1, &bufferResource->Instance, &offset);
}

void VulkanRenderPass::SetDepthBias(float constant, float clamp, float slope)
{
	commandBuffers[frameIndex].setDepthBias(constant, clamp, slope);
}

/* Type of Resources:
 * - InstanceGeometryResource
 * - GeometryResource
 * - Sampler2DResource
 * - Sampler3DResource
 * -
 */

void VulkanRenderPass::Draw(const uint32_t& instanceCount, const uint32_t& vertexCount)
{
	commandBuffers[frameIndex].setViewportWithCount(1, &viewport);
	commandBuffers[frameIndex].setScissorWithCount(1, &viewScissor);

	const std::vector<vk::WriteDescriptorSet>& writeDescriptors = currentResources[frameIndex];
	if (!writeDescriptors.empty())
	{
		commandBuffers[frameIndex].pushDescriptorSetKHR(boundPipeline->BindPoint, boundPipeline->Layout, 0, writeDescriptors);
	}

//	for (const auto& pushConstantBinding : boundPipeline->getPushConstantBindings(frameIndex))
//	{
//		commandBuffers[frameIndex].pushConstants(boundPipeline->Layout, pushConstantBinding.stage, 0, pushConstantBinding.totalSize, pushConstantBinding.Data);
//	}

	if (hasIndexData[frameIndex])
	{
		commandBuffers[frameIndex].drawIndexed(vertexCount, instanceCount, 0, 0, 0);
	}
	else
	{
		commandBuffers[frameIndex].draw(vertexCount, instanceCount, 0, 0);
	}
}

SubmitResult VulkanRenderPass::Submit(std::vector<std::shared_ptr<VulkanLock>> waitOnLock, VulkanLock* notifyFence)
{
	commandBuffers[frameIndex].endRendering();
	if (createInfo.RenderToSwapChain)
	{
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.dstAccessMask = {},
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
		imageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR,
		imageMemoryBarrier.image = renderTargetAttachments[swapChainIndex].Instance.Instance,
		imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		commandBuffers[frameIndex].pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::DependencyFlags{},
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imageMemoryBarrier
		);
	}

	commandBuffers[frameIndex].end();

	vk::SubmitInfo submitInfo{};

	std::vector<vk::Semaphore> semaphores;

	for (auto& waitOn : waitOnLock)
	{
		semaphores.push_back(std::dynamic_pointer_cast<VulkanLock>(waitOn)->GetVkSemaphore());
	}

	if (createInfo.RenderToSwapChain)
	{
		semaphores.push_back(swapChainImageAvailable[frameIndex]->GetVkSemaphore());
	}

	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.waitSemaphoreCount = semaphores.size();
	submitInfo.pWaitSemaphores = semaphores.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[frameIndex];

	if (createInfo.RenderToSwapChain)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &swapChainImageRendered[frameIndex]->GetVkSemaphore();
	}
	else
	{
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
	}

	notifyFence->Reset();
	auto submitResult = context->Queues[QueueType::Graphics].submit(1, &submitInfo, notifyFence->GetVkFence());

	VkCheckResult(submitResult);
	if (createInfo.RenderToSwapChain)
	{
		return PresentPassToSwapChain();
	}

	return SubmitResult::Success;
}

SubmitResult VulkanRenderPass::PresentPassToSwapChain()
{
	vk::PresentInfoKHR presentInfo{};

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &swapChainImageRendered[frameIndex]->GetVkSemaphore();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context->SwapChain;
	presentInfo.pImageIndices = &swapChainIndex;
	presentInfo.pResults = nullptr;

	const auto presentResult = context->Queues[QueueType::Presentation].presentKHR(presentInfo);
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
	{
		return SubmitResult::SwapChainInvalidated;
	}
	else if (presentResult != vk::Result::eSuccess)
	{
		return SubmitResult::OtherError;
	}

	return SubmitResult::Success;
}

void VulkanRenderPass::UpdateViewport(const uint32_t& width, const uint32_t& height)
{
	FUNCTION_BREAK(width == 0 || height == 0);
	viewport.x = viewportOffset.x;
	viewport.y = viewportOffset.x;
	viewport.width = width;
	viewport.height = static_cast<float>(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	viewScissor.offset = vk::Offset2D(viewport.x, viewport.y);
	viewScissor.extent = vk::Extent2D(width, height);

	viewportExtent.width = width;
	viewportExtent.height = height;
}

VulkanImage VulkanRenderPass::CreateAttachment(const vk::Format& format, const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspect)
{
	VulkanImage newAttachment{};
	vk::ImageCreateInfo imageCreateInfo{};

	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = createInfo.Width == 0 ? context->SurfaceExtent.width : createInfo.Width;
	imageCreateInfo.extent.height = createInfo.Height == 0 ? context->SurfaceExtent.height : createInfo.Height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.usage = usage;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	imageCreateInfo.samples = VulkanEnumConverter::ConverSampleCount(createInfo.MSAASampleCount);
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

	vk::Image image;
	VmaAllocation allocation;

	vmaCreateImage(context->Vma, (VkImageCreateInfo*)&imageCreateInfo, &allocationCreateInfo, (VkImage*)&image, &allocation, nullptr);

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

	newAttachment.ImageView = context->LogicalDevice.createImageView(imageViewCreateInfo);

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

		newAttachment.Sampler = context->LogicalDevice.createSampler(samplerCreateInfo);
	}

	return newAttachment;
}

VulkanRenderPass::~VulkanRenderPass()
{
	context->LogicalDevice.resetCommandPool(context->GraphicsQueueCommandPool);

	for (auto& attachment : renderTargetAttachments)
	{
		if (!attachment.IsSwapChain)
		{
			attachment.Instance.Dispose(context);
		}
	}
}