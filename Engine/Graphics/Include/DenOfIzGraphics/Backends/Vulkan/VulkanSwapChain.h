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
#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
#include "Resource/VulkanImageResource.h"
#include "Resource/VulkanSemaphore.h"
#include "VulkanEnumConverter.h"

namespace DenOfIz
{

    class VulkanSwapChain : public ISwapChain
    {
    private:
        SwapChainCreateInfo m_createInfo;
        VulkanContext *m_context;
        vk::SurfaceKHR m_surface;
        vk::SwapchainKHR m_swapChain;
        std::vector<vk::Image> m_swapChainImages;
        std::vector<vk::ImageView> m_swapChainImageViews;

        std::vector<std::unique_ptr<VulkanImageResource>> m_renderTargets;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

    public:
        VulkanSwapChain(VulkanContext *context, const SwapChainCreateInfo &createInfo);
        ~VulkanSwapChain() override;

        uint32_t AcquireNextImage(ISemaphore *imageReadySemaphore) override;
        void Resize(uint32_t width, uint32_t height) override;
        ImageFormat GetPreferredFormat() override;

        inline ITextureResource *GetRenderTarget(uint32_t frame) override { return m_renderTargets.at(frame).get(); }
        inline vk::SwapchainKHR *GetSwapChain() { return &m_swapChain; }
        inline Viewport GetViewport() override { return { 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height) }; }

    private:
        void CreateSwapChain();
        void CreateImageView(vk::ImageView &imageView, const vk::Image &image, const vk::Format &format, const vk::ImageAspectFlags &aspectFlags) const;
        void ChooseExtent2D(const vk::SurfaceCapabilitiesKHR &capabilities);
        void CreateSwapChainImages(vk::Format format);
        void Dispose() const;
    };

} // namespace DenOfIz

#endif
