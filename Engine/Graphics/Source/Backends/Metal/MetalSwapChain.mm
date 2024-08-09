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

using namespace DenOfIz;

MetalSwapChain::MetalSwapChain( MetalContext *context, const SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_view                     = (__bridge NSView *)m_desc.WindowHandle->GetNativeHandle( );
    m_view.autoresizesSubviews = TRUE;

    m_layer = [CAMetalLayer layer];
    Resize( m_desc.Width, m_desc.Height );
}

MetalSwapChain::~MetalSwapChain( )
{
    [m_layer release];
}

uint32_t MetalSwapChain::AcquireNextImage( ISemaphore *imageAvailableSemaphore )
{
    m_currentDrawable = [m_layer nextDrawable];
    m_currentFrame    = ( m_currentFrame + 1 ) % m_desc.NumBuffers;

    m_renderTargets[ m_currentFrame ]->UpdateTexture( m_currentDrawable.texture );
    return m_currentFrame;
}

Format MetalSwapChain::GetPreferredFormat( )
{
    return Format::R8Unorm;
}

ITextureResource *MetalSwapChain::GetRenderTarget( uint32_t frame )
{
    return nullptr;
}

Viewport MetalSwapChain::GetViewport( )
{
    return Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) };
}

void MetalSwapChain::Resize( uint32_t width, uint32_t height )
{
    TextureDesc textureDesc{ };
    textureDesc.Width        = width;
    textureDesc.Height       = height;
    textureDesc.Format       = Format::R8Unorm;
    textureDesc.InitialState = ResourceState::RenderTarget;
    textureDesc.HeapType     = HeapType::GPU;

    m_renderTargets.resize( m_desc.NumBuffers );

    for ( uint32_t i = 0; i < m_desc.NumBuffers; ++i )
    {
        id<CAMetalDrawable> drawable = [m_layer nextDrawable];
        m_renderTargets[ i ]         = std::make_unique<MetalTextureResource>( m_context, textureDesc, drawable.texture, "SwapChainImage" );
    }
}

id<MTLDrawable> MetalSwapChain::Drawable( )
{
    return m_currentDrawable;
}
