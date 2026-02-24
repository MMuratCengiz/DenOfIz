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

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Input/InputData.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_CommonEventData
    {
        uint32_t Timestamp;
        uint32_t WindowID;
    } DenOfIz_CommonEventData;

    typedef struct DenOfIz_KeyboardEventData
    {
        DenOfIz_CommonEventData Common;
        DenOfIz_KeyState        State;
        uint32_t                Repeat;
        DenOfIz_KeyCode         KeyCode;
        uint32_t                Mod;
        uint32_t                ScanCode;
    } DenOfIz_KeyboardEventData;

    typedef struct DenOfIz_TextEditingEventData
    {
        DenOfIz_CommonEventData Common;
        DenOfIz_StringView      Text;
        int32_t                 Start;
        int32_t                 Length;
    } DenOfIz_TextEditingEventData;

    typedef struct DenOfIz_TextInputEventData
    {
        DenOfIz_CommonEventData Common;
        DenOfIz_StringView      Text;
    } DenOfIz_TextInputEventData;

    typedef struct DenOfIz_MouseButtonState
    {
        bool LeftButton;
        bool MiddleButton;
        bool RightButton;
        bool X1Button;
        bool X2Button;
    } DenOfIz_MouseButtonState;

    typedef struct DenOfIz_MouseMotionEventData
    {
        DenOfIz_CommonEventData  Common;
        uint32_t                 MouseID;
        DenOfIz_MouseButtonState Buttons;
        float                    X;
        float                    Y;
        float                    RelX;
        float                    RelY;
    } DenOfIz_MouseMotionEventData;

    typedef struct DenOfIz_MouseButtonEventData
    {
        DenOfIz_CommonEventData Common;
        uint32_t                MouseID;
        DenOfIz_MouseButton     Button;
        DenOfIz_KeyState        State;
        uint32_t                Clicks;
        float                   X;
        float                   Y;
    } DenOfIz_MouseButtonEventData;

    typedef struct DenOfIz_MouseWheelEventData
    {
        DenOfIz_CommonEventData     Common;
        uint32_t                    MouseID;
        float                       X;
        float                       Y;
        DenOfIz_MouseWheelDirection Direction;
    } DenOfIz_MouseWheelEventData;

    typedef struct DenOfIz_WindowEventData
    {
        DenOfIz_CommonEventData Common;
        DenOfIz_WindowEventType Event;
        int32_t                 Data1;
        int32_t                 Data2;
    } DenOfIz_WindowEventData;

    typedef struct DenOfIz_ControllerAxisEventData
    {
        DenOfIz_CommonEventData Common;
        uint32_t                JoystickID;
        DenOfIz_ControllerAxis  Axis;
        int16_t                 Value;
    } DenOfIz_ControllerAxisEventData;

    typedef struct DenOfIz_ControllerButtonEventData
    {
        DenOfIz_CommonEventData  Common;
        uint32_t                 JoystickID;
        DenOfIz_ControllerButton Button;
        DenOfIz_KeyState         State;
    } DenOfIz_ControllerButtonEventData;

    typedef struct DenOfIz_ControllerDeviceEventData
    {
        DenOfIz_CommonEventData Common;
        uint32_t                JoystickID;
    } DenOfIz_ControllerDeviceEventData;

    typedef struct DenOfIz_QuitEventData
    {
        DenOfIz_CommonEventData Common;
    } DenOfIz_QuitEventData;

    typedef struct DenOfIz_Event
    {
        DenOfIz_EventType                 Type;
        DenOfIz_CommonEventData           Common;
        DenOfIz_KeyboardEventData         Key;
        DenOfIz_TextEditingEventData      Edit;
        DenOfIz_TextInputEventData        Text;
        DenOfIz_MouseMotionEventData      MouseMotion;
        DenOfIz_MouseButtonEventData      MouseButton;
        DenOfIz_MouseWheelEventData       MouseWheel;
        DenOfIz_WindowEventData           Window;
        DenOfIz_ControllerAxisEventData   ControllerAxis;
        DenOfIz_ControllerButtonEventData ControllerButton;
        DenOfIz_ControllerDeviceEventData ControllerDevice;
        DenOfIz_QuitEventData             Quit;
    } DenOfIz_Event;

#ifdef __cplusplus
}
#endif
