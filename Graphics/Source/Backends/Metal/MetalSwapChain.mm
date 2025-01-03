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

#include <DenOfIzGraphics/Backends/Metal/MetalSwapChain.h>
#include "DenOfIzGraphics/Backends/Metal/MetalSemaphore.h"

using namespace DenOfIz;

MetalSwapChain::MetalSwapChain( MetalContext *context, const SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_view                     = m_desc.WindowHandle->GetNativeHandle( ).contentView;
    m_view.autoresizesSubviews = YES;
    m_view.layer               = [CAMetalLayer layer];
    auto layer                 = (CAMetalLayer *)m_view.layer;
    layer.device               = m_context->Device;
    layer.pixelFormat          = MetalEnumConverter::ConvertFormat( m_desc.BackBufferFormat );
    layer.framebufferOnly      = NO;
    layer.contentsScale        = [m_view.window backingScaleFactor];
    layer.displaySyncEnabled   = NO;
    layer.maximumDrawableCount = m_desc.NumBuffers;
    Resize( m_desc.Width, m_desc.Height );

    m_presentCommandBuffer = [m_context->CommandQueue commandBuffer];
}

MetalSwapChain::~MetalSwapChain( )
{
}

uint32_t MetalSwapChain::AcquireNextImage( ISemaphore *imageAvailableSemaphore )
{
    @autoreleasepool
    {
        auto            layer     = (CAMetalLayer *)m_view.layer;
        MetalSemaphore *semaphore = static_cast<MetalSemaphore *>( imageAvailableSemaphore );
        m_currentDrawable         = [layer nextDrawable];
        m_renderTargets[ m_currentFrame ]->UpdateTexture( m_drawableDesc, m_currentDrawable.texture );
    }
    return m_currentFrame;
}

Format MetalSwapChain::GetPreferredFormat( )
{
    return m_desc.BackBufferFormat;
}

ITextureResource *MetalSwapChain::GetRenderTarget( uint32_t frame )
{
    return m_renderTargets[ frame ].get( );
}

Viewport MetalSwapChain::GetViewport( )
{
    return Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) };
}

void MetalSwapChain::Resize( uint32_t width, uint32_t height )
{
    @autoreleasepool
    {
        auto layer = (CAMetalLayer *)m_view.layer;
        [layer setDrawableSize:CGSizeMake( width, height )];

        m_drawableDesc.Width        = width;
        m_drawableDesc.Height       = height;
        m_drawableDesc.Format       = m_desc.BackBufferFormat;
        m_drawableDesc.InitialUsage = ResourceUsage::RenderTarget;
        m_drawableDesc.HeapType     = HeapType::GPU;
        m_drawableDesc.DebugName    = "SwapChain";

        m_renderTargets.resize( m_desc.NumBuffers );

        for ( uint32_t i = 0; i < m_desc.NumBuffers; ++i )
        {
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if ( m_renderTargets[ i ] )
            {
                m_renderTargets[ i ]->UpdateTexture( m_drawableDesc, drawable.texture );
            }
            else
            {
                m_renderTargets[ i ] = std::make_unique<MetalTextureResource>( m_context, m_drawableDesc, drawable.texture );
            }
        }
    }
}

id<MTLDrawable> MetalSwapChain::Drawable( )
{
    return m_currentDrawable;
}

void MetalSwapChain::Present( const InteropArray<ISemaphore *> &waitOnSemaphores )
{
    @autoreleasepool
    {
        m_presentCommandBuffer = [m_context->CommandQueue commandBuffer];
        [m_presentCommandBuffer presentDrawable:m_currentDrawable];
        [m_presentCommandBuffer commit];

        m_presentCommandBuffer = nil;
        m_currentDrawable      = nil;
        m_currentFrame         = ( m_currentFrame + 1 ) % m_desc.NumBuffers;
    }
}
