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

#include "DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

#define SWAPCHAIN_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISwapChain, handle )

extern "C"
{

    void DenOfIz_SwapChain_GetPreferredFormat( DenOfIz_SwapChain swapChain, DenOfIz_Format *outFormat )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) || outFormat == NULL )
        {
            return;
        }
        *outFormat = SWAPCHAIN_IMPL( swapChain )->GetPreferredFormat( );
    }

    void DenOfIz_SwapChain_AcquireNextImage( DenOfIz_SwapChain swapChain, uint32_t *outImageIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) || outImageIndex == NULL )
        {
            return;
        }
        *outImageIndex = SWAPCHAIN_IMPL( swapChain )->AcquireNextImage( );
    }

    DenOfIz_PresentResult DenOfIz_SwapChain_Present( DenOfIz_SwapChain swapChain, uint32_t imageIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return DENOFIZ_PRESENT_RESULT_ERROR;
        }
        return SWAPCHAIN_IMPL( swapChain )->Present( imageIndex );
    }

    void DenOfIz_SwapChain_Resize( DenOfIz_SwapChain swapChain, uint32_t width, uint32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return;
        }
        SWAPCHAIN_IMPL( swapChain )->Resize( width, height );
    }

    void DenOfIz_SwapChain_GetRenderTarget( DenOfIz_SwapChain swapChain, uint32_t image, DenOfIz_Texture *outRenderTarget )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) || outRenderTarget == NULL )
        {
            return;
        }
        DenOfIz::ITexture *renderTarget = SWAPCHAIN_IMPL( swapChain )->GetRenderTarget( image );
        *outRenderTarget                = DENOFIZ_TO_HANDLE( renderTarget );
    }

    void DenOfIz_SwapChain_GetViewport( DenOfIz_SwapChain swapChain, const DenOfIz_Viewport **outViewport )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) || outViewport == NULL )
        {
            return;
        }
        *outViewport = &SWAPCHAIN_IMPL( swapChain )->GetViewport( );
    }

    void DenOfIz_SwapChain_Destroy( DenOfIz_SwapChain swapChain )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return;
        }
        delete SWAPCHAIN_IMPL( swapChain );
    }
}
