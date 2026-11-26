/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

    DenOfIz_Result DenOfIz_SwapChain_AcquireNextImage( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outBackBuffer )
    {
        if ( outBackBuffer == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outBackBuffer = DENOFIZ_NULL_HANDLE;
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }

        DenOfIz::ISwapChain *impl       = SWAPCHAIN_IMPL( swapChain );
        uint32_t             imageIndex = 0;
        const DenOfIz_Result result     = impl->AcquireNextImage( imageIndex );
        if ( result != DENOFIZ_SUCCESS )
        {
            return result;
        }
        DenOfIz::ITexture *backBuffer = impl->GetRenderTarget( imageIndex );
        if ( backBuffer == nullptr )
        {
            return DENOFIZ_ERROR_UNKNOWN;
        }
        *outBackBuffer = DENOFIZ_TO_HANDLE( backBuffer );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_SwapChain_Present( DenOfIz_SwapChain swapChain )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        DenOfIz::ISwapChain *impl   = SWAPCHAIN_IMPL( swapChain );
        const DenOfIz_Result result = impl->Present( );
        if ( DenOfIz::DeferredDestroyQueue *destroyQueue = impl->DestroyQueue( ) )
        {
            destroyQueue->EndFrame( );
        }
        return result;
    }

    void DenOfIz_SwapChain_GetDepthBuffer( DenOfIz_SwapChain swapChain, DenOfIz_Texture *outDepthBuffer )
    {
        if ( outDepthBuffer == NULL )
        {
            return;
        }
        *outDepthBuffer = DENOFIZ_NULL_HANDLE;
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) )
        {
            return;
        }
        *outDepthBuffer = DENOFIZ_TO_HANDLE( SWAPCHAIN_IMPL( swapChain )->GetDepthBuffer( ) );
    }

    void DenOfIz_SwapChain_GetDesc( DenOfIz_SwapChain swapChain, DenOfIz_SwapChainDesc *outDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( swapChain ) || outDesc == NULL )
        {
            return;
        }
        *outDesc = SWAPCHAIN_IMPL( swapChain )->GetDesc( );
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
