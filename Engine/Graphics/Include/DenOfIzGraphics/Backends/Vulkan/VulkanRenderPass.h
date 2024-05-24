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
#ifdef BUILD_VK

#include "VulkanContext.h"
#include "VulkanUtilities.h"
#include "VulkanPipeline.h"
#include "../Interface/IRenderPass.h"
#include "Resource/VulkanSamplerResource.h"
#include "Resource/VulkanBufferResource.h"
#include "Resource/VulkanLock.h"

namespace DenOfIz
{

struct RenderTargetAttachment
{
	bool IsSwapChain; // Disposing is different when swap chain.
	VulkanImage Image;
};

class VulkanRenderPass : boost::noncopyable, public IRenderPass
{
	VulkanContext* m_context;
	VulkanPipeline* m_boundPipeline;
	vk::Extent2D m_viewportExtent;

	vk::Offset2D m_viewportOffset;
	std::unique_ptr<VulkanLock> m_swapChainImageAvailable;
	std::unique_ptr<VulkanLock> m_swapChainImageRendered;
	std::vector<vk::WriteDescriptorSet> m_currentResources;
	vk::Image m_renderTarget;
	vk::ImageView m_renderTargetImageView;
	std::vector<RenderTargetAttachment> m_renderTargetAttachments;
	vk::CommandBuffer m_commandBuffer;
	bool m_hasIndexData;

	uint32_t m_swapChainIndex{};
	uint32_t m_frameIndex{};

	vk::Viewport m_viewport{};
	vk::Rect2D m_viewScissor{};

	RenderPassCreateInfo m_createInfo;

public:
	explicit VulkanRenderPass(VulkanContext* context, const RenderPassCreateInfo& createInfo);

	void UpdateViewport(const uint32_t& width, const uint32_t& height) override;
	void SetDepthBias(float constant, float clamp, float slope) const override;

	void Begin(std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f }) override;
	void BindPipeline(IPipeline* pipeline) override;
	void BindResource(IResource* resource) override;
	// If using indices = 0 then the index buffer will be ignored.
	void BindIndexBuffer(IBufferResource* resource) override;
	// Use vertices = 0 safely when drawing using index buffers
	void BindVertexBuffer(IBufferResource* resource) const override;
	void Draw(const uint32_t& instanceCount, const uint32_t& vertexCount) const override;

	SubmitResult Submit(const std::vector<std::shared_ptr<ILock>>& waitOnLock, ILock* notifyFence);
	~VulkanRenderPass();

private:
	void AcquireNextImage();
	void CreateRenderTarget();
	VulkanImage
	CreateAttachment(const vk::Format& format, const vk::ImageUsageFlags& usage, const vk::ImageAspectFlags& aspect) const;
	SubmitResult PresentPassToSwapChain() const;
};

}

#endif
