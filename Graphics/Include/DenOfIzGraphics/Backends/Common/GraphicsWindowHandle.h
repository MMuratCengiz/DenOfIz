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
#include <DenOfIzGraphics/Utilities/Interop.h>

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#ifdef BUILD_VK
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "SDL2/SDL_vulkan.h"
#endif
#endif

#ifdef WINDOW_MANAGER_NATIVE
#error "Not implemented yet"
#endif

#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
typedef HWND TWindowHandle;
#elif __APPLE__
typedef NSWindow *TWindowHandle;
#elif __linux__
// SDL Required on linux for now
typedef SDL_Window* TWindowHandle;
#endif

namespace DenOfIz
{

    struct DZ_API GraphicsWindowSurface
    {
        uint32_t Width;
        uint32_t Height;
    };

    class DZ_API GraphicsWindowHandle
    {
#ifdef WINDOW_MANAGER_SDL
        SDL_Window *m_sdlWindow;
#endif
        TWindowHandle m_windowHandle;

    public:
        GraphicsWindowHandle( ) = default;

#ifdef WINDOW_MANAGER_SDL
        void CreateFromSDLWindow( SDL_Window *window );
        void CreateViaSDLWindowID( uint32_t windowID );
        void CreateFromSDLWindowRawPtr( void *);
#else
#error "Not implemented yet"
#endif

        [[nodiscard]] TWindowHandle GetNativeHandle( ) const;

#ifdef __APPLE__
        [[nodiscard]] NSView *GetNativeView( ) const
        {

            return m_windowHandle.contentView;
        }
#endif

        [[nodiscard]] const GraphicsWindowSurface GetSurface( ) const;
        ~GraphicsWindowHandle( ) = default;

#ifdef WINDOW_MANAGER_SDL
    private:
        void InitSDL( );
#endif
    };

} // namespace DenOfIz
