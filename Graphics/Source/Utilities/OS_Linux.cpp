/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Utilities/OS.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

namespace
{
    float g_cachedDisplayScale = 0.0f;

    void ProbeSDLScales( )
    {
        SDL_Window *probe = SDL_CreateWindow( "DenOfIz_DPI_Probe", 1, 1, SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY );
        if ( probe != NULL )
        {
            g_cachedDisplayScale = SDL_GetWindowDisplayScale( probe );
            SDL_DestroyWindow( probe );
        }

        if ( g_cachedDisplayScale <= 0.0f )
        {
            g_cachedDisplayScale = 1.0f;
        }
    }
} // namespace

extern "C"
{

    float DenOfIz_OS_GetDisplayScale( int display )
    {
        return 1.0f;
    }

    float DenOfIz_OS_GetWindowSizeScaleFactor( int display )
    {
        if ( SDL_WasInit( SDL_INIT_VIDEO ) )
        {
            ProbeSDLScales( );
            return g_cachedDisplayScale;
        }

        return 1.0f;
    }
}
