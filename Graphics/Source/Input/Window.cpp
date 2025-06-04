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

#include <utility>

#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

using namespace DenOfIz;

namespace
{
#ifdef WINDOW_MANAGER_SDL
    uint32_t ToSDLWindowFlags( const WindowFlags &flags )
    {
        uint32_t sdlFlags = 0;
        if ( flags.Fullscreen )
        {
            sdlFlags |= SDL_WINDOW_FULLSCREEN;
        }
        if ( flags.OpenGL )
        {
            sdlFlags |= SDL_WINDOW_OPENGL;
        }
        if ( flags.Shown )
        {
            sdlFlags |= SDL_WINDOW_SHOWN;
        }
        if ( flags.Hidden )
        {
            sdlFlags |= SDL_WINDOW_HIDDEN;
        }
        if ( flags.Borderless )
        {
            sdlFlags |= SDL_WINDOW_BORDERLESS;
        }
        if ( flags.Resizable )
        {
            sdlFlags |= SDL_WINDOW_RESIZABLE;
        }
        if ( flags.Minimized )
        {
            sdlFlags |= SDL_WINDOW_MINIMIZED;
        }
        if ( flags.Maximized )
        {
            sdlFlags |= SDL_WINDOW_MAXIMIZED;
        }
        if ( flags.InputGrabbed )
        {
            sdlFlags |= SDL_WINDOW_INPUT_GRABBED;
        }
        if ( flags.InputFocus )
        {
            sdlFlags |= SDL_WINDOW_INPUT_FOCUS;
        }
        if ( flags.MouseFocus )
        {
            sdlFlags |= SDL_WINDOW_MOUSE_FOCUS;
        }
        if ( flags.FullscreenDesktop )
        {
            sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        if ( flags.Foreign )
        {
            sdlFlags |= SDL_WINDOW_FOREIGN;
        }
        if ( flags.HighDPI )
        {
            sdlFlags |= SDL_WINDOW_ALLOW_HIGHDPI;
        }
        if ( flags.MouseCapture )
        {
            sdlFlags |= SDL_WINDOW_MOUSE_CAPTURE;
        }
        if ( flags.AlwaysOnTop )
        {
            sdlFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
        }
        if ( flags.SkipTaskbar )
        {
            sdlFlags |= SDL_WINDOW_SKIP_TASKBAR;
        }
        if ( flags.Utility )
        {
            sdlFlags |= SDL_WINDOW_UTILITY;
        }
        if ( flags.Tooltip )
        {
            sdlFlags |= SDL_WINDOW_TOOLTIP;
        }
        if ( flags.PopupMenu )
        {
            sdlFlags |= SDL_WINDOW_POPUP_MENU;
        }
#ifdef BUILD_VK
        {
            sdlFlags |= SDL_WINDOW_VULKAN;
        }
#endif
        return sdlFlags;
    }

    void FromSDLWindowFlags( WindowFlags &flags, const uint32_t sdlFlags )
    {
        flags.None              = sdlFlags == 0;
        flags.Fullscreen        = ( sdlFlags & SDL_WINDOW_FULLSCREEN ) != 0;
        flags.Shown             = ( sdlFlags & SDL_WINDOW_SHOWN ) != 0;
        flags.Hidden            = ( sdlFlags & SDL_WINDOW_HIDDEN ) != 0;
        flags.Borderless        = ( sdlFlags & SDL_WINDOW_BORDERLESS ) != 0;
        flags.Resizable         = ( sdlFlags & SDL_WINDOW_RESIZABLE ) != 0;
        flags.Minimized         = ( sdlFlags & SDL_WINDOW_MINIMIZED ) != 0;
        flags.Maximized         = ( sdlFlags & SDL_WINDOW_MAXIMIZED ) != 0;
        flags.InputGrabbed      = ( sdlFlags & SDL_WINDOW_INPUT_GRABBED ) != 0;
        flags.InputFocus        = ( sdlFlags & SDL_WINDOW_INPUT_FOCUS ) != 0;
        flags.MouseFocus        = ( sdlFlags & SDL_WINDOW_MOUSE_FOCUS ) != 0;
        flags.FullscreenDesktop = ( sdlFlags & SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0;
        flags.Foreign           = ( sdlFlags & SDL_WINDOW_FOREIGN ) != 0;
        flags.HighDPI           = ( sdlFlags & SDL_WINDOW_ALLOW_HIGHDPI ) != 0;
        flags.MouseCapture      = ( sdlFlags & SDL_WINDOW_MOUSE_CAPTURE ) != 0;
        flags.AlwaysOnTop       = ( sdlFlags & SDL_WINDOW_ALWAYS_ON_TOP ) != 0;
        flags.SkipTaskbar       = ( sdlFlags & SDL_WINDOW_SKIP_TASKBAR ) != 0;
        flags.Utility           = ( sdlFlags & SDL_WINDOW_UTILITY ) != 0;
        flags.Tooltip           = ( sdlFlags & SDL_WINDOW_TOOLTIP ) != 0;
        flags.PopupMenu         = ( sdlFlags & SDL_WINDOW_POPUP_MENU ) != 0;
    }
#endif
} // anonymous namespace

struct Window::Impl
{
    WindowDesc                            m_properties;
    std::unique_ptr<GraphicsWindowHandle> m_windowHandle;
    uint32_t                              m_windowID = 0;
#ifdef WINDOW_MANAGER_SDL
    SDL_Window *m_sdlWindow = nullptr;
#endif

    explicit Impl( WindowDesc properties ) : m_properties(std::move( properties ))
    {
        m_windowHandle = std::make_unique<GraphicsWindowHandle>( );
    }

    ~Impl( )
    {
        Destroy( );
    }

    void Destroy( )
    {
        if ( m_sdlWindow )
        {
            SDL_DestroyWindow( m_sdlWindow );
            m_sdlWindow = nullptr;
        }
        m_windowID = 0;
    }
};

Window::Window( const WindowDesc &properties ) : m_impl( std::make_unique<Impl>( properties ) )
{
#ifdef WINDOW_MANAGER_SDL
    const uint32_t flags = ToSDLWindowFlags( m_impl->m_properties.Flags );

    int x = m_impl->m_properties.X;
    int y = m_impl->m_properties.Y;

    if ( m_impl->m_properties.Position == WindowPosition::Centered )
    {
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;
    }

    m_impl->m_sdlWindow = SDL_CreateWindow( m_impl->m_properties.Title.Get( ), x, y, m_impl->m_properties.Width, m_impl->m_properties.Height, flags );

    if ( m_impl->m_sdlWindow )
    {
        m_impl->m_windowID     = SDL_GetWindowID( m_impl->m_sdlWindow );
        m_impl->m_windowHandle = std::make_unique<GraphicsWindowHandle>( );
        m_impl->m_windowHandle->CreateFromSDLWindowId( m_impl->m_windowID );
    }
#endif
}

Window::~Window( ) = default;

void Window::Destroy( ) const
{
    if ( m_impl )
    {
        m_impl->Destroy( );
    }
}

void Window::Show( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_ShowWindow( m_impl->m_sdlWindow );
    }
#endif
}

void Window::Hide( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_HideWindow( m_impl->m_sdlWindow );
    }
#endif
}

void Window::Minimize( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_MinimizeWindow( m_impl->m_sdlWindow );
    }
#endif
}

void Window::Maximize( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_MaximizeWindow( m_impl->m_sdlWindow );
    }
#endif
}

void Window::Raise( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_RaiseWindow( m_impl->m_sdlWindow );
    }
#endif
}

void Window::Restore( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_RestoreWindow( m_impl->m_sdlWindow );
    }
#endif
}

GraphicsWindowHandle* Window::GetGraphicsWindowHandle( ) const
{
    return m_impl->m_windowHandle.get( );
}

uint32_t Window::GetWindowID( ) const
{
    return m_impl ? m_impl->m_windowID : 0;
}

WindowSize Window::GetSize( ) const
{
    WindowSize size = { 0, 0 };

#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_GetWindowSize( m_impl->m_sdlWindow, &size.Width, &size.Height );
    }
#endif

    return size;
}

void Window::SetSize( const int width, const int height ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowSize( m_impl->m_sdlWindow, width, height );
    }
#endif
}

InteropString Window::GetTitle( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        return { SDL_GetWindowTitle( m_impl->m_sdlWindow ) };
    }
#endif

    return m_impl ? m_impl->m_properties.Title : InteropString{ };
}

void Window::SetTitle( const InteropString &title ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowTitle( m_impl->m_sdlWindow, title.Get( ) );
    }
#endif

    if ( m_impl )
    {
        m_impl->m_properties.Title = title;
    }
}

bool Window::GetFullscreen( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        const uint32_t flags = SDL_GetWindowFlags( m_impl->m_sdlWindow );
        return ( flags & SDL_WINDOW_FULLSCREEN ) != 0 || ( flags & SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0;
    }
#endif

    return m_impl ? ( m_impl->m_properties.Flags.Fullscreen || m_impl->m_properties.Flags.FullscreenDesktop ) : false;
}

void Window::SetFullscreen( const bool fullscreen ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowFullscreen( m_impl->m_sdlWindow, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );

        // Update flags
        const uint32_t flags = SDL_GetWindowFlags( m_impl->m_sdlWindow );
        FromSDLWindowFlags( m_impl->m_properties.Flags, flags );
    }
#endif
}

void Window::SetPosition( const int x, const int y ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowPosition( m_impl->m_sdlWindow, x, y );
    }
#endif

    if ( m_impl )
    {
        m_impl->m_properties.X = x;
        m_impl->m_properties.Y = y;
    }
}

WindowCoords Window::GetPosition( ) const
{
    WindowCoords result = { 0, 0 };
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_GetWindowPosition( m_impl->m_sdlWindow, &result.X, &result.Y );
        return result;
    }
#endif

    if ( m_impl )
    {
        result = { m_impl->m_properties.X, m_impl->m_properties.Y };
    }
    return result;
}

void Window::SetResizable( const bool resizable ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowResizable( m_impl->m_sdlWindow, resizable ? SDL_TRUE : SDL_FALSE );

        const uint32_t flags = SDL_GetWindowFlags( m_impl->m_sdlWindow );
        FromSDLWindowFlags( m_impl->m_properties.Flags, flags );
    }
#endif
}

void Window::SetBordered( const bool bordered ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowBordered( m_impl->m_sdlWindow, bordered ? SDL_TRUE : SDL_FALSE );

        const uint32_t flags = SDL_GetWindowFlags( m_impl->m_sdlWindow );
        FromSDLWindowFlags( m_impl->m_properties.Flags, flags );
    }
#endif
}

void Window::SetMinimumSize( const int minWidth, const int minHeight ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowMinimumSize( m_impl->m_sdlWindow, minWidth, minHeight );
    }
#endif
}

void Window::SetMaximumSize( const int maxWidth, const int maxHeight ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        SDL_SetWindowMaximumSize( m_impl->m_sdlWindow, maxWidth, maxHeight );
    }
#endif
}

bool Window::IsShown( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_impl && m_impl->m_sdlWindow )
    {
        const uint32_t flags = SDL_GetWindowFlags( m_impl->m_sdlWindow );
        return ( flags & SDL_WINDOW_SHOWN ) != 0;
    }
#endif

    return m_impl ? m_impl->m_properties.Flags.Shown : false;
}
