/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h>
#include "VulkanSemaphore.h"
#include "VulkanTexture.h"

namespace DenOfIz
{

    class VulkanSwapChain final : public ISwapChain
    {
        VkQueue                  m_queue = VK_NULL_HANDLE;
        DenOfIz_SwapChainDesc    m_desc;
        VulkanContext           *m_context;
        VkSurfaceKHR             m_surface   = VK_NULL_HANDLE;
        VkSwapchainKHR           m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage>     m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        VkColorSpaceKHR  m_colorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;

        std::vector<std::unique_ptr<VulkanTexture>> m_renderTargets;
        std::unique_ptr<VulkanTexture>              m_depthBuffer;

        std::vector<VkSemaphore> m_acquireSemaphores;
        std::vector<VkSemaphore> m_presentSemaphores;
        std::vector<VkFence> m_acquireFences;
        uint32_t             m_acquireSemaphoreIndex = 0;
        uint32_t             m_acquiredSlot          = 0;

        VkSemaphore    m_pendingAcquireSemaphore = VK_NULL_HANDLE;
        bool           m_imageAcquired           = false;
        uint32_t       m_acquiredImageIndex      = 0;
        DenOfIz_Result m_lastAcquireResult       = DENOFIZ_ERROR_OUT_OF_DATE;
        bool           m_needsResize             = false;
        bool           m_surfaceLost             = false;

        uint32_t         m_width  = 0;
        uint32_t         m_height = 0;
        uint32_t         m_windowWidth  = 0;
        uint32_t         m_windowHeight = 0;
        DenOfIz_Viewport m_viewport{ };

    public:
        VulkanSwapChain( VulkanContext *context, const DenOfIz_SwapChainDesc &desc );
        ~VulkanSwapChain( ) override;

        DenOfIz_Result        AcquireNextImage( uint32_t &outImageIndex ) override;
        DenOfIz_Result        Present( ) override;
        DenOfIz_Result        Resize( uint32_t width, uint32_t height ) override;
        DenOfIz_Format        GetPreferredFormat( ) override;

        ITexture                    *GetRenderTarget( uint32_t image ) override;
        ITexture                    *GetDepthBuffer( ) override;
        VkSwapchainKHR              *GetSwapChain( );
        const DenOfIz_Viewport      &GetViewport( ) override;
        const DenOfIz_SwapChainDesc &GetDesc( ) override;

        [[nodiscard]] VkQueue GetQueue( ) const;
        VkSemaphore ConsumeAcquireSemaphore( );

    private:
        bool           CreateSurface( );
        void           DestroySurface( );
        void           ChoosePresentMode( );
        DenOfIz_Result Recreate( uint32_t requestedWidth, uint32_t requestedHeight );
        VkExtent2D     ChooseExtent( const VkSurfaceCapabilitiesKHR &capabilities, uint32_t requestedWidth, uint32_t requestedHeight ) const;
        bool           SurfaceExtentChanged( ) const;
        bool           WindowSizeChanged( uint32_t &outWidth, uint32_t &outHeight ) const;
        void           CreateImageView( VkImageView &imageView, const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags ) const;
        void           CreateSwapChainImages( VkFormat format );
        void           CreateDepthBuffer( );
        void           CreateSemaphores( );
        void           DestroySemaphores( );
        void           RecreateAcquireSemaphore( uint32_t index );
        void           DisposeImages( );
    };

} // namespace DenOfIz
