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

    class VulkanRenderPass : boost::noncopyable
    {
        VulkanContext *m_Context;
        VulkanPipeline *m_BoundPipeline;
        vk::Extent2D m_ViewportExtent;

        vk::Offset2D m_ViewportOffset;
        std::unique_ptr<VulkanLock> m_SwapChainImageAvailable;
        std::unique_ptr<VulkanLock> m_SwapChainImageRendered;
        std::vector<vk::WriteDescriptorSet> m_CurrentResources;
        vk::Image m_RenderTarget;
        vk::ImageView m_RenderTargetImageView;
        std::vector<RenderTargetAttachment> m_RenderTargetAttachments;
        vk::CommandBuffer m_CommandBuffer;
        bool m_HasIndexData;

        uint32_t m_SwapChainIndex{};
        uint32_t m_FrameIndex{};

        vk::Viewport m_Viewport{};
        vk::Rect2D m_ViewScissor{};

        RenderPassCreateInfo m_CreateInfo;

    public:
        explicit VulkanRenderPass( VulkanContext *context, const RenderPassCreateInfo &createInfo );

        void UpdateViewport( const uint32_t &width, const uint32_t &height );

        void Begin( std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f } );
        void BindPipeline( VulkanPipeline *pipeline );
        void BindResource( IResource *resource );
        // If using indices = 0 then the index buffer will be ignored.
        void BindIndexBuffer( IBufferResource *resource );
        // Use vertices = 0 safely when drawing using index buffers
        void BindVertexBuffer( IBufferResource *resource ) const;
        void Draw( const uint32_t &instanceCount, const uint32_t &vertexCount ) const;

        // Uncommon - Operations:
        void SetDepthBias( float constant, float clamp, float slope ) const;
        // --

        SubmitResult Submit( const std::vector<std::shared_ptr<VulkanLock>> &waitOnLock, VulkanLock *notifyFence );
        ~VulkanRenderPass();

    private:
        void AcquireNextImage();
        void CreateRenderTarget();
        VulkanImage
        CreateAttachment( const vk::Format &format, const vk::ImageUsageFlags &usage, const vk::ImageAspectFlags &aspect ) const;
        SubmitResult PresentPassToSwapChain() const;
    };

}

#endif
