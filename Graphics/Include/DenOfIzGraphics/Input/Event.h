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
    struct DZ_API CommonEventData
    {
        uint32_t Timestamp;
        uint32_t WindowID;
    };

    struct DZ_API KeyboardEventData
    {
        CommonEventData Common;
        KeyState        State;
        uint32_t        Repeat;
        KeyCode         Keycode;
        BitSet<KeyMod>  Mod;
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

    struct DZ_API MouseButtonState
    {
        bool LeftButton   = false;
        bool MiddleButton = false;
        bool RightButton  = false;
        bool X1Button     = false;
        bool X2Button     = false;

#ifdef WINDOW_MANAGER_SDL
        static MouseButtonState FromSDLButtonState( const uint32_t state )
        {
            MouseButtonState result;
            result.LeftButton   = ( state & SDL_BUTTON_LMASK ) != 0;
            result.MiddleButton = ( state & SDL_BUTTON_MMASK ) != 0;
            result.RightButton  = ( state & SDL_BUTTON_RMASK ) != 0;
            result.X1Button     = ( state & SDL_BUTTON_X1MASK ) != 0;
            result.X2Button     = ( state & SDL_BUTTON_X2MASK ) != 0;
            return result;
        }

        uint32_t ToSDLButtonState( ) const
        {
            uint32_t state = 0;
            if ( LeftButton )
            {
                state |= SDL_BUTTON_LMASK;
            }
            if ( MiddleButton )
            {
                state |= SDL_BUTTON_MMASK;
            }
            if ( RightButton )
            {
                state |= SDL_BUTTON_RMASK;
            }
            if ( X1Button )
            {
                state |= SDL_BUTTON_X1MASK;
            }
            if ( X2Button )
            {
                state |= SDL_BUTTON_X2MASK;
            }
            return state;
        }
#endif
    };

    struct DZ_API MouseMotionEventData
    {
        CommonEventData  Common;
        uint32_t         MouseID;
        MouseButtonState Buttons;
        int32_t          X;
        int32_t          Y;
        int32_t          RelX;
        int32_t          RelY;
    };

    struct DZ_API MouseButtonEventData
    {
        CommonEventData Common;
        uint32_t        MouseID;
        MouseButton     Button;
        KeyState        State;
        uint32_t        Clicks;
        int32_t         X;
        int32_t         Y;
    };

    struct DZ_API MouseWheelEventData
    {
        CommonEventData     Common;
        uint32_t            MouseID;
        int32_t             X;
        int32_t             Y;
        MouseWheelDirection Direction;
    };

    struct DZ_API WindowEventData
    {
        CommonEventData Common;
        WindowEventType Event;
        int32_t         Data1;
        int32_t         Data2;
    };

    struct DZ_API ControllerAxisEventData
    {
        CommonEventData Common;
        uint32_t        JoystickID;
        ControllerAxis  Axis;
        int16_t         Value;
    };

    struct DZ_API ControllerButtonEventData
    {
        CommonEventData  Common;
        uint32_t         JoystickID;
        ControllerButton Button;
        KeyState         State;
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
        EventType Type;

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

        Event( ) : Type( EventType::None )
        {
            memset( &Common, 0, sizeof( Common ) );
        }
    };

} // namespace DenOfIz
