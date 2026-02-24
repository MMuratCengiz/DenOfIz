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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUSwapChain.h"
#include <cstdint>
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#ifdef BUILD_WEBGPU_NATIVE
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#endif

using namespace DenOfIz;

WebGPUSwapChain::WebGPUSwapChain( WebGPUContext *context, const DenOfIz_SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    CreateSurface( );
    ConfigureSwapChain( );
    CreateDepthStencil( );

    m_viewport.X      = 0.0f;
    m_viewport.Y      = 0.0f;
    m_viewport.Width  = static_cast<float>( m_desc.Width );
    m_viewport.Height = static_cast<float>( m_desc.Height );

#if DZ_WEBGPU_USE_DAWN_API
    m_currentSurfaceTexture  = { };
    m_surfaceTextureAcquired = false;
#else
    m_currentTextureView = nullptr;
#endif
}

WebGPUSwapChain::~WebGPUSwapChain( )
{
    m_renderTargets.clear( );
    m_depthStencil.reset( );

#if DZ_WEBGPU_USE_DAWN_API
    if ( m_surfaceTextureAcquired && m_currentSurfaceTexture.texture )
    {
        wgpuTextureRelease( m_currentSurfaceTexture.texture );
    }
#else
    if ( m_swapChain )
    {
        wgpuSwapChainRelease( m_swapChain );
    }
#endif

    if ( m_surface )
    {
        wgpuSurfaceRelease( m_surface );
    }
}

void WebGPUSwapChain::CreateSurface( )
{
#ifdef BUILD_WEBGPU_NATIVE
    if ( !m_desc.WindowHandle )
    {
        spdlog::critical( "WebGPU: Invalid window handle for surface creation" );
        return;
    }

    uint32_t windowId;
    DenOfIz_GraphicsWindowHandle_GetSDLWindowId( m_desc.WindowHandle, &windowId );
    SDL_Window *sdlWindow = SDL_GetWindowFromID( windowId );
    m_surface             = SDL_GetWGPUSurface( m_context->Instance, sdlWindow );
#elif DZ_WEBGPU_USE_DAWN_API
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc = { };
    canvasDesc.chain.sType                                   = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasDesc.chain.next                                    = nullptr;
    canvasDesc.selector                                      = DZ_WEBGPU_STRING( "canvas" );

    WGPUSurfaceDescriptor surfaceDesc = { };
    surfaceDesc.nextInChain           = &canvasDesc.chain;

    m_surface = wgpuInstanceCreateSurface( m_context->Instance, &surfaceDesc );
    if ( !m_surface )
    {
        canvasDesc.selector = DZ_WEBGPU_STRING( "#canvas" );
        m_surface           = wgpuInstanceCreateSurface( m_context->Instance, &surfaceDesc );
        if ( !m_surface )
        {
            spdlog::critical( "WebGPU: Failed to create surface from canvas selector 'canvas' or '#canvas'" );
        }
    }
#else
    WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = { };
    canvasDesc.chain.sType                                 = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    canvasDesc.chain.next                                  = nullptr;
    canvasDesc.selector                                    = "canvas";

    WGPUSurfaceDescriptor surfaceDesc = { };
    surfaceDesc.nextInChain           = &canvasDesc.chain;
    surfaceDesc.label                 = nullptr;

    m_surface = wgpuInstanceCreateSurface( m_context->Instance, &surfaceDesc );
    if ( !m_surface )
    {
        canvasDesc.selector = "#canvas";
        m_surface           = wgpuInstanceCreateSurface( m_context->Instance, &surfaceDesc );
        if ( !m_surface )
        {
            spdlog::critical( "WebGPU: Failed to create surface from canvas selector 'canvas' or '#canvas'" );
        }
    }
#endif

    if ( !m_surface )
    {
        spdlog::critical( "WebGPU: Failed to create surface" );
    }
}

void WebGPUSwapChain::ConfigureSwapChain( )
{
    if ( !m_surface )
    {
        spdlog::critical( "WebGPU: Cannot configure swap chain - surface is null" );
        return;
    }

    if ( !m_context->Device )
    {
        spdlog::critical( "WebGPU: Cannot configure swap chain - device is null" );
        return;
    }

#if DZ_WEBGPU_USE_DAWN_API
    if ( m_surfaceTextureAcquired && m_currentSurfaceTexture.texture )
    {
        wgpuTextureRelease( m_currentSurfaceTexture.texture );
        m_surfaceTextureAcquired = false;
    }
#else
    m_renderTargets.clear( );
    m_currentTextureView = nullptr;
    if ( m_swapChain )
    {
        wgpuSwapChainRelease( m_swapChain );
        m_swapChain = nullptr;
    }
#endif

    DenOfIz_GraphicsWindowSurface windowSurface;
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &windowSurface );
    if ( m_desc.Width == 0 )
    {
        m_desc.Width = windowSurface.Width;
    }
    if ( m_desc.Height == 0 )
    {
        m_desc.Height = windowSurface.Height;
    }

    WGPUTextureFormat surfaceFormat = DenOfIz_WebGPUEnumConverter_ConvertFormat( m_desc.BackBufferFormat );
    if ( m_desc.BackBufferFormat == DENOFIZ_FORMAT_UNDEFINED )
    {
#if DZ_WEBGPU_USE_DAWN_API
        WGPUSurfaceCapabilities capabilities = { };
        if ( wgpuSurfaceGetCapabilities( m_surface, m_context->Adapter, &capabilities ) == WGPUStatus_Success && capabilities.formatCount > 0 )
        {
            surfaceFormat = capabilities.formats[ 0 ];
            wgpuSurfaceCapabilitiesFreeMembers( capabilities );
        }
        else
        {
            surfaceFormat = WGPUTextureFormat_BGRA8Unorm;
        }
#else
        surfaceFormat = wgpuSurfaceGetPreferredFormat( m_surface, m_context->Adapter );
        if ( surfaceFormat == WGPUTextureFormat_Undefined )
        {
            surfaceFormat = WGPUTextureFormat_BGRA8Unorm;
        }
#endif
    }

#if DZ_WEBGPU_USE_DAWN_API
    WGPUSurfaceConfiguration config = { };
    config.device                   = m_context->Device;
    config.format                   = surfaceFormat;
    config.usage                    = DenOfIz_WebGPUEnumConverter_ConvertSwapChainTextureUsage( DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT, m_desc.ImageUsages );
    config.viewFormatCount          = 0;
    config.viewFormats              = nullptr;
    config.alphaMode                = WGPUCompositeAlphaMode_Auto;
    config.width                    = m_desc.Width;
    config.height                   = m_desc.Height;
    if ( m_desc.AllowTearing )
    {
        config.presentMode = WGPUPresentMode_Immediate;
    }
    else if ( m_desc.NumBuffers >= 3 )
    {
        config.presentMode = WGPUPresentMode_Mailbox;
    }
    else
    {
        config.presentMode = WGPUPresentMode_Fifo;
    }
#ifdef __EMSCRIPTEN__
    config.presentMode = WGPUPresentMode_Undefined;
#endif
    wgpuSurfaceConfigure( m_surface, &config );
#else
    m_surfaceFormat = surfaceFormat;

    WGPUSwapChainDescriptor swapChainDesc = { };
    swapChainDesc.label                   = nullptr;
    swapChainDesc.usage                   = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.format                  = surfaceFormat;
    swapChainDesc.width                   = m_desc.Width;
    swapChainDesc.height                  = m_desc.Height;
    swapChainDesc.presentMode             = WGPUPresentMode_Fifo;

    m_swapChain = wgpuDeviceCreateSwapChain( m_context->Device, m_surface, &swapChainDesc );
    if ( !m_swapChain )
    {
        spdlog::critical( "WebGPU: Failed to create swap chain" );
        return;
    }
#endif

    m_renderTargets.clear( );
    m_renderTargets.resize( m_desc.NumBuffers );
}

void WebGPUSwapChain::CreateDepthStencil( )
{
    if ( m_desc.DepthBufferFormat == DENOFIZ_FORMAT_UNDEFINED )
    {
        return;
    }

    DenOfIz_TextureDesc depthDesc = { };
    depthDesc.Width               = m_desc.Width;
    depthDesc.Height              = m_desc.Height;
    depthDesc.Depth               = 1;
    depthDesc.ArraySize           = 1;
    depthDesc.MipLevels           = 1;
    depthDesc.Format              = m_desc.DepthBufferFormat;
    depthDesc.MSAASampleCount     = DENOFIZ_MSAA_SAMPLE_COUNT_1;
    depthDesc.HeapType            = DENOFIZ_HEAP_TYPE_GPU;
    depthDesc.Usage               = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    depthDesc.DebugName           = DENOFIZ_STRING( "SwapChain DepthStencil" );

    m_depthStencil = std::make_unique<WebGPUTexture>( m_context, depthDesc );
}

DenOfIz_Format WebGPUSwapChain::GetPreferredFormat( )
{
    if ( !m_surface || !m_context->Adapter )
    {
        spdlog::warn( "WebGPU: GetPreferredFormat called without surface or adapter, returning default" );
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    }

#if DZ_WEBGPU_USE_DAWN_API
    WGPUSurfaceCapabilities capabilities = { };
    if ( wgpuSurfaceGetCapabilities( m_surface, m_context->Adapter, &capabilities ) == WGPUStatus_Success && capabilities.formatCount > 0 )
    {
        const WGPUTextureFormat preferredFormat = capabilities.formats[ 0 ];
        wgpuSurfaceCapabilitiesFreeMembers( capabilities );
        switch ( preferredFormat )
        {
        case WGPUTextureFormat_BGRA8Unorm:
        case WGPUTextureFormat_BGRA8UnormSrgb:
            return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        case WGPUTextureFormat_RGBA8Unorm:
            return DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        case WGPUTextureFormat_RGBA8UnormSrgb:
            return DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB;
        default:
            return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        }
    }
    return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
#else
    WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat( m_surface, m_context->Adapter );
    switch ( preferredFormat )
    {
    case WGPUTextureFormat_BGRA8Unorm:
    case WGPUTextureFormat_BGRA8UnormSrgb:
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    case WGPUTextureFormat_RGBA8Unorm:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    case WGPUTextureFormat_RGBA8UnormSrgb:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB;
    default:
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    }
#endif
}

uint32_t WebGPUSwapChain::AcquireNextImage( )
{
#if DZ_WEBGPU_USE_DAWN_API
    if ( m_surfaceTextureAcquired && m_currentSurfaceTexture.texture )
    {
        wgpuTextureRelease( m_currentSurfaceTexture.texture );
        m_surfaceTextureAcquired = false;
    }

    m_currentSurfaceTexture = { };
    wgpuSurfaceGetCurrentTexture( m_surface, &m_currentSurfaceTexture );

    if ( m_currentSurfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
         m_currentSurfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal )
    {
        spdlog::error( "WebGPU: Failed to acquire surface texture: status = {}", static_cast<int>( m_currentSurfaceTexture.status ) );
        m_surfaceTextureAcquired = false;
        return UINT32_MAX;
    }

    m_surfaceTextureAcquired = true;
#else
    if ( !m_swapChain )
    {
        spdlog::error( "WebGPU: Swap chain not initialized" );
        return UINT32_MAX;
    }

    m_currentTextureView = wgpuSwapChainGetCurrentTextureView( m_swapChain );
    if ( !m_currentTextureView )
    {
        spdlog::error( "WebGPU: Failed to get current texture view from swap chain" );
        return UINT32_MAX;
    }
#endif

    m_currentImageIndex = ( m_currentImageIndex + 1 ) % m_desc.NumBuffers;
    return m_currentImageIndex;
}

DenOfIz_PresentResult WebGPUSwapChain::Present( uint32_t imageIndex )
{
#if DZ_WEBGPU_USE_DAWN_API
    if ( !m_surface )
    {
        return DENOFIZ_PRESENT_RESULT_DEVICE_LOST;
    }

#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent( m_surface );
#endif

    if ( m_surfaceTextureAcquired && m_currentSurfaceTexture.texture )
    {
        wgpuTextureRelease( m_currentSurfaceTexture.texture );
        m_surfaceTextureAcquired = false;
    }
#else
    if ( !m_swapChain )
    {
        return DENOFIZ_PRESENT_RESULT_DEVICE_LOST;
    }
#endif

    return DENOFIZ_PRESENT_RESULT_SUCCESS;
}

void WebGPUSwapChain::Resize( const uint32_t width, const uint32_t height )
{
    if ( width == 0 || height == 0 )
    {
        return;
    }

#if DZ_WEBGPU_USE_DAWN_API
    if ( m_surfaceTextureAcquired && m_currentSurfaceTexture.texture )
    {
        wgpuTextureRelease( m_currentSurfaceTexture.texture );
        m_surfaceTextureAcquired = false;
    }
#else
    m_currentTextureView = nullptr;
#endif

    m_desc.Width  = width;
    m_desc.Height = height;

    m_viewport.Width  = static_cast<float>( width );
    m_viewport.Height = static_cast<float>( height );

    ConfigureSwapChain( );
    CreateDepthStencil( );
}

ITexture *WebGPUSwapChain::GetRenderTarget( const uint32_t image )
{
    if ( image >= m_renderTargets.size( ) )
    {
        spdlog::error( "WebGPU: Invalid swap chain image index" );
        return nullptr;
    }

#if DZ_WEBGPU_USE_DAWN_API
    if ( !m_surfaceTextureAcquired )
    {
        AcquireNextImage( );
    }

    if ( !m_currentSurfaceTexture.texture )
    {
        spdlog::error( "WebGPU: Failed to get current texture from surface" );
        return nullptr;
    }
#else
    if ( !m_currentTextureView )
    {
        AcquireNextImage( );
    }

    if ( !m_currentTextureView )
    {
        spdlog::error( "WebGPU: Failed to get current texture view from swap chain" );
        return nullptr;
    }
#endif

    DenOfIz_TextureDesc desc = { };
    desc.Width               = m_desc.Width;
    desc.Height              = m_desc.Height;
    desc.Depth               = 1;
    desc.ArraySize           = 1;
    desc.MipLevels           = 1;
    desc.Format              = m_desc.BackBufferFormat;
    desc.MSAASampleCount     = DENOFIZ_MSAA_SAMPLE_COUNT_1;
    desc.HeapType            = DENOFIZ_HEAP_TYPE_GPU;
    desc.Usage               = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    desc.DebugName           = DENOFIZ_STRING( "SwapChain RenderTarget" );

#if DZ_WEBGPU_USE_DAWN_API
    if ( !m_renderTargets[ image ] )
    {
        m_renderTargets[ image ] = std::make_unique<WebGPUTexture>( m_context, desc, m_currentSurfaceTexture.texture );
    }
    else
    {
        m_renderTargets[ image ]->UpdateExternalTexture( m_currentSurfaceTexture.texture );
    }
#else
    if ( !m_renderTargets[ image ] )
    {
        m_renderTargets[ image ] = std::make_unique<WebGPUTexture>( m_context, desc, m_currentTextureView );
    }
    else
    {
        m_renderTargets[ image ]->UpdateExternalTextureView( m_currentTextureView );
    }
#endif

    return m_renderTargets[ image ].get( );
}

const DenOfIz_Viewport &WebGPUSwapChain::GetViewport( )
{
    return m_viewport;
}
