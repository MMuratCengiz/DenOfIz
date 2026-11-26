/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanSwapChain.h"
#include <algorithm>
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanSwapChain::VulkanSwapChain( VulkanContext *context, const DenOfIz_SwapChainDesc &desc ) : m_desc( desc ), m_context( context )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.WindowHandle ) )
    {
        spdlog::error( "[VulkanSwapChain] WindowHandle cannot be null" );
        return;
    }
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.CommandQueue ) )
    {
        spdlog::error( "[VulkanSwapChain] CommandQueue cannot be null" );
        return;
    }
    const auto *commandQueue = dynamic_cast<VulkanCommandQueue *>( DENOFIZ_FROM_HANDLE( ICommandQueue, desc.CommandQueue ) );
    if ( commandQueue == nullptr )
    {
        spdlog::error( "[VulkanSwapChain] CommandQueue is not a Vulkan command queue" );
        return;
    }
    m_queue = commandQueue->GetQueue( );

    if ( !CreateSurface( ) )
    {
        return;
    }

    VkBool32 presentSupported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR( m_context->PhysicalDevice, commandQueue->GetQueueFamilyIndex( ), m_surface, &presentSupported );
    if ( presentSupported != VK_TRUE )
    {
        spdlog::error( "[VulkanSwapChain] CommandQueue cannot present to this window" );
    }

    Recreate( m_desc.Width, m_desc.Height );
}

VulkanSwapChain::~VulkanSwapChain( )
{
    if ( m_context->LogicalDevice != VK_NULL_HANDLE && !m_context->IsDeviceLost )
    {
        m_context->CheckDeviceLost( vkDeviceWaitIdle( m_context->LogicalDevice ), "vkDeviceWaitIdle" );
    }
    m_context->AutoSync->RemoveSwapChain( this );
    DisposeImages( );
    m_renderTargets.clear( );
    m_depthBuffer.reset( );
    DestroySemaphores( );
    if ( m_swapChain != VK_NULL_HANDLE )
    {
        vkDestroySwapchainKHR( m_context->LogicalDevice, m_swapChain, nullptr );
        m_swapChain = VK_NULL_HANDLE;
    }
    DestroySurface( );
}

bool VulkanSwapChain::CreateSurface( )
{
    DZ_NOT_NULL( m_context );
    uint32_t windowId = 0;
    DenOfIz_GraphicsWindowHandle_GetSDLWindowId( m_desc.WindowHandle, &windowId );
    SDL_Window *sdlWindow = SDL_GetWindowFromID( windowId );
    if ( sdlWindow == nullptr )
    {
        spdlog::critical( "[VulkanSwapChain] Window no longer exists: {}", SDL_GetError( ) );
        return false;
    }
    m_surface = VK_NULL_HANDLE;
    if ( !SDL_Vulkan_CreateSurface( sdlWindow, m_context->Instance, nullptr, &m_surface ) || m_surface == VK_NULL_HANDLE )
    {
        spdlog::critical( "[VulkanSwapChain] Failed to create Vulkan surface: {}", SDL_GetError( ) );
        m_surface = VK_NULL_HANDLE;
        return false;
    }
    m_surfaceLost = false;
    ChoosePresentMode( );
    return true;
}

void VulkanSwapChain::DestroySurface( )
{
    if ( m_surface != VK_NULL_HANDLE )
    {
        vkDestroySurfaceKHR( m_context->Instance, m_surface, nullptr );
        m_surface = VK_NULL_HANDLE;
    }
}

void VulkanSwapChain::ChoosePresentMode( )
{
    uint32_t                      count = 0;
    std::vector<VkPresentModeKHR> presentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR( m_context->PhysicalDevice, m_surface, &count, nullptr );
    presentModes.resize( count );
    if ( count > 0 )
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR( m_context->PhysicalDevice, m_surface, &count, presentModes.data( ) );
        presentModes.resize( count );
    }

    const auto isSupported = [ & ]( const VkPresentModeKHR mode ) { return std::find( presentModes.begin( ), presentModes.end( ), mode ) != presentModes.end( ); };

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if ( m_desc.PresentMode == DENOFIZ_PRESENT_MODE_IMMEDIATE && isSupported( VK_PRESENT_MODE_IMMEDIATE_KHR ) )
    {
        m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if ( m_desc.PresentMode != DENOFIZ_PRESENT_MODE_FIFO && isSupported( VK_PRESENT_MODE_MAILBOX_KHR ) )
    {
        m_presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

VkExtent2D VulkanSwapChain::ChooseExtent( const VkSurfaceCapabilitiesKHR &capabilities, uint32_t requestedWidth, uint32_t requestedHeight ) const
{
    if ( capabilities.currentExtent.width != UINT32_MAX )
    {
        return capabilities.currentExtent;
    }

    if ( requestedWidth == 0 || requestedHeight == 0 )
    {
        DenOfIz_GraphicsWindowSurface surface{ };
        DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
        requestedWidth  = surface.Width;
        requestedHeight = surface.Height;
    }
    if ( requestedWidth == 0 || requestedHeight == 0 )
    {
        return VkExtent2D{ 0, 0 };
    }

    VkExtent2D extent{ };
    extent.width  = std::clamp( requestedWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
    extent.height = std::clamp( requestedHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );
    return extent;
}

DenOfIz_Result VulkanSwapChain::Recreate( const uint32_t requestedWidth, const uint32_t requestedHeight )
{
    if ( m_context->IsDeviceLost )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( m_queue == VK_NULL_HANDLE )
    {
        return DENOFIZ_ERROR_INITIALIZATION;
    }

    VkSurfaceCapabilitiesKHR capabilities{ };
    VkResult                 result = VK_ERROR_SURFACE_LOST_KHR;
    for ( uint32_t attempt = 0; attempt < 2; ++attempt )
    {
        if ( m_surface == VK_NULL_HANDLE || m_surfaceLost )
        {
            if ( m_context->CheckDeviceLost( vkDeviceWaitIdle( m_context->LogicalDevice ), "vkDeviceWaitIdle" ) )
            {
                return DENOFIZ_ERROR_DEVICE_LOST;
            }
            m_context->AutoSync->RemoveSwapChain( this );
            DisposeImages( );
            DestroySemaphores( );
            if ( m_swapChain != VK_NULL_HANDLE )
            {
                vkDestroySwapchainKHR( m_context->LogicalDevice, m_swapChain, nullptr );
                m_swapChain = VK_NULL_HANDLE;
            }
            m_imageAcquired           = false;
            m_pendingAcquireSemaphore = VK_NULL_HANDLE;
            DestroySurface( );
            if ( !CreateSurface( ) )
            {
                m_surfaceLost = true;
                return DENOFIZ_ERROR_SURFACE_LOST;
            }
        }

        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_context->PhysicalDevice, m_surface, &capabilities );
        if ( result != VK_ERROR_SURFACE_LOST_KHR )
        {
            break;
        }
        m_surfaceLost = true;
    }
    if ( m_context->CheckDeviceLost( result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR" ) )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanSwapChain] Failed to query surface capabilities: {}", static_cast<int>( result ) );
        return result == VK_ERROR_SURFACE_LOST_KHR ? DENOFIZ_ERROR_SURFACE_LOST : DENOFIZ_ERROR_UNKNOWN;
    }

    const VkExtent2D extent = ChooseExtent( capabilities, requestedWidth, requestedHeight );
    if ( extent.width == 0 || extent.height == 0 )
    {
        return DENOFIZ_NOT_READY;
    }

    if ( m_swapChain != VK_NULL_HANDLE && !m_needsResize && extent.width == m_width && extent.height == m_height )
    {
        DenOfIz_GraphicsWindowSurface currentWindowSurface{ };
        DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &currentWindowSurface );
        m_windowWidth  = currentWindowSurface.Width;
        m_windowHeight = currentWindowSurface.Height;
        return DENOFIZ_SUCCESS;
    }

    if ( m_context->CheckDeviceLost( vkDeviceWaitIdle( m_context->LogicalDevice ), "vkDeviceWaitIdle" ) )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }

    uint32_t imageCount = std::max( m_desc.NumBuffers, capabilities.minImageCount );
    if ( capabilities.maxImageCount > 0 )
    {
        imageCount = std::min( imageCount, capabilities.maxImageCount );
    }

    const VkFormat    format       = DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.BackBufferFormat );
    VkImageUsageFlags imageUsage   = DenOfIz_VulkanEnumConverter_ConvertTextureUsage( DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT, m_desc.ImageUsages );
    const auto        unsupported  = imageUsage & ~capabilities.supportedUsageFlags;
    if ( unsupported != 0 )
    {
        spdlog::warn( "[VulkanSwapChain] Unsupported back buffer usages dropped: 0x{:x}", static_cast<uint32_t>( unsupported ) );
        imageUsage &= capabilities.supportedUsageFlags;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if ( ( capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ) == 0 )
    {
        for ( const auto candidate : { VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR } )
        {
            if ( capabilities.supportedCompositeAlpha & candidate )
            {
                compositeAlpha = candidate;
                break;
            }
        }
    }

    VkSwapchainCreateInfoKHR createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = format;
    createInfo.imageColorSpace  = m_colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = imageUsage;

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
    createInfo.compositeAlpha = compositeAlpha;
    createInfo.presentMode    = m_presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = m_swapChain;

    const VkSwapchainKHR oldSwapChain = m_swapChain;
    VkSwapchainKHR       newSwapChain = VK_NULL_HANDLE;
    result                            = vkCreateSwapchainKHR( m_context->LogicalDevice, &createInfo, nullptr, &newSwapChain );

    m_context->AutoSync->RemoveSwapChain( this );
    DisposeImages( );
    DestroySemaphores( );
    m_swapChain               = VK_NULL_HANDLE;
    m_imageAcquired           = false;
    m_pendingAcquireSemaphore = VK_NULL_HANDLE;
    m_lastAcquireResult       = DENOFIZ_ERROR_OUT_OF_DATE;
    if ( oldSwapChain != VK_NULL_HANDLE )
    {
        vkDestroySwapchainKHR( m_context->LogicalDevice, oldSwapChain, nullptr );
    }

    if ( result != VK_SUCCESS && result != VK_ERROR_DEVICE_LOST && oldSwapChain != VK_NULL_HANDLE )
    {
        newSwapChain            = VK_NULL_HANDLE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        result                  = vkCreateSwapchainKHR( m_context->LogicalDevice, &createInfo, nullptr, &newSwapChain );
    }
    if ( m_context->CheckDeviceLost( result, "vkCreateSwapchainKHR" ) )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanSwapChain] Failed to create swap chain ({}x{}, {} images): {}", extent.width, extent.height, imageCount, static_cast<int>( result ) );
        m_needsResize = true;
        if ( result == VK_ERROR_SURFACE_LOST_KHR )
        {
            m_surfaceLost = true;
            return DENOFIZ_ERROR_SURFACE_LOST;
        }
        if ( result == VK_ERROR_OUT_OF_HOST_MEMORY )
        {
            return DENOFIZ_ERROR_OUT_OF_HOST_MEMORY;
        }
        if ( result == VK_ERROR_OUT_OF_DEVICE_MEMORY )
        {
            return DENOFIZ_ERROR_OUT_OF_DEVICE_MEMORY;
        }
        return result == VK_ERROR_FORMAT_NOT_SUPPORTED ? DENOFIZ_ERROR_FORMAT_NOT_SUPPORTED : DENOFIZ_ERROR_INITIALIZATION;
    }

    DenOfIz_GraphicsWindowSurface windowSurface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &windowSurface );
    m_windowWidth  = windowSurface.Width;
    m_windowHeight = windowSurface.Height;

    m_swapChain   = newSwapChain;
    m_width       = extent.width;
    m_height      = extent.height;
    m_viewport    = DenOfIz_Viewport{ 0.0f, 0.0f, static_cast<float>( m_width ), static_cast<float>( m_height ) };
    m_needsResize = false;

    CreateSwapChainImages( format );
    CreateSemaphores( );
    CreateDepthBuffer( );

    m_desc.Width      = m_width;
    m_desc.Height     = m_height;
    m_desc.NumBuffers = static_cast<uint32_t>( m_swapChainImages.size( ) );

    return DENOFIZ_SUCCESS;
}

void VulkanSwapChain::CreateSwapChainImages( const VkFormat format )
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR( m_context->LogicalDevice, m_swapChain, &imageCount, nullptr );
    m_swapChainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( m_context->LogicalDevice, m_swapChain, &imageCount, m_swapChainImages.data( ) );
    m_swapChainImages.resize( imageCount );
    m_swapChainImageViews.assign( m_swapChainImages.size( ), VK_NULL_HANDLE );

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
        if ( static_cast<size_t>( index ) < m_renderTargets.size( ) )
        {
            m_renderTargets[ index ]->UpdateExternal( image, m_swapChainImageViews[ index ], desc );
        }
        else
        {
            m_renderTargets.push_back( std::make_unique<VulkanTexture>( image, m_swapChainImageViews[ index ], format, VK_IMAGE_ASPECT_COLOR_BIT, desc ) );
        }
        index++;
    }
}

void VulkanSwapChain::CreateDepthBuffer( )
{
    if ( m_desc.DepthBufferFormat == DENOFIZ_FORMAT_UNDEFINED )
    {
        return;
    }
    if ( m_depthBuffer != nullptr && m_depthBuffer->GetWidth( ) == m_width && m_depthBuffer->GetHeight( ) == m_height )
    {
        return;
    }
    m_depthBuffer.reset( );
    DenOfIz_TextureDesc desc{ };
    desc.Width                 = m_width;
    desc.Height                = m_height;
    desc.Depth                 = 1;
    desc.ArraySize             = 1;
    desc.MipLevels             = 1;
    desc.Format                = m_desc.DepthBufferFormat;
    desc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    desc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
    desc.ClearDepthStencilHint = { 1.0f, 0.0f };
    desc.DebugName             = DENOFIZ_STRING( "SwapChain Depth Buffer" );
    m_depthBuffer              = std::make_unique<VulkanTexture>( m_context, desc );
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

    VK_CHECK_RESULT( vkCreateImageView( m_context->LogicalDevice, &imageViewCreateInfo, nullptr, &imageView ) );
}

void VulkanSwapChain::DisposeImages( )
{
    for ( const auto &renderTarget : m_renderTargets )
    {
        m_context->AutoSync->RemoveTextureResource( renderTarget.get( ) );
    }
    for ( const auto &imageView : m_swapChainImageViews )
    {
        if ( imageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView( m_context->LogicalDevice, imageView, nullptr );
        }
    }

    m_swapChainImageViews.clear( );
    m_swapChainImages.clear( );
}

bool VulkanSwapChain::SurfaceExtentChanged( ) const
{
    VkSurfaceCapabilitiesKHR capabilities{ };
    if ( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_context->PhysicalDevice, m_surface, &capabilities ) != VK_SUCCESS )
    {
        return true;
    }
    if ( capabilities.currentExtent.width == UINT32_MAX )
    {
        return false;
    }
    if ( capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0 )
    {
        return false;
    }
    return capabilities.currentExtent.width != m_width || capabilities.currentExtent.height != m_height;
}

bool VulkanSwapChain::WindowSizeChanged( uint32_t &outWidth, uint32_t &outHeight ) const
{
    DenOfIz_GraphicsWindowSurface windowSurface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &windowSurface );
    outWidth  = windowSurface.Width;
    outHeight = windowSurface.Height;
    return outWidth != m_windowWidth || outHeight != m_windowHeight;
}

DenOfIz_Result VulkanSwapChain::AcquireNextImage( uint32_t &outImageIndex )
{
    outImageIndex = 0;
    if ( m_context->IsDeviceLost )
    {
        m_lastAcquireResult = DENOFIZ_ERROR_DEVICE_LOST;
        return m_lastAcquireResult;
    }
    if ( m_imageAcquired )
    {
        outImageIndex = m_acquiredImageIndex;
        return DENOFIZ_SUCCESS;
    }
    uint32_t windowWidth  = 0;
    uint32_t windowHeight = 0;
    if ( m_swapChain == VK_NULL_HANDLE || m_surfaceLost || m_needsResize || WindowSizeChanged( windowWidth, windowHeight ) )
    {
        if ( const DenOfIz_Result recreateResult = Recreate( 0, 0 ); recreateResult != DENOFIZ_SUCCESS )
        {
            m_lastAcquireResult = recreateResult;
            return m_lastAcquireResult;
        }
    }

    uint32_t          nextImage        = 0;
    const VkSemaphore acquireSemaphore = m_acquireSemaphores[ m_acquireSemaphoreIndex ];
    const VkFence     acquireFence     = m_acquireFences[ m_acquireSemaphoreIndex ];
    if ( m_context->CheckDeviceLost( vkWaitForFences( m_context->LogicalDevice, 1, &acquireFence, VK_TRUE, UINT64_MAX ), "vkWaitForFences" ) )
    {
        m_lastAcquireResult = DENOFIZ_ERROR_DEVICE_LOST;
        return m_lastAcquireResult;
    }

    constexpr uint64_t timeout = 100ull * 1000ull * 1000ull;
    const VkResult     result  = vkAcquireNextImageKHR( m_context->LogicalDevice, m_swapChain, timeout, acquireSemaphore, VK_NULL_HANDLE, &nextImage );
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        RecreateAcquireSemaphore( m_acquireSemaphoreIndex );
        if ( m_context->CheckDeviceLost( result, "vkAcquireNextImageKHR" ) )
        {
            m_lastAcquireResult = DENOFIZ_ERROR_DEVICE_LOST;
            return m_lastAcquireResult;
        }
        switch ( result )
        {
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            m_needsResize       = true;
            m_lastAcquireResult = DENOFIZ_ERROR_OUT_OF_DATE;
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            m_surfaceLost       = true;
            m_lastAcquireResult = DENOFIZ_ERROR_SURFACE_LOST;
            break;
        case VK_TIMEOUT:
        case VK_NOT_READY:
            m_lastAcquireResult = DENOFIZ_TIMEOUT;
            break;
        default:
            spdlog::error( "[VulkanSwapChain] vkAcquireNextImageKHR failed: {}", static_cast<int>( result ) );
            m_lastAcquireResult = DENOFIZ_ERROR_UNKNOWN;
            break;
        }
        return m_lastAcquireResult;
    }
    if ( nextImage >= m_swapChainImages.size( ) )
    {
        spdlog::error( "[VulkanSwapChain] Acquired image index out of range: {}", nextImage );
        m_lastAcquireResult = DENOFIZ_ERROR_UNKNOWN;
        return m_lastAcquireResult;
    }

    if ( result == VK_SUBOPTIMAL_KHR && SurfaceExtentChanged( ) )
    {
        m_needsResize = true;
    }

    m_pendingAcquireSemaphore = acquireSemaphore;
    m_imageAcquired           = true;
    m_acquiredImageIndex      = nextImage;
    m_acquiredSlot            = m_acquireSemaphoreIndex;
    m_acquireSemaphoreIndex   = ( m_acquireSemaphoreIndex + 1 ) % static_cast<uint32_t>( m_acquireSemaphores.size( ) );
    m_lastAcquireResult       = DENOFIZ_SUCCESS;
    m_context->AutoSync->AcquireImage( this, m_renderTargets[ nextImage ].get( ) );

    outImageIndex = nextImage;
    return DENOFIZ_SUCCESS;
}

DenOfIz_Result VulkanSwapChain::Present( )
{
    if ( m_context->IsDeviceLost )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( !m_imageAcquired )
    {
        return DENOFIZ_NOT_READY;
    }

    m_context->AutoSync->ClearSwapChainSync( this );

    const VkSemaphore          presentSemaphore = m_presentSemaphores[ m_acquiredImageIndex ];
    const VkSemaphore          acquireSemaphore = m_pendingAcquireSemaphore;
    const VkPipelineStageFlags waitStage        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkSubmitInfo submitInfo{ };
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = acquireSemaphore != VK_NULL_HANDLE ? 1 : 0;
    submitInfo.pWaitSemaphores      = acquireSemaphore != VK_NULL_HANDLE ? &acquireSemaphore : nullptr;
    submitInfo.pWaitDstStageMask    = acquireSemaphore != VK_NULL_HANDLE ? &waitStage : nullptr;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &presentSemaphore;

    const uint32_t presentedImage = m_acquiredImageIndex;
    m_pendingAcquireSemaphore     = VK_NULL_HANDLE;
    m_imageAcquired               = false;

    const VkFence acquireFence = m_acquireFences[ m_acquiredSlot ];
    vkResetFences( m_context->LogicalDevice, 1, &acquireFence );
    VkResult result = vkQueueSubmit( m_queue, 1, &submitInfo, acquireFence );
    if ( m_context->CheckDeviceLost( result, "vkQueueSubmit" ) )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanSwapChain] Failed to submit presentation sync: {}", static_cast<int>( result ) );
        m_needsResize = true;
        return DENOFIZ_ERROR_UNKNOWN;
    }

    VkPresentInfoKHR presentInfo{ };
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &presentSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapChain;
    presentInfo.pImageIndices      = &presentedImage;
    presentInfo.pResults           = nullptr;

    result = vkQueuePresentKHR( m_queue, &presentInfo );
    if ( m_context->CheckDeviceLost( result, "vkQueuePresentKHR" ) )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    switch ( result )
    {
    case VK_SUCCESS:
        break;
    case VK_SUBOPTIMAL_KHR:
        if ( SurfaceExtentChanged( ) )
        {
            m_needsResize = true;
        }
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        m_needsResize = true;
        break;
    case VK_ERROR_SURFACE_LOST_KHR:
        m_surfaceLost = true;
        m_needsResize = true;
        break;
    default:
        spdlog::error( "[VulkanSwapChain] vkQueuePresentKHR failed: {}", static_cast<int>( result ) );
        return DENOFIZ_ERROR_UNKNOWN;
    }
    return DENOFIZ_SUCCESS;
}

DenOfIz_Format VulkanSwapChain::GetPreferredFormat( )
{
    if ( m_surface == VK_NULL_HANDLE )
    {
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    }
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, nullptr );
    std::vector<VkSurfaceFormatKHR> formats( count );
    if ( count > 0 )
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR( m_context->PhysicalDevice, m_surface, &count, formats.data( ) );
    }
    if ( count == 0 )
    {
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    }
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

DenOfIz_Result VulkanSwapChain::Resize( const uint32_t width, const uint32_t height )
{
    return Recreate( width, height );
}

ITexture *VulkanSwapChain::GetRenderTarget( const uint32_t image )
{
    if ( image >= m_swapChainImages.size( ) )
    {
        return nullptr;
    }
    return m_renderTargets[ image ].get( );
}

VkSwapchainKHR *VulkanSwapChain::GetSwapChain( )
{
    return &m_swapChain;
}

const DenOfIz_Viewport &VulkanSwapChain::GetViewport( )
{
    return m_viewport;
}

ITexture *VulkanSwapChain::GetDepthBuffer( )
{
    return m_depthBuffer.get( );
}

const DenOfIz_SwapChainDesc &VulkanSwapChain::GetDesc( )
{
    return m_desc;
}

VkQueue VulkanSwapChain::GetQueue( ) const
{
    return m_queue;
}

VkSemaphore VulkanSwapChain::ConsumeAcquireSemaphore( )
{
    const VkSemaphore semaphore = m_pendingAcquireSemaphore;
    m_pendingAcquireSemaphore   = VK_NULL_HANDLE;
    return semaphore;
}

void VulkanSwapChain::CreateSemaphores( )
{
    const uint32_t imageCount = static_cast<uint32_t>( m_swapChainImages.size( ) );
    m_acquireSemaphores.assign( imageCount, VK_NULL_HANDLE );
    m_presentSemaphores.assign( imageCount, VK_NULL_HANDLE );
    m_acquireFences.assign( imageCount, VK_NULL_HANDLE );
    m_acquireSemaphoreIndex = 0;
    m_acquiredSlot          = 0;

    VkFenceCreateInfo fenceCreateInfo{ };
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo{ };
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for ( uint32_t i = 0; i < imageCount; ++i )
    {
        VK_CHECK_RESULT( vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &m_acquireSemaphores[ i ] ) );
        VK_CHECK_RESULT( vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &m_presentSemaphores[ i ] ) );
        VK_CHECK_RESULT( vkCreateFence( m_context->LogicalDevice, &fenceCreateInfo, nullptr, &m_acquireFences[ i ] ) );
    }
}

void VulkanSwapChain::RecreateAcquireSemaphore( const uint32_t index )
{
    if ( index >= m_acquireSemaphores.size( ) )
    {
        return;
    }
    VkSemaphoreCreateInfo semaphoreCreateInfo{ };
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    if ( vkCreateSemaphore( m_context->LogicalDevice, &semaphoreCreateInfo, nullptr, &semaphore ) != VK_SUCCESS )
    {
        return;
    }
    vkDestroySemaphore( m_context->LogicalDevice, m_acquireSemaphores[ index ], nullptr );
    m_acquireSemaphores[ index ] = semaphore;
}

void VulkanSwapChain::DestroySemaphores( )
{
    for ( const VkSemaphore semaphore : m_acquireSemaphores )
    {
        if ( semaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore( m_context->LogicalDevice, semaphore, nullptr );
        }
    }
    for ( const VkSemaphore semaphore : m_presentSemaphores )
    {
        if ( semaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore( m_context->LogicalDevice, semaphore, nullptr );
        }
    }
    for ( const VkFence fence : m_acquireFences )
    {
        if ( fence != VK_NULL_HANDLE )
        {
            vkDestroyFence( m_context->LogicalDevice, fence, nullptr );
        }
    }
    m_acquireSemaphores.clear( );
    m_presentSemaphores.clear( );
    m_acquireFences.clear( );
}
