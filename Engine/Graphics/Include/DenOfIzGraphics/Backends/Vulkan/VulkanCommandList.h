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

#pragma once

#include "VulkanContext.h"
#include "../Interface/ICommandList.h"
#include "Resource/VulkanSemaphore.h"
#include "Resource/VulkanFence.h"

namespace DenOfIz
{

class VulkanCommandList : public ICommandList
{
private:
	VulkanContext* m_context;
public:
	VulkanCommandList(VulkanContext* context);

	void Reset() override;
	void Begin() override;
	void End() override;
	void Submit(IFence* notify) override;
	uint32_t AcquireNextImage(ISwapChain* swapChain, ISemaphore* lock) override;
	void BindPipeline(IPipeline* pipeline) override;
	void BindVertexBuffer(IBufferResource* buffer) override;
	void BindIndexBuffer(IBufferResource* buffer) override;
	void BindViewport(float x, float y, float width, float height) override;
	void BindScissorRect(float x, float y, float width, float height) override;
	void BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void* data) override;
	void BindBufferResource(IBufferResource* resource) override;
	void BindImageResource(IImageResource* resource) override;
	void SetDepthBias(float constantFactor, float clamp, float slopeFactor) override;
	void ClearColor(float r, float g, float b, float a) override;
	void ClearDepthStencil(float depth, uint32_t stencil) override;
	void SetImageBarrier(IImageResource* resource, ImageBarrier barrier) override;
	void SetBufferBarrier(IBufferResource* resource, BufferBarrier barrier) override;
	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
	void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
	void CopyBuffer(IBufferResource* src, IBufferResource* dst, uint32_t size) override;
	void TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout) override;
	void ClearColorAttachment(uint32_t attachmentIndex, float r, float g, float b, float a) override;
	void ClearDepthStencilAttachment(float depth, uint32_t stencil) override;
	void Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks) override;
};

}