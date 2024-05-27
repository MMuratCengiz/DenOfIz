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
#include "DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanDescriptorTable.h"

using namespace DenOfIz;

VulkanCommandList::VulkanCommandList(VulkanContext* context, CommandListCreateInfo createInfo):m_context(context), m_createInfo(std::move(createInfo))
{
	auto commandPool = m_context->GraphicsQueueCommandPool;
	switch (m_createInfo.QueueType)
	{
	case QueueType::Presentation:
	case QueueType::Graphics:
		commandPool = m_context->GraphicsQueueCommandPool;
		break;
	case QueueType::Compute:
		commandPool = m_context->ComputeQueueCommandPool;
		break;
	case QueueType::Transfer:
		commandPool = m_context->TransferQueueCommandPool;
		break;
	}

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary; // Todo
	allocInfo.commandBufferCount = 1;

	m_commandBuffer = m_context->LogicalDevice.allocateCommandBuffers(allocInfo)[0];
}

void VulkanCommandList::Reset()
{
	m_commandBuffer.reset();
}

void VulkanCommandList::Begin()
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = {};
	m_commandBuffer.begin(beginInfo);
}

void VulkanCommandList::BeginRendering(const RenderingInfo& renderingInfo)
{
	vk::RenderingInfo renderInfo{};

	SDL_Surface* surface = SDL_GetWindowSurface(m_context->Window);

	float widthAdapted = renderingInfo.RenderAreaWidth == 0 ? static_cast<float>(surface->w) : static_cast<float>(renderingInfo.RenderAreaWidth);
	float heightAdapted = renderingInfo.RenderAreaHeight == 0 ? static_cast<float>(surface->h) : static_cast<float>(renderingInfo.RenderAreaHeight);

	renderInfo.renderArea.extent = vk::Extent2D(widthAdapted, heightAdapted);
	renderInfo.renderArea.offset = vk::Offset2D(renderingInfo.RenderAreaOffsetX, renderingInfo.RenderAreaOffsetY);
	renderInfo.layerCount = renderingInfo.LayerCount;

	std::vector<vk::RenderingAttachmentInfo> colorAttachments;

	for (const auto& colorAttachment : renderingInfo.ColorAttachments)
	{
		VulkanImageResource* vkColorAttachmentResource = dynamic_cast<VulkanImageResource*>(colorAttachment.Resource);

		vk::RenderingAttachmentInfo colorAttachmentInfo{};
		colorAttachmentInfo.imageView = vkColorAttachmentResource->GetImageView();
		colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colorAttachmentInfo.loadOp = VulkanEnumConverter::ConvertLoadOp(colorAttachment.LoadOp);
		colorAttachmentInfo.storeOp = VulkanEnumConverter::ConvertStoreOp(colorAttachment.StoreOp);
		colorAttachmentInfo.clearValue.color = colorAttachment.ClearColor;

		colorAttachments.push_back(colorAttachmentInfo);
	}

	renderInfo.setColorAttachments(colorAttachments);

	// Todo these need to be fixed.
	if (renderingInfo.DepthAttachment->Resource != nullptr)
	{
		VulkanImageResource* vkDepthStencilResource = dynamic_cast<VulkanImageResource*>(renderingInfo.DepthAttachment->Resource);

		vk::RenderingAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.imageView = vkDepthStencilResource->GetImageView();
		depthAttachmentInfo.imageLayout = VulkanEnumConverter::ConvertImageLayout(renderingInfo.DepthAttachment->Layout);
		depthAttachmentInfo.loadOp = VulkanEnumConverter::ConvertLoadOp(renderingInfo.DepthAttachment->LoadOp);
		depthAttachmentInfo.storeOp = VulkanEnumConverter::ConvertStoreOp(renderingInfo.DepthAttachment->StoreOp);
		depthAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue(renderingInfo.DepthAttachment->ClearDepth[0], renderingInfo.DepthAttachment->ClearDepth[1]);

		renderInfo.setPDepthAttachment(&depthAttachmentInfo);
	}

	if (renderingInfo.StencilAttachment->Resource != nullptr)
	{
		VulkanImageResource* vkDepthStencilResource = dynamic_cast<VulkanImageResource*>(renderingInfo.StencilAttachment->Resource);

		vk::RenderingAttachmentInfo stencilAttachmentInfo{};
		stencilAttachmentInfo.imageView = vkDepthStencilResource->GetImageView();
		stencilAttachmentInfo.imageLayout = VulkanEnumConverter::ConvertImageLayout(renderingInfo.StencilAttachment->Layout);
		stencilAttachmentInfo.loadOp = VulkanEnumConverter::ConvertLoadOp(renderingInfo.StencilAttachment->LoadOp);
		stencilAttachmentInfo.storeOp = VulkanEnumConverter::ConvertStoreOp(renderingInfo.StencilAttachment->StoreOp);
		stencilAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue(renderingInfo.StencilAttachment->ClearDepth[0], renderingInfo.DepthAttachment->ClearDepth[1]);

		renderInfo.setPStencilAttachment(&stencilAttachmentInfo);
	}

	m_commandBuffer.beginRendering(renderInfo);
}

void VulkanCommandList::EndRendering()
{
	m_commandBuffer.endRendering();
}

void VulkanCommandList::End()
{
	m_commandBuffer.end();
}

void VulkanCommandList::Submit(IFence* notify, std::vector<ISemaphore*> waitOnLocks)
{
	vk::SubmitInfo submitInfo{};

	std::vector<vk::PipelineStageFlags> waitStages(waitOnLocks.size());
	std::vector<vk::Semaphore> semaphores(waitOnLocks.size());
	for (ISemaphore* waitOn : waitOnLocks)
	{
		semaphores.push_back(reinterpret_cast<VulkanSemaphore*>(waitOn)->GetSemaphore());
		waitStages.push_back(vk::PipelineStageFlagBits::eAllCommands);
	}

	submitInfo.waitSemaphoreCount = semaphores.size();
	submitInfo.pWaitSemaphores = semaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffer;
	submitInfo.signalSemaphoreCount = semaphores.size();
	submitInfo.pSignalSemaphores = semaphores.data();

	notify->Reset();
	const auto submitResult = m_context->Queues[m_createInfo.QueueType].submit(1, &submitInfo, reinterpret_cast<VulkanFence*>(notify)->GetFence());

	VK_CHECK_RESULT(submitResult);
}

uint32_t VulkanCommandList::AcquireNextImage(ISwapChain* swapChain, ISemaphore* lock)
{
	return swapChain->AcquireNextImage();
}

void VulkanCommandList::BindPipeline(IPipeline* pipeline)
{
	VulkanPipeline* vkPipeline = dynamic_cast<VulkanPipeline*>(pipeline);
	m_commandBuffer.bindPipeline(vkPipeline->BindPoint, vkPipeline->Instance);
}

void VulkanCommandList::BindVertexBuffer(IBufferResource* buffer)
{
	const auto bufferResource = static_cast<VulkanBufferResource*>(buffer);
	constexpr vk::DeviceSize offset = 0;
	m_commandBuffer.bindVertexBuffers(0, 1, &bufferResource->GetBuffer(), &offset);
}

void VulkanCommandList::BindIndexBuffer(IBufferResource* buffer)
{
	const auto bufferResource = static_cast<VulkanBufferResource*>(buffer);
	constexpr vk::DeviceSize offset = 0;
	m_commandBuffer.bindIndexBuffer(bufferResource->GetBuffer(), offset, vk::IndexType::eUint32);
}

void VulkanCommandList::BindViewport(float offsetX, float offsetY, float width, float height)
{
	RETURN_IF(width == 0 || height == 0);
	m_viewport.x = offsetX;
	m_viewport.y = offsetY;
	m_viewport.width = width;
	m_viewport.height = static_cast<float>(height);
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

void VulkanCommandList::BindDescriptorTable(IDescriptorTable* table)
{
	VulkanDescriptorTable* vkTable = dynamic_cast<VulkanDescriptorTable*>(table);

}

void VulkanCommandList::BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void* data)
{
}

void VulkanCommandList::BindBufferResource(IBufferResource* resource)
{

}

void VulkanCommandList::BindImageResource(IImageResource* resource)
{

}

void VulkanCommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor)
{
	m_commandBuffer.setDepthBias(constantFactor, clamp, slopeFactor);
}

void VulkanCommandList::SetImageBarrier(IImageResource* resource, ImageBarrier barrier)
{
	VulkanImageResource* vkResource = dynamic_cast<VulkanImageResource*>(resource);

	vk::ImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
	imageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imageMemoryBarrier.image = vkResource->GetImage();
	imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags{}, 0, nullptr, 0,
			nullptr, 1, &imageMemoryBarrier);
}

void VulkanCommandList::SetBufferBarrier(IBufferResource* resource, BufferBarrier barrier)
{
	VulkanBufferResource* vkResource = dynamic_cast<VulkanBufferResource*>(resource);

	vk::BufferMemoryBarrier bufferMemoryBarrier{};
	bufferMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	bufferMemoryBarrier.size = vkResource->GetSize();
	bufferMemoryBarrier.buffer = vkResource->GetBuffer();

	m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags{}, 0, nullptr, 1,
			&bufferMemoryBarrier, 0, nullptr);
}

void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	m_commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	m_commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	assertm(m_createInfo.QueueType == QueueType::Compute, "Dispatch can only be called on compute queues.");
}

void VulkanCommandList::CopyBuffer(IBufferResource* src, IBufferResource* dst, uint32_t size)
{

}

void VulkanCommandList::TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout)
{

}

void VulkanCommandList::Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks)
{
	assertm(m_createInfo.QueueType == QueueType::Graphics || m_createInfo.QueueType == QueueType::Presentation, "Present can only be called on presentation queues.");

	vk::PresentInfoKHR presentInfo{};

	std::vector<vk::Semaphore> vkWaitOnSemaphores(waitOnLocks.size());
	for (int i = 0; i < waitOnLocks.size(); i++)
	{
		vkWaitOnSemaphores[i] = reinterpret_cast<VulkanSemaphore*>(waitOnLocks[i])->GetSemaphore();
	}

	presentInfo.waitSemaphoreCount = vkWaitOnSemaphores.size();
	presentInfo.pWaitSemaphores = vkWaitOnSemaphores.data();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = reinterpret_cast<VulkanSwapChain*>(swapChain)->GetSwapChain();
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	const auto presentResult = m_context->Queues[QueueType::Presentation].presentKHR(presentInfo);

	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
	{
		// TODO
	}

	VK_CHECK_RESULT(presentResult);
}
