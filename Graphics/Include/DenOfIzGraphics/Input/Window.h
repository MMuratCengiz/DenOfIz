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

#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <DenOfIzGraphics/Utilities/Engine.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#ifdef BUILD_VK
#include "SDL2/SDL_vulkan.h"
#endif
#endif

namespace DenOfIz
{
    struct DZ_API WindowFlags
    {
        bool None              = false;
        bool Fullscreen        = false;
        bool OpenGL            = false;
        bool Shown             = false;
        bool Hidden            = false;
        bool Borderless        = false;
        bool Resizable         = false;
        bool Minimized         = false;
        bool Maximized         = false;
        bool InputGrabbed      = false;
        bool InputFocus        = false;
        bool MouseFocus        = false;
        bool FullscreenDesktop = false;
        bool Foreign           = false;
        bool HighDPI           = false;
        bool MouseCapture      = false;
        bool AlwaysOnTop       = false;
        bool SkipTaskbar       = false;
        bool Utility           = false;
        bool Tooltip           = false;
        bool PopupMenu         = false;
        bool Vulkan            = false;

#ifdef WINDOW_MANAGER_SDL
        [[nodiscard]] uint32_t ToSDLWindowFlags( ) const
        {
            uint32_t flags = 0;
            if ( Fullscreen )
            {
                flags |= SDL_WINDOW_FULLSCREEN;
            }
            if ( OpenGL )
            {
                flags |= SDL_WINDOW_OPENGL;
            }
            if ( Shown )
            {
                flags |= SDL_WINDOW_SHOWN;
            }
            if ( Hidden )
            {
                flags |= SDL_WINDOW_HIDDEN;
            }
            if ( Borderless )
            {
                flags |= SDL_WINDOW_BORDERLESS;
            }
            if ( Resizable )
            {
                flags |= SDL_WINDOW_RESIZABLE;
            }
            if ( Minimized )
            {
                flags |= SDL_WINDOW_MINIMIZED;
            }
            if ( Maximized )
            {
                flags |= SDL_WINDOW_MAXIMIZED;
            }
            if ( InputGrabbed )
            {
                flags |= SDL_WINDOW_INPUT_GRABBED;
            }
            if ( InputFocus )
            {
                flags |= SDL_WINDOW_INPUT_FOCUS;
            }
            if ( MouseFocus )
            {
                flags |= SDL_WINDOW_MOUSE_FOCUS;
            }
            if ( FullscreenDesktop )
            {
                flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            if ( Foreign )
            {
                flags |= SDL_WINDOW_FOREIGN;
            }
            if ( HighDPI )
            {
                flags |= SDL_WINDOW_ALLOW_HIGHDPI;
            }
            if ( MouseCapture )
            {
                flags |= SDL_WINDOW_MOUSE_CAPTURE;
            }
            if ( AlwaysOnTop )
            {
                flags |= SDL_WINDOW_ALWAYS_ON_TOP;
            }
            if ( SkipTaskbar )
            {
                flags |= SDL_WINDOW_SKIP_TASKBAR;
            }
            if ( Utility )
            {
                flags |= SDL_WINDOW_UTILITY;
            }
            if ( Tooltip )
            {
                flags |= SDL_WINDOW_TOOLTIP;
            }
            if ( PopupMenu )
            {
                flags |= SDL_WINDOW_POPUP_MENU;
            }
#ifdef BUILD_VK
            if ( Vulkan )
            {
                flags |= SDL_WINDOW_VULKAN;
            }
#endif
            return flags;
        }

        void FromSDLWindowFlags( const uint32_t flags )
        {
            None              = flags == 0;
            Fullscreen        = ( flags & SDL_WINDOW_FULLSCREEN ) != 0;
            OpenGL            = ( flags & SDL_WINDOW_OPENGL ) != 0;
            Shown             = ( flags & SDL_WINDOW_SHOWN ) != 0;
            Hidden            = ( flags & SDL_WINDOW_HIDDEN ) != 0;
            Borderless        = ( flags & SDL_WINDOW_BORDERLESS ) != 0;
            Resizable         = ( flags & SDL_WINDOW_RESIZABLE ) != 0;
            Minimized         = ( flags & SDL_WINDOW_MINIMIZED ) != 0;
            Maximized         = ( flags & SDL_WINDOW_MAXIMIZED ) != 0;
            InputGrabbed      = ( flags & SDL_WINDOW_INPUT_GRABBED ) != 0;
            InputFocus        = ( flags & SDL_WINDOW_INPUT_FOCUS ) != 0;
            MouseFocus        = ( flags & SDL_WINDOW_MOUSE_FOCUS ) != 0;
            FullscreenDesktop = ( flags & SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0;
            Foreign           = ( flags & SDL_WINDOW_FOREIGN ) != 0;
            HighDPI           = ( flags & SDL_WINDOW_ALLOW_HIGHDPI ) != 0;
            MouseCapture      = ( flags & SDL_WINDOW_MOUSE_CAPTURE ) != 0;
            AlwaysOnTop       = ( flags & SDL_WINDOW_ALWAYS_ON_TOP ) != 0;
            SkipTaskbar       = ( flags & SDL_WINDOW_SKIP_TASKBAR ) != 0;
            Utility           = ( flags & SDL_WINDOW_UTILITY ) != 0;
            Tooltip           = ( flags & SDL_WINDOW_TOOLTIP ) != 0;
            PopupMenu         = ( flags & SDL_WINDOW_POPUP_MENU ) != 0;
#ifdef BUILD_VK
            Vulkan = ( flags & SDL_WINDOW_VULKAN ) != 0;
#endif
        }
#endif
    };

    enum class DZ_API WindowPosition
    {
        Undefined = 0x1FFF0000,
        Centered  = 0x2FFF0000
    };

    struct DZ_API WindowSize
    {
        int Width;
        int Height;
    };

    struct DZ_API WindowProperties
    {
        InteropString Title;
        int           X;
        int           Y;
        int           Width;
        int           Height;
        WindowFlags   Flags;
    };

    class DZ_API Window
    {
        WindowProperties     m_properties{ };
        GraphicsWindowHandle m_windowHandle{ };
        bool                 m_initialized;
        uint32_t             m_windowID;

#ifdef WINDOW_MANAGER_SDL
        SDL_Window *m_sdlWindow;
#endif

    public:
        Window( );
        explicit Window( const WindowProperties &properties );
        ~Window( );

        void Create( const WindowProperties &properties );
        void Destroy( );

        // Window control functions
        void Show( ) const;
        void Hide( ) const;
        void Minimize( ) const;
        void Maximize( ) const;
        void Raise( ) const;
        void Restore( ) const;

        [[nodiscard]] GraphicsWindowHandle GetGraphicsWindowHandle( ) const;
        [[nodiscard]] uint32_t             GetWindowID( ) const;
        [[nodiscard]] WindowSize           GetSize( ) const;
        void                               SetSize( int width, int height ) const;
        [[nodiscard]] InteropString        GetTitle( ) const;
        void                               SetTitle( const InteropString &title );
        [[nodiscard]] bool                 GetFullscreen( ) const;
        void                               SetFullscreen( bool fullscreen );
        void                               SetPosition( int x, int y );
        void                               GetPosition( int *x, int *y ) const;
        void                               SetResizable( bool resizable );
        void                               SetBordered( bool bordered );
        void                               SetMinimumSize( int minWidth, int minHeight ) const;
        void                               SetMaximumSize( int maxWidth, int maxHeight ) const;
        [[nodiscard]] bool                 IsShown( ) const;
        [[nodiscard]] bool                 IsInitialized( ) const;

#ifdef WINDOW_MANAGER_SDL
        [[nodiscard]] SDL_Window *GetSDLWindow( ) const;
#endif

    private:
        static void InitializeSDL( );
        static bool s_sdlInitialized;
    };

} // namespace DenOfIz
