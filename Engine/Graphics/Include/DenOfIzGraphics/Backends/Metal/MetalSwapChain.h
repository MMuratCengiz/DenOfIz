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

#include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
#include "MetalContext.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{

    class MetalSwapChain : public ISwapChain
    {
        MetalContext                                      *m_context;
        SwapChainDesc                                      m_desc;
        id<CAMetalDrawable>                                m_currentDrawable;
        NSView                                            *m_view;
        CAMetalLayer                                      *m_layer;
        uint32_t                                           m_currentFrame = 0;
        std::vector<std::unique_ptr<MetalTextureResource>> m_renderTargets;

    public:
        MetalSwapChain( MetalContext *context, const SwapChainDesc &desc );
        ~MetalSwapChain( ) override;

        uint32_t          AcquireNextImage( ISemaphore *imageAvailableSemaphore ) override;
        Format            GetPreferredFormat( ) override;
        ITextureResource *GetRenderTarget( uint32_t frame ) override;
        Viewport          GetViewport( ) override;
        id<MTLDrawable>   Drawable( );
        void              Resize( uint32_t width, uint32_t height ) override;
    };

} // namespace DenOfIz
