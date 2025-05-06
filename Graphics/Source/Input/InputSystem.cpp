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

#include <DenOfIzGraphics/Input/InputSystem.h>

using namespace DenOfIz;

bool InputSystem::s_sdlInitialized = false;

InputSystem::InputSystem( ) : m_initialized( false )
{
    for ( bool &controllerInitialized : m_controllerInitialized )
    {
        controllerInitialized = false;
    }
}

InputSystem::~InputSystem( )
{
    Shutdown( );
}

void InputSystem::Initialize( )
{
    if ( m_initialized )
    {
        return;
    }

    InitializeSDL( );
    m_initialized = true;
}

void InputSystem::Shutdown( )
{
    if ( m_initialized )
    {
        for ( int i = 0; i < 4; ++i )
        {
            CloseController( i );
        }

        m_initialized = false;
    }
}

void InputSystem::InitializeSDL( )
{
    if ( !s_sdlInitialized )
    {
        SDL_SetMainReady( );
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER );
        s_sdlInitialized = true;
    }
}

bool InputSystem::PollEvent( Event &outEvent )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_Event sdlEvent;
    if ( SDL_PollEvent( &sdlEvent ) )
    {
        ConvertSDLEventToEvent( sdlEvent, outEvent );
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
        ConvertSDLEventToEvent( sdlEvent, outEvent );
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
        ConvertSDLEventToEvent( sdlEvent, outEvent );
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

void InputSystem::FlushEvents( const uint32_t minType, const uint32_t maxType )
{
#ifdef WINDOW_MANAGER_SDL
    SDL_FlushEvents( minType, maxType );
#endif
}

void InputSystem::PushEvent( const Event &event )
{
#ifdef WINDOW_MANAGER_SDL
    // Not implemented as it would require converting back to SDL_Event
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

const uint8_t *InputSystem::GetKeyboardState( )
{
#ifdef WINDOW_MANAGER_SDL
    return SDL_GetKeyboardState( nullptr );
#else
    return nullptr;
#endif
}

bool InputSystem::IsKeyPressed( const KeyCode key )
{
    return GetKeyState( key ) == KeyState::Pressed;
}

KeyModifiers InputSystem::GetModState( )
{
    KeyModifiers modifiers;
#ifdef WINDOW_MANAGER_SDL
    const uint16_t sdlMods = SDL_GetModState( );
    modifiers.FromSDLKeymod( sdlMods );
#endif
    return modifiers;
}

void InputSystem::SetModState( const KeyModifiers &modifiers )
{
#ifdef WINDOW_MANAGER_SDL
    uint16_t sdlMods = modifiers.ToSDLKeymod( );
    SDL_SetModState( static_cast<SDL_Keymod>( sdlMods ) );
#endif
}

#ifdef WINDOW_MANAGER_SDL
void InputSystem::ConvertSDLEventToEvent( const SDL_Event &sdlEvent, Event &outEvent )
{
    // TODO implement
}
#endif

bool InputSystem::IsInitialized( ) const
{
    return m_initialized;
}
