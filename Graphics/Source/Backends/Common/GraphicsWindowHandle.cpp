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

#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <SDL2/SDL_syswm.h>

using namespace DenOfIz;

#ifdef WINDOW_MANAGER_SDL
void GraphicsWindowHandle::InitSDL( )
{
    if ( SDL_WasInit( SDL_INIT_VIDEO ) == 0 )
    {
        LOG( INFO ) << "Initializing SDL";
        if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
        {
            LOG( FATAL ) << "Failed to initialize SDL: " << SDL_GetError( );
        }
        atexit( SDL_Quit );
    }
}

void GraphicsWindowHandle::CreateFromSDLWindow( SDL_Window *window )
{
    InitSDL( );
    m_sdlWindow = window;
    SDL_SysWMinfo info;
    SDL_VERSION( &info.version );
    if ( SDL_GetWindowWMInfo( window, &info ) )
    {
#ifdef _WIN32
        m_windowHandle = info.info.win.window;
#elif __APPLE__
        m_windowHandle = info.info.cocoa.window;
#elif __linux__
        m_windowHandle = window;
#endif
    }

    if ( m_windowHandle == nullptr )
    {
        LOG( FATAL ) << "Failed to get window handle";
    }
}

void GraphicsWindowHandle::CreateViaSDLWindowID( uint32_t windowID )
{
    InitSDL( );
    SDL_Window *window = SDL_GetWindowFromID( windowID );
    if ( window == nullptr )
    {
        LOG( FATAL ) << "Failed to get window from SDL ID: " << windowID << " - " << SDL_GetError( );
    }
    CreateFromSDLWindow( window );
}

void GraphicsWindowHandle::CreateFromSDLWindowRawPtr( void *window )
{
    InitSDL( );
    SDL_Window *pWindow = static_cast<SDL_Window *>( window );
    if ( pWindow == nullptr )
    {
        LOG( FATAL ) << "Failed to get SDLWindow from raw pointer";
    }
    else
    {
        LOG( INFO ) << "Found window with title: " << SDL_GetWindowTitle( pWindow ) << ", id: " << SDL_GetWindowID( pWindow );
    }
    CreateFromSDLWindow( pWindow );
}

#endif
const GraphicsWindowSurface GraphicsWindowHandle::GetSurface( ) const
{
#ifdef WINDOW_MANAGER_SDL
    GraphicsWindowSurface result{ };
    SDL_Surface          *surface = SDL_GetWindowSurface( m_sdlWindow );
    result.Width                  = surface->w;
    result.Height                 = surface->h;
    return result;
#else
#error "Not implemented yet"
#endif
}

TWindowHandle GraphicsWindowHandle::GetNativeHandle( ) const
{
    return m_windowHandle;
}
