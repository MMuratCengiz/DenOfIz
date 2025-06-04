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
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include <cstring> // For std::memset

using namespace DenOfIz;

// Implementation struct
struct InputSystem::Impl
{
	Controller m_controllers[ 4 ];
	bool       m_controllerInitialized[ 4 ] = { false, false, false, false };

	Impl( )
	{
		for ( bool &controllerInitialized : m_controllerInitialized )
		{
			controllerInitialized = false;
		}
	}

	~Impl( )
	{
		for ( int i = 0; i < 4; ++i )
		{
			if ( m_controllerInitialized[ i ] )
			{
				m_controllers[ i ].Close( );
				m_controllerInitialized[ i ] = false;
			}
		}
	}

	// Helper function to safely copy strings
	static void SafeCopyString( char *dest, size_t destSize, const char *src )
	{
		if ( dest && src && destSize > 0 )
		{
			size_t srcLen = std::strlen( src );
			size_t copyLen = ( srcLen < destSize - 1 ) ? srcLen : destSize - 1;
			std::memcpy( dest, src, copyLen );
			dest[ copyLen ] = '\0';
		}
	}

	// SDL button state conversion functions
	static MouseButtonState FromSDLButtonState( uint32_t state )
	{
		MouseButtonState result;
		result.LeftButton   = ( state & SDL_BUTTON_LMASK ) != 0;
		result.MiddleButton = ( state & SDL_BUTTON_MMASK ) != 0;
		result.RightButton  = ( state & SDL_BUTTON_RMASK ) != 0;
		result.X1Button     = ( state & SDL_BUTTON_X1MASK ) != 0;
		result.X2Button     = ( state & SDL_BUTTON_X2MASK ) != 0;
		return result;
	}

	static uint32_t ToSDLButtonState( const MouseButtonState& buttons )
	{
		uint32_t state = 0;
		if ( buttons.LeftButton )   state |= SDL_BUTTON_LMASK;
		if ( buttons.MiddleButton ) state |= SDL_BUTTON_MMASK;
		if ( buttons.RightButton )  state |= SDL_BUTTON_RMASK;
		if ( buttons.X1Button )     state |= SDL_BUTTON_X1MASK;
		if ( buttons.X2Button )     state |= SDL_BUTTON_X2MASK;
		return state;
	}

#ifdef WINDOW_MANAGER_SDL
	// SDL event conversion functions
	static void ConvertSDLEventToEvent( const SDL_Event &sdlEvent, Event &outEvent );
	static BitSet<KeyMod> ConvertKeyMod( SDL_Keymod sdlMods );
	static SDL_Keymod ConvertToSdlKeyMod( const BitSet<KeyMod> &modifiers );
#endif
};

InputSystem::InputSystem( )
	: m_impl( std::make_unique<Impl>( ) )
{
}

InputSystem::~InputSystem( ) = default;

InputSystem::InputSystem( InputSystem&& other ) noexcept = default;
InputSystem& InputSystem::operator=( InputSystem&& other ) noexcept = default;

bool InputSystem::PollEvent( Event &outEvent )
{
#ifdef WINDOW_MANAGER_SDL
	SDL_Event sdlEvent;
	if ( SDL_PollEvent( &sdlEvent ) )
	{
		Impl::ConvertSDLEventToEvent( sdlEvent, outEvent );
		return true;
	}
#endif
	return false;
}

bool InputSystem::WaitEvent( Event &outEvent )
{
#ifdef WINDOW_MANAGER_SDL
	SDL_Event sdlEvent;
	if ( SDL_WaitEvent( &sdlEvent ) )
	{
		Impl::ConvertSDLEventToEvent( sdlEvent, outEvent );
		return true;
	}
#endif
	return false;
}

bool InputSystem::WaitEventTimeout( Event &outEvent, const int timeout )
{
#ifdef WINDOW_MANAGER_SDL
	SDL_Event sdlEvent;
	if ( SDL_WaitEventTimeout( &sdlEvent, timeout ) )
	{
		Impl::ConvertSDLEventToEvent( sdlEvent, outEvent );
		return true;
	}
#endif
	return false;
}

void InputSystem::PumpEvents( )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_PumpEvents( );
#endif
}

void InputSystem::FlushEvents( const EventType minType, const EventType maxType )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_FlushEvents( static_cast<uint32_t>( minType ), static_cast<uint32_t>( maxType ) );
#endif
}

void InputSystem::PushEvent( const Event &event )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_Event sdlEvent = { };
    sdlEvent.type      = static_cast<uint32_t>( event.Type );

    switch ( event.Type )
    {
    case EventType::KeyDown:
    case EventType::KeyUp:
        sdlEvent.key.timestamp       = event.Key.Common.Timestamp;
        sdlEvent.key.windowID        = event.Key.Common.WindowID;
        sdlEvent.key.state           = event.Key.State == KeyState::Pressed ? 1 : 0;
        sdlEvent.key.repeat          = event.Key.Repeat;
        sdlEvent.key.keysym.sym      = static_cast<SDL_Keycode>( event.Key.Keycode );
        sdlEvent.key.keysym.mod      = event.Key.Mod.Value( );
        sdlEvent.key.keysym.scancode = static_cast<SDL_Scancode>( event.Key.Scancode );
        break;

    case EventType::MouseMotion:
        sdlEvent.motion.timestamp = event.Motion.Common.Timestamp;
        sdlEvent.motion.windowID  = event.Motion.Common.WindowID;
        sdlEvent.motion.which     = event.Motion.MouseID;
        sdlEvent.motion.state     = Impl::ToSDLButtonState( event.Motion.Buttons );
        sdlEvent.motion.x         = event.Motion.X;
        sdlEvent.motion.y         = event.Motion.Y;
        sdlEvent.motion.xrel      = event.Motion.RelX;
        sdlEvent.motion.yrel      = event.Motion.RelY;
        break;

    case EventType::MouseButtonDown:
    case EventType::MouseButtonUp:
        sdlEvent.button.timestamp = event.Button.Common.Timestamp;
        sdlEvent.button.windowID  = event.Button.Common.WindowID;
        sdlEvent.button.which     = event.Button.MouseID;
        sdlEvent.button.button    = static_cast<uint8_t>( event.Button.Button );
        sdlEvent.button.state     = event.Button.State == KeyState::Pressed ? 1 : 0;
        sdlEvent.button.clicks    = event.Button.Clicks;
        sdlEvent.button.x         = event.Button.X;
        sdlEvent.button.y         = event.Button.Y;
        break;

    case EventType::MouseWheel:
        sdlEvent.wheel.timestamp = event.Wheel.Common.Timestamp;
        sdlEvent.wheel.windowID  = event.Wheel.Common.WindowID;
        sdlEvent.wheel.which     = event.Wheel.MouseID;
        sdlEvent.wheel.x         = event.Wheel.X;
        sdlEvent.wheel.y         = event.Wheel.Y;
        sdlEvent.wheel.direction = static_cast<uint32_t>( event.Wheel.Direction );
        break;

    case EventType::WindowEvent:
        sdlEvent.window.timestamp = event.Window.Common.Timestamp;
        sdlEvent.window.windowID  = event.Window.Common.WindowID;
        sdlEvent.window.event     = static_cast<uint8_t>( event.Window.Event );
        sdlEvent.window.data1     = event.Window.Data1;
        sdlEvent.window.data2     = event.Window.Data2;
        break;

    case EventType::ControllerAxisMotion:
        sdlEvent.caxis.timestamp = event.ControllerAxis.Common.Timestamp;
        sdlEvent.caxis.which     = event.ControllerAxis.JoystickID;
        sdlEvent.caxis.axis      = static_cast<uint8_t>( event.ControllerAxis.Axis );
        sdlEvent.caxis.value     = event.ControllerAxis.Value;
        break;

    case EventType::ControllerButtonDown:
    case EventType::ControllerButtonUp:
        sdlEvent.cbutton.timestamp = event.ControllerButton.Common.Timestamp;
        sdlEvent.cbutton.which     = event.ControllerButton.JoystickID;
        sdlEvent.cbutton.button    = static_cast<uint8_t>( event.ControllerButton.Button );
        sdlEvent.cbutton.state     = event.ControllerButton.State == KeyState::Pressed ? 1 : 0;
        break;

    case EventType::Quit:
        sdlEvent.quit.timestamp = event.Quit.Common.Timestamp;
        break;

    default:
        // For types that haven't been implemented, don't push the event
        return;
    }

    SDL_PushEvent( &sdlEvent );
#endif
}

// Keyboard functions
KeyState InputSystem::GetKeyState( KeyCode key )
{
#ifdef WINDOW_MANAGER_SDL
    const uint8_t *state    = SDL_GetKeyboardState( nullptr );
    const auto     scancode = SDL_GetScancodeFromKey( static_cast<SDL_Keycode>( key ) );
    return state[ scancode ] ? KeyState::Pressed : KeyState::Released;
#else
    return KeyState::Released;
#endif
}

bool InputSystem::IsKeyPressed( const KeyCode key )
{
    return GetKeyState( key ) == KeyState::Pressed;
}

DenOfIz::BitSet<KeyMod> InputSystem::GetModState( )
{
#ifdef WINDOW_MANAGER_SDL
	const uint16_t sdlMods = SDL_GetModState( );
	return Impl::ConvertKeyMod( static_cast<SDL_Keymod>( sdlMods ) );
#else
	return BitSet<KeyMod>( );
#endif
}

void InputSystem::SetModState( const BitSet<KeyMod> &modifiers )
{
#ifdef WINDOW_MANAGER_SDL
	SDL_SetModState( Impl::ConvertToSdlKeyMod( modifiers ) );
#endif
}

KeyCode InputSystem::GetKeyFromName( const InteropString &name )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_Keycode keycode = SDL_GetKeyFromName( name.Get( ) );
    return static_cast<KeyCode>( keycode );
#else
    return KeyCode::Unknown;
#endif
}

InteropString InputSystem::GetKeyName( const KeyCode key )
{
#ifdef WINDOW_MANAGER_SDL
    return { SDL_GetKeyName( static_cast<SDL_Keycode>( key ) ) };
#else
    return InteropString( );
#endif
}

InteropString InputSystem::GetScancodeName( const uint32_t scancode )
{
#ifdef WINDOW_MANAGER_SDL
    return { SDL_GetScancodeName( static_cast<SDL_Scancode>( scancode ) ) };
#else
    return InteropString( );
#endif
}

#ifdef WINDOW_MANAGER_SDL
void InputSystem::Impl::ConvertSDLEventToEvent( const SDL_Event &sdlEvent, Event &outEvent )
{
    outEvent.Type = static_cast<EventType>( sdlEvent.type );

    switch ( sdlEvent.type )
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        outEvent.Key.Common.Timestamp = sdlEvent.key.timestamp;
        outEvent.Key.Common.WindowID  = sdlEvent.key.windowID;
        outEvent.Key.State            = sdlEvent.key.state ? KeyState::Pressed : KeyState::Released;
        outEvent.Key.Repeat           = sdlEvent.key.repeat;
        outEvent.Key.Keycode          = static_cast<KeyCode>( sdlEvent.key.keysym.sym );
        outEvent.Key.Mod              = ConvertKeyMod( static_cast<SDL_Keymod>( sdlEvent.key.keysym.mod ) );
        outEvent.Key.Scancode         = sdlEvent.key.keysym.scancode;
        break;

    case SDL_TEXTEDITING:
        outEvent.Edit.Common.Timestamp = sdlEvent.edit.timestamp;
        outEvent.Edit.Common.WindowID  = sdlEvent.edit.windowID;
        Impl::SafeCopyString( outEvent.Edit.Text, sizeof( outEvent.Edit.Text ), sdlEvent.edit.text );
        outEvent.Edit.Start  = sdlEvent.edit.start;
        outEvent.Edit.Length = sdlEvent.edit.length;
        break;

    case SDL_TEXTINPUT:
        outEvent.Text.Common.Timestamp = sdlEvent.text.timestamp;
        outEvent.Text.Common.WindowID  = sdlEvent.text.windowID;
        Impl::SafeCopyString( outEvent.Text.Text, sizeof( outEvent.Text.Text ), sdlEvent.text.text );
        break;

    case SDL_MOUSEMOTION:
        outEvent.Motion.Common.Timestamp = sdlEvent.motion.timestamp;
        outEvent.Motion.Common.WindowID  = sdlEvent.motion.windowID;
        outEvent.Motion.MouseID          = sdlEvent.motion.which;
        outEvent.Motion.Buttons          = Impl::FromSDLButtonState( sdlEvent.motion.state );
        outEvent.Motion.X                = sdlEvent.motion.x;
        outEvent.Motion.Y                = sdlEvent.motion.y;
        outEvent.Motion.RelX             = sdlEvent.motion.xrel;
        outEvent.Motion.RelY             = sdlEvent.motion.yrel;
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        outEvent.Button.Common.Timestamp = sdlEvent.button.timestamp;
        outEvent.Button.Common.WindowID  = sdlEvent.button.windowID;
        outEvent.Button.MouseID          = sdlEvent.button.which;
        outEvent.Button.Button           = static_cast<MouseButton>( sdlEvent.button.button );
        outEvent.Button.State            = sdlEvent.button.state ? KeyState::Pressed : KeyState::Released;
        outEvent.Button.Clicks           = sdlEvent.button.clicks;
        outEvent.Button.X                = sdlEvent.button.x;
        outEvent.Button.Y                = sdlEvent.button.y;
        break;

    case SDL_MOUSEWHEEL:
        outEvent.Wheel.Common.Timestamp = sdlEvent.wheel.timestamp;
        outEvent.Wheel.Common.WindowID  = sdlEvent.wheel.windowID;
        outEvent.Wheel.MouseID          = sdlEvent.wheel.which;
        outEvent.Wheel.X                = sdlEvent.wheel.x;
        outEvent.Wheel.Y                = sdlEvent.wheel.y;
        outEvent.Wheel.Direction        = static_cast<MouseWheelDirection>( sdlEvent.wheel.direction );
        break;

    case SDL_WINDOWEVENT:
        outEvent.Window.Common.Timestamp = sdlEvent.window.timestamp;
        outEvent.Window.Common.WindowID  = sdlEvent.window.windowID;
        outEvent.Window.Event            = static_cast<WindowEventType>( sdlEvent.window.event );
        outEvent.Window.Data1            = sdlEvent.window.data1;
        outEvent.Window.Data2            = sdlEvent.window.data2;
        break;

    case SDL_CONTROLLERAXISMOTION:
        outEvent.ControllerAxis.Common.Timestamp = sdlEvent.caxis.timestamp;
        outEvent.ControllerAxis.Common.WindowID  = 0; // SDL doesn't provide window ID for controller events
        outEvent.ControllerAxis.JoystickID       = sdlEvent.caxis.which;
        outEvent.ControllerAxis.Axis             = static_cast<ControllerAxis>( sdlEvent.caxis.axis );
        outEvent.ControllerAxis.Value            = sdlEvent.caxis.value;
        break;

    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        outEvent.ControllerButton.Common.Timestamp = sdlEvent.cbutton.timestamp;
        outEvent.ControllerButton.Common.WindowID  = 0; // SDL doesn't provide window ID for controller events
        outEvent.ControllerButton.JoystickID       = sdlEvent.cbutton.which;
        outEvent.ControllerButton.Button           = static_cast<ControllerButton>( sdlEvent.cbutton.button );
        outEvent.ControllerButton.State            = sdlEvent.cbutton.state ? KeyState::Pressed : KeyState::Released;
        break;

    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
        outEvent.ControllerDevice.Common.Timestamp = sdlEvent.cdevice.timestamp;
        outEvent.ControllerDevice.Common.WindowID  = 0; // SDL doesn't provide window ID for controller events
        outEvent.ControllerDevice.JoystickID       = sdlEvent.cdevice.which;
        break;

    case SDL_QUIT:
        outEvent.Quit.Common.Timestamp = sdlEvent.quit.timestamp;
        outEvent.Quit.Common.WindowID  = 0; // SDL doesn't provide window ID for quit events
        break;
    default:
        break;
    }
}

// Keep namespace (Unambigious reference on OSX)
DenOfIz::BitSet<KeyMod> InputSystem::Impl::ConvertKeyMod( const SDL_Keymod sdlMods )
{
    BitSet result = KeyMod::None;
#ifdef WINDOW_MANAGER_SDL
    if ( sdlMods & KMOD_LSHIFT )
    {
        result.Set( KeyMod::LShift );
    }
    if ( sdlMods & KMOD_RSHIFT )
    {
        result.Set( KeyMod::RShift );
    }
    if ( sdlMods & KMOD_LCTRL )
    {
        result.Set( KeyMod::LCtrl );
    }
    if ( sdlMods & KMOD_RCTRL )
    {
        result.Set( KeyMod::RCtrl );
    }
    if ( sdlMods & KMOD_LALT )
    {
        result.Set( KeyMod::LAlt );
    }
    if ( sdlMods & KMOD_RALT )
    {
        result.Set( KeyMod::RAlt );
    }
    if ( sdlMods & KMOD_LGUI )
    {
        result.Set( KeyMod::LGui );
    }
    if ( sdlMods & KMOD_RGUI )
    {
        result.Set( KeyMod::RGui );
    }
    if ( sdlMods & KMOD_NUM )
    {
        result.Set( KeyMod::NumLock );
    }
    if ( sdlMods & KMOD_CAPS )
    {
        result.Set( KeyMod::CapsLock );
    }
    if ( sdlMods & KMOD_MODE )
    {
        result.Set( KeyMod::AltGr );
    }
    if ( sdlMods & KMOD_SHIFT )
    {
        result.Set( KeyMod::Shift );
    }
    if ( sdlMods & KMOD_CTRL )
    {
        result.Set( KeyMod::Ctrl );
    }
    if ( sdlMods & KMOD_ALT )
    {
        result.Set( KeyMod::Alt );
    }
    if ( sdlMods & KMOD_GUI )
    {
        result.Set( KeyMod::Gui );
    }
#endif
    return result;
}

SDL_Keymod InputSystem::Impl::ConvertToSdlKeyMod( const BitSet<KeyMod> &modifiers )
{
    uint16_t sdlMods = 0;
    if ( modifiers.IsSet( KeyMod::LShift ) )
    {
        sdlMods |= KMOD_LSHIFT;
    }
    if ( modifiers.IsSet( KeyMod::RShift ) )
    {
        sdlMods |= KMOD_RSHIFT;
    }
    if ( modifiers.IsSet( KeyMod::LCtrl ) )
    {
        sdlMods |= KMOD_LCTRL;
    }
    if ( modifiers.IsSet( KeyMod::RCtrl ) )
    {
        sdlMods |= KMOD_RCTRL;
    }
    if ( modifiers.IsSet( KeyMod::LAlt ) )
    {
        sdlMods |= KMOD_LALT;
    }
    if ( modifiers.IsSet( KeyMod::RAlt ) )
    {
        sdlMods |= KMOD_RALT;
    }
    if ( modifiers.IsSet( KeyMod::LGui ) )
    {
        sdlMods |= KMOD_LGUI;
    }
    if ( modifiers.IsSet( KeyMod::RGui ) )
    {
        sdlMods |= KMOD_RGUI;
    }
    if ( modifiers.IsSet( KeyMod::NumLock ) )
    {
        sdlMods |= KMOD_NUM;
    }
    if ( modifiers.IsSet( KeyMod::CapsLock ) )
    {
        sdlMods |= KMOD_CAPS;
    }
    if ( modifiers.IsSet( KeyMod::AltGr ) )
    {
        sdlMods |= KMOD_MODE;
    }
    if ( modifiers.IsSet( KeyMod::Shift ) )
    {
        sdlMods |= KMOD_SHIFT;
    }
    if ( modifiers.IsSet( KeyMod::Ctrl ) )
    {
        sdlMods |= KMOD_CTRL;
    }
    if ( modifiers.IsSet( KeyMod::Alt ) )
    {
        sdlMods |= KMOD_ALT;
    }
    if ( modifiers.IsSet( KeyMod::Gui ) )
    {
        sdlMods |= KMOD_GUI;
    }
    return static_cast<SDL_Keymod>( sdlMods );
}
#endif

MouseCoords InputSystem::GetMouseState( )
{
    MouseCoords result{ };
#ifdef WINDOW_MANAGER_SDL
    SDL_GetMouseState( &result.X, &result.Y );
    return result;
#else
    return { 0, 0 };
#endif
}

MouseCoords InputSystem::GetGlobalMouseState( )
{
    MouseCoords result{ };
#ifdef WINDOW_MANAGER_SDL
    SDL_GetGlobalMouseState( &result.X, &result.Y );
    return result;
#else
    return { 0, 0 };
#endif
}

uint32_t InputSystem::GetMouseButtons( )
{
#ifdef WINDOW_MANAGER_SDL
    return SDL_GetMouseState( nullptr, nullptr );
#else
    return 0;
#endif
}

MouseCoords InputSystem::GetRelativeMouseState( )
{
    MouseCoords result{ };
#ifdef WINDOW_MANAGER_SDL
    SDL_GetRelativeMouseState( &result.X, &result.Y );
    return result;
#else
    return { 0, 0 };
#endif
}

void InputSystem::WarpMouseInWindow( const Window &window, const int x, const int y )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_Window* sdlWindow = SDL_GetWindowFromID( window.GetWindowID( ) );
    if ( sdlWindow )
    {
        SDL_WarpMouseInWindow( sdlWindow, x, y );
    }
#endif
}

void InputSystem::WarpMouseGlobal( const int x, const int y )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_WarpMouseGlobal( x, y );
#endif
}

bool InputSystem::GetRelativeMouseMode( )
{
#ifdef WINDOW_MANAGER_SDL
    return SDL_GetRelativeMouseMode( ) == SDL_TRUE;
#else
    return false;
#endif
}

void InputSystem::SetRelativeMouseMode( const bool enabled )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_SetRelativeMouseMode( enabled ? SDL_TRUE : SDL_FALSE );
#endif
}

void InputSystem::CaptureMouse( const bool enabled )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_CaptureMouse( enabled ? SDL_TRUE : SDL_FALSE );
#endif
}

bool InputSystem::GetMouseFocus( const uint32_t windowID )
{
#ifdef WINDOW_MANAGER_SDL
    if ( SDL_Window *window = SDL_GetWindowFromID( windowID ) )
    {
        return ( SDL_GetWindowFlags( window ) & SDL_WINDOW_MOUSE_FOCUS ) != 0;
    }
#endif
    return false;
}

void InputSystem::ShowCursor( const bool show )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_ShowCursor( show ? SDL_ENABLE : SDL_DISABLE );
#endif
}

bool InputSystem::IsCursorShown( )
{
#ifdef WINDOW_MANAGER_SDL
    return SDL_ShowCursor( SDL_QUERY ) == SDL_ENABLE;
#else
    return true;
#endif
}

InteropArray<int> InputSystem::GetConnectedControllerIndices( )
{
    return Controller::GetConnectedControllerIndices( );
}

int InputSystem::GetNumControllers( )
{
    return Controller::GetControllerCount( );
}

bool InputSystem::OpenController( const int playerIndex, const int controllerIndex )
{
	if ( playerIndex < 0 || playerIndex >= 4 )
	{
		return false;
	}

	if ( m_impl->m_controllerInitialized[ playerIndex ] )
	{
		CloseController( playerIndex );
	}

	const bool result                      = m_impl->m_controllers[ playerIndex ].Open( controllerIndex );
	m_impl->m_controllerInitialized[ playerIndex ] = result;

	if ( result )
	{
		return m_impl->m_controllers[ playerIndex ].SetPlayerIndex( playerIndex );
	}

	return result;
}

void InputSystem::CloseController( const int playerIndex )
{
	if ( playerIndex < 0 || playerIndex >= 4 )
	{
		return;
	}

	if ( m_impl->m_controllerInitialized[ playerIndex ] )
	{
		m_impl->m_controllers[ playerIndex ].Close( );
		m_impl->m_controllerInitialized[ playerIndex ] = false;
	}
}

bool InputSystem::IsControllerConnected( const int playerIndex ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 )
	{
		return false;
	}

	return m_impl->m_controllerInitialized[ playerIndex ] && m_impl->m_controllers[ playerIndex ].IsConnected( );
}

Controller *InputSystem::GetController( const int playerIndex )
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return nullptr;
	}

	return &m_impl->m_controllers[ playerIndex ];
}

const Controller *InputSystem::GetController( const int playerIndex ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return nullptr;
	}

	return &m_impl->m_controllers[ playerIndex ];
}

bool InputSystem::IsControllerButtonPressed( const int playerIndex, const ControllerButton button ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return false;
	}

	return m_impl->m_controllers[ playerIndex ].IsButtonPressed( button );
}

int16_t InputSystem::GetControllerAxisValue( const int playerIndex, const ControllerAxis axis ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return 0;
	}

	return m_impl->m_controllers[ playerIndex ].GetAxisValue( axis );
}

InteropString InputSystem::GetControllerName( const int playerIndex ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return { };
	}

	return m_impl->m_controllers[ playerIndex ].GetName( );
}

bool InputSystem::SetControllerRumble( const int playerIndex, const uint16_t lowFrequency, const uint16_t highFrequency, const uint32_t durationMs ) const
{
	if ( playerIndex < 0 || playerIndex >= 4 || !m_impl->m_controllerInitialized[ playerIndex ] )
	{
		return false;
	}

	return m_impl->m_controllers[ playerIndex ].SetRumble( lowFrequency, highFrequency, durationMs );
}
