/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/SwapChain.h"
#include "../Common/DeferredDestroyQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "ICommandQueue.h"

namespace DenOfIz
{

    class ISwapChain : public IDeviceObject
    {
    public:
        virtual ~ISwapChain( ) = default;

        virtual DenOfIz_Format GetPreferredFormat( ) = 0;
        virtual DenOfIz_Result AcquireNextImage( uint32_t &outImageIndex ) = 0;
        virtual DenOfIz_Result Present( )                                  = 0;
        virtual DenOfIz_Result Resize( uint32_t width, uint32_t height )   = 0;

        virtual ITexture                    *GetRenderTarget( uint32_t image ) = 0;
        virtual ITexture                    *GetDepthBuffer( )                 = 0;
        virtual const DenOfIz_Viewport      &GetViewport( )                    = 0;
        virtual const DenOfIz_SwapChainDesc &GetDesc( )                        = 0;
    };

} // namespace DenOfIz
