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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanSwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanSwapChain::VulkanSwapChain( VulkanContext *context, const DenOfIz_SwapChainDesc &desc ) : m_desc( desc ), m_context( context )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.WindowHandle ) )
    {
        spdlog::error( "WindowHandle cannot be null" );
    }
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.CommandQueue ) )
    {
        spdlog::error( "CommandQueue cannot be null" );
    }
    m_queue = dynamic_cast<VulkanCommandQueue *>( DENOFIZ_FROM_HANDLE( ICommandQueue, desc.CommandQueue ) )->GetQueue( );

    CreateSurface( );
    CreateSwapChain( );
    CreateSemaphores( );
}

void VulkanSwapChain::CreateSurface( )
{
    DZ_NOT_NULL( m_context );
    uint32_t windowId = 0;
    DenOfIz_GraphicsWindowHandle_GetSDLWindowId( m_desc.WindowHandle, &windowId );
    SDL_Window *sdlWindow = SDL_GetWindowFromID( windowId );
    if ( !SDL_Vulkan_CreateSurface( sdlWindow, m_context->Instance, nullptr, &m_surface ) )
    {
        spdlog::critical( "Failed to create Vulkan surface: {}", SDL_GetError( ) );
    }
    uint32_t                        count;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, nullptr );
    surfaceFormats.resize( count );
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, surfaceFormats.data( ) );
    std::vector<VkPresentModeKHR> presentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR( m_context->PhysicalDevice, m_surface, &count, nullptr );
    presentModes.resize( count );
    vkGetPhysicalDeviceSurfacePresentModesKHR( m_context->PhysicalDevice, m_surface, &count, presentModes.data( ) );

    VkPresentModeKHR desiredMode = VK_PRESENT_MODE_FIFO_KHR; // Default to FIFO (vsync)
    if ( m_desc.AllowTearing )
    {
        desiredMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if ( m_desc.NumBuffers >= 3 )
    {
        desiredMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for ( const auto mode : presentModes )
    {
        if ( mode == desiredMode )
        {
            m_presentMode = mode;
            break;
        }
    }
    if ( m_presentMode == VK_PRESENT_MODE_FIFO_KHR && m_desc.AllowTearing )
    {
        for ( const auto mode : presentModes )
        {
            if ( mode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                m_presentMode = mode;
                break;
            }
        }
    }
    m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

void VulkanSwapChain::CreateSwapChain( )
{
    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK_RESULT( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_context->PhysicalDevice, m_surface, &capabilities ) );

    ChooseExtent2D( capabilities );

    uint32_t desiredImageCount = m_desc.NumBuffers;
    uint32_t imageCount        = capabilities.minImageCount + 1;

    if ( capabilities.maxImageCount > 0 )
    {
        imageCount = std::min( imageCount, capabilities.maxImageCount );
    }

    if ( imageCount < desiredImageCount )
    {
        spdlog::debug( "Requested buffer count {} is not supported. Using {}", desiredImageCount, imageCount );
        desiredImageCount = imageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = desiredImageCount;
    createInfo.imageFormat      = DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.BackBufferFormat );
    createInfo.imageColorSpace  = m_colorSpace;
    createInfo.imageExtent      = VkExtent2D{ m_width, m_height };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = DenOfIz_VulkanEnumConverter_ConvertTextureUsage( DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT, m_desc.ImageUsages );
    m_viewport                  = { 0.0f, 0.0f, static_cast<float>( m_width ), static_cast<float>( m_height ) };

    const uint32_t qfIndexes[ 2 ] = { m_context->QueueFamilies.at( VulkanQueueType::Graphics ).Index, m_context->QueueFamilies.at( VulkanQueueType::Presentation ).Index };

    if ( qfIndexes[ 0 ] != qfIndexes[ 1 ] )
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = qfIndexes;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = m_presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE; // Initial creation has no old swap chain

    const VkResult result = vkCreateSwapchainKHR( m_context->LogicalDevice, &createInfo, nullptr, &m_swapChain );
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "Failed to create initial swap chain: {}", static_cast<int>( result ) );
        return;
    }

    CreateSwapChainImages( DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.BackBufferFormat ) );
}

void VulkanSwapChain::CreateSwapChainImages( const VkFormat format )
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR( m_context->LogicalDevice, m_swapChain, &imageCount, nullptr );
    m_swapChainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( m_context->LogicalDevice, m_swapChain, &imageCount, m_swapChainImages.data( ) );
    m_swapChainImageViews.resize( m_swapChainImages.size( ) );

    int index = 0;
    for ( auto image : m_swapChainImages )
    {
        CreateImageView( m_swapChainImageViews[ index ], image, format, VK_IMAGE_ASPECT_COLOR_BIT );
        DenOfIz_TextureDesc desc{ };
        desc.Width     = m_width;
        desc.Height    = m_height;
        desc.Depth     = 1;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.Format    = m_desc.BackBufferFormat;
        m_renderTargets.push_back( std::make_unique<VulkanTexture>( image, m_swapChainImageViews[ index ], format, VK_IMAGE_ASPECT_COLOR_BIT, desc ) );
        index++;
    }
}

void VulkanSwapChain::ChooseExtent2D( const VkSurfaceCapabilitiesKHR &capabilities )
{
    if ( m_desc.Width != 0 || m_desc.Height != 0 )
    {
        m_width  = m_desc.Width;
        m_height = m_desc.Height;
        return;
    }

    if ( capabilities.currentExtent.width != UINT32_MAX )
    {
        m_width  = capabilities.currentExtent.width;
        m_height = capabilities.currentExtent.height;
        return;
    }

    DenOfIz_GraphicsWindowSurface surface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
    m_width  = std::clamp( surface.Width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
    m_height = std::clamp( surface.Height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );
}

void VulkanSwapChain::CreateImageView( VkImageView &imageView, const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags ) const
{
    VkImageViewCreateInfo imageViewCreateInfo{ };
    imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image                           = image;
    imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format                          = format;
    imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask     = aspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;

    vkCreateImageView( m_context->LogicalDevice, &imageViewCreateInfo, nullptr, &imageView );
}

void VulkanSwapChain::Dispose( )
{
    for ( const auto &imageView : m_swapChainImageViews )
    {
        vkDestroyImageView( m_context->LogicalDevice, imageView, nullptr );
    }

    m_swapChainImageViews.clear( );
    m_swapChainImages.clear( );
    m_renderTargets.clear( );
}

uint32_t VulkanSwapChain::AcquireNextImage( )
{
    if ( m_width == 0 || m_height == 0 )
    {
        spdlog::warn( "Cannot AcquireNextImage on Vulkan, width == 0 || height == 0, window might be minimized." );
        return 0;
    }

    uint32_t          nextImage        = 0;
    const VkSemaphore acquireSemaphore = m_acquireSemaphores[ m_currentSemaphoreIndex ];

    constexpr uint64_t timeout = 60 * 1000 * 1000;
    const VkResult     result  = vkAcquireNextImageKHR( m_context->LogicalDevice, m_swapChain, timeout, acquireSemaphore, VK_NULL_HANDLE, &nextImage );
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        return 0;
    }

    m_lastUsedAcquireSemaphore = acquireSemaphore;
    m_context->AutoSync->AcquireImage( this, m_renderTargets[ nextImage ].get( ) );
    m_currentSemaphoreIndex = ( m_currentSemaphoreIndex + 1 ) % m_acquireSemaphores.size( );
    return nextImage;
}

DenOfIz_PresentResult VulkanSwapChain::Present( uint32_t imageIndex )
{
    VkPresentInfoKHR presentInfo{ };
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &m_lastUsedRenderCompleteSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapChain;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    switch ( vkQueuePresentKHR( m_queue, &presentInfo ) )
    {
    case VK_SUCCESS:
        return DENOFIZ_PRESENT_RESULT_SUCCESS;
    case VK_SUBOPTIMAL_KHR:
        return DENOFIZ_PRESENT_RESULT_SUBOPTIMAL;
    case VK_ERROR_OUT_OF_DATE_KHR:
        return DENOFIZ_PRESENT_RESULT_SUBOPTIMAL;
    case VK_ERROR_DEVICE_LOST:
        return DENOFIZ_PRESENT_RESULT_DEVICE_LOST;
    default:
        return DENOFIZ_PRESENT_RESULT_DEVICE_LOST;
    }
}

VulkanSwapChain::~VulkanSwapChain( )
{
    Dispose( );
    DestroySemaphores( );
    vkDestroySwapchainKHR( m_context->LogicalDevice, m_swapChain, nullptr );
    vkDestroySurfaceKHR( m_context->Instance, m_surface, nullptr );
}

DenOfIz_Format VulkanSwapChain::GetPreferredFormat( )
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, nullptr );
    std::vector<VkSurfaceFormatKHR> formats( count );
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, formats.data( ) );
    // Todo missing cases
    switch ( formats[ 0 ].format )
    {
    case VK_FORMAT_B8G8R8A8_UNORM:
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB;
    default:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    }
}

void VulkanSwapChain::Resize( const uint32_t width, const uint32_t height )
{
    if ( width == 0 || height == 0 )
    {
        return;
    }

    const VkSwapchainKHR oldSwapChain = m_swapChain;
    m_swapChain                       = VK_NULL_HANDLE;

    m_width    = width;
    m_height   = height;
    m_viewport = DenOfIz_Viewport{ 0.0f, 0.0f, static_cast<float>( m_width ), static_cast<float>( m_height ) };

    Dispose( );

    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK_RESULT( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_context->PhysicalDevice, m_surface, &capabilities ) );

    uint32_t desiredImageCount = m_desc.NumBuffers;
    uint32_t imageCount        = capabilities.minImageCount + 1;

    if ( capabilities.maxImageCount > 0 )
    {
        imageCount = std::min( imageCount, capabilities.maxImageCount );
    }

    if ( imageCount < desiredImageCount )
    {
        spdlog::debug( "Requested buffer count {} is not supported. Using {}", desiredImageCount, imageCount );
        desiredImageCount = imageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = desiredImageCount;
    createInfo.imageFormat      = DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.BackBufferFormat );
    createInfo.imageColorSpace  = m_colorSpace;
    createInfo.imageExtent      = VkExtent2D{ m_width, m_height };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = DenOfIz_VulkanEnumConverter_ConvertTextureUsage( DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT, m_desc.ImageUsages );

    const uint32_t qfIndexes[ 2 ] = { m_context->QueueFamilies.at( VulkanQueueType::Graphics ).Index, m_context->QueueFamilies.at( VulkanQueueType::Presentation ).Index };

    if ( qfIndexes[ 0 ] != qfIndexes[ 1 ] )
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = qfIndexes;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = m_presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = oldSwapChain;

    const VkResult result = vkCreateSwapchainKHR( m_context->LogicalDevice, &createInfo, nullptr, &m_swapChain );
    if ( result != VK_SUCCESS )
    {
        // Create fresh swapChain
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        VK_CHECK_RESULT( vkCreateSwapchainKHR( m_context->LogicalDevice, &createInfo, nullptr, &m_swapChain ) );
    }

    if ( oldSwapChain != VK_NULL_HANDLE )
    {
        vkDestroySwapchainKHR( m_context->LogicalDevice, oldSwapChain, nullptr );
    }

    CreateSwapChainImages( DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.BackBufferFormat ) );
}

ITexture *VulkanSwapChain::GetRenderTarget( const uint32_t image )
{
    return m_renderTargets.at( image ).get( );
}

VkSwapchainKHR *VulkanSwapChain::GetSwapChain( )
{
    return &m_swapChain;
}

const DenOfIz_Viewport &VulkanSwapChain::GetViewport( )
{
    return m_viewport;
}

VkSemaphore VulkanSwapChain::GetCurrentAcquireSemaphore( ) const
{
    return m_lastUsedAcquireSemaphore;
}

VkSemaphore VulkanSwapChain::GetCurrentRenderCompleteSemaphore( ) const
{
    return m_renderCompleteSemaphores[ m_currentSemaphoreIndex ];
}

void VulkanSwapChain::SetLastUsedRenderCompleteSemaphore( VkSemaphore semaphore )
{
    m_lastUsedRenderCompleteSemaphore = semaphore;
}

void VulkanSwapChain::CreateSemaphores( )
{
    const uint32_t imageCount = static_cast<uint32_t>( m_swapChainImages.size( ) );
    m_acquireSemaphores.resize( imageCount );
    m_renderCompleteSemaphores.resize( imageCount );

    VkSemaphoreCreateInfo semaphoreCreateInfo{ };
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for ( uint32_t i = 0; i < imageCount; ++i )
    {
        VK_CHECK_RESULT( vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &m_acquireSemaphores[ i ] ) );
        VK_CHECK_RESULT( vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &m_renderCompleteSemaphores[ i ] ) );
    }
}

void VulkanSwapChain::DestroySemaphores( )
{
    for ( const VkSemaphore semaphore : m_acquireSemaphores )
    {
        vkDestroySemaphore( m_context->LogicalDevice, semaphore, nullptr );
    }
    for ( const VkSemaphore semaphore : m_renderCompleteSemaphores )
    {
        vkDestroySemaphore( m_context->LogicalDevice, semaphore, nullptr );
    }
    m_acquireSemaphores.clear( );
    m_renderCompleteSemaphores.clear( );
}
