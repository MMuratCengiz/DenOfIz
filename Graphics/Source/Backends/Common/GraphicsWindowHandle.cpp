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

#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace DenOfIz
{
    struct GraphicsWindowHandleImpl
    {
        SDL_Window *SdlWindow = nullptr;
        uint32_t    WindowId  = 0;

        void InitSDL( ) const
        {
            if ( SDL_WasInit( SDL_INIT_VIDEO ) == 0 )
            {
                spdlog::info( "Initializing SDL" );
                if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
                {
                    spdlog::critical( "Failed to initialize SDL: {}", SDL_GetError( ) );
                }
                atexit( SDL_Quit );
            }
        }
    };
} // namespace DenOfIz

#define WINDOW_HANDLE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::GraphicsWindowHandleImpl, handle )

extern "C"
{

    DenOfIz_GraphicsWindowHandle DenOfIz_GraphicsWindowHandle_Create( )
    {
        auto *impl = new DenOfIz::GraphicsWindowHandleImpl( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_GraphicsWindowHandle_CreateFromSDLWindowId( DenOfIz_GraphicsWindowHandle handle, uint32_t windowId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) )
        {
            return;
        }
        DenOfIz::GraphicsWindowHandleImpl *impl = WINDOW_HANDLE_IMPL( handle );
        impl->InitSDL( );
        impl->WindowId = windowId;

        SDL_Window *window = SDL_GetWindowFromID( windowId );
        if ( window == nullptr )
        {
            spdlog::critical( "Failed to get window from SDL ID: {} - {}", windowId, SDL_GetError( ) );
        }
        impl->SdlWindow = window;
    }

    void DenOfIz_GraphicsWindowHandle_GetSDLWindowId( DenOfIz_GraphicsWindowHandle handle, uint32_t *outWindowId )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) || outWindowId == NULL )
        {
            return;
        }
        *outWindowId = WINDOW_HANDLE_IMPL( handle )->WindowId;
    }

    void DenOfIz_GraphicsWindowHandle_GetSurface( DenOfIz_GraphicsWindowHandle handle, DenOfIz_GraphicsWindowSurface *outSurface )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) || outSurface == NULL )
        {
            return;
        }
        DenOfIz::GraphicsWindowHandleImpl *impl = WINDOW_HANDLE_IMPL( handle );

        outSurface->Width  = 0;
        outSurface->Height = 0;

        if ( impl->SdlWindow )
        {
            int w = 0, h = 0;
            SDL_GetWindowSizeInPixels( impl->SdlWindow, &w, &h );
            outSurface->Width  = static_cast<uint32_t>( w );
            outSurface->Height = static_cast<uint32_t>( h );
        }
    }

    void DenOfIz_GraphicsWindowHandle_IsValid( DenOfIz_GraphicsWindowHandle handle, bool *outIsValid )
    {
        if ( outIsValid == NULL )
        {
            return;
        }
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) )
        {
            *outIsValid = false;
            return;
        }
        DenOfIz::GraphicsWindowHandleImpl *impl = WINDOW_HANDLE_IMPL( handle );
        *outIsValid                             = impl->WindowId != 0 && impl->SdlWindow != nullptr;
    }

    void DenOfIz_GraphicsWindowHandle_SetFullScreenMode( DenOfIz_GraphicsWindowHandle handle, DenOfIz_FullScreenMode mode )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) )
        {
            return;
        }
        DenOfIz::GraphicsWindowHandleImpl *impl = WINDOW_HANDLE_IMPL( handle );
        if ( !impl->SdlWindow )
        {
            spdlog::error( "Cannot set fullscreen mode on invalid window" );
            return;
        }

        switch ( mode )
        {
        case DENOFIZ_FULLSCREEN_MODE_WINDOWED:
            SDL_SetWindowFullscreen( impl->SdlWindow, 0 );
            break;
        case DENOFIZ_FULLSCREEN_MODE_BORDERLESS:
            SDL_SetWindowFullscreen( impl->SdlWindow, SDL_WINDOW_BORDERLESS );
            break;
        case DENOFIZ_FULLSCREEN_MODE_EXCLUSIVE:
            SDL_SetWindowFullscreen( impl->SdlWindow, SDL_WINDOW_FULLSCREEN );
            break;
        }
    }

    void DenOfIz_GraphicsWindowHandle_GetFullScreenMode( DenOfIz_GraphicsWindowHandle handle, DenOfIz_FullScreenMode *outMode )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) || outMode == NULL )
        {
            return;
        }
        DenOfIz::GraphicsWindowHandleImpl *impl = WINDOW_HANDLE_IMPL( handle );
        if ( !impl->SdlWindow )
        {
            *outMode = DENOFIZ_FULLSCREEN_MODE_WINDOWED;
            return;
        }

        Uint32 flags = SDL_GetWindowFlags( impl->SdlWindow );

        if ( flags & SDL_WINDOW_FULLSCREEN )
        {
            *outMode = DENOFIZ_FULLSCREEN_MODE_EXCLUSIVE;
        }
        else if ( flags & SDL_WINDOW_BORDERLESS )
        {
            *outMode = DENOFIZ_FULLSCREEN_MODE_BORDERLESS;
        }
        else
        {
            *outMode = DENOFIZ_FULLSCREEN_MODE_WINDOWED;
        }
    }

    void DenOfIz_GraphicsWindowHandle_Destroy( DenOfIz_GraphicsWindowHandle handle )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( handle ) )
        {
            return;
        }
        delete WINDOW_HANDLE_IMPL( handle );
    }
}
