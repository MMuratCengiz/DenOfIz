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

    for ( int i = 0; i < m_desc.NumBuffers; i++ )
    {
        m_renderTargets.push_back(
            new MetalTextureResource( m_context, TextureDesc( m_desc.Width, m_desc.Height, Format::R8Unorm, Usa::RenderTarget ) ) );
    }
}

MetalSwapChain::~MetalSwapChain( )
{
}

uint32_t MetalSwapChain::AcquireNextImage( ISemaphore *imageAvailableSemaphore )
{
    m_currentDrawable = [m_layer nextDrawable];

    m_currentFrame = ( m_currentFrame + 1 ) % m_desc.NumBuffers;
    *pImageIndex       = pSwapChain->mIndex;

    pSwapChain->ppRenderTargets[ pSwapChain->mIndex ]->pTexture->pTexture = pSwapChain->mMTKDrawable.texture;
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
}

void MetalSwapChain::CreateSwapChain( )
{
}
