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

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif

namespace DenOfIz
{
    struct DZ_API ControllerDeviceInfo
    {
        uint32_t InstanceID;
        InteropString Name;
        bool IsConnected;
        uint32_t VendorID;
        uint32_t ProductID;
        uint16_t Version;
        uint32_t PlayerIndex;
    };

    class DZ_API Controller
    {
    private:
        bool m_initialized;
        int m_controllerIndex;
        uint32_t m_instanceID;
        static bool s_sdlInitialized;

#ifdef WINDOW_MANAGER_SDL
        SDL_GameController* m_gameController;
        SDL_Joystick* m_joystick;
#endif

    public:
        Controller();
        explicit Controller(int controllerIndex);
        ~Controller();

        bool Open(int controllerIndex);
        void Close();

        // Controller button/axis functions
        [[nodiscard]] bool IsButtonPressed(ControllerButton button) const;
        [[nodiscard]] int16_t GetAxisValue(ControllerAxis axis) const;
        
        // Rumble functionality
        bool HasRumble() const;
        bool SetRumble(uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs ) const;
        bool SetTriggerRumble(bool leftTrigger, bool rightTrigger, uint16_t strength, uint32_t durationMs ) const;
        
        // Mapping functions
        [[nodiscard]] InteropString GetButtonName(ControllerButton button) const;
        [[nodiscard]] InteropString GetAxisName(ControllerAxis axis) const;
        [[nodiscard]] bool HasButton(ControllerButton button) const;
        [[nodiscard]] bool HasAxis(ControllerAxis axis) const;
        [[nodiscard]] InteropString GetMapping() const;
        
        // Controller identification/information
        [[nodiscard]] bool IsConnected() const;
        [[nodiscard]] InteropString GetName() const;
        [[nodiscard]] uint32_t GetInstanceID() const;
        [[nodiscard]] ControllerDeviceInfo GetDeviceInfo() const;
        [[nodiscard]] bool SetPlayerIndex(int playerIndex ) const;
        [[nodiscard]] int GetPlayerIndex() const;

#ifdef WINDOW_MANAGER_SDL
        [[nodiscard]] SDL_GameController* GetSDLGameController() const;
        [[nodiscard]] SDL_Joystick* GetSDLJoystick() const;
#endif

        // Static utility functions
        static void InitializeSDL();
        static InteropArray<int> GetConnectedControllerIndices();
        static bool IsGameController(int joystickIndex);
        static int GetControllerCount();
        static InteropString GetControllerNameForIndex(int joystickIndex);
    };

} // namespace DenOfIz