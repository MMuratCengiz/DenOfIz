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

#include "DenOfIzGraphics/Backends/Interface/SwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "ICommandQueue.h"

namespace DenOfIz
{

    //! @brief To render to screen by 1. imageIndex = AcquireNextImage( ) and 2. GetRenderTarget( imageIndex ). The render target can then be used in {@ref
    //! ICommandList::BeginRendering( )}.
    //! @see
    //! @code
    //! auto imageIndex = swapChain->AcquireNextImage( imageReadySemaphore );
    //! DenOfIz_SwapChainDesc swapChainDesc { ... };
    //! auto swapChain = logicalDevice->CreateSwapChain( swapChainDesc );
    //! auto renderTarget = swapChain->GetRenderTarget( imageIndex );
    //! @endcode
    class ISwapChain
    {
    public:
        virtual ~ISwapChain( ) = default;

        virtual DenOfIz_Format        GetPreferredFormat( )                     = 0;
        virtual uint32_t              AcquireNextImage( )                       = 0;
        virtual DenOfIz_PresentResult Present( uint32_t imageIndex )            = 0;
        virtual void                  Resize( uint32_t width, uint32_t height ) = 0;

        virtual ITexture               *GetRenderTarget( uint32_t image ) = 0;
        virtual const DenOfIz_Viewport &GetViewport( )                    = 0;
    };

} // namespace DenOfIz
