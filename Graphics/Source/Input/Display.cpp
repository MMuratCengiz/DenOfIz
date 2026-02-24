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

#include "DenOfIzGraphics/Input/Display.h"
#include <vector>

#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

namespace
{
    std::vector<DenOfIz_Display> s_cachedDisplays;
    bool                         s_displaysInitialized = false;

    void InitializeDisplays( )
    {
        if ( s_displaysInitialized )
        {
            return;
        }
        SDL_Init( SDL_INIT_VIDEO );
        s_cachedDisplays.clear( );

        if ( SDL_DisplayID *displays = SDL_GetDisplays( nullptr ) )
        {
            const SDL_DisplayID primaryDisplayID = SDL_GetPrimaryDisplay( );

            for ( int i = 0; displays[ i ] != 0; ++i )
            {
                const SDL_DisplayID displayID = displays[ i ];
                SDL_Rect            bounds;

                if ( SDL_GetDisplayBounds( displayID, &bounds ) )
                {
                    DenOfIz_Display display{ };
                    display.ID          = displayID;
                    display.Size.Width  = bounds.w;
                    display.Size.Height = bounds.h;
                    display.IsPrimary   = ( displayID == primaryDisplayID );
                    display.DpiScale    = SDL_GetDisplayContentScale( displayID );

                    s_cachedDisplays.push_back( display );
                }
            }

            SDL_free( displays );
        }

        s_displaysInitialized = true;
    }
} // namespace

extern "C"
{

    DenOfIz_DisplayArray DenOfIz_Display_GetDisplays( )
    {
        InitializeDisplays( );

        DenOfIz_DisplayArray result;
        result.Elements    = s_cachedDisplays.data( );
        result.NumElements = s_cachedDisplays.size( );
        return result;
    }

    DenOfIz_Display DenOfIz_Display_GetPrimaryDisplay( )
    {
        InitializeDisplays( );
        for ( const auto &display : s_cachedDisplays )
        {
            if ( display.IsPrimary )
            {
                return display;
            }
        }
        if ( !s_cachedDisplays.empty( ) )
        {
            return s_cachedDisplays[ 0 ];
        }
        DenOfIz_Display emptyDisplay{ };
        return emptyDisplay;
    }

    void DenOfIz_Display_RefreshDisplays( )
    {
        s_displaysInitialized = false;
        InitializeDisplays( );
    }
}
