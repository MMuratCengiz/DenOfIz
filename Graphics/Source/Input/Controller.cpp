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

#include <string>
#include <vector>

#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

#define CONTROLLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::Controller, handle )

namespace DenOfIz
{
    class Controller
    {
    public:
        bool          m_initialized     = false;
        int           m_controllerIndex = -1;
        uint32_t      m_instanceID      = 0;
        SDL_Gamepad  *m_gameController  = nullptr;
        SDL_Joystick *m_joystick        = nullptr;
        std::string   m_mappingCache;

        Controller( );
        explicit Controller( int controllerIndex );
        ~Controller( );

        static void InitializeSDL( );
        bool        Open( int controllerIndex );
        void        Close( );

        bool    IsButtonPressed( DenOfIz_ControllerButton button ) const;
        int16_t GetAxisValue( DenOfIz_ControllerAxis axis ) const;

        bool HasRumble( ) const;
        bool SetRumble( uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs );
        bool SetTriggerRumble( bool leftTrigger, bool rightTrigger, uint16_t strength, uint32_t durationMs );

        DenOfIz_StringView GetButtonName( DenOfIz_ControllerButton button ) const;
        DenOfIz_StringView GetAxisName( DenOfIz_ControllerAxis axis ) const;
        bool               HasButton( DenOfIz_ControllerButton button ) const;
        bool               HasAxis( DenOfIz_ControllerAxis axis ) const;
        DenOfIz_StringView GetMapping( );

        bool                         IsConnected( ) const;
        DenOfIz_StringView           GetName( ) const;
        uint32_t                     GetInstanceID( ) const;
        DenOfIz_ControllerDeviceInfo GetDeviceInfo( ) const;
        bool                         SetPlayerIndex( int playerIndex );
        int                          GetPlayerIndex( ) const;

        static DenOfIz_Int32Array GetConnectedControllerIndices( );
        static bool               IsGameController( int joystickIndex );
        static int                GetControllerCount( );
        static DenOfIz_StringView GetControllerNameForIndex( int joystickIndex );
    };
} // namespace DenOfIz

using namespace DenOfIz;

static bool                 s_sdlInitialized = false;
static std::vector<int32_t> s_connectedControllers;

Controller::Controller( )
{
}

Controller::Controller( const int controllerIndex )
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
        if ( SDL_WasInit( SDL_INIT_GAMEPAD ) == 0 )
        {
            spdlog::info( "Initializing SDL Game Controller" );
            if ( SDL_InitSubSystem( SDL_INIT_GAMEPAD ) != 0 )
            {
                spdlog::critical( "Failed to initialize SDL Game Controller: {}", SDL_GetError( ) );
            }
        }
        s_sdlInitialized = true;
    }
}

bool Controller::Open( const int controllerIndex )
{
    Close( );

    InitializeSDL( );

    if ( !IsGameController( controllerIndex ) )
    {
        spdlog::warn( "Device at index {} is not a game controller", controllerIndex );
        return false;
    }

    m_gameController = SDL_OpenGamepad( controllerIndex );
    if ( m_gameController == nullptr )
    {
        spdlog::error( "Could not open game controller {} : {}", controllerIndex, SDL_GetError( ) );
        return false;
    }

    m_joystick = SDL_GetGamepadJoystick( m_gameController );
    if ( m_joystick == nullptr )
    {
        SDL_CloseGamepad( m_gameController );
        m_gameController = nullptr;
        spdlog::error( "Could not get joystick from controller {} : {}", controllerIndex, SDL_GetError( ) );
        return false;
    }

    m_controllerIndex = controllerIndex;
    m_instanceID      = SDL_GetJoystickID( m_joystick );
    m_initialized     = true;

    std::string name = StringViewToStdString( GetName( ) );
    spdlog::info( "Opened controller {} ( {} ), instance ID: {}", controllerIndex, name, m_instanceID );
    return true;
}

void Controller::Close( )
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

bool Controller::IsButtonPressed( const DenOfIz_ControllerButton button ) const
{
    if ( !m_initialized )
    {
        return false;
    }

    return SDL_GetGamepadButton( m_gameController, static_cast<SDL_GamepadButton>( button ) ) == 1;
}

int16_t Controller::GetAxisValue( const DenOfIz_ControllerAxis axis ) const
{
    if ( !m_initialized )
    {
        return 0;
    }

    return SDL_GetGamepadAxis( m_gameController, static_cast<SDL_GamepadAxis>( axis ) );
}

bool Controller::HasRumble( ) const
{
    if ( !m_initialized )
    {
        return false;
    }

    return false;
}

bool Controller::SetRumble( const uint16_t lowFrequencyRumble, const uint16_t highFrequencyRumble, const uint32_t durationMs )
{
    if ( !m_initialized )
    {
        return false;
    }

    return false;
}

bool Controller::SetTriggerRumble( const bool leftTrigger, const bool rightTrigger, const uint16_t strength, const uint32_t durationMs )
{
    if ( !m_initialized )
    {
        return false;
    }

    return false;
}

DenOfIz_StringView Controller::GetButtonName( const DenOfIz_ControllerButton button ) const
{
    if ( !m_initialized )
    {
        return { };
    }

    const char *gamePadString = SDL_GetGamepadStringForButton( static_cast<SDL_GamepadButton>( button ) );
    return DENOFIZ_STRING( gamePadString );
}

DenOfIz_StringView Controller::GetAxisName( const DenOfIz_ControllerAxis axis ) const
{
    if ( !m_initialized )
    {
        return { };
    }

    const char *gamePadString = SDL_GetGamepadStringForAxis( static_cast<SDL_GamepadAxis>( axis ) );
    return DENOFIZ_STRING( gamePadString );
}

bool Controller::HasButton( const DenOfIz_ControllerButton button ) const
{
    if ( !m_initialized )
    {
        return false;
    }

    return SDL_GamepadHasButton( m_gameController, static_cast<SDL_GamepadButton>( button ) );
}

bool Controller::HasAxis( const DenOfIz_ControllerAxis axis ) const
{
    if ( !m_initialized )
    {
        return false;
    }

    return SDL_GamepadHasAxis( m_gameController, static_cast<SDL_GamepadAxis>( axis ) );
}

DenOfIz_StringView Controller::GetMapping( )
{
    if ( !m_initialized )
    {
        return { };
    }

    char *mappingStr = SDL_GetGamepadMapping( m_gameController );
    if ( mappingStr != nullptr )
    {
        m_mappingCache = mappingStr;
        SDL_free( mappingStr );
        return { m_mappingCache.c_str( ) };
    }

    return { };
}

bool Controller::IsConnected( ) const
{
    if ( !m_initialized )
    {
        return false;
    }

    return SDL_GamepadConnected( m_gameController );
}

DenOfIz_StringView Controller::GetName( ) const
{
    if ( !m_initialized )
    {
        return { };
    }

    return { SDL_GetGamepadName( m_gameController ) };
}

uint32_t Controller::GetInstanceID( ) const
{
    return m_instanceID;
}

DenOfIz_ControllerDeviceInfo Controller::GetDeviceInfo( ) const
{
    DenOfIz_ControllerDeviceInfo info = { };

    if ( !m_initialized )
    {
        return info;
    }

    info.InstanceID  = m_instanceID;
    info.Name        = GetName( );
    info.IsConnected = IsConnected( );
    info.PlayerIndex = GetPlayerIndex( );

    const uint16_t vendor = SDL_GetGamepadVendor( m_gameController );
    info.VendorID         = vendor;

    const uint16_t product = SDL_GetGamepadProduct( m_gameController );
    info.ProductID         = product;

    const uint16_t version = SDL_GetGamepadProductVersion( m_gameController );
    info.Version           = version;
    return info;
}

bool Controller::SetPlayerIndex( const int playerIndex )
{
    if ( !m_initialized )
    {
        return false;
    }

    SDL_SetGamepadPlayerIndex( m_gameController, playerIndex );
    return true;
}

int Controller::GetPlayerIndex( ) const
{
    if ( !m_initialized )
    {
        return -1;
    }

    return SDL_GetGamepadPlayerIndex( m_gameController );
}

DenOfIz_Int32Array Controller::GetConnectedControllerIndices( )
{
    InitializeSDL( );

    s_connectedControllers.clear( );

    int numJoysticks;
    SDL_GetJoysticks( &numJoysticks );
    for ( int i = 0; i < numJoysticks; i++ )
    {
        if ( IsGameController( i ) )
        {
            s_connectedControllers.push_back( i );
        }
    }

    DenOfIz_Int32Array result;
    result.Elements    = s_connectedControllers.data( );
    result.NumElements = static_cast<uint32_t>( s_connectedControllers.size( ) );
    return result;
}

bool Controller::IsGameController( const int joystickIndex )
{
    InitializeSDL( );
    return SDL_IsGamepad( joystickIndex );
}

int Controller::GetControllerCount( )
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

DenOfIz_StringView Controller::GetControllerNameForIndex( const int joystickIndex )
{
    InitializeSDL( );

    if ( IsGameController( joystickIndex ) )
    {
        return { SDL_GetJoystickName( SDL_GetJoystickFromID( joystickIndex ) ) };
    }

    return { };
}

extern "C"
{

    DenOfIz_Controller DenOfIz_Controller_Create( )
    {
        auto *controller = new Controller( );
        return DENOFIZ_TO_HANDLE( controller );
    }

    DenOfIz_Controller DenOfIz_Controller_CreateWithIndex( int32_t controllerIndex )
    {
        auto *controller = new Controller( controllerIndex );
        return DENOFIZ_TO_HANDLE( controller );
    }

    void DenOfIz_Controller_Destroy( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return;
        }
        delete CONTROLLER_IMPL( controller );
    }

    bool DenOfIz_Controller_Open( DenOfIz_Controller controller, int32_t controllerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->Open( controllerIndex );
    }

    void DenOfIz_Controller_Close( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return;
        }
        CONTROLLER_IMPL( controller )->Close( );
    }

    bool DenOfIz_Controller_IsButtonPressed( DenOfIz_Controller controller, DenOfIz_ControllerButton button )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->IsButtonPressed( button );
    }

    int16_t DenOfIz_Controller_GetAxisValue( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return 0;
        }
        return CONTROLLER_IMPL( controller )->GetAxisValue( axis );
    }

    bool DenOfIz_Controller_HasRumble( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->HasRumble( );
    }

    bool DenOfIz_Controller_SetRumble( DenOfIz_Controller controller, uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->SetRumble( lowFrequencyRumble, highFrequencyRumble, durationMs );
    }

    bool DenOfIz_Controller_SetTriggerRumble( DenOfIz_Controller controller, bool leftTrigger, bool rightTrigger, uint16_t strength, uint32_t durationMs )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->SetTriggerRumble( leftTrigger, rightTrigger, strength, durationMs );
    }

    DenOfIz_StringView DenOfIz_Controller_GetButtonName( DenOfIz_Controller controller, DenOfIz_ControllerButton button )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return { };
        }
        return CONTROLLER_IMPL( controller )->GetButtonName( button );
    }

    DenOfIz_StringView DenOfIz_Controller_GetAxisName( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return { };
        }
        return CONTROLLER_IMPL( controller )->GetAxisName( axis );
    }

    bool DenOfIz_Controller_HasButton( DenOfIz_Controller controller, DenOfIz_ControllerButton button )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->HasButton( button );
    }

    bool DenOfIz_Controller_HasAxis( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->HasAxis( axis );
    }

    DenOfIz_StringView DenOfIz_Controller_GetMapping( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return { };
        }
        return CONTROLLER_IMPL( controller )->GetMapping( );
    }

    bool DenOfIz_Controller_IsConnected( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->IsConnected( );
    }

    DenOfIz_StringView DenOfIz_Controller_GetName( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return { };
        }
        return CONTROLLER_IMPL( controller )->GetName( );
    }

    uint32_t DenOfIz_Controller_GetInstanceID( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return 0;
        }
        return CONTROLLER_IMPL( controller )->GetInstanceID( );
    }

    DenOfIz_ControllerDeviceInfo DenOfIz_Controller_GetDeviceInfo( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return { };
        }
        return CONTROLLER_IMPL( controller )->GetDeviceInfo( );
    }

    bool DenOfIz_Controller_SetPlayerIndex( DenOfIz_Controller controller, int32_t playerIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return false;
        }
        return CONTROLLER_IMPL( controller )->SetPlayerIndex( playerIndex );
    }

    int32_t DenOfIz_Controller_GetPlayerIndex( DenOfIz_Controller controller )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( controller ) )
        {
            return -1;
        }
        return CONTROLLER_IMPL( controller )->GetPlayerIndex( );
    }

    void DenOfIz_Controller_InitializeSDL( )
    {
        Controller::InitializeSDL( );
    }

    DenOfIz_Int32Array DenOfIz_Controller_GetConnectedControllerIndices( )
    {
        return Controller::GetConnectedControllerIndices( );
    }

    bool DenOfIz_Controller_IsGameController( int32_t joystickIndex )
    {
        return Controller::IsGameController( joystickIndex );
    }

    int32_t DenOfIz_Controller_GetControllerCount( )
    {
        return Controller::GetControllerCount( );
    }

    DenOfIz_StringView DenOfIz_Controller_GetControllerNameForIndex( int32_t joystickIndex )
    {
        return Controller::GetControllerNameForIndex( joystickIndex );
    }
}
