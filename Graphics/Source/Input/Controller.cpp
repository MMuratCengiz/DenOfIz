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

#include "DenOfIzGraphics/Input/Controller.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

struct Controller::Impl
{
    bool     m_initialized;
    int      m_controllerIndex;
    uint32_t m_instanceID;
#ifdef WINDOW_MANAGER_SDL
    SDL_GameController *m_gameController;
    SDL_Joystick       *m_joystick;
#endif

    Impl( ) :
        m_initialized( false ), m_controllerIndex( -1 ), m_instanceID( 0 )
#ifdef WINDOW_MANAGER_SDL
        ,
        m_gameController( nullptr ), m_joystick( nullptr )
#endif
    {
    }
};

static bool s_sdlInitialized = false;

Controller::Controller( ) : m_impl( std::make_unique<Impl>( ) )
{
}

Controller::Controller( const int controllerIndex ) : m_impl( std::make_unique<Impl>( ) )
{
    Open( controllerIndex );
}

Controller::~Controller( )
{
    Close( );
}

void Controller::InitializeSDL( )
{
    if ( !s_sdlInitialized )
    {
#ifdef WINDOW_MANAGER_SDL
        if ( SDL_WasInit( SDL_INIT_GAMECONTROLLER ) == 0 )
        {
            spdlog::info( "Initializing SDL Game Controller" );
            if ( SDL_InitSubSystem( SDL_INIT_GAMECONTROLLER ) != 0 )
            {
                spdlog::critical( "Failed to initialize SDL Game Controller: {}", SDL_GetError( ) );
            }
        }
#endif
        s_sdlInitialized = true;
    }
}

bool Controller::Open( const int controllerIndex ) const
{
    Close( );

    InitializeSDL( );

#ifdef WINDOW_MANAGER_SDL
    if ( !IsGameController( controllerIndex ) )
    {
        spdlog::warn( "Device at index {} is not a game controller", controllerIndex );
        return false;
    }

    m_impl->m_gameController = SDL_GameControllerOpen( controllerIndex );
    if ( m_impl->m_gameController == nullptr )
    {
        spdlog::error( "Could not open game controller {} : {}", controllerIndex, SDL_GetError( ) );
        return false;
    }

    m_impl->m_joystick = SDL_GameControllerGetJoystick( m_impl->m_gameController );
    if ( m_impl->m_joystick == nullptr )
    {
        SDL_GameControllerClose( m_impl->m_gameController );
        m_impl->m_gameController = nullptr;
        spdlog::error( "Could not get joystick from controller {} : {}", controllerIndex, SDL_GetError( ) );
        return false;
    }

    m_impl->m_controllerIndex = controllerIndex;
    m_impl->m_instanceID      = SDL_JoystickInstanceID( m_impl->m_joystick );
    m_impl->m_initialized     = true;

    spdlog::info( "Opened controller {} ( {} ), instance ID: {}", controllerIndex, GetName( ).Get( ), m_impl->m_instanceID );
    return true;
#else
    return false;
#endif
}

void Controller::Close( ) const
{
    if ( !m_impl->m_initialized )
    {
        return;
    }

#ifdef WINDOW_MANAGER_SDL
    if ( m_impl->m_gameController != nullptr )
    {
        SDL_GameControllerClose( m_impl->m_gameController );
        m_impl->m_gameController = nullptr;
        m_impl->m_joystick       = nullptr;
    }
#endif

    m_impl->m_controllerIndex = -1;
    m_impl->m_instanceID      = 0;
    m_impl->m_initialized     = false;
}

bool Controller::IsButtonPressed( ControllerButton button ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerGetButton( m_impl->m_gameController, static_cast<SDL_GameControllerButton>( button ) ) == 1;
#else
    return false;
#endif
}

int16_t Controller::GetAxisValue( ControllerAxis axis ) const
{
    if ( !m_impl->m_initialized )
    {
        return 0;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerGetAxis( m_impl->m_gameController, static_cast<SDL_GameControllerAxis>( axis ) );
#else
    return 0;
#endif
}

bool Controller::HasRumble( ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerHasRumble( m_impl->m_gameController ) == SDL_TRUE;
#else
    return false;
#endif
}

bool Controller::SetRumble( const uint16_t lowFrequencyRumble, const uint16_t highFrequencyRumble, const uint32_t durationMs ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerRumble( m_impl->m_gameController, lowFrequencyRumble, highFrequencyRumble, durationMs ) == 0;
#else
    return false;
#endif
}

bool Controller::SetTriggerRumble( const bool leftTrigger, const bool rightTrigger, const uint16_t strength, const uint32_t durationMs ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    if ( leftTrigger && rightTrigger )
    {
        return SDL_GameControllerRumbleTriggers( m_impl->m_gameController, strength, strength, durationMs ) == 0;
    }
    if ( leftTrigger )
    {
        return SDL_GameControllerRumbleTriggers( m_impl->m_gameController, strength, 0, durationMs ) == 0;
    }
    if ( rightTrigger )
    {
        return SDL_GameControllerRumbleTriggers( m_impl->m_gameController, 0, strength, durationMs ) == 0;
    }
#endif

    return false;
}

InteropString Controller::GetButtonName( ControllerButton button ) const
{
    if ( !m_impl->m_initialized )
    {
        return { };
    }

#ifdef WINDOW_MANAGER_SDL
    return { SDL_GameControllerGetStringForButton( static_cast<SDL_GameControllerButton>( button ) ) };
#else
    return InteropString( );
#endif
}

InteropString Controller::GetAxisName( ControllerAxis axis ) const
{
    if ( !m_impl->m_initialized )
    {
        return { };
    }

#ifdef WINDOW_MANAGER_SDL
    return { SDL_GameControllerGetStringForAxis( static_cast<SDL_GameControllerAxis>( axis ) ) };
#else
    return InteropString( );
#endif
}

bool Controller::HasButton( ControllerButton button ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerHasButton( m_impl->m_gameController, static_cast<SDL_GameControllerButton>( button ) ) == SDL_TRUE;
#else
    return false;
#endif
}

bool Controller::HasAxis( ControllerAxis axis ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerHasAxis( m_impl->m_gameController, static_cast<SDL_GameControllerAxis>( axis ) ) == SDL_TRUE;
#else
    return false;
#endif
}

InteropString Controller::GetMapping( ) const
{
    if ( !m_impl->m_initialized )
    {
        return { };
    }

#ifdef WINDOW_MANAGER_SDL
    return { SDL_GameControllerMapping( m_impl->m_gameController ) };
#else
    return InteropString( );
#endif
}

bool Controller::IsConnected( ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerGetAttached( m_impl->m_gameController ) == SDL_TRUE;
#else
    return false;
#endif
}

InteropString Controller::GetName( ) const
{
    if ( !m_impl->m_initialized )
    {
        return { };
    }

#ifdef WINDOW_MANAGER_SDL
    return { SDL_GameControllerName( m_impl->m_gameController ) };
#else
    return InteropString( );
#endif
}

uint32_t Controller::GetInstanceID( ) const
{
    return m_impl->m_instanceID;
}

ControllerDeviceInfo Controller::GetDeviceInfo( ) const
{
    ControllerDeviceInfo info = { };

    if ( !m_impl->m_initialized )
    {
        return info;
    }

#ifdef WINDOW_MANAGER_SDL
    info.InstanceID  = m_impl->m_instanceID;
    info.Name        = GetName( );
    info.IsConnected = IsConnected( );
    info.PlayerIndex = GetPlayerIndex( );

    // Get vendor and product information
    const uint16_t vendor = SDL_GameControllerGetVendor( m_impl->m_gameController );
    info.VendorID         = vendor;

    const uint16_t product = SDL_GameControllerGetProduct( m_impl->m_gameController );
    info.ProductID         = product;

    const uint16_t version = SDL_GameControllerGetProductVersion( m_impl->m_gameController );
    info.Version           = version;
#endif

    return info;
}

bool Controller::SetPlayerIndex( const int playerIndex ) const
{
    if ( !m_impl->m_initialized )
    {
        return false;
    }

#ifdef WINDOW_MANAGER_SDL
    SDL_GameControllerSetPlayerIndex( m_impl->m_gameController, playerIndex );
    return true;
#else
    return false;
#endif
}

int Controller::GetPlayerIndex( ) const
{
    if ( !m_impl->m_initialized )
    {
        return -1;
    }

#ifdef WINDOW_MANAGER_SDL
    return SDL_GameControllerGetPlayerIndex( m_impl->m_gameController );
#else
    return -1;
#endif
}

InteropArray<int> Controller::GetConnectedControllerIndices( )
{
    InitializeSDL( );

    InteropArray<int> result;

#ifdef WINDOW_MANAGER_SDL
    const int numJoysticks = SDL_NumJoysticks( );
    for ( int i = 0; i < numJoysticks; i++ )
    {
        if ( IsGameController( i ) )
        {
            result.AddElement( i );
        }
    }
#endif

    return result;
}

bool Controller::IsGameController( const int joystickIndex )
{
    InitializeSDL( );

#ifdef WINDOW_MANAGER_SDL
    return SDL_IsGameController( joystickIndex ) == SDL_TRUE;
#else
    return false;
#endif
}

int Controller::GetControllerCount( )
{
    InitializeSDL( );

#ifdef WINDOW_MANAGER_SDL
    int       count        = 0;
    const int numJoysticks = SDL_NumJoysticks( );

    for ( int i = 0; i < numJoysticks; i++ )
    {
        if ( SDL_IsGameController( i ) )
        {
            count++;
        }
    }

    return count;
#else
    return 0;
#endif
}

InteropString Controller::GetControllerNameForIndex( const int joystickIndex )
{
    InitializeSDL( );

#ifdef WINDOW_MANAGER_SDL
    if ( IsGameController( joystickIndex ) )
    {
        return { SDL_GameControllerNameForIndex( joystickIndex ) };
    }
#endif

    return { };
}
