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

static const char *DENOFIZ_WINDOW_DATA_KEY = "denofiz";

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

    SDL_FlashOperation ToSDLFlashOperation( DenOfIz_FlashOperation op )
    {
        switch ( op )
        {
        case DENOFIZ_FLASH_BRIEFLY:
            return SDL_FLASH_BRIEFLY;
        case DENOFIZ_FLASH_UNTIL_FOCUSED:
            return SDL_FLASH_UNTIL_FOCUSED;
        case DENOFIZ_FLASH_CANCEL:
        default:
            return SDL_FLASH_CANCEL;
        }
    }
}


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
        void                         SetFullscreenMode( const DenOfIz_DisplayMode *mode );
        DenOfIz_DisplayMode          GetFullscreenMode( ) const;
        void                         SetPosition( int32_t x, int32_t y );
        int32_t                      GetPositionX( ) const;
        int32_t                      GetPositionY( ) const;
        void                         SetResizable( bool resizable );
        void                         SetBordered( bool bordered );
        void                         SetMinimumSize( int32_t minWidth, int32_t minHeight );
        DenOfIz_DisplaySize          GetMinimumSize( ) const;
        void                         SetMaximumSize( int32_t maxWidth, int32_t maxHeight );
        DenOfIz_DisplaySize          GetMaximumSize( ) const;
        void                         SetAspectRatio( float minAspect, float maxAspect );
        DenOfIz_AspectRatio          GetAspectRatio( ) const;
        bool                         IsShown( ) const;
        bool                         IsMinimized( ) const;
        bool                         IsMaximized( ) const;
        uint32_t                     GetFlags( ) const;
        void                         Sync( );

        void  SetIcon( const DenOfIz_WindowIconDesc *icon );
        void  Flash( DenOfIz_FlashOperation operation );
        void  SetOpacity( float opacity );
        float GetOpacity( ) const;

        void SetAlwaysOnTop( bool onTop );
        void SetFocusable( bool focusable );
        void SetModal( bool modal );

        DenOfIz_Window GetParent( ) const;
        void           SetParent( DenOfIz_Window parent );

        void         SetMouseGrab( bool grabbed );
        bool         GetMouseGrab( ) const;
        void         SetKeyboardGrab( bool grabbed );
        bool         GetKeyboardGrab( ) const;
        void         SetMouseRect( const DenOfIz_Rect *rect );
        DenOfIz_Rect GetMouseRect( ) const;
        void         WarpMouse( float x, float y );

        DenOfIz_WindowBordersSize GetBordersSize( ) const;
        DenOfIz_Rect              GetSafeArea( ) const;

        void                  SetProgressState( DenOfIz_ProgressState state );
        DenOfIz_ProgressState GetProgressState( ) const;
        void                  SetProgressValue( float value );
        float                 GetProgressValue( ) const;

        void     ShowSystemMenu( int32_t x, int32_t y );
        uint32_t GetDisplayID( ) const;
        uint32_t GetPixelFormat( ) const;
    };
}


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
        return;
    }

    SDL_SetPointerProperty( SDL_GetWindowProperties( m_sdlWindow ), DENOFIZ_WINDOW_DATA_KEY, this );

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
        return;
    }

    SDL_SetPointerProperty( SDL_GetWindowProperties( m_sdlWindow ), DENOFIZ_WINDOW_DATA_KEY, this );

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

void Window::SetFullscreenMode( const DenOfIz_DisplayMode *mode )
{
    if ( !m_sdlWindow )
    {
        return;
    }

    if ( mode == nullptr )
    {
        SDL_SetWindowFullscreenMode( m_sdlWindow, nullptr );
        return;
    }

    SDL_DisplayMode sdlMode = { };
    sdlMode.displayID       = mode->DisplayID;
    sdlMode.format          = static_cast< SDL_PixelFormat >( mode->Format );
    sdlMode.w               = mode->Width;
    sdlMode.h               = mode->Height;
    sdlMode.pixel_density   = mode->PixelDensity;
    sdlMode.refresh_rate    = mode->RefreshRate;
    SDL_SetWindowFullscreenMode( m_sdlWindow, &sdlMode );
}

DenOfIz_DisplayMode Window::GetFullscreenMode( ) const
{
    DenOfIz_DisplayMode result = { };
    if ( !m_sdlWindow )
    {
        return result;
    }

    const SDL_DisplayMode *sdlMode = SDL_GetWindowFullscreenMode( m_sdlWindow );
    if ( sdlMode )
    {
        result.DisplayID    = sdlMode->displayID;
        result.Format       = static_cast< uint32_t >( sdlMode->format );
        result.Width        = sdlMode->w;
        result.Height       = sdlMode->h;
        result.PixelDensity = sdlMode->pixel_density;
        result.RefreshRate  = sdlMode->refresh_rate;
    }
    return result;
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

DenOfIz_DisplaySize Window::GetMinimumSize( ) const
{
    DenOfIz_DisplaySize size = { 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_GetWindowMinimumSize( m_sdlWindow, &size.Width, &size.Height );
    }
    return size;
}

void Window::SetMaximumSize( const int32_t maxWidth, const int32_t maxHeight )
{
    SDL_SetWindowMaximumSize( m_sdlWindow, maxWidth, maxHeight );
}

DenOfIz_DisplaySize Window::GetMaximumSize( ) const
{
    DenOfIz_DisplaySize size = { 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_GetWindowMaximumSize( m_sdlWindow, &size.Width, &size.Height );
    }
    return size;
}

void Window::SetAspectRatio( const float minAspect, const float maxAspect )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowAspectRatio( m_sdlWindow, minAspect, maxAspect );
    }
}

DenOfIz_AspectRatio Window::GetAspectRatio( ) const
{
    DenOfIz_AspectRatio ratio = { 0.0f, 0.0f };
    if ( m_sdlWindow )
    {
        SDL_GetWindowAspectRatio( m_sdlWindow, &ratio.MinAspect, &ratio.MaxAspect );
    }
    return ratio;
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


void Window::SetIcon( const DenOfIz_WindowIconDesc *icon )
{
    if ( !m_sdlWindow || !icon || !icon->Pixels.Elements || icon->Width <= 0 || icon->Height <= 0 )
    {
        return;
    }

    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        icon->Width, icon->Height, SDL_PIXELFORMAT_RGBA32,
        const_cast< Byte * >( icon->Pixels.Elements ), icon->Width * 4 );

    if ( surface )
    {
        SDL_SetWindowIcon( m_sdlWindow, surface );
        SDL_DestroySurface( surface );
    }
}

void Window::Flash( const DenOfIz_FlashOperation operation )
{
    if ( m_sdlWindow )
    {
        SDL_FlashWindow( m_sdlWindow, ToSDLFlashOperation( operation ) );
    }
}

void Window::SetOpacity( const float opacity )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowOpacity( m_sdlWindow, opacity );
    }
}

float Window::GetOpacity( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowOpacity( m_sdlWindow );
    }
    return 1.0f;
}


void Window::SetAlwaysOnTop( const bool onTop )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowAlwaysOnTop( m_sdlWindow, onTop );
    }
}

void Window::SetFocusable( const bool focusable )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowFocusable( m_sdlWindow, focusable );
    }
}

void Window::SetModal( const bool modal )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowModal( m_sdlWindow, modal );
    }
}


DenOfIz_Window Window::GetParent( ) const
{
    if ( !m_sdlWindow )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    SDL_Window *parentSdl = SDL_GetWindowParent( m_sdlWindow );
    if ( !parentSdl )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    auto *parentWrapper = static_cast< Window * >(
        SDL_GetPointerProperty( SDL_GetWindowProperties( parentSdl ), DENOFIZ_WINDOW_DATA_KEY, nullptr ) );
    if ( !parentWrapper )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    return DENOFIZ_TO_HANDLE( parentWrapper );
}

void Window::SetParent( const DenOfIz_Window parent )
{
    if ( !m_sdlWindow )
    {
        return;
    }

    if ( !DENOFIZ_HANDLE_IS_VALID( parent ) )
    {
        SDL_SetWindowParent( m_sdlWindow, nullptr );
        return;
    }

    auto *parentImpl = WINDOW_IMPL( parent );
    SDL_SetWindowParent( m_sdlWindow, parentImpl->m_sdlWindow );
}


void Window::SetMouseGrab( const bool grabbed )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowMouseGrab( m_sdlWindow, grabbed );
    }
}

bool Window::GetMouseGrab( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowMouseGrab( m_sdlWindow );
    }
    return false;
}

void Window::SetKeyboardGrab( const bool grabbed )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowKeyboardGrab( m_sdlWindow, grabbed );
    }
}

bool Window::GetKeyboardGrab( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowKeyboardGrab( m_sdlWindow );
    }
    return false;
}

void Window::SetMouseRect( const DenOfIz_Rect *rect )
{
    if ( !m_sdlWindow )
    {
        return;
    }

    if ( rect == nullptr )
    {
        SDL_SetWindowMouseRect( m_sdlWindow, nullptr );
        return;
    }

    SDL_Rect sdlRect = { rect->X, rect->Y, rect->Width, rect->Height };
    SDL_SetWindowMouseRect( m_sdlWindow, &sdlRect );
}

DenOfIz_Rect Window::GetMouseRect( ) const
{
    DenOfIz_Rect result = { 0, 0, 0, 0 };
    if ( !m_sdlWindow )
    {
        return result;
    }

    const SDL_Rect *sdlRect = SDL_GetWindowMouseRect( m_sdlWindow );
    if ( sdlRect )
    {
        result.X      = sdlRect->x;
        result.Y      = sdlRect->y;
        result.Width  = sdlRect->w;
        result.Height = sdlRect->h;
    }
    return result;
}

void Window::WarpMouse( const float x, const float y )
{
    if ( m_sdlWindow )
    {
        SDL_WarpMouseInWindow( m_sdlWindow, x, y );
    }
}


DenOfIz_WindowBordersSize Window::GetBordersSize( ) const
{
    DenOfIz_WindowBordersSize borders = { 0, 0, 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_GetWindowBordersSize( m_sdlWindow, &borders.Top, &borders.Left, &borders.Bottom, &borders.Right );
    }
    return borders;
}

DenOfIz_Rect Window::GetSafeArea( ) const
{
    DenOfIz_Rect result = { 0, 0, 0, 0 };
    if ( m_sdlWindow )
    {
        SDL_Rect sdlRect = { };
        if ( SDL_GetWindowSafeArea( m_sdlWindow, &sdlRect ) )
        {
            result.X      = sdlRect.x;
            result.Y      = sdlRect.y;
            result.Width  = sdlRect.w;
            result.Height = sdlRect.h;
        }
    }
    return result;
}


void Window::SetProgressState( const DenOfIz_ProgressState state )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowProgressState( m_sdlWindow, static_cast< SDL_ProgressState >( state ) );
    }
}

DenOfIz_ProgressState Window::GetProgressState( ) const
{
    if ( m_sdlWindow )
    {
        return static_cast< DenOfIz_ProgressState >( SDL_GetWindowProgressState( m_sdlWindow ) );
    }
    return DENOFIZ_PROGRESS_STATE_NONE;
}

void Window::SetProgressValue( const float value )
{
    if ( m_sdlWindow )
    {
        SDL_SetWindowProgressValue( m_sdlWindow, value );
    }
}

float Window::GetProgressValue( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetWindowProgressValue( m_sdlWindow );
    }
    return 0.0f;
}


void Window::ShowSystemMenu( const int32_t x, const int32_t y )
{
    if ( m_sdlWindow )
    {
        SDL_ShowWindowSystemMenu( m_sdlWindow, x, y );
    }
}

uint32_t Window::GetDisplayID( ) const
{
    if ( m_sdlWindow )
    {
        return SDL_GetDisplayForWindow( m_sdlWindow );
    }
    return 0;
}

uint32_t Window::GetPixelFormat( ) const
{
    if ( m_sdlWindow )
    {
        return static_cast< uint32_t >( SDL_GetWindowPixelFormat( m_sdlWindow ) );
    }
    return 0;
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
        auto *parentWindow = WINDOW_IMPL( desc->Parent );
        auto *window       = new Window( *desc, parentWindow->m_sdlWindow );
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

    void DenOfIz_Window_SetSize( DenOfIz_Window window, int32_t width, int32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetSize( width, height );
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

    void DenOfIz_Window_SetFullscreenMode( DenOfIz_Window window, const DenOfIz_DisplayMode *mode )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetFullscreenMode( mode );
    }

    DenOfIz_DisplayMode DenOfIz_Window_GetFullscreenMode( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { };
        }
        return WINDOW_IMPL( window )->GetFullscreenMode( );
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

    DenOfIz_DisplaySize DenOfIz_Window_GetMinimumSize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0 };
        }
        return WINDOW_IMPL( window )->GetMinimumSize( );
    }

    void DenOfIz_Window_SetMaximumSize( DenOfIz_Window window, int32_t maxWidth, int32_t maxHeight )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetMaximumSize( maxWidth, maxHeight );
    }

    DenOfIz_DisplaySize DenOfIz_Window_GetMaximumSize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0 };
        }
        return WINDOW_IMPL( window )->GetMaximumSize( );
    }

    void DenOfIz_Window_SetAspectRatio( DenOfIz_Window window, float minAspect, float maxAspect )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetAspectRatio( minAspect, maxAspect );
    }

    DenOfIz_AspectRatio DenOfIz_Window_GetAspectRatio( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0.0f, 0.0f };
        }
        return WINDOW_IMPL( window )->GetAspectRatio( );
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

    
    void DenOfIz_Window_SetIcon( DenOfIz_Window window, const DenOfIz_WindowIconDesc *icon )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetIcon( icon );
    }

    void DenOfIz_Window_Flash( DenOfIz_Window window, DenOfIz_FlashOperation operation )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->Flash( operation );
    }

    void DenOfIz_Window_SetOpacity( DenOfIz_Window window, float opacity )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetOpacity( opacity );
    }

    float DenOfIz_Window_GetOpacity( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 1.0f;
        }
        return WINDOW_IMPL( window )->GetOpacity( );
    }

    
    void DenOfIz_Window_SetAlwaysOnTop( DenOfIz_Window window, bool onTop )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetAlwaysOnTop( onTop );
    }

    void DenOfIz_Window_SetFocusable( DenOfIz_Window window, bool focusable )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetFocusable( focusable );
    }

    void DenOfIz_Window_SetModal( DenOfIz_Window window, bool modal )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetModal( modal );
    }

    
    DenOfIz_Window DenOfIz_Window_GetParent( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return WINDOW_IMPL( window )->GetParent( );
    }

    void DenOfIz_Window_SetParent( DenOfIz_Window window, DenOfIz_Window parent )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetParent( parent );
    }

    
    void DenOfIz_Window_SetMouseGrab( DenOfIz_Window window, bool grabbed )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetMouseGrab( grabbed );
    }

    bool DenOfIz_Window_GetMouseGrab( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->GetMouseGrab( );
    }

    void DenOfIz_Window_SetKeyboardGrab( DenOfIz_Window window, bool grabbed )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetKeyboardGrab( grabbed );
    }

    bool DenOfIz_Window_GetKeyboardGrab( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return false;
        }
        return WINDOW_IMPL( window )->GetKeyboardGrab( );
    }

    void DenOfIz_Window_SetMouseRect( DenOfIz_Window window, const DenOfIz_Rect *rect )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetMouseRect( rect );
    }

    DenOfIz_Rect DenOfIz_Window_GetMouseRect( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0, 0, 0 };
        }
        return WINDOW_IMPL( window )->GetMouseRect( );
    }

    void DenOfIz_Window_WarpMouse( DenOfIz_Window window, float x, float y )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->WarpMouse( x, y );
    }

    
    DenOfIz_WindowBordersSize DenOfIz_Window_GetBordersSize( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0, 0, 0 };
        }
        return WINDOW_IMPL( window )->GetBordersSize( );
    }

    DenOfIz_Rect DenOfIz_Window_GetSafeArea( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return { 0, 0, 0, 0 };
        }
        return WINDOW_IMPL( window )->GetSafeArea( );
    }

    
    void DenOfIz_Window_SetProgressState( DenOfIz_Window window, DenOfIz_ProgressState state )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetProgressState( state );
    }

    DenOfIz_ProgressState DenOfIz_Window_GetProgressState( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return DENOFIZ_PROGRESS_STATE_NONE;
        }
        return WINDOW_IMPL( window )->GetProgressState( );
    }

    void DenOfIz_Window_SetProgressValue( DenOfIz_Window window, float value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->SetProgressValue( value );
    }

    float DenOfIz_Window_GetProgressValue( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0.0f;
        }
        return WINDOW_IMPL( window )->GetProgressValue( );
    }

    
    void DenOfIz_Window_ShowSystemMenu( DenOfIz_Window window, int32_t x, int32_t y )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }
        WINDOW_IMPL( window )->ShowSystemMenu( x, y );
    }

    uint32_t DenOfIz_Window_GetDisplayID( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetDisplayID( );
    }

    uint32_t DenOfIz_Window_GetPixelFormat( DenOfIz_Window window )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return 0;
        }
        return WINDOW_IMPL( window )->GetPixelFormat( );
    }
}
