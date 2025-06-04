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

#include "DenOfIzGraphics/Input/Window.h"

using namespace DenOfIz;

Window::Window( const WindowDesc &properties ) : m_properties( properties ), m_windowID( 0 ), m_sdlWindow( nullptr )
{
#ifdef WINDOW_MANAGER_SDL
    const uint32_t flags = m_properties.Flags.ToSDLWindowFlags( );

    int x = m_properties.X;
    int y = m_properties.Y;

    if ( m_properties.Position == WindowPosition::Centered )
    {
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;
    }

    m_sdlWindow = SDL_CreateWindow( m_properties.Title.Get( ), x, y, m_properties.Width, m_properties.Height, flags );

    if ( m_sdlWindow )
    {
        m_windowID     = SDL_GetWindowID( m_sdlWindow );
        m_windowHandle = { };
        m_windowHandle.CreateFromSDLWindow( m_sdlWindow );
    }
#endif
}

Window::~Window( )
{
    Destroy( );
}

void Window::Destroy( )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_DestroyWindow( m_sdlWindow );
        m_sdlWindow = nullptr;
    }
#endif
    m_windowID = 0;
}

void Window::Show( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_ShowWindow( m_sdlWindow );
    }
#endif
}

void Window::Hide( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_HideWindow( m_sdlWindow );
    }
#endif
}

void Window::Minimize( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_MinimizeWindow( m_sdlWindow );
    }
#endif
}

void Window::Maximize( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_MaximizeWindow( m_sdlWindow );
    }
#endif
}

void Window::Raise( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_RaiseWindow( m_sdlWindow );
    }
#endif
}

void Window::Restore( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_RestoreWindow( m_sdlWindow );
    }
#endif
}

GraphicsWindowHandle Window::GetGraphicsWindowHandle( ) const
{
    return m_windowHandle;
}

uint32_t Window::GetWindowID( ) const
{
    return m_windowID;
}

WindowSize Window::GetSize( ) const
{
    WindowSize size = { 0, 0 };

#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_GetWindowSize( m_sdlWindow, &size.Width, &size.Height );
    }
#endif

    return size;
}

void Window::SetSize( const int width, const int height ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowSize( m_sdlWindow, width, height );
    }
#endif
}

InteropString Window::GetTitle( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        return { SDL_GetWindowTitle( m_sdlWindow ) };
    }
#endif

    return m_properties.Title;
}

void Window::SetTitle( const InteropString &title )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowTitle( m_sdlWindow, title.Get( ) );
    }
#endif

    m_properties.Title = title;
}

bool Window::GetFullscreen( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
        return ( flags & SDL_WINDOW_FULLSCREEN ) != 0 || ( flags & SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0;
    }
#endif

    return m_properties.Flags.Fullscreen || m_properties.Flags.FullscreenDesktop;
}

void Window::SetFullscreen( const bool fullscreen )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowFullscreen( m_sdlWindow, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );

        // Update flags
        const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
        m_properties.Flags.FromSDLWindowFlags( flags );
    }
#endif
}

void Window::SetPosition( const int x, const int y )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowPosition( m_sdlWindow, x, y );
    }
#endif

    m_properties.X = x;
    m_properties.Y = y;
}

WindowCoords Window::GetPosition( ) const
{
    WindowCoords result = { 0, 0 };
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_GetWindowPosition( m_sdlWindow, &result.X, &result.Y );
        return result;
    }
#endif

    result = { m_properties.X, m_properties.Y };
    return result;
}

void Window::SetResizable( const bool resizable )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowResizable( m_sdlWindow, resizable ? SDL_TRUE : SDL_FALSE );

        const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
        m_properties.Flags.FromSDLWindowFlags( flags );
    }
#endif
}

void Window::SetBordered( const bool bordered )
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowBordered( m_sdlWindow, bordered ? SDL_TRUE : SDL_FALSE );

        const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
        m_properties.Flags.FromSDLWindowFlags( flags );
    }
#endif
}

void Window::SetMinimumSize( const int minWidth, const int minHeight ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowMinimumSize( m_sdlWindow, minWidth, minHeight );
    }
#endif
}

void Window::SetMaximumSize( const int maxWidth, const int maxHeight ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        SDL_SetWindowMaximumSize( m_sdlWindow, maxWidth, maxHeight );
    }
#endif
}

bool Window::IsShown( ) const
{
#ifdef WINDOW_MANAGER_SDL
    if ( m_sdlWindow )
    {
        const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
        return ( flags & SDL_WINDOW_SHOWN ) != 0;
    }
#endif

    return m_properties.Flags.Shown;
}

SDL_Window *Window::GetSDLWindow( ) const
{
    return m_sdlWindow;
}
