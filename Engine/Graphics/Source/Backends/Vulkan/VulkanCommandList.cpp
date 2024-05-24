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

using namespace DenOfIz;


VulkanCommandList::VulkanCommandList(VulkanContext* context): m_context(context)
{

}

void VulkanCommandList::Reset()
{

}

void VulkanCommandList::Begin()
{

}

void VulkanCommandList::End()
{

}

void VulkanCommandList::Submit(IFence* notify)
{

}

uint32_t VulkanCommandList::AcquireNextImage(ISwapChain* swapChain, ISemaphore* lock)
{
	return swapChain->AcquireNextImage();
}

void VulkanCommandList::BindPipeline(IPipeline* pipeline)
{

}

void VulkanCommandList::BindVertexBuffer(IBufferResource* buffer)
{

}

void VulkanCommandList::BindIndexBuffer(IBufferResource* buffer)
{

}

void VulkanCommandList::BindViewport(float x, float y, float width, float height)
{

}

void VulkanCommandList::BindScissorRect(float x, float y, float width, float height)
{

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

}

void VulkanCommandList::ClearColor(float r, float g, float b, float a)
{

}

void VulkanCommandList::ClearDepthStencil(float depth, uint32_t stencil)
{

}

void VulkanCommandList::SetImageBarrier(IImageResource* resource, ImageBarrier barrier)
{

}

void VulkanCommandList::SetBufferBarrier(IBufferResource* resource, BufferBarrier barrier)
{

}

void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{

}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{

}

void VulkanCommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{

}

void VulkanCommandList::CopyBuffer(IBufferResource* src, IBufferResource* dst, uint32_t size)
{

}

void VulkanCommandList::TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout)
{

}

void VulkanCommandList::ClearColorAttachment(uint32_t attachmentIndex, float r, float g, float b, float a)
{

}

void VulkanCommandList::ClearDepthStencilAttachment(float depth, uint32_t stencil)
{

}

void VulkanCommandList::Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks)
{
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
