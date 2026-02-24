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

#include <DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h>
#include "VulkanSemaphore.h"
#include "VulkanTexture.h"

namespace DenOfIz
{

    class VulkanSwapChain final : public ISwapChain
    {
        VkQueue                  m_queue;
        DenOfIz_SwapChainDesc    m_desc;
        VulkanContext           *m_context;
        VkSurfaceKHR             m_surface{ };
        VkSwapchainKHR           m_swapChain{ };
        std::vector<VkImage>     m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        VkColorSpaceKHR  m_colorSpace{ };
        VkPresentModeKHR m_presentMode{ };

        std::vector<std::unique_ptr<VulkanTexture>> m_renderTargets;

        std::vector<VkSemaphore> m_acquireSemaphores;
        std::vector<VkSemaphore> m_renderCompleteSemaphores;
        uint32_t                 m_currentSemaphoreIndex           = 0;
        VkSemaphore              m_lastUsedAcquireSemaphore        = VK_NULL_HANDLE;
        VkSemaphore              m_lastUsedRenderCompleteSemaphore = VK_NULL_HANDLE;

        uint32_t         m_width  = 0;
        uint32_t         m_height = 0;
        DenOfIz_Viewport m_viewport;

    public:
        VulkanSwapChain( VulkanContext *context, const DenOfIz_SwapChainDesc &desc );
        ~VulkanSwapChain( ) override;

        uint32_t              AcquireNextImage( ) override;
        DenOfIz_PresentResult Present( uint32_t imageIndex ) override;
        void                  CreateSurface( );
        void                  Resize( uint32_t width, uint32_t height ) override;
        DenOfIz_Format        GetPreferredFormat( ) override;

        ITexture               *GetRenderTarget( uint32_t image ) override;
        VkSwapchainKHR         *GetSwapChain( );
        const DenOfIz_Viewport &GetViewport( ) override;
        VkSemaphore             GetCurrentAcquireSemaphore( ) const;
        VkSemaphore             GetCurrentRenderCompleteSemaphore( ) const;
        void                    SetLastUsedRenderCompleteSemaphore( VkSemaphore semaphore );

    private:
        void CreateSwapChain( );
        void CreateImageView( VkImageView &imageView, const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags ) const;
        void ChooseExtent2D( const VkSurfaceCapabilitiesKHR &capabilities );
        void CreateSwapChainImages( VkFormat format );
        void CreateSemaphores( );
        void DestroySemaphores( );
        void Dispose( );
    };

} // namespace DenOfIz
