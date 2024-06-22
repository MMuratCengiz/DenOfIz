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

#include <DenOfIzCore/Engine.h>

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#ifdef BUILD_VK
#include "SDL2/SDL_vulkan.h"
#endif
#endif

#ifdef WINDOW_MANAGER_NATIVE
#error "Not implemented yet"
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

#ifdef _WIN32
        HWND m_windowHandle;
#elif __APPLE__
        NSWindow *m_windowHandle;
#elif __linux__
#endif

    public:
        GraphicsWindowHandle() = default;
#ifdef WINDOW_MANAGER_SDL
        void Create(SDL_Window *window)
        {
            m_sdlWindow = window;
#ifdef _WIN32
            SDL_SysWMinfo info;
            SDL_VERSION(&info.version);
            if ( SDL_GetWindowWMInfo(window, &info) )
            {
                m_windowHandle = info.info.win.window;
            }
#elif __APPLE__
#endif

            if ( m_windowHandle == nullptr )
            {
                LOG(FATAL) << "WindowHandle" << "Failed to get window handle";
            }
        }
#else
#error "Not implemented yet"
#endif

#ifdef _WIN32
        HWND
#elif __APPLE__
        NSWindow *
#elif __linux__
#error "Not implemented yet"
#endif
        GetNativeHandle() const
        {
            return m_windowHandle;
        }

        const GraphicsWindowSurface GetSurface() const
        {
#ifdef WINDOW_MANAGER_SDL
            GraphicsWindowSurface result;
            SDL_Surface *surface = SDL_GetWindowSurface(m_sdlWindow);
            result.Width = surface->w;
            result.Height = surface->h;
            return result;
#else
#error "Not implemented yet"
#endif
        }

#ifdef BUILD_VK
        const std::vector<const char *> GetVkRequiredExtensions() const
        {
#ifdef WINDOW_MANAGER_SDL
            uint32_t extensionCount = 0;
            SDL_Vulkan_GetInstanceExtensions(m_sdlWindow, &extensionCount, nullptr);
            std::vector<const char *> extensions(extensionCount);
            SDL_Vulkan_GetInstanceExtensions(m_sdlWindow, &extensionCount, extensions.data());
            return extensions;
#else
#error "Not implemented yet"
#endif
        }
#endif
        ~GraphicsWindowHandle() = default;
    };

} // namespace DenOfIz
