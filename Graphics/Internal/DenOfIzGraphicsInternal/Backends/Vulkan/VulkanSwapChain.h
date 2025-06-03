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

#include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
#include "VulkanSemaphore.h"
#include "VulkanTextureResource.h"

namespace DenOfIz
{

    class VulkanSwapChain final : public ISwapChain
    {
        VkQueue                  m_queue;
        SwapChainDesc            m_desc;
        VulkanContext           *m_context;
        VkSurfaceKHR             m_surface{ };
        VkSwapchainKHR           m_swapChain{ };
        std::vector<VkImage>     m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        VkColorSpaceKHR  m_colorSpace{ };
        VkPresentModeKHR m_presentMode{ };

        std::vector<std::unique_ptr<VulkanTextureResource>> m_renderTargets;

        uint32_t m_width  = 0;
        uint32_t m_height = 0;
        Viewport m_viewport;

    public:
        VulkanSwapChain( VulkanContext *context, const SwapChainDesc &desc );
        ~VulkanSwapChain( ) override;

        uint32_t      AcquireNextImage( ISemaphore *imageReadySemaphore ) override;
        PresentResult Present( const PresentDesc &presentDesc ) override;
        void          CreateSurface( );
        void          Resize( uint32_t width, uint32_t height ) override;
        Format        GetPreferredFormat( ) override;

        ITextureResource *GetRenderTarget( uint32_t image ) override;
        VkSwapchainKHR   *GetSwapChain( );
        const Viewport   &GetViewport( ) override;

    private:
        void CreateSwapChain( );
        void CreateImageView( VkImageView &imageView, const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags ) const;
        void ChooseExtent2D( const VkSurfaceCapabilitiesKHR &capabilities );
        void CreateSwapChainImages( VkFormat format );
        void Dispose( );
    };

} // namespace DenOfIz
