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

#include <string>
#include <utility>

#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

#define WINDOW_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Window, handle )

namespace
{
    uint32_t ToSDLWindowFlags( const DenOfIz_WindowFlags &flags )
    {
        uint32_t sdlFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if ( flags.Fullscreen )
        {
            sdlFlags |= SDL_WINDOW_FULLSCREEN;
        }
        if ( flags.OpenGL )
        {
            sdlFlags |= SDL_WINDOW_OPENGL;
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
        if ( flags.MouseGrabbed )
        {
            sdlFlags |= SDL_WINDOW_MOUSE_GRABBED;
        }
        if ( flags.InputFocus )
        {
            sdlFlags |= SDL_WINDOW_INPUT_FOCUS;
        }
        if ( flags.MouseFocus )
        {
            sdlFlags |= SDL_WINDOW_MOUSE_FOCUS;
        }
        if ( flags.MouseCapture )
        {
            sdlFlags |= SDL_WINDOW_MOUSE_CAPTURE;
        }
        if ( flags.AlwaysOnTop )
        {
            sdlFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
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
        sdlFlags |= SDL_WINDOW_VULKAN;
#endif
        return sdlFlags;
    }

    void FromSDLWindowFlags( DenOfIz_WindowFlags &flags, const uint32_t sdlFlags )
    {
        flags.None         = sdlFlags == 0;
        flags.Fullscreen   = ( sdlFlags & SDL_WINDOW_FULLSCREEN ) != 0;
        flags.Hidden       = ( sdlFlags & SDL_WINDOW_HIDDEN ) != 0;
        flags.Borderless   = ( sdlFlags & SDL_WINDOW_BORDERLESS ) != 0;
        flags.Resizable    = ( sdlFlags & SDL_WINDOW_RESIZABLE ) != 0;
        flags.Minimized    = ( sdlFlags & SDL_WINDOW_MINIMIZED ) != 0;
        flags.Maximized    = ( sdlFlags & SDL_WINDOW_MAXIMIZED ) != 0;
        flags.MouseGrabbed = ( sdlFlags & SDL_WINDOW_MOUSE_GRABBED ) != 0;
        flags.InputFocus   = ( sdlFlags & SDL_WINDOW_INPUT_FOCUS ) != 0;
        flags.MouseFocus   = ( sdlFlags & SDL_WINDOW_MOUSE_FOCUS ) != 0;
        flags.MouseCapture = ( sdlFlags & SDL_WINDOW_MOUSE_CAPTURE ) != 0;
        flags.AlwaysOnTop  = ( sdlFlags & SDL_WINDOW_ALWAYS_ON_TOP ) != 0;
        flags.Utility      = ( sdlFlags & SDL_WINDOW_UTILITY ) != 0;
        flags.Tooltip      = ( sdlFlags & SDL_WINDOW_TOOLTIP ) != 0;
        flags.PopupMenu    = ( sdlFlags & SDL_WINDOW_POPUP_MENU ) != 0;
    }
} // anonymous namespace

namespace DenOfIz
{
    class Window
    {
    public:
        std::string                  m_title;
        DenOfIz_WindowDesc           m_properties;
        DenOfIz_GraphicsWindowHandle m_windowHandle = DENOFIZ_NULL_HANDLE;
        uint32_t                     m_windowID     = 0;
        SDL_Window                  *m_sdlWindow    = nullptr;

        explicit Window( const DenOfIz_WindowDesc &desc );
        Window( const DenOfIz_PopupWindowDesc &desc, SDL_Window *parentSdlWindow );
        ~Window( );

        void                         Destroy( );
        void                         Show( );
        void                         Hide( );
        void                         Minimize( );
        void                         Maximize( );
        void                         Raise( );
        void                         Restore( );
        DenOfIz_GraphicsWindowHandle GetGraphicsWindowHandle( ) const;
        uint32_t                     GetWindowID( ) const;
        DenOfIz_DisplaySize          GetSize( ) const;
        DenOfIz_DisplaySize          GetSizeInPixels( ) const;
        void                         SetSize( int32_t width, int32_t height );
        float                        GetPixelDensity( ) const;
        float                        GetDisplayScale( ) const;
        DenOfIz_StringView           GetTitle( ) const;
        void                         SetTitle( DenOfIz_StringView title );
        bool                         GetFullscreen( ) const;
        void                         SetFullscreen( bool fullscreen );
        void                         SetPosition( int32_t x, int32_t y );
        int32_t                      GetPositionX( ) const;
        int32_t                      GetPositionY( ) const;
        void                         SetResizable( bool resizable );
        void                         SetBordered( bool bordered );
        void                         SetMinimumSize( int32_t minWidth, int32_t minHeight );
        void                         SetMaximumSize( int32_t maxWidth, int32_t maxHeight );
        bool                         IsShown( ) const;
        bool                         IsMinimized( ) const;
        bool                         IsMaximized( ) const;
        uint32_t                     GetFlags( ) const;
        void                         Sync( );
    };
} // namespace DenOfIz

using namespace DenOfIz;

Window::Window( const DenOfIz_WindowDesc &desc ) : m_properties( desc )
{
    const uint32_t flags = ToSDLWindowFlags( m_properties.Flags );
    m_title              = StringViewToStdString( desc.Title );

    int x = m_properties.X;
    int y = m_properties.Y;

    if ( m_properties.Position == DENOFIZ_WINDOW_POSITION_CENTERED )
    {
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;
    }

    m_sdlWindow = SDL_CreateWindow( m_title.c_str( ), m_properties.Width, m_properties.Height, flags );
    if ( !m_sdlWindow )
    {
        spdlog::critical( "Unable to create SDL window: {}", SDL_GetError( ) );
    }

    m_windowID     = SDL_GetWindowID( m_sdlWindow );
    m_windowHandle = DenOfIz_GraphicsWindowHandle_Create( );
    DenOfIz_GraphicsWindowHandle_CreateFromSDLWindowId( m_windowHandle, m_windowID );
}

Window::Window( const DenOfIz_PopupWindowDesc &desc, SDL_Window *parentSdlWindow )
{
    const uint32_t flags = ToSDLWindowFlags( desc.Flags );

    m_sdlWindow = SDL_CreatePopupWindow( parentSdlWindow, desc.OffsetX, desc.OffsetY, desc.Width, desc.Height, flags );
    if ( !m_sdlWindow )
    {
        spdlog::critical( "Unable to create SDL popup window: {}", SDL_GetError( ) );
    }

    m_windowID     = SDL_GetWindowID( m_sdlWindow );
    m_windowHandle = DenOfIz_GraphicsWindowHandle_Create( );
    DenOfIz_GraphicsWindowHandle_CreateFromSDLWindowId( m_windowHandle, m_windowID );
}

Window::~Window( )
{
    Destroy( );
}

void Window::Destroy( )
{
    SDL_DestroyWindow( m_sdlWindow );
    m_sdlWindow = nullptr;
    if ( DENOFIZ_HANDLE_IS_VALID( m_windowHandle ) )
    {
        DenOfIz_GraphicsWindowHandle_Destroy( m_windowHandle );
        m_windowHandle = DENOFIZ_NULL_HANDLE;
    }
}

void Window::Show( )
{
    SDL_ShowWindow( m_sdlWindow );
}

void Window::Hide( )
{
    SDL_HideWindow( m_sdlWindow );
}

void Window::Minimize( )
{
    SDL_MinimizeWindow( m_sdlWindow );
}

void Window::Maximize( )
{
    SDL_MaximizeWindow( m_sdlWindow );
}

void Window::Raise( )
{
    SDL_RaiseWindow( m_sdlWindow );
}

void Window::Restore( )
{
    SDL_RestoreWindow( m_sdlWindow );
}

DenOfIz_GraphicsWindowHandle Window::GetGraphicsWindowHandle( ) const
{
    return m_windowHandle;
}

uint32_t Window::GetWindowID( ) const
{
    return m_windowID;
}

DenOfIz_DisplaySize Window::GetSize( ) const
{
    DenOfIz_DisplaySize size = { 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_GetWindowSize( m_sdlWindow, &size.Width, &size.Height );
    }
    return size;
}

DenOfIz_DisplaySize Window::GetSizeInPixels( ) const
{
    DenOfIz_DisplaySize size = { 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_GetWindowSizeInPixels( m_sdlWindow, &size.Width, &size.Height );
    }
    return size;
}

float Window::GetPixelDensity( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowPixelDensity( m_sdlWindow );
    }
    return 1.0f;
}

float Window::GetDisplayScale( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowDisplayScale( m_sdlWindow );
    }
    return 1.0f;
}

void Window::SetSize( const int32_t width, const int32_t height )
{
    SDL_SetWindowSize( m_sdlWindow, width, height );
}

DenOfIz_StringView Window::GetTitle( ) const
{
    return { SDL_GetWindowTitle( m_sdlWindow ) };
}

void Window::SetTitle( const DenOfIz_StringView title )
{
    if ( title.Chars && title.NumChars > 0 )
    {
        m_title.assign( title.Chars, title.Chars + title.NumChars );
    }

    SDL_SetWindowTitle( m_sdlWindow, m_title.c_str( ) );
    m_properties.Title = DENOFIZ_STRING( m_title.c_str( ) );
}

bool Window::GetFullscreen( ) const
{
    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    return ( flags & SDL_WINDOW_FULLSCREEN ) != 0;
}

void Window::SetFullscreen( const bool fullscreen )
{
    SDL_SetWindowFullscreen( m_sdlWindow, fullscreen );

    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    FromSDLWindowFlags( m_properties.Flags, flags );
}

void Window::SetPosition( const int32_t x, const int32_t y )
{
    SDL_SetWindowPosition( m_sdlWindow, x, y );
    m_properties.X = x;
    m_properties.Y = y;
}

int32_t Window::GetPositionX( ) const
{
    int x = 0;
    SDL_GetWindowPosition( m_sdlWindow, &x, nullptr );
    return x;
}

int32_t Window::GetPositionY( ) const
{
    int y = 0;
    SDL_GetWindowPosition( m_sdlWindow, nullptr, &y );
    return y;
}

void Window::SetResizable( const bool resizable )
{
    SDL_SetWindowResizable( m_sdlWindow, resizable );

    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    FromSDLWindowFlags( m_properties.Flags, flags );
}

void Window::SetBordered( const bool bordered )
{
    SDL_SetWindowBordered( m_sdlWindow, bordered );

    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    FromSDLWindowFlags( m_properties.Flags, flags );
}

void Window::SetMinimumSize( const int32_t minWidth, const int32_t minHeight )
{
    SDL_SetWindowMinimumSize( m_sdlWindow, minWidth, minHeight );
}

void Window::SetMaximumSize( const int32_t maxWidth, const int32_t maxHeight )
{
    SDL_SetWindowMaximumSize( m_sdlWindow, maxWidth, maxHeight );
}

bool Window::IsShown( ) const
{
    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    return ( flags & SDL_WINDOW_HIDDEN ) == 0;
}

bool Window::IsMinimized( ) const
{
    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    return ( flags & SDL_WINDOW_MINIMIZED ) != 0;
}

bool Window::IsMaximized( ) const
{
    const uint32_t flags = SDL_GetWindowFlags( m_sdlWindow );
    return ( flags & SDL_WINDOW_MAXIMIZED ) != 0;
}

uint32_t Window::GetFlags( ) const
{
    return SDL_GetWindowFlags( m_sdlWindow );
}

void Window::Sync( )
{
    SDL_SyncWindow( m_sdlWindow );
}

extern "C"
{

    DenOfIz_Window DenOfIz_Window_Create( const DenOfIz_WindowDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *window = new Window( *desc );
        return DENOFIZ_TO_HANDLE( window );
    }

    DenOfIz_Window DenOfIz_Window_CreatePopup( const DenOfIz_PopupWindowDesc *desc )
    {
        if ( desc == NULL || !DENOFIZ_HANDLE_IS_VALID( desc->Parent ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *parentWindow   = WINDOW_IMPL( desc->Parent );
        auto *window         = new Window( *desc, parentWindow->m_sdlWindow );
        return DENOFIZ_TO_HANDLE( window );
    }

    void DenOfIz_Window_Destroy( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        delete WINDOW_IMPL( window );
    }

    void DenOfIz_Window_Show( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Show( );
    }

    void DenOfIz_Window_Hide( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Hide( );
    }

    void DenOfIz_Window_Minimize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Minimize( );
    }

    void DenOfIz_Window_Maximize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Maximize( );
    }

    void DenOfIz_Window_Raise( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Raise( );
    }

    void DenOfIz_Window_Restore( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Restore( );
    }

    DenOfIz_GraphicsWindowHandle DenOfIz_Window_GetGraphicsWindowHandle( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return WINDOW_IMPL( window )->GetGraphicsWindowHandle( );
    }

    uint32_t DenOfIz_Window_GetWindowID( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetWindowID( );
    }

    DenOfIz_DisplaySize DenOfIz_Window_GetSize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0 };
        }
        return WINDOW_IMPL( window )->GetSize( );
    }

    DenOfIz_DisplaySize DenOfIz_Window_GetSizeInPixels( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0 };
        }
        return WINDOW_IMPL( window )->GetSizeInPixels( );
    }

    float DenOfIz_Window_GetPixelDensity( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 1.0f;
        }
        return WINDOW_IMPL( window )->GetPixelDensity( );
    }

    float DenOfIz_Window_GetDisplayScale( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 1.0f;
        }
        return WINDOW_IMPL( window )->GetDisplayScale( );
    }

    void DenOfIz_Window_SetSize( DenOfIz_Window window, int32_t width, int32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetSize( width, height );
    }

    DenOfIz_StringView DenOfIz_Window_GetTitle( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { };
        }
        return WINDOW_IMPL( window )->GetTitle( );
    }

    void DenOfIz_Window_SetTitle( DenOfIz_Window window, DenOfIz_StringView title )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetTitle( title );
    }

    bool DenOfIz_Window_GetFullscreen( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->GetFullscreen( );
    }

    void DenOfIz_Window_SetFullscreen( DenOfIz_Window window, bool fullscreen )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetFullscreen( fullscreen );
    }

    void DenOfIz_Window_SetPosition( DenOfIz_Window window, int32_t x, int32_t y )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetPosition( x, y );
    }

    int32_t DenOfIz_Window_GetPositionX( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetPositionX( );
    }

    int32_t DenOfIz_Window_GetPositionY( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetPositionY( );
    }

    void DenOfIz_Window_SetResizable( DenOfIz_Window window, bool resizable )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetResizable( resizable );
    }

    void DenOfIz_Window_SetBordered( DenOfIz_Window window, bool bordered )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetBordered( bordered );
    }

    void DenOfIz_Window_SetMinimumSize( DenOfIz_Window window, int32_t minWidth, int32_t minHeight )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetMinimumSize( minWidth, minHeight );
    }

    void DenOfIz_Window_SetMaximumSize( DenOfIz_Window window, int32_t maxWidth, int32_t maxHeight )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetMaximumSize( maxWidth, maxHeight );
    }

    bool DenOfIz_Window_IsShown( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->IsShown( );
    }

    bool DenOfIz_Window_IsMinimized( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->IsMinimized( );
    }

    bool DenOfIz_Window_IsMaximized( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->IsMaximized( );
    }

    uint32_t DenOfIz_Window_GetFlags( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetFlags( );
    }

    void DenOfIz_Window_Sync( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Sync( );
    }
}
