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

#include <DenOfIzGraphics/Utilities/Engine.h>

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#ifdef BUILD_VK
#include "SDL2/SDL_vulkan.h"
#endif
#endif
#ifdef __APPLE__
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#endif

#ifdef WINDOW_MANAGER_NATIVE
#error "Not implemented yet"
#endif

#ifdef _WIN32
typedef HWND TWindowHandle;
#elif __APPLE__
typedef NSWindow *TWindowHandle;
#elif __linux__
// Todo
typedef void *TWindowHandle;
#endif

namespace DenOfIz
{

    struct GraphicsWindowSurface
    {
        uint32_t Width;
        uint32_t Height;
    };

    class GraphicsWindowHandle
    {
    private:
#ifdef WINDOW_MANAGER_SDL
        SDL_Window *m_sdlWindow;
#endif

        TWindowHandle m_windowHandle;

    public:
        GraphicsWindowHandle( ) = default;

#ifdef WINDOW_MANAGER_SDL
        void Create( SDL_Window *window )
        {
            m_sdlWindow = window;
            SDL_SysWMinfo info;
            SDL_VERSION( &info.version );
            if ( SDL_GetWindowWMInfo( window, &info ) )
            {
#ifdef _WIN32
                m_windowHandle = info.info.win.window;
#elif __APPLE__
                m_windowHandle = info.info.cocoa.window;
#endif
            }

            if ( m_windowHandle == nullptr )
            {
                LOG( FATAL ) << "Failed to get window handle";
            }
        }
#else
#error "Not implemented yet"
#endif

        [[nodiscard]] TWindowHandle GetNativeHandle( ) const
        {
            return m_windowHandle;
        }

#ifdef __APPLE__
        [[nodiscard]] NSView *GetNativeView( ) const
        {

            return m_windowHandle.contentView;
        }
#endif

        [[nodiscard]] const GraphicsWindowSurface GetSurface( ) const
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
        ~GraphicsWindowHandle( ) = default;
    };

} // namespace DenOfIz
