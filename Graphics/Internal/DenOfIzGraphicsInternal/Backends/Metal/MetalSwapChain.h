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

#include <DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h>
#include "MetalCommandQueue.h"
#include "MetalContext.h"
#include "MetalTexture.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

namespace DenOfIz
{

    class MetalSwapChain final : public ISwapChain
    {
        MetalContext                              *m_context;
        MetalCommandQueue                         *m_commandQueue;
        DenOfIz_SwapChainDesc                      m_desc;
        DenOfIz_TextureDesc                        m_drawableDesc{ };
        id<CAMetalDrawable>                        m_drawable;
        id<MTLCommandBuffer>                       m_presentCommandBuffer;
        SDL_MetalView                              m_metalView = nullptr;
        NSView                                    *m_view;
        CAMetalLayer                              *m_metalLayer;
        uint32_t                                   m_currentFrame = 0;
        std::vector<std::unique_ptr<MetalTexture>> m_renderTargets;
        DenOfIz_Viewport                           m_viewport;

    public:
        MetalSwapChain( MetalContext *context, const DenOfIz_SwapChainDesc &desc );
        ~MetalSwapChain( ) override;

        uint32_t                AcquireNextImage( ) override;
        DenOfIz_Format          GetPreferredFormat( ) override;
        ITexture               *GetRenderTarget( uint32_t image ) override;
        const DenOfIz_Viewport &GetViewport( ) override;
        void                    Resize( uint32_t width, uint32_t height ) override;
        DenOfIz_PresentResult   Present( uint32_t imageIndex ) override;

    private:
        void CreateMetalView( );
    };

} // namespace DenOfIz
