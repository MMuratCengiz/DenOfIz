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

#include "DenOfIzGraphics/Utilities/Engine.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

// A full map of SDL2
namespace DenOfIz
{
    namespace KeyMod
    {
        DZ_API constexpr uint32_t None     = 1 << 0;
        DZ_API constexpr uint32_t LShift   = 1 << 1;
        DZ_API constexpr uint32_t RShift   = 1 << 2;
        DZ_API constexpr uint32_t LCtrl    = 1 << 3;
        DZ_API constexpr uint32_t RCtrl    = 1 << 4;
        DZ_API constexpr uint32_t LAlt     = 1 << 5;
        DZ_API constexpr uint32_t RAlt     = 1 << 6;
        DZ_API constexpr uint32_t LGui     = 1 << 7;
        DZ_API constexpr uint32_t RGui     = 1 << 8;
        DZ_API constexpr uint32_t NumLock  = 1 << 9;
        DZ_API constexpr uint32_t CapsLock = 1 << 10;
        DZ_API constexpr uint32_t AltGr    = 1 << 11;
        DZ_API constexpr uint32_t Shift    = 1 << 12;
        DZ_API constexpr uint32_t Ctrl     = 1 << 13;
        DZ_API constexpr uint32_t Alt      = 1 << 14;
        DZ_API constexpr uint32_t Gui      = 1 << 15;
    } // namespace KeyMod

    enum class DZ_API KeyCode
    {
        Unknown = 0,

        // Basic keys
        Return      = '\r',
        Escape      = '\033',
        Backspace   = '\b',
        Tab         = '\t',
        Space       = ' ',
        Exclaim     = '!',
        DoubleQuote = '"',
        Hash        = '#',
        Percent     = '%',
        Dollar      = '$',
        Ampersand   = '&',
        Quote       = '\'',
        LeftParen   = '(',
        RightParen  = ')',
        Asterisk    = '*',
        Plus        = '+',
        Comma       = ',',
        Minus       = '-',
        Period      = '.',
        Slash       = '/',

        // Numbers
        Num0 = '0',
        Num1 = '1',
        Num2 = '2',
        Num3 = '3',
        Num4 = '4',
        Num5 = '5',
        Num6 = '6',
        Num7 = '7',
        Num8 = '8',
        Num9 = '9',

        // Punctuation
        Colon     = ':',
        SemiColon = ';',
        Less      = '<',
        Equals    = '=',
        Greater   = '>',
        Question  = '?',
        At        = '@',

        A = 'a',
        B = 'b',
        C = 'c',
        D = 'd',
        E = 'e',
        F = 'f',
        G = 'g',
        H = 'h',
        I = 'i',
        J = 'j',
        K = 'k',
        L = 'l',
        M = 'm',
        N = 'n',
        O = 'o',
        P = 'p',
        Q = 'q',
        R = 'r',
        S = 's',
        T = 't',
        U = 'u',
        V = 'v',
        W = 'w',
        X = 'x',
        Y = 'y',
        Z = 'z',

        // Brackets and misc
        LeftBracket  = '[',
        Backslash    = '\\',
        RightBracket = ']',
        Caret        = '^',
        Underscore   = '_',
        BackQuote    = '`',

        // Special keys
        CapsLock = 0x40000039,

        // Function keys
        F1  = 0x4000003A,
        F2  = 0x4000003B,
        F3  = 0x4000003C,
        F4  = 0x4000003D,
        F5  = 0x4000003E,
        F6  = 0x4000003F,
        F7  = 0x40000040,
        F8  = 0x40000041,
        F9  = 0x40000042,
        F10 = 0x40000043,
        F11 = 0x40000044,
        F12 = 0x40000045,

        // Navigation and editing keys
        PrintScreen = 0x40000046,
        ScrollLock  = 0x40000047,
        Pause       = 0x40000048,
        Insert      = 0x40000049,
        Home        = 0x4000004A,
        PageUp      = 0x4000004B,
        Delete      = 0x7F,
        End         = 0x4000004D,
        PageDown    = 0x4000004E,
        Right       = 0x4000004F,
        Left        = 0x40000050,
        Down        = 0x40000051,
        Up          = 0x40000052,

        // Keypad
        NumLockClear   = 0x40000053,
        KeypadDivide   = 0x40000054,
        KeypadMultiply = 0x40000055,
        KeypadMinus    = 0x40000056,
        KeypadPlus     = 0x40000057,
        KeypadEnter    = 0x40000058,
        Keypad1        = 0x40000059,
        Keypad2        = 0x4000005A,
        Keypad3        = 0x4000005B,
        Keypad4        = 0x4000005C,
        Keypad5        = 0x4000005D,
        Keypad6        = 0x4000005E,
        Keypad7        = 0x4000005F,
        Keypad8        = 0x40000060,
        Keypad9        = 0x40000061,
        Keypad0        = 0x40000062,
        KeypadPeriod   = 0x40000063,

        // Modifier keys
        LCtrl  = 0x400000E0,
        LShift = 0x400000E1,
        LAlt   = 0x400000E2,
        LGui   = 0x400000E3,
        RCtrl  = 0x400000E4,
        RShift = 0x400000E5,
        RAlt   = 0x400000E6,
        RGui   = 0x400000E7
    };

    enum class DZ_API MouseButton
    {
        Left   = 1,
        Middle = 2,
        Right  = 3,
        X1     = 4,
        X2     = 5
    };

    enum class DZ_API MouseWheelDirection
    {
        Normal  = 0,
        Flipped = 1
    };

    enum class DZ_API ControllerButton
    {
        Invalid       = -1,
        A             = 0,
        B             = 1,
        X             = 2,
        Y             = 3,
        Back          = 4,
        Guide         = 5,
        Start         = 6,
        LeftStick     = 7,
        RightStick    = 8,
        LeftShoulder  = 9,
        RightShoulder = 10,
        DPadUp        = 11,
        DPadDown      = 12,
        DPadLeft      = 13,
        DPadRight     = 14,
        Misc1         = 15, // Xbox Series X share button, PS5 microphone button
        Paddle1       = 16, // Xbox Elite paddle P1
        Paddle2       = 17, // Xbox Elite paddle P3
        Paddle3       = 18, // Xbox Elite paddle P2
        Paddle4       = 19, // Xbox Elite paddle P4
        Touchpad      = 20, // PS4/PS5 touchpad button
        Max           = 21
    };

    enum class DZ_API ControllerAxis
    {
        Invalid      = -1,
        LeftX        = 0,
        LeftY        = 1,
        RightX       = 2,
        RightY       = 3,
        TriggerLeft  = 4,
        TriggerRight = 5,
        Max          = 6
    };

    enum class DZ_API WindowEventType
    {
        None        = 0,
        Shown       = 1,
        Hidden      = 2,
        Exposed     = 3,
        Moved       = 4,
        Resized     = 5,
        SizeChanged = 6,
        Minimized   = 7,
        Maximized   = 8,
        Restored    = 9,
        Enter       = 10,
        Leave       = 11,
        FocusGained = 12,
        FocusLost   = 13,
        Close       = 14
    };

    enum class DZ_API EventType
    {
        None                     = 0,
        Quit                     = 0x100,
        AppTerminating           = 0x101,
        AppLowMemory             = 0x102,
        AppWillEnterBackground   = 0x103,
        AppDidEnterBackground    = 0x104,
        AppWillEnterForeground   = 0x105,
        AppDidEnterForeground    = 0x106,
        DisplayEvent             = 0x150,
        WindowEvent              = 0x200,
        KeyDown                  = 0x300,
        KeyUp                    = 0x301,
        TextEditing              = 0x302,
        TextInput                = 0x303,
        KeymapChanged            = 0x304,
        MouseMotion              = 0x400,
        MouseButtonDown          = 0x401,
        MouseButtonUp            = 0x402,
        MouseWheel               = 0x403,
        JoyAxisMotion            = 0x600,
        JoyBallMotion            = 0x601,
        JoyHatMotion             = 0x602,
        JoyButtonDown            = 0x603,
        JoyButtonUp              = 0x604,
        JoyDeviceAdded           = 0x605,
        JoyDeviceRemoved         = 0x606,
        ControllerAxisMotion     = 0x650,
        ControllerButtonDown     = 0x651,
        ControllerButtonUp       = 0x652,
        ControllerDeviceAdded    = 0x653,
        ControllerDeviceRemoved  = 0x654,
        ControllerDeviceRemapped = 0x655,
        FingerDown               = 0x700,
        FingerUp                 = 0x701,
        FingerMotion             = 0x702,
        DollarGesture            = 0x800,
        DollarRecord             = 0x801,
        MultiGesture             = 0x802,
        ClipboardUpdate          = 0x900,
        DropFile                 = 0x1000,
        DropText                 = 0x1001,
        DropBegin                = 0x1002,
        DropComplete             = 0x1003,
        AudioDeviceAdded         = 0x1100,
        AudioDeviceRemoved       = 0x1101,
        SensorUpdate             = 0x1200,
        RenderTargetsReset       = 0x2000,
        RenderDeviceReset        = 0x2001
    };

    enum class DZ_API KeyState
    {
        Released = 0,
        Pressed  = 1
    };

} // namespace DenOfIz
