/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        DenOfIz_SwapChainDesc                      m_desc;
        DenOfIz_TextureDesc                        m_drawableDesc{ };
        id<CAMetalDrawable>                        m_drawable;
        id<MTLCommandBuffer>                       m_presentCommandBuffer;
        SDL_MetalView                              m_metalView = nullptr;
        NSView                                    *m_view       = nil;
        CAMetalLayer                              *m_metalLayer = nil;
        uint32_t                                   m_currentFrame = 0;
        std::vector<std::unique_ptr<MetalTexture>> m_renderTargets;
        std::unique_ptr<MetalTexture>              m_depthBuffer;
        DenOfIz_Viewport                           m_viewport{ };

    public:
        MetalSwapChain( MetalContext *context, const DenOfIz_SwapChainDesc &desc );
        ~MetalSwapChain( ) override;

        DenOfIz_Result          AcquireNextImage( uint32_t &outImageIndex ) override;
        DenOfIz_Format          GetPreferredFormat( ) override;
        ITexture               *GetRenderTarget( uint32_t image ) override;
        ITexture               *GetDepthBuffer( ) override;
        const DenOfIz_Viewport &GetViewport( ) override;
        const DenOfIz_SwapChainDesc &GetDesc( ) override;
        DenOfIz_Result          Resize( uint32_t width, uint32_t height ) override;
        DenOfIz_Result          Present( ) override;

    private:
        void CreateMetalView( );
        void UpdateContentsScale( );
    };

} // namespace DenOfIz
