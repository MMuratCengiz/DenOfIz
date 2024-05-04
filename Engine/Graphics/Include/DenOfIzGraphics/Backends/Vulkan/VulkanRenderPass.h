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

#pragma once

#include "VulkanContext.h"
#include "VulkanUtilities.h"
#include "VulkanPipeline.h"
#include "Resource/VulkanSamplerResource.h"
#include "Resource/VulkanBufferResource.h"
#include "Resource/VulkanLock.h"

namespace DenOfIz
{

struct RenderTargetAttachment
{
	bool IsSwapChain; // Disposing is different when swap chain.
	VulkanImage Instance;
};

class VulkanRenderPass
{
private:
	VulkanContext* context;
	VulkanPipeline* boundPipeline;
	vk::Extent2D viewportExtent;

	vk::Offset2D viewportOffset;
	// Per frame fields:
	std::vector<std::unique_ptr<VulkanLock>> swapChainImageAvailable;
	std::vector<std::unique_ptr<VulkanLock>> swapChainImageRendered;
	std::vector<std::vector<vk::WriteDescriptorSet>> currentResources;
	std::vector<RenderTargetAttachment> renderTargetAttachments;
	std::vector<vk::CommandBuffer> commandBuffers;
	std::vector<bool> hasIndexData;
	//--

	uint32_t swapChainIndex{};
	uint32_t frameIndex{};

	vk::Viewport viewport{};
	vk::Rect2D viewScissor{};

	RenderPassCreateInfo createInfo;

public:
	explicit VulkanRenderPass(VulkanContext* context, const RenderPassCreateInfo& createInfo);

	void UpdateViewport(const uint32_t& width, const uint32_t& height);

	void Begin(uint32_t frameIndex, std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });
	void BindPipeline(VulkanPipeline* pipeline);
	void BindResource(IResource* resource);
	// If using indices = 0 then the index buffer will be ignored.
	void BindIndexBuffer(IBufferResource* resource);
	// Use vertices = 0 safely when drawing using index buffers
	void BindVertexBuffer(IBufferResource* resource);
	void Draw(const uint32_t& instanceCount, const uint32_t& vertexCount);

	// Uncommon - Operations:
	void SetDepthBias(float constant, float clamp, float slope);
	// --

	SubmitResult Submit(std::vector<std::shared_ptr<VulkanLock> > waitOnLock, VulkanLock* notifyFence);
	~VulkanRenderPass();
private:
	void AcquireNextImage();
	void CreateRenderTarget();
	VulkanImage
	CreateAttachment(const vk::Format& format, const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspect);
	SubmitResult PresentPassToSwapChain();
};

}
