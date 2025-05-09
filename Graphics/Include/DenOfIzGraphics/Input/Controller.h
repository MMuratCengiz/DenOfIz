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

#include <DenOfIzGraphics/Input/InputData.h>
#include <DenOfIzGraphics/Utilities/Engine.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Backends/Common/IncludeSDL.h>

namespace DenOfIz
{
    struct DZ_API ControllerDeviceInfo
    {
        uint32_t      InstanceID;
        InteropString Name;
        bool          IsConnected;
        uint32_t      VendorID;
        uint32_t      ProductID;
        uint16_t      Version;
        uint32_t      PlayerIndex;
    };

    class Controller
    {
        bool        m_initialized;
        int         m_controllerIndex;
        uint32_t    m_instanceID;
        static bool s_sdlInitialized;

#ifdef WINDOW_MANAGER_SDL
        SDL_GameController *m_gameController;
        SDL_Joystick       *m_joystick;
#endif

    public:
        DZ_API Controller( );
        DZ_API explicit Controller( int controllerIndex );
        DZ_API ~Controller( );

        DZ_API bool Open( int controllerIndex );
        DZ_API void Close( );

        DZ_API [[nodiscard]] bool    IsButtonPressed( ControllerButton button ) const;
        DZ_API [[nodiscard]] int16_t GetAxisValue( ControllerAxis axis ) const;

        DZ_API bool HasRumble( ) const;
        DZ_API bool SetRumble( uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs ) const;
        DZ_API bool SetTriggerRumble( bool leftTrigger, bool rightTrigger, uint16_t strength, uint32_t durationMs ) const;

        DZ_API [[nodiscard]] InteropString GetButtonName( ControllerButton button ) const;
        DZ_API [[nodiscard]] InteropString GetAxisName( ControllerAxis axis ) const;
        DZ_API [[nodiscard]] bool          HasButton( ControllerButton button ) const;
        DZ_API [[nodiscard]] bool          HasAxis( ControllerAxis axis ) const;
        DZ_API [[nodiscard]] InteropString GetMapping( ) const;

        DZ_API [[nodiscard]] bool                 IsConnected( ) const;
        DZ_API [[nodiscard]] InteropString        GetName( ) const;
        DZ_API [[nodiscard]] uint32_t             GetInstanceID( ) const;
        DZ_API [[nodiscard]] ControllerDeviceInfo GetDeviceInfo( ) const;
        DZ_API [[nodiscard]] bool                 SetPlayerIndex( int playerIndex ) const;
        DZ_API [[nodiscard]] int                  GetPlayerIndex( ) const;

        DZ_API static void              InitializeSDL( );
        DZ_API static InteropArray<int> GetConnectedControllerIndices( );
        DZ_API static bool              IsGameController( int joystickIndex );
        DZ_API static int               GetControllerCount( );
        DZ_API static InteropString     GetControllerNameForIndex( int joystickIndex );
    };

} // namespace DenOfIz
