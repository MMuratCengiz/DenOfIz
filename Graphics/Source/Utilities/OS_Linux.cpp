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
