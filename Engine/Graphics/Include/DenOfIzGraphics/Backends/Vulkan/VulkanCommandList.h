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
#include "VulkanPipeline.h"
#include "Resource/VulkanImageResource.h"
#include "Resource/VulkanBufferResource.h"
#include "../Interface/ICommandList.h"
#include "Resource/VulkanSemaphore.h"
#include "Resource/VulkanFence.h"
#include "Resource/VulkanPipelineBarrierHelper.h"

namespace DenOfIz
{

class VulkanCommandList : public ICommandList
{
private:
	CommandListCreateInfo m_createInfo;
	VulkanContext* m_context;
	VulkanPipeline* m_boundPipeline;

	vk::CommandBuffer m_commandBuffer;
	vk::Viewport m_viewport;
	vk::Rect2D m_scissorRect;
public:
	VulkanCommandList(VulkanContext* context, CommandListCreateInfo createInfo);

	void Begin() override;
	void BeginRendering(const RenderingInfo& renderingInfo) override;
	void EndRendering() override;
	void Execute(const ExecuteInfo& submitInfo) override;
	void BindPipeline(IPipeline* pipeline) override;
	void BindVertexBuffer(IBufferResource* buffer) override;
	void BindIndexBuffer(IBufferResource* buffer, const IndexType& indexType) override;
	void BindViewport(float offsetX, float offsetY, float width, float height) override;
	void BindScissorRect(float offsetX, float offsetY, float width, float height) override;
	void BindDescriptorTable(IDescriptorTable* table) override;
	void BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void* data) override;
	void BindBufferResource(IBufferResource* resource) override;
	void BindImageResource(IImageResource* resource) override;
	void SetDepthBias(float constantFactor, float clamp, float slopeFactor) override;
	void SetPipelineBarrier(const PipelineBarrier& barrier) override;
	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
	void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
	void TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout) override;
	void Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks) override;
};

}