/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalSwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalSemaphore.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

#include <algorithm>

using namespace DenOfIz;

MetalSwapChain::MetalSwapChain( MetalContext *context, const DenOfIz_SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_drawable             = nil;
    m_presentCommandBuffer = nil;
    m_desc.NumBuffers      = std::max( 1u, m_desc.NumBuffers );
    CreateMetalView( );

    const uint32_t width  = m_desc.Width;
    const uint32_t height = m_desc.Height;
    m_desc.Width  = 0;
    m_desc.Height = 0;
    Resize( width, height );
}

void MetalSwapChain::CreateMetalView( )
{
    uint32_t windowId = 0;
    DenOfIz_GraphicsWindowHandle_GetSDLWindowId( m_desc.WindowHandle, &windowId );
    SDL_Window *window = SDL_GetWindowFromID( windowId );
    if ( window == nullptr )
    {
        return;
    }
    m_metalView = SDL_Metal_CreateView( window );
    if ( m_metalView == nullptr )
    {
        return;
    }
    m_view                          = (__bridge NSView *)m_metalView;
    m_view.wantsLayer               = YES;
    m_metalLayer                    = (__bridge CAMetalLayer *)SDL_Metal_GetLayer( m_metalView );
    m_metalLayer.device             = m_context->Device;
    m_metalLayer.pixelFormat        = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.BackBufferFormat );
    m_metalLayer.framebufferOnly    = NO;
    m_metalLayer.displaySyncEnabled = m_desc.PresentMode == DENOFIZ_PRESENT_MODE_FIFO ? YES : NO;
    m_metalLayer.maximumDrawableCount = std::min( std::max( 2u, m_desc.NumBuffers ), 3u );
    UpdateContentsScale( );
}

void MetalSwapChain::UpdateContentsScale( )
{
    const CGFloat backingScale = [m_view.window backingScaleFactor];
    if ( backingScale > 0.0 && m_metalLayer.contentsScale != backingScale )
    {
        m_metalLayer.contentsScale = backingScale;
    }
}

MetalSwapChain::~MetalSwapChain( )
{
    if ( m_metalView )
    {
        SDL_Metal_DestroyView( m_metalView );
        m_metalView = nullptr;
    }
}

DenOfIz_Result MetalSwapChain::AcquireNextImage( uint32_t &outImageIndex )
{
    outImageIndex = m_currentFrame;
    if ( m_drawable != nil )
    {
        return DENOFIZ_SUCCESS;
    }
    DenOfIz_GraphicsWindowSurface windowSurface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &windowSurface );
    if ( windowSurface.Width == 0 || windowSurface.Height == 0 )
    {
        return DENOFIZ_NOT_READY;
    }
    if ( windowSurface.Width != m_desc.Width || windowSurface.Height != m_desc.Height )
    {
        if ( const DenOfIz_Result resizeResult = Resize( windowSurface.Width, windowSurface.Height ); resizeResult != DENOFIZ_SUCCESS )
        {
            return resizeResult;
        }
    }
    if ( m_metalLayer == nil || m_renderTargets.empty( ) )
    {
        return DENOFIZ_NOT_READY;
    }

    @autoreleasepool
    {
        const CGSize expectedSize = CGSizeMake( m_desc.Width, m_desc.Height );
        if ( !CGSizeEqualToSize( m_metalLayer.drawableSize, expectedSize ) )
        {
            m_metalLayer.drawableSize = expectedSize;
        }

        id<CAMetalDrawable> drawable = [m_metalLayer nextDrawable];
        if ( drawable == nil )
        {
            return DENOFIZ_TIMEOUT;
        }

        m_drawable     = drawable;
        m_currentFrame = ( m_currentFrame + 1 ) % static_cast<uint32_t>( m_renderTargets.size( ) );
        m_renderTargets[ m_currentFrame ]->UpdateTexture( m_drawableDesc, m_drawable.texture );
    }
    outImageIndex = m_currentFrame;
    return DENOFIZ_SUCCESS;
}

DenOfIz_Format MetalSwapChain::GetPreferredFormat( )
{
    return m_desc.BackBufferFormat;
}

ITexture *MetalSwapChain::GetRenderTarget( uint32_t image )
{
    if ( image >= m_renderTargets.size( ) )
    {
        return nullptr;
    }
    return m_renderTargets[ image ].get( );
}

ITexture *MetalSwapChain::GetDepthBuffer( )
{
    return m_depthBuffer.get( );
}

const DenOfIz_SwapChainDesc &MetalSwapChain::GetDesc( )
{
    return m_desc;
}

const DenOfIz_Viewport& MetalSwapChain::GetViewport( )
{
    return m_viewport;
}

DenOfIz_Result MetalSwapChain::Resize( uint32_t width, uint32_t height )
{
    if ( width == 0 || height == 0 )
    {
        DenOfIz_GraphicsWindowSurface windowSurface{ };
        DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &windowSurface );
        width  = windowSurface.Width;
        height = windowSurface.Height;
    }
    if ( width == 0 || height == 0 )
    {
        return DENOFIZ_NOT_READY;
    }
    if ( m_metalLayer == nil )
    {
        return DENOFIZ_ERROR_INITIALIZATION;
    }
    if ( width == m_desc.Width && height == m_desc.Height && !m_renderTargets.empty( ) )
    {
        UpdateContentsScale( );
        return DENOFIZ_SUCCESS;
    }

    @autoreleasepool
    {
        m_drawable             = nil;
        m_presentCommandBuffer = nil;

        UpdateContentsScale( );
        [m_metalLayer setDrawableSize:CGSizeMake( width, height )];

        m_drawableDesc.Width     = width;
        m_drawableDesc.Height    = height;
        m_drawableDesc.Depth     = 1;
        m_drawableDesc.ArraySize = 1;
        m_drawableDesc.MipLevels = 1;
        m_drawableDesc.Format    = m_desc.BackBufferFormat;
        m_drawableDesc.Usage     = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
        m_drawableDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
        m_drawableDesc.DebugName = DENOFIZ_STRING( "SwapChain" );

        m_desc.Width  = width;
        m_desc.Height = height;
        m_viewport    = DenOfIz_Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) };

        m_renderTargets.resize( m_desc.NumBuffers );
        for ( uint32_t i = 0; i < m_desc.NumBuffers; ++i )
        {
            if ( m_renderTargets[i] != nullptr )
            {
                m_renderTargets[i]->UpdateTexture( m_drawableDesc, nil );
            }
            else
            {
                m_renderTargets[i] = std::make_unique<MetalTexture>( m_context, m_drawableDesc, nil );
            }
        }

        m_depthBuffer.reset( );
        if ( m_desc.DepthBufferFormat != DENOFIZ_FORMAT_UNDEFINED )
        {
            DenOfIz_TextureDesc depthDesc{ };
            depthDesc.Width                 = width;
            depthDesc.Height                = height;
            depthDesc.Depth                 = 1;
            depthDesc.ArraySize             = 1;
            depthDesc.MipLevels             = 1;
            depthDesc.Format                = m_desc.DepthBufferFormat;
            depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
            depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
            depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
            depthDesc.DebugName             = DENOFIZ_STRING( "SwapChain Depth Buffer" );
            m_depthBuffer                   = std::make_unique<MetalTexture>( m_context, depthDesc );
        }
    }
    return DENOFIZ_SUCCESS;
}

DenOfIz_Result MetalSwapChain::Present( )
{
    if ( m_drawable == nil )
    {
        return DENOFIZ_NOT_READY;
    }

    @autoreleasepool
    {
        m_presentCommandBuffer = [m_context->CommandQueue commandBuffer];

        m_presentCommandBuffer.label = @"PRESENT";
        [m_presentCommandBuffer presentDrawable:m_drawable];

        if ( m_currentFrame < m_renderTargets.size( ) )
        {
            m_renderTargets[ m_currentFrame ]->UpdateTexture( m_drawableDesc, nil );
        }
        m_drawable = nil;

        [m_presentCommandBuffer commit];
        m_context->TrackCommandBuffer( m_presentCommandBuffer );
        m_presentCommandBuffer = nil;

        return DENOFIZ_SUCCESS;
    }
}
