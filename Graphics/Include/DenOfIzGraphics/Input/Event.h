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
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    class Window;
    struct DZ_API CommonEventData
    {
        uint32_t Timestamp;
        uint32_t WindowID;
    };

    struct DZ_API KeyboardEventData
    {
        CommonEventData Common;
        uint32_t        State;
        uint32_t        Repeat;
        KeyCode         Keycode;
        uint16_t        Mod;
        uint32_t        Scancode;
    };

    struct DZ_API TextEditingEventData
    {
        CommonEventData Common;
        char            Text[ 32 ];
        int32_t         Start;
        int32_t         Length;
    };

    struct DZ_API TextInputEventData
    {
        CommonEventData Common;
        char            Text[ 32 ];
    };

    struct DZ_API MouseMotionEventData
    {
        CommonEventData Common;
        uint32_t        MouseID;
        uint32_t        State;
        int32_t         X;
        int32_t         Y;
        int32_t         RelX;
        int32_t         RelY;
    };

    struct DZ_API MouseButtonEventData
    {
        CommonEventData Common;
        uint32_t        MouseID;
        uint32_t        Button;
        uint32_t        State;
        uint32_t        Clicks;
        int32_t         X;
        int32_t         Y;
    };

    struct DZ_API MouseWheelEventData
    {
        CommonEventData Common;
        uint32_t        MouseID;
        int32_t         X;
        int32_t         Y;
        uint32_t        Direction;
    };

    struct DZ_API WindowEventData
    {
        CommonEventData Common;
        uint32_t        Event;
        int32_t         Data1;
        int32_t         Data2;
    };

    struct DZ_API ControllerAxisEventData
    {
        CommonEventData Common;
        uint32_t        JoystickID;
        uint8_t         Axis;
        int16_t         Value;
    };

    struct DZ_API ControllerButtonEventData
    {
        CommonEventData Common;
        uint32_t        JoystickID;
        uint8_t         Button;
        uint8_t         State;
    };

    struct DZ_API ControllerDeviceEventData
    {
        CommonEventData Common;
        uint32_t        JoystickID;
    };

    struct DZ_API QuitEventData
    {
        CommonEventData Common;
    };

    struct DZ_API Event
    {
        uint32_t Type;

        union
        {
            CommonEventData           Common;
            KeyboardEventData         Key;
            TextEditingEventData      Edit;
            TextInputEventData        Text;
            MouseMotionEventData      Motion;
            MouseButtonEventData      Button;
            MouseWheelEventData       Wheel;
            WindowEventData           Window;
            ControllerAxisEventData   ControllerAxis;
            ControllerButtonEventData ControllerButton;
            ControllerDeviceEventData ControllerDevice;
            QuitEventData             Quit;
        };
    };

} // namespace DenOfIz
