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

#include "DenOfIzGraphics/Input/InputSystem.h"

#include <cstring>
#include <string>

#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"

#define INPUT_SYSTEM_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::InputSystem, handle )
#define WINDOW_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Window, handle )

namespace DenOfIz
{
    class Window;

    class InternalController
    {
    public:
        bool          m_initialized     = false;
        int           m_controllerIndex = -1;
        uint32_t      m_instanceID      = 0;
        SDL_Gamepad  *m_gameController  = nullptr;
        SDL_Joystick *m_joystick        = nullptr;
        std::string   m_mappingCache;

        static void        InitializeSDL( );
        bool               Open( int controllerIndex );
        void               Close( );
        bool               IsButtonPressed( DenOfIz_ControllerButton button ) const;
        int16_t            GetAxisValue( DenOfIz_ControllerAxis axis ) const;
        bool               SetRumble( uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs );
        bool               IsConnected( ) const;
        DenOfIz_StringView GetName( ) const;
        bool               SetPlayerIndex( int playerIndex );
        int                GetPlayerIndex( ) const;
        static bool        IsGameController( int joystickIndex );
        static int         GetControllerCount( );
    };

    class InputSystem
    {
    public:
        InternalController   m_controllers[ 4 ];
        bool                 m_controllerInitialized[ 4 ]                                          = { false, false, false, false };
        SDL_Cursor          *m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_COUNT ) ] = { nullptr };
        SDL_Cursor          *m_defaultCursor                                                       = nullptr;
        DenOfIz_SystemCursor m_currentCursor                                                       = DENOFIZ_SYSTEM_CURSOR_DEFAULT;

        InputSystem( );
        ~InputSystem( );

        void InitializeCursors( );
        void DestroyCursors( );

        static DenOfIz_MouseButtonState FromSDLButtonState( uint32_t state );
        static uint32_t                 ToSDLButtonState( const DenOfIz_MouseButtonState &buttons );
        static void                     ConvertSDLEventToEvent( const SDL_Event &sdlEvent, DenOfIz_Event &outEvent );
        static uint32_t                 ConvertKeyMod( SDL_Keymod sdlMods );
        static SDL_Keymod               ConvertToSdlKeyMod( uint32_t modifiers );

        bool               OpenController( int playerIndex, int controllerIndex );
        void               CloseController( int playerIndex );
        bool               IsControllerConnected( int playerIndex ) const;
        DenOfIz_Controller GetController( int playerIndex );
        bool               IsControllerButtonPressed( int playerIndex, DenOfIz_ControllerButton button ) const;
        int16_t            GetControllerAxisValue( int playerIndex, DenOfIz_ControllerAxis axis ) const;
        DenOfIz_StringView GetControllerName( int playerIndex ) const;
        bool               SetControllerRumble( int playerIndex, uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs );
    };
} // namespace DenOfIz

using namespace DenOfIz;

static bool s_sdlControllerInitialized = false;

void InternalController::InitializeSDL( )
{
    if ( !s_sdlControllerInitialized )
    {
        if ( SDL_WasInit( SDL_INIT_GAMEPAD ) == 0 )
        {
            if ( SDL_InitSubSystem( SDL_INIT_GAMEPAD ) != 0 )
            {
            }
        }
        s_sdlControllerInitialized = true;
    }
}

bool InternalController::Open( const int controllerIndex )
{
    Close( );
    InitializeSDL( );

    if ( !IsGameController( controllerIndex ) )
    {
        return false;
    }

    m_gameController = SDL_OpenGamepad( controllerIndex );
    if ( m_gameController == nullptr )
    {
        return false;
    }

    m_joystick = SDL_GetGamepadJoystick( m_gameController );
    if ( m_joystick == nullptr )
    {
        SDL_CloseGamepad( m_gameController );
        m_gameController = nullptr;
        return false;
    }

    m_controllerIndex = controllerIndex;
    m_instanceID      = SDL_GetJoystickID( m_joystick );
    m_initialized     = true;
    return true;
}

void InternalController::Close( )
{
    if ( !m_initialized )
    {
        return;
    }

    if ( m_gameController != nullptr )
    {
        SDL_CloseGamepad( m_gameController );
        m_gameController = nullptr;
        m_joystick       = nullptr;
    }

    m_controllerIndex = -1;
    m_instanceID      = 0;
    m_initialized     = false;
}

bool InternalController::IsButtonPressed( const DenOfIz_ControllerButton button ) const
{
    if ( !m_initialized )
    {
        return false;
    }
    return SDL_GetGamepadButton( m_gameController, static_cast<SDL_GamepadButton>( button ) ) == 1;
}

int16_t InternalController::GetAxisValue( const DenOfIz_ControllerAxis axis ) const
{
    if ( !m_initialized )
    {
        return 0;
    }
    return SDL_GetGamepadAxis( m_gameController, static_cast<SDL_GamepadAxis>( axis ) );
}

bool InternalController::SetRumble( const uint16_t lowFrequencyRumble, const uint16_t highFrequencyRumble, const uint32_t durationMs )
{
    if ( !m_initialized )
    {
        return false;
    }
    return false;
}

bool InternalController::IsConnected( ) const
{
    if ( !m_initialized )
    {
        return false;
    }
    return SDL_GamepadConnected( m_gameController );
}

DenOfIz_StringView InternalController::GetName( ) const
{
    if ( !m_initialized )
    {
        return { };
    }
    return { SDL_GetGamepadName( m_gameController ) };
}

bool InternalController::SetPlayerIndex( const int playerIndex )
{
    if ( !m_initialized )
    {
        return false;
    }
    SDL_SetGamepadPlayerIndex( m_gameController, playerIndex );
    return true;
}

int InternalController::GetPlayerIndex( ) const
{
    if ( !m_initialized )
    {
        return -1;
    }
    return SDL_GetGamepadPlayerIndex( m_gameController );
}

bool InternalController::IsGameController( const int joystickIndex )
{
    InitializeSDL( );
    return SDL_IsGamepad( joystickIndex );
}

int InternalController::GetControllerCount( )
{
    InitializeSDL( );
    int count = 0;
    int numJoysticks;
    SDL_GetJoysticks( &numJoysticks );
    for ( int i = 0; i < numJoysticks; i++ )
    {
        if ( SDL_IsGamepad( i ) )
        {
            count++;
        }
    }
    return count;
}

InputSystem::InputSystem( )
{
    for ( bool &controllerInitialized : m_controllerInitialized )
    {
        controllerInitialized = false;
    }

    m_defaultCursor = SDL_GetDefaultCursor( );
    InitializeCursors( );
}

InputSystem::~InputSystem( )
{
    for ( int i = 0; i < 4; ++i )
    {
        if ( m_controllerInitialized[ i ] )
        {
            m_controllers[ i ].Close( );
            m_controllerInitialized[ i ] = false;
        }
    }

    DestroyCursors( );
}

void InputSystem::InitializeCursors( )
{
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_DEFAULT ) ]     = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_DEFAULT );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_TEXT ) ]        = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_TEXT );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_WAIT ) ]        = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_WAIT );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_CROSSHAIR ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_CROSSHAIR );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_PROGRESS ) ]    = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_PROGRESS );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NWSE_RESIZE ) ] = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NWSE_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NESW_RESIZE ) ] = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NESW_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_EW_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_EW_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NS_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NS_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_MOVE ) ]        = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_MOVE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NOT_ALLOWED ) ] = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NOT_ALLOWED );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_POINTER ) ]     = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_POINTER );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NW_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NW_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_N_RESIZE ) ]    = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_N_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_NE_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NE_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_E_RESIZE ) ]    = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_E_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_SE_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SE_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_S_RESIZE ) ]    = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_S_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_SW_RESIZE ) ]   = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SW_RESIZE );
    m_systemCursors[ static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_W_RESIZE ) ]    = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_W_RESIZE );
}

void InputSystem::DestroyCursors( )
{
    for ( size_t i = 0; i < static_cast<size_t>( DENOFIZ_SYSTEM_CURSOR_COUNT ); ++i )
    {
        if ( m_systemCursors[ i ] != nullptr )
        {
            SDL_DestroyCursor( m_systemCursors[ i ] );
            m_systemCursors[ i ] = nullptr;
        }
    }
}

DenOfIz_MouseButtonState InputSystem::FromSDLButtonState( const uint32_t state )
{
    DenOfIz_MouseButtonState result = { };
    result.LeftButton               = ( state & SDL_BUTTON_LMASK ) != 0;
    result.MiddleButton             = ( state & SDL_BUTTON_MMASK ) != 0;
    result.RightButton              = ( state & SDL_BUTTON_RMASK ) != 0;
    result.X1Button                 = ( state & SDL_BUTTON_X1MASK ) != 0;
    result.X2Button                 = ( state & SDL_BUTTON_X2MASK ) != 0;
    return result;
}

uint32_t InputSystem::ToSDLButtonState( const DenOfIz_MouseButtonState &buttons )
{
    uint32_t state = 0;
    if ( buttons.LeftButton )
    {
        state |= SDL_BUTTON_LMASK;
    }
    if ( buttons.MiddleButton )
    {
        state |= SDL_BUTTON_MMASK;
    }
    if ( buttons.RightButton )
    {
        state |= SDL_BUTTON_RMASK;
    }
    if ( buttons.X1Button )
    {
        state |= SDL_BUTTON_X1MASK;
    }
    if ( buttons.X2Button )
    {
        state |= SDL_BUTTON_X2MASK;
    }
    return state;
}

uint32_t InputSystem::ConvertKeyMod( const SDL_Keymod sdlMods )
{
    uint32_t result = DENOFIZ_KEY_MOD_NONE;
    if ( sdlMods & SDL_KMOD_LSHIFT )
    {
        result = result | DENOFIZ_KEY_MOD_LSHIFT;
    }
    if ( sdlMods & SDL_KMOD_RSHIFT )
    {
        result = result | DENOFIZ_KEY_MOD_RSHIFT;
    }
    if ( sdlMods & SDL_KMOD_LCTRL )
    {
        result = result | DENOFIZ_KEY_MOD_LCTRL;
    }
    if ( sdlMods & SDL_KMOD_RCTRL )
    {
        result = result | DENOFIZ_KEY_MOD_RCTRL;
    }
    if ( sdlMods & SDL_KMOD_LALT )
    {
        result = result | DENOFIZ_KEY_MOD_LALT;
    }
    if ( sdlMods & SDL_KMOD_RALT )
    {
        result = result | DENOFIZ_KEY_MOD_RALT;
    }
    if ( sdlMods & SDL_KMOD_LGUI )
    {
        result = result | DENOFIZ_KEY_MOD_LGUI;
    }
    if ( sdlMods & SDL_KMOD_RGUI )
    {
        result = result | DENOFIZ_KEY_MOD_RGUI;
    }
    if ( sdlMods & SDL_KMOD_NUM )
    {
        result = result | DENOFIZ_KEY_MOD_NUMLOCK;
    }
    if ( sdlMods & SDL_KMOD_CAPS )
    {
        result = result | DENOFIZ_KEY_MOD_CAPSLOCK;
    }
    if ( sdlMods & SDL_KMOD_MODE )
    {
        result = result | DENOFIZ_KEY_MOD_ALTGR;
    }
    if ( sdlMods & SDL_KMOD_SHIFT )
    {
        result = result | DENOFIZ_KEY_MOD_SHIFT;
    }
    if ( sdlMods & SDL_KMOD_CTRL )
    {
        result = result | DENOFIZ_KEY_MOD_CTRL;
    }
    if ( sdlMods & SDL_KMOD_ALT )
    {
        result = result | DENOFIZ_KEY_MOD_ALT;
    }
    if ( sdlMods & SDL_KMOD_GUI )
    {
        result = result | DENOFIZ_KEY_MOD_GUI;
    }
    return result;
}

SDL_Keymod InputSystem::ConvertToSdlKeyMod( const uint32_t modifiers )
{
    uint16_t sdlMods = 0;
    if ( modifiers & DENOFIZ_KEY_MOD_LSHIFT )
    {
        sdlMods |= SDL_KMOD_LSHIFT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_RSHIFT )
    {
        sdlMods |= SDL_KMOD_RSHIFT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_LCTRL )
    {
        sdlMods |= SDL_KMOD_LCTRL;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_RCTRL )
    {
        sdlMods |= SDL_KMOD_RCTRL;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_LALT )
    {
        sdlMods |= SDL_KMOD_LALT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_RALT )
    {
        sdlMods |= SDL_KMOD_RALT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_LGUI )
    {
        sdlMods |= SDL_KMOD_LGUI;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_RGUI )
    {
        sdlMods |= SDL_KMOD_RGUI;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_NUMLOCK )
    {
        sdlMods |= SDL_KMOD_NUM;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_CAPSLOCK )
    {
        sdlMods |= SDL_KMOD_CAPS;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_ALTGR )
    {
        sdlMods |= SDL_KMOD_MODE;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_SHIFT )
    {
        sdlMods |= SDL_KMOD_SHIFT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_CTRL )
    {
        sdlMods |= SDL_KMOD_CTRL;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_ALT )
    {
        sdlMods |= SDL_KMOD_ALT;
    }
    if ( modifiers & DENOFIZ_KEY_MOD_GUI )
    {
        sdlMods |= SDL_KMOD_GUI;
    }
    return static_cast<SDL_Keymod>( sdlMods );
}

void InputSystem::ConvertSDLEventToEvent( const SDL_Event &sdlEvent, DenOfIz_Event &outEvent )
{
    outEvent.Type = static_cast<DenOfIz_EventType>( sdlEvent.type );

    switch ( sdlEvent.type )
    {
    case SDL_EVENT_KEY_DOWN:
        outEvent.Type                 = DENOFIZ_EVENT_TYPE_KEY_DOWN;
        outEvent.Key.Common.Timestamp = sdlEvent.key.timestamp;
        outEvent.Key.Common.WindowID  = sdlEvent.key.windowID;
        outEvent.Key.State            = DENOFIZ_KEY_STATE_PRESSED;
        outEvent.Key.Repeat           = sdlEvent.key.repeat;
        outEvent.Key.KeyCode          = static_cast<DenOfIz_KeyCode>( sdlEvent.key.key );
        outEvent.Key.Mod              = ConvertKeyMod( static_cast<SDL_Keymod>( sdlEvent.key.mod ) );
        outEvent.Key.ScanCode         = sdlEvent.key.scancode;
        break;

    case SDL_EVENT_KEY_UP:
        outEvent.Type                 = DENOFIZ_EVENT_TYPE_KEY_UP;
        outEvent.Key.Common.Timestamp = sdlEvent.key.timestamp;
        outEvent.Key.Common.WindowID  = sdlEvent.key.windowID;
        outEvent.Key.State            = DENOFIZ_KEY_STATE_RELEASED;
        outEvent.Key.Repeat           = sdlEvent.key.repeat;
        outEvent.Key.KeyCode          = static_cast<DenOfIz_KeyCode>( sdlEvent.key.key );
        outEvent.Key.Mod              = ConvertKeyMod( static_cast<SDL_Keymod>( sdlEvent.key.mod ) );
        outEvent.Key.ScanCode         = sdlEvent.key.scancode;
        break;

    case SDL_EVENT_TEXT_EDITING:
        outEvent.Type                  = DENOFIZ_EVENT_TYPE_TEXT_EDITING;
        outEvent.Edit.Common.Timestamp = sdlEvent.edit.timestamp;
        outEvent.Edit.Common.WindowID  = sdlEvent.edit.windowID;
        outEvent.Edit.Text.Chars       = sdlEvent.edit.text;
        outEvent.Edit.Text.NumChars    = sdlEvent.edit.text ? std::strlen( sdlEvent.edit.text ) : 0;
        outEvent.Edit.Start            = sdlEvent.edit.start;
        outEvent.Edit.Length           = sdlEvent.edit.length;
        break;

    case SDL_EVENT_TEXT_INPUT:
        outEvent.Type                  = DENOFIZ_EVENT_TYPE_TEXT_INPUT;
        outEvent.Text.Common.Timestamp = sdlEvent.text.timestamp;
        outEvent.Text.Common.WindowID  = sdlEvent.text.windowID;
        outEvent.Text.Text.Chars       = sdlEvent.text.text;
        outEvent.Text.Text.NumChars    = sdlEvent.text.text ? std::strlen( sdlEvent.text.text ) : 0;
        break;

    case SDL_EVENT_MOUSE_MOTION:
    {
        outEvent.Type                         = DENOFIZ_EVENT_TYPE_MOUSE_MOTION;
        outEvent.MouseMotion.Common.Timestamp = sdlEvent.motion.timestamp;
        outEvent.MouseMotion.Common.WindowID  = sdlEvent.motion.windowID;
        outEvent.MouseMotion.MouseID          = sdlEvent.motion.which;
        outEvent.MouseMotion.Buttons          = FromSDLButtonState( sdlEvent.motion.state );
        float pixelDensity                    = 1.0f;
        if ( SDL_Window *window = SDL_GetWindowFromID( sdlEvent.motion.windowID ) )
        {
            pixelDensity = SDL_GetWindowPixelDensity( window );
        }
        outEvent.MouseMotion.X    = sdlEvent.motion.x * pixelDensity;
        outEvent.MouseMotion.Y    = sdlEvent.motion.y * pixelDensity;
        outEvent.MouseMotion.RelX = sdlEvent.motion.xrel * pixelDensity;
        outEvent.MouseMotion.RelY = sdlEvent.motion.yrel * pixelDensity;
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
        outEvent.Type                         = DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN;
        outEvent.MouseButton.Common.Timestamp = sdlEvent.button.timestamp;
        outEvent.MouseButton.Common.WindowID  = sdlEvent.button.windowID;
        outEvent.MouseButton.MouseID          = sdlEvent.button.which;
        outEvent.MouseButton.Button           = static_cast<DenOfIz_MouseButton>( sdlEvent.button.button );
        outEvent.MouseButton.State            = DENOFIZ_KEY_STATE_PRESSED;
        outEvent.MouseButton.Clicks           = sdlEvent.button.clicks;
        float pixelDensity                    = 1.0f;
        if ( SDL_Window *window = SDL_GetWindowFromID( sdlEvent.button.windowID ) )
        {
            pixelDensity = SDL_GetWindowPixelDensity( window );
        }
        outEvent.MouseButton.X = sdlEvent.button.x * pixelDensity;
        outEvent.MouseButton.Y = sdlEvent.button.y * pixelDensity;
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        outEvent.Type                         = DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_UP;
        outEvent.MouseButton.Common.Timestamp = sdlEvent.button.timestamp;
        outEvent.MouseButton.Common.WindowID  = sdlEvent.button.windowID;
        outEvent.MouseButton.MouseID          = sdlEvent.button.which;
        outEvent.MouseButton.Button           = static_cast<DenOfIz_MouseButton>( sdlEvent.button.button );
        outEvent.MouseButton.State            = DENOFIZ_KEY_STATE_RELEASED;
        outEvent.MouseButton.Clicks           = sdlEvent.button.clicks;
        float pixelDensity                    = 1.0f;
        if ( SDL_Window *window = SDL_GetWindowFromID( sdlEvent.button.windowID ) )
        {
            pixelDensity = SDL_GetWindowPixelDensity( window );
        }
        outEvent.MouseButton.X = sdlEvent.button.x * pixelDensity;
        outEvent.MouseButton.Y = sdlEvent.button.y * pixelDensity;
        break;
    }

    case SDL_EVENT_MOUSE_WHEEL:
        outEvent.Type                        = DENOFIZ_EVENT_TYPE_MOUSE_WHEEL;
        outEvent.MouseWheel.Common.Timestamp = sdlEvent.wheel.timestamp;
        outEvent.MouseWheel.Common.WindowID  = sdlEvent.wheel.windowID;
        outEvent.MouseWheel.MouseID          = sdlEvent.wheel.which;
        outEvent.MouseWheel.X                = sdlEvent.wheel.x;
        outEvent.MouseWheel.Y                = sdlEvent.wheel.y;
        outEvent.MouseWheel.Direction        = static_cast<DenOfIz_MouseWheelDirection>( sdlEvent.wheel.direction );
        break;

    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_SIZE_CHANGED;
        if ( SDL_Window *window = SDL_GetWindowFromID( sdlEvent.window.windowID ) )
        {
            int pixelW = 0;
            int pixelH = 0;
            SDL_GetWindowSizeInPixels( window, &pixelW, &pixelH );
            outEvent.Window.Data1 = pixelW;
            outEvent.Window.Data2 = pixelH;
        }
        else
        {
            outEvent.Window.Data1 = sdlEvent.window.data1;
            outEvent.Window.Data2 = sdlEvent.window.data2;
        }
        break;

    case SDL_EVENT_WINDOW_RESIZED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_RESIZED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_MOVED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_MOVED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_MINIMIZED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_MINIMIZED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_MAXIMIZED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_MAXIMIZED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_RESTORED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_RESTORED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_SHOWN:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_SHOWN;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_HIDDEN:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_HIDDEN;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_CLOSE;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_EXPOSED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_EXPOSED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_ENTER;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_LEAVE;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_FOCUS_GAINED;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_WINDOW_FOCUS_LOST:
        outEvent.Type                    = DENOFIZ_EVENT_TYPE_WINDOW_EVENT;
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = DENOFIZ_WINDOW_EVENT_TYPE_FOCUS_LOST;
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        outEvent.Type                            = DENOFIZ_EVENT_TYPE_CONTROLLER_AXIS_MOTION;
        outEvent.ControllerAxis.Common.Timestamp = sdlEvent.gaxis.timestamp;
        outEvent.ControllerAxis.Common.WindowID  = 0;
        outEvent.ControllerAxis.JoystickID       = sdlEvent.gaxis.which;
        outEvent.ControllerAxis.Axis             = static_cast<DenOfIz_ControllerAxis>( sdlEvent.gaxis.axis );
        outEvent.ControllerAxis.Value            = sdlEvent.gaxis.value;
        break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        outEvent.Type                              = DENOFIZ_EVENT_TYPE_CONTROLLER_BUTTON_DOWN;
        outEvent.ControllerButton.Common.Timestamp = sdlEvent.gbutton.timestamp;
        outEvent.ControllerButton.Common.WindowID  = 0;
        outEvent.ControllerButton.JoystickID       = sdlEvent.gbutton.which;
        outEvent.ControllerButton.Button           = static_cast<DenOfIz_ControllerButton>( sdlEvent.gbutton.button );
        outEvent.ControllerButton.State            = DENOFIZ_KEY_STATE_PRESSED;
        break;

    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        outEvent.Type                              = DENOFIZ_EVENT_TYPE_CONTROLLER_BUTTON_UP;
        outEvent.ControllerButton.Common.Timestamp = sdlEvent.gbutton.timestamp;
        outEvent.ControllerButton.Common.WindowID  = 0;
        outEvent.ControllerButton.JoystickID       = sdlEvent.gbutton.which;
        outEvent.ControllerButton.Button           = static_cast<DenOfIz_ControllerButton>( sdlEvent.gbutton.button );
        outEvent.ControllerButton.State            = DENOFIZ_KEY_STATE_RELEASED;
        break;

    case SDL_EVENT_GAMEPAD_ADDED:
        outEvent.Type                              = DENOFIZ_EVENT_TYPE_CONTROLLER_DEVICE_ADDED;
        outEvent.ControllerDevice.Common.Timestamp = sdlEvent.gdevice.timestamp;
        outEvent.ControllerDevice.Common.WindowID  = 0;
        outEvent.ControllerDevice.JoystickID       = sdlEvent.gdevice.which;
        break;

    case SDL_EVENT_GAMEPAD_REMOVED:
        outEvent.Type                              = DENOFIZ_EVENT_TYPE_CONTROLLER_DEVICE_REMOVED;
        outEvent.ControllerDevice.Common.Timestamp = sdlEvent.gdevice.timestamp;
        outEvent.ControllerDevice.Common.WindowID  = 0;
        outEvent.ControllerDevice.JoystickID       = sdlEvent.gdevice.which;
        break;

    case SDL_EVENT_GAMEPAD_REMAPPED:
        outEvent.Type                              = DENOFIZ_EVENT_TYPE_CONTROLLER_DEVICE_REMAPPED;
        outEvent.ControllerDevice.Common.Timestamp = sdlEvent.gdevice.timestamp;
        outEvent.ControllerDevice.Common.WindowID  = 0;
        outEvent.ControllerDevice.JoystickID       = sdlEvent.gdevice.which;
        break;

    case SDL_EVENT_QUIT:
        outEvent.Type                  = DENOFIZ_EVENT_TYPE_QUIT;
        outEvent.Quit.Common.Timestamp = sdlEvent.quit.timestamp;
        outEvent.Quit.Common.WindowID  = 0;
        break;

    default:
        break;
    }
}

bool InputSystem::OpenController( const int playerIndex, const int controllerIndex )
{
    if ( playerIndex < 0 || playerIndex >= 4 )
    {
        return false;
    }

    if ( m_controllerInitialized[ playerIndex ] )
    {
        CloseController( playerIndex );
    }

    const bool result                      = m_controllers[ playerIndex ].Open( controllerIndex );
    m_controllerInitialized[ playerIndex ] = result;

    if ( result )
    {
        return m_controllers[ playerIndex ].SetPlayerIndex( playerIndex );
    }

    return result;
}

void InputSystem::CloseController( const int playerIndex )
{
    if ( playerIndex < 0 || playerIndex >= 4 )
    {
        return;
    }

    if ( m_controllerInitialized[ playerIndex ] )
    {
        m_controllers[ playerIndex ].Close( );
        m_controllerInitialized[ playerIndex ] = false;
    }
}

bool InputSystem::IsControllerConnected( const int playerIndex ) const
{
    if ( playerIndex < 0 || playerIndex >= 4 )
    {
        return false;
    }

    return m_controllerInitialized[ playerIndex ] && m_controllers[ playerIndex ].IsConnected( );
}

DenOfIz_Controller InputSystem::GetController( const int playerIndex )
{
    if ( playerIndex < 0 || playerIndex >= 4 || !m_controllerInitialized[ playerIndex ] )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    return DENOFIZ_NULL_HANDLE;
}

bool InputSystem::IsControllerButtonPressed( const int playerIndex, const DenOfIz_ControllerButton button ) const
{
    if ( playerIndex < 0 || playerIndex >= 4 || !m_controllerInitialized[ playerIndex ] )
    {
        return false;
    }

    return m_controllers[ playerIndex ].IsButtonPressed( button );
}

int16_t InputSystem::GetControllerAxisValue( const int playerIndex, const DenOfIz_ControllerAxis axis ) const
{
    if ( playerIndex < 0 || playerIndex >= 4 || !m_controllerInitialized[ playerIndex ] )
    {
        return 0;
    }

    return m_controllers[ playerIndex ].GetAxisValue( axis );
}

DenOfIz_StringView InputSystem::GetControllerName( const int playerIndex ) const
{
    if ( playerIndex < 0 || playerIndex >= 4 || !m_controllerInitialized[ playerIndex ] )
    {
        return { };
    }

    return m_controllers[ playerIndex ].GetName( );
}

bool InputSystem::SetControllerRumble( const int playerIndex, const uint16_t lowFrequency, const uint16_t highFrequency, const uint32_t durationMs )
{
    if ( playerIndex < 0 || playerIndex >= 4 || !m_controllerInitialized[ playerIndex ] )
    {
        return false;
    }

    return m_controllers[ playerIndex ].SetRumble( lowFrequency, highFrequency, durationMs );
}

static InputSystem *s_staticInputSystem = nullptr;

static InputSystem *GetStaticInputSystem( )
{
    if ( s_staticInputSystem == nullptr )
    {
        s_staticInputSystem = new InputSystem( );
    }
    return s_staticInputSystem;
}

extern "C"
{

    DenOfIz_InputSystem DenOfIz_InputSystem_Create( )
    {
        auto *inputSystem = new InputSystem( );
        return DENOFIZ_TO_HANDLE( inputSystem );
    }

    void DenOfIz_InputSystem_Destroy( DenOfIz_InputSystem inputSystem )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return;
        }
        delete INPUT_SYSTEM_IMPL( inputSystem );
    }

    bool DenOfIz_InputSystem_PollEvent( DenOfIz_Event *outEvent )
    {
        if ( outEvent == NULL )
        {
            return false;
        }

        SDL_Event sdlEvent;
        if ( SDL_PollEvent( &sdlEvent ) )
        {
            InputSystem::ConvertSDLEventToEvent( sdlEvent, *outEvent );
            return true;
        }
        return false;
    }

    bool DenOfIz_InputSystem_WaitEvent( DenOfIz_Event *outEvent )
    {
        if ( outEvent == NULL )
        {
            return false;
        }

        SDL_Event sdlEvent;
        if ( SDL_WaitEvent( &sdlEvent ) )
        {
            InputSystem::ConvertSDLEventToEvent( sdlEvent, *outEvent );
            return true;
        }
        return false;
    }

    bool DenOfIz_InputSystem_WaitEventTimeout( DenOfIz_Event *outEvent, int32_t timeout )
    {
        if ( outEvent == NULL )
        {
            return false;
        }

        SDL_Event sdlEvent;
        if ( SDL_WaitEventTimeout( &sdlEvent, timeout ) )
        {
            InputSystem::ConvertSDLEventToEvent( sdlEvent, *outEvent );
            return true;
        }
        return false;
    }

    void DenOfIz_InputSystem_PumpEvents( )
    {
        SDL_PumpEvents( );
    }

    void DenOfIz_InputSystem_FlushEvents( DenOfIz_EventType minType, DenOfIz_EventType maxType )
    {
        SDL_FlushEvents( static_cast<uint32_t>( minType ), static_cast<uint32_t>( maxType ) );
    }

    void DenOfIz_InputSystem_PushEvent( const DenOfIz_Event *ev )
    {
        if ( ev == NULL )
        {
            return;
        }

        SDL_Event sdlEvent = { };
        sdlEvent.type      = static_cast<uint32_t>( ev->Type );

        switch ( ev->Type )
        {
        case DENOFIZ_EVENT_TYPE_KEY_DOWN:
            sdlEvent.type          = SDL_EVENT_KEY_DOWN;
            sdlEvent.key.timestamp = ev->Key.Common.Timestamp;
            sdlEvent.key.windowID  = ev->Key.Common.WindowID;
            sdlEvent.key.down      = true;
            sdlEvent.key.repeat    = ev->Key.Repeat;
            sdlEvent.key.key       = static_cast<SDL_Keycode>( ev->Key.KeyCode );
            sdlEvent.key.mod       = ev->Key.Mod;
            sdlEvent.key.scancode  = static_cast<SDL_Scancode>( ev->Key.ScanCode );
            break;

        case DENOFIZ_EVENT_TYPE_KEY_UP:
            sdlEvent.type          = SDL_EVENT_KEY_UP;
            sdlEvent.key.timestamp = ev->Key.Common.Timestamp;
            sdlEvent.key.windowID  = ev->Key.Common.WindowID;
            sdlEvent.key.down      = false;
            sdlEvent.key.repeat    = ev->Key.Repeat;
            sdlEvent.key.key       = static_cast<SDL_Keycode>( ev->Key.KeyCode );
            sdlEvent.key.mod       = ev->Key.Mod;
            sdlEvent.key.scancode  = static_cast<SDL_Scancode>( ev->Key.ScanCode );
            break;

        case DENOFIZ_EVENT_TYPE_MOUSE_MOTION:
            sdlEvent.type             = SDL_EVENT_MOUSE_MOTION;
            sdlEvent.motion.timestamp = ev->MouseMotion.Common.Timestamp;
            sdlEvent.motion.windowID  = ev->MouseMotion.Common.WindowID;
            sdlEvent.motion.which     = ev->MouseMotion.MouseID;
            sdlEvent.motion.state     = InputSystem::ToSDLButtonState( ev->MouseMotion.Buttons );
            sdlEvent.motion.x         = ev->MouseMotion.X;
            sdlEvent.motion.y         = ev->MouseMotion.Y;
            sdlEvent.motion.xrel      = ev->MouseMotion.RelX;
            sdlEvent.motion.yrel      = ev->MouseMotion.RelY;
            break;

        case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN:
            sdlEvent.type             = SDL_EVENT_MOUSE_BUTTON_DOWN;
            sdlEvent.button.timestamp = ev->MouseButton.Common.Timestamp;
            sdlEvent.button.windowID  = ev->MouseButton.Common.WindowID;
            sdlEvent.button.which     = ev->MouseButton.MouseID;
            sdlEvent.button.button    = static_cast<uint8_t>( ev->MouseButton.Button );
            sdlEvent.button.down      = true;
            sdlEvent.button.clicks    = ev->MouseButton.Clicks;
            sdlEvent.button.x         = ev->MouseButton.X;
            sdlEvent.button.y         = ev->MouseButton.Y;
            break;

        case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_UP:
            sdlEvent.type             = SDL_EVENT_MOUSE_BUTTON_UP;
            sdlEvent.button.timestamp = ev->MouseButton.Common.Timestamp;
            sdlEvent.button.windowID  = ev->MouseButton.Common.WindowID;
            sdlEvent.button.which     = ev->MouseButton.MouseID;
            sdlEvent.button.button    = static_cast<uint8_t>( ev->MouseButton.Button );
            sdlEvent.button.down      = false;
            sdlEvent.button.clicks    = ev->MouseButton.Clicks;
            sdlEvent.button.x         = ev->MouseButton.X;
            sdlEvent.button.y         = ev->MouseButton.Y;
            break;

        case DENOFIZ_EVENT_TYPE_MOUSE_WHEEL:
            sdlEvent.type            = SDL_EVENT_MOUSE_WHEEL;
            sdlEvent.wheel.timestamp = ev->MouseWheel.Common.Timestamp;
            sdlEvent.wheel.windowID  = ev->MouseWheel.Common.WindowID;
            sdlEvent.wheel.which     = ev->MouseWheel.MouseID;
            sdlEvent.wheel.x         = ev->MouseWheel.X;
            sdlEvent.wheel.y         = ev->MouseWheel.Y;
            sdlEvent.wheel.direction = static_cast<SDL_MouseWheelDirection>( ev->MouseWheel.Direction );
            break;

        case DENOFIZ_EVENT_TYPE_WINDOW_EVENT:
            sdlEvent.window.timestamp = ev->Window.Common.Timestamp;
            sdlEvent.window.windowID  = ev->Window.Common.WindowID;
            sdlEvent.window.data1     = ev->Window.Data1;
            sdlEvent.window.data2     = ev->Window.Data2;
            switch ( ev->Window.Event )
            {
            case DENOFIZ_WINDOW_EVENT_TYPE_SHOWN:
                sdlEvent.type = SDL_EVENT_WINDOW_SHOWN;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_HIDDEN:
                sdlEvent.type = SDL_EVENT_WINDOW_HIDDEN;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_EXPOSED:
                sdlEvent.type = SDL_EVENT_WINDOW_EXPOSED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_MOVED:
                sdlEvent.type = SDL_EVENT_WINDOW_MOVED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_RESIZED:
                sdlEvent.type = SDL_EVENT_WINDOW_RESIZED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_SIZE_CHANGED:
                sdlEvent.type = SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_MINIMIZED:
                sdlEvent.type = SDL_EVENT_WINDOW_MINIMIZED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_MAXIMIZED:
                sdlEvent.type = SDL_EVENT_WINDOW_MAXIMIZED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_RESTORED:
                sdlEvent.type = SDL_EVENT_WINDOW_RESTORED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_ENTER:
                sdlEvent.type = SDL_EVENT_WINDOW_MOUSE_ENTER;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_LEAVE:
                sdlEvent.type = SDL_EVENT_WINDOW_MOUSE_LEAVE;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_FOCUS_GAINED:
                sdlEvent.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_FOCUS_LOST:
                sdlEvent.type = SDL_EVENT_WINDOW_FOCUS_LOST;
                break;
            case DENOFIZ_WINDOW_EVENT_TYPE_CLOSE:
                sdlEvent.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
                break;
            default:
                return;
            }
            break;

        case DENOFIZ_EVENT_TYPE_CONTROLLER_AXIS_MOTION:
            sdlEvent.type            = SDL_EVENT_GAMEPAD_AXIS_MOTION;
            sdlEvent.gaxis.timestamp = ev->ControllerAxis.Common.Timestamp;
            sdlEvent.gaxis.which     = ev->ControllerAxis.JoystickID;
            sdlEvent.gaxis.axis      = static_cast<uint8_t>( ev->ControllerAxis.Axis );
            sdlEvent.gaxis.value     = ev->ControllerAxis.Value;
            break;

        case DENOFIZ_EVENT_TYPE_CONTROLLER_BUTTON_DOWN:
            sdlEvent.type              = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
            sdlEvent.gbutton.timestamp = ev->ControllerButton.Common.Timestamp;
            sdlEvent.gbutton.which     = ev->ControllerButton.JoystickID;
            sdlEvent.gbutton.button    = static_cast<uint8_t>( ev->ControllerButton.Button );
            sdlEvent.gbutton.down      = true;
            break;

        case DENOFIZ_EVENT_TYPE_CONTROLLER_BUTTON_UP:
            sdlEvent.type              = SDL_EVENT_GAMEPAD_BUTTON_UP;
            sdlEvent.gbutton.timestamp = ev->ControllerButton.Common.Timestamp;
            sdlEvent.gbutton.which     = ev->ControllerButton.JoystickID;
            sdlEvent.gbutton.button    = static_cast<uint8_t>( ev->ControllerButton.Button );
            sdlEvent.gbutton.down      = false;
            break;

        case DENOFIZ_EVENT_TYPE_QUIT:
            sdlEvent.type           = SDL_EVENT_QUIT;
            sdlEvent.quit.timestamp = ev->Quit.Common.Timestamp;
            break;

        default:
            return;
        }

        SDL_PushEvent( &sdlEvent );
    }

    DenOfIz_KeyState DenOfIz_InputSystem_GetKeyState( DenOfIz_KeyCode key )
    {
        const bool *state    = SDL_GetKeyboardState( nullptr );
        const auto  scancode = SDL_GetScancodeFromKey( static_cast<SDL_Keycode>( key ), nullptr );
        return state[ scancode ] ? DENOFIZ_KEY_STATE_PRESSED : DENOFIZ_KEY_STATE_RELEASED;
    }

    bool DenOfIz_InputSystem_IsKeyPressed( DenOfIz_KeyCode key )
    {
        return DenOfIz_InputSystem_GetKeyState( key ) == DENOFIZ_KEY_STATE_PRESSED;
    }

    uint32_t DenOfIz_InputSystem_GetModState( )
    {
        const uint16_t sdlMods = SDL_GetModState( );
        return InputSystem::ConvertKeyMod( static_cast<SDL_Keymod>( sdlMods ) );
    }

    void DenOfIz_InputSystem_SetModState( uint32_t modifiers )
    {
        SDL_SetModState( InputSystem::ConvertToSdlKeyMod( modifiers ) );
    }

    DenOfIz_KeyCode DenOfIz_InputSystem_GetKeyFromName( DenOfIz_StringView name )
    {
        std::string nameString;
        if ( name.Chars && name.NumChars > 0 )
        {
            nameString.assign( name.Chars, name.Chars + name.NumChars );
        }

        SDL_Keycode keycode = SDL_GetKeyFromName( nameString.c_str( ) );
        return static_cast<DenOfIz_KeyCode>( keycode );
    }

    DenOfIz_StringView DenOfIz_InputSystem_GetKeyName( DenOfIz_KeyCode key )
    {
        return { SDL_GetKeyName( static_cast<SDL_Keycode>( key ) ) };
    }

    DenOfIz_StringView DenOfIz_InputSystem_GetScancodeName( uint32_t scancode )
    {
        return { SDL_GetScancodeName( static_cast<SDL_Scancode>( scancode ) ) };
    }

    DenOfIz_MouseCoords DenOfIz_InputSystem_GetMouseState( )
    {
        DenOfIz_MouseCoords result = { };
        SDL_GetMouseState( &result.X, &result.Y );
        if ( SDL_Window *window = SDL_GetMouseFocus( ) )
        {
            const float pixelDensity = SDL_GetWindowPixelDensity( window );
            result.X *= pixelDensity;
            result.Y *= pixelDensity;
        }
        return result;
    }

    DenOfIz_MouseCoords DenOfIz_InputSystem_GetGlobalMouseState( )
    {
        DenOfIz_MouseCoords result = { };
        SDL_GetGlobalMouseState( &result.X, &result.Y );
        return result;
    }

    DenOfIz_MouseButtonState DenOfIz_InputSystem_GetMouseButtonState( )
    {
        const uint32_t sdlState = SDL_GetMouseState( nullptr, nullptr );
        return InputSystem::FromSDLButtonState( sdlState );
    }

    bool DenOfIz_InputSystem_IsMouseButtonPressed( DenOfIz_MouseButton button )
    {
        const uint32_t sdlState = SDL_GetMouseState( nullptr, nullptr );

        switch ( button )
        {
        case DENOFIZ_MOUSE_BUTTON_LEFT:
            return ( sdlState & SDL_BUTTON_LMASK ) != 0;
        case DENOFIZ_MOUSE_BUTTON_MIDDLE:
            return ( sdlState & SDL_BUTTON_MMASK ) != 0;
        case DENOFIZ_MOUSE_BUTTON_RIGHT:
            return ( sdlState & SDL_BUTTON_RMASK ) != 0;
        case DENOFIZ_MOUSE_BUTTON_X1:
            return ( sdlState & SDL_BUTTON_X1MASK ) != 0;
        case DENOFIZ_MOUSE_BUTTON_X2:
            return ( sdlState & SDL_BUTTON_X2MASK ) != 0;
        default:
            return false;
        }
    }

    uint32_t DenOfIz_InputSystem_GetMouseButtons( )
    {
        return SDL_GetMouseState( nullptr, nullptr );
    }

    DenOfIz_MouseCoords DenOfIz_InputSystem_GetRelativeMouseState( )
    {
        DenOfIz_MouseCoords result = { };
        SDL_GetRelativeMouseState( &result.X, &result.Y );
        if ( SDL_Window *window = SDL_GetMouseFocus( ) )
        {
            const float pixelDensity = SDL_GetWindowPixelDensity( window );
            result.X *= pixelDensity;
            result.Y *= pixelDensity;
        }
        return result;
    }

    void DenOfIz_InputSystem_WarpMouseInWindow( DenOfIz_Window window, float x, float y )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }

        uint32_t windowID = DenOfIz_Window_GetWindowID( window );
        if ( SDL_Window *sdlWindow = SDL_GetWindowFromID( windowID ) )
        {
            const float pixelDensity = SDL_GetWindowPixelDensity( sdlWindow );
            SDL_WarpMouseInWindow( sdlWindow, x / pixelDensity, y / pixelDensity );
        }
    }

    bool DenOfIz_InputSystem_GetRelativeMouseMode( uint32_t windowId )
    {
        if ( SDL_Window *window = SDL_GetWindowFromID( windowId ) )
        {
            return SDL_GetWindowRelativeMouseMode( window );
        }
        return false;
    }

    void DenOfIz_InputSystem_SetRelativeMouseMode( uint32_t windowId, bool enabled )
    {
        if ( SDL_Window *window = SDL_GetWindowFromID( windowId ) )
        {
            SDL_SetWindowRelativeMouseMode( window, enabled );
        }
    }

    void DenOfIz_InputSystem_CaptureMouse( bool enabled )
    {
        SDL_CaptureMouse( enabled ? true : false );
    }

    bool DenOfIz_InputSystem_GetMouseFocus( uint32_t windowID )
    {
        if ( SDL_Window *window = SDL_GetWindowFromID( windowID ) )
        {
            return ( SDL_GetWindowFlags( window ) & SDL_WINDOW_MOUSE_FOCUS ) != 0;
        }
        return false;
    }

    void DenOfIz_InputSystem_ShowCursor( bool show )
    {
        if ( show )
        {
            SDL_ShowCursor( );
        }
        else
        {
            SDL_HideCursor( );
        }
    }

    bool DenOfIz_InputSystem_IsCursorShown( )
    {
        return SDL_CursorVisible( );
    }

    void DenOfIz_InputSystem_SetCursor( DenOfIz_SystemCursor cursor )
    {
        InputSystem *inputSystem = GetStaticInputSystem( );

        if ( cursor >= DENOFIZ_SYSTEM_CURSOR_COUNT )
        {
            return;
        }

        if ( inputSystem->m_systemCursors[ static_cast<size_t>( cursor ) ] != nullptr )
        {
            SDL_SetCursor( inputSystem->m_systemCursors[ static_cast<size_t>( cursor ) ] );
            inputSystem->m_currentCursor = cursor;
        }
    }

    void DenOfIz_InputSystem_ResetCursor( )
    {
        InputSystem *inputSystem = GetStaticInputSystem( );

        if ( inputSystem->m_defaultCursor != nullptr )
        {
            SDL_SetCursor( inputSystem->m_defaultCursor );
            inputSystem->m_currentCursor = DENOFIZ_SYSTEM_CURSOR_DEFAULT;
        }
    }

    void DenOfIz_InputSystem_StartTextInput( )
    {
        SDL_StartTextInput( SDL_GetKeyboardFocus( ) );
    }

    void DenOfIz_InputSystem_StopTextInput( )
    {
        SDL_StopTextInput( SDL_GetKeyboardFocus( ) );
    }

    bool DenOfIz_InputSystem_IsTextInputActive( )
    {
        return SDL_TextInputActive( SDL_GetKeyboardFocus( ) );
    }

    void DenOfIz_InputSystem_SetTextInputRect( DenOfIz_Window window, int32_t x, int32_t y, int32_t w, int32_t h )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( window ) )
        {
            return;
        }

        uint32_t windowID = DenOfIz_Window_GetWindowID( window );
        if ( SDL_Window *sdlWindow = SDL_GetWindowFromID( windowID ) )
        {
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
            SDL_SetTextInputArea( sdlWindow, &rect, 0 );
        }
    }

    DenOfIz_Int32Array DenOfIz_InputSystem_GetConnectedControllerIndices( )
    {
        return DenOfIz_Controller_GetConnectedControllerIndices( );
    }

    int32_t DenOfIz_InputSystem_GetNumControllers( )
    {
        return DenOfIz_Controller_GetControllerCount( );
    }

    bool DenOfIz_InputSystem_OpenController( DenOfIz_InputSystem inputSystem, int32_t playerIndex, int32_t controllerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return false;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->OpenController( playerIndex, controllerIndex );
    }

    void DenOfIz_InputSystem_CloseController( DenOfIz_InputSystem inputSystem, int32_t playerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return;
        }
        INPUT_SYSTEM_IMPL( inputSystem )->CloseController( playerIndex );
    }

    bool DenOfIz_InputSystem_IsControllerConnected( DenOfIz_InputSystem inputSystem, int32_t playerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return false;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->IsControllerConnected( playerIndex );
    }

    DenOfIz_Controller DenOfIz_InputSystem_GetController( DenOfIz_InputSystem inputSystem, int32_t playerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->GetController( playerIndex );
    }

    bool DenOfIz_InputSystem_IsControllerButtonPressed( DenOfIz_InputSystem inputSystem, int32_t playerIndex, DenOfIz_ControllerButton button )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return false;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->IsControllerButtonPressed( playerIndex, button );
    }

    int16_t DenOfIz_InputSystem_GetControllerAxisValue( DenOfIz_InputSystem inputSystem, int32_t playerIndex, DenOfIz_ControllerAxis axis )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return 0;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->GetControllerAxisValue( playerIndex, axis );
    }

    DenOfIz_StringView DenOfIz_InputSystem_GetControllerName( DenOfIz_InputSystem inputSystem, int32_t playerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return { };
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->GetControllerName( playerIndex );
    }

    bool DenOfIz_InputSystem_SetControllerRumble( DenOfIz_InputSystem inputSystem, int32_t playerIndex, uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputSystem ) )
        {
            return false;
        }
        return INPUT_SYSTEM_IMPL( inputSystem )->SetControllerRumble( playerIndex, lowFrequency, highFrequency, durationMs );
    }
}
