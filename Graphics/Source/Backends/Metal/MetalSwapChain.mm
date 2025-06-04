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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalSwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalSemaphore.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

using namespace DenOfIz;

MetalSwapChain::MetalSwapChain( MetalContext *context, const SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    SDL_SysWMinfo wmInfo;
	SDL_VERSION( &wmInfo.version );
    SDL_Window* window = m_desc.WindowHandle->GetSDLWindow( );
	if ( SDL_GetWindowWMInfo( window, &wmInfo ) )
	{
		NSWindow* nsWindow = wmInfo.info.cocoa.window;
        m_view = (NSView*)nsWindow.contentView;
	}
    m_view.autoresizesSubviews        = YES;
    m_view.layer                      = [CAMetalLayer layer];
    m_metalLayer                      = (CAMetalLayer *)m_view.layer;
    m_metalLayer.device               = m_context->Device;
    m_metalLayer.pixelFormat          = MetalEnumConverter::ConvertFormat( m_desc.BackBufferFormat );
    m_metalLayer.framebufferOnly      = NO;
    m_metalLayer.contentsScale        = [m_view.window backingScaleFactor];
    m_metalLayer.displaySyncEnabled   = NO;
    m_metalLayer.maximumDrawableCount = m_desc.NumBuffers;
    Resize( m_desc.Width, m_desc.Height );

    m_drawable = nil;
    m_presentCommandBuffer = nil;
}

MetalSwapChain::~MetalSwapChain( )
{
}

uint32_t MetalSwapChain::AcquireNextImage( ISemaphore *imageAvailableSemaphore )
{
    @autoreleasepool
    {
        m_drawable     = [m_metalLayer nextDrawable];
        m_currentFrame = ( m_currentFrame + 1 ) % m_desc.NumBuffers;
        m_renderTargets[ m_currentFrame ]->UpdateTexture( m_drawableDesc, m_drawable.texture );
    }
    return m_currentFrame;
}

Format MetalSwapChain::GetPreferredFormat( )
{
    return m_desc.BackBufferFormat;
}

ITextureResource *MetalSwapChain::GetRenderTarget( uint32_t image )
{
    return m_renderTargets[ image ].get( );
}

const Viewport& MetalSwapChain::GetViewport( )
{
    return m_viewport;
}

void MetalSwapChain::Resize( uint32_t width, uint32_t height )
{
    if (width == 0 || height == 0) {
        return;
    }
    
    @autoreleasepool
    {
        m_drawable = nil;
        m_presentCommandBuffer = nil;
        
        [m_metalLayer setDrawableSize:CGSizeMake(width, height)];
        
        m_drawableDesc.Width        = width;
        m_drawableDesc.Height       = height;
        m_drawableDesc.Format       = m_desc.BackBufferFormat;
        m_drawableDesc.InitialUsage = ResourceUsage::RenderTarget;
        m_drawableDesc.HeapType     = HeapType::GPU;
        m_drawableDesc.DebugName    = "SwapChain";
        
        m_desc.Width  = width;
        m_desc.Height = height;
        m_viewport    = Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) };

        
        m_renderTargets.clear();
        m_renderTargets.resize(m_desc.NumBuffers);
        for (uint32_t i = 0; i < m_desc.NumBuffers; ++i)
        {
            m_renderTargets[i] = std::make_unique<MetalTextureResource>(m_context, m_drawableDesc, nil);
        }
    }
}

PresentResult MetalSwapChain::Present( const PresentDesc& presentDesc )
{
    @autoreleasepool
    {
        m_presentCommandBuffer = [m_context->CommandQueue commandBuffer];

        m_presentCommandBuffer.label = @"PRESENT";
        [m_presentCommandBuffer presentDrawable:m_drawable];

        m_renderTargets[ presentDesc.Image ]->UpdateTexture( m_drawableDesc, nil );
        m_drawable = nil;

        [m_presentCommandBuffer commit];
        m_presentCommandBuffer = nil;

        return PresentResult::Success;
    }
}
