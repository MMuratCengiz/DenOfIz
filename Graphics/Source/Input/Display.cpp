/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Input/Display.h"
#include <algorithm>
#include <vector>

#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

namespace
{
    std::vector<DenOfIz_Display> s_cachedDisplays;
    bool                         s_displaysInitialized = false;

    float GetDisplayScale( const SDL_DisplayID displayID )
    {
        float scale = SDL_GetDisplayContentScale( displayID );
        if ( const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode( displayID ) )
        {
            scale = std::max( scale, mode->pixel_density );
        }
        return scale > 0.0f ? scale : 1.0f;
    }

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
                    display.DisplayScale = GetDisplayScale( displayID );

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
