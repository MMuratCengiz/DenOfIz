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

#include <DenOfIzGraphics/Input/Controller.h>
#include <DenOfIzGraphics/Input/Event.h>
#include <DenOfIzGraphics/Input/Window.h>
#include <DenOfIzGraphics/Utilities/Engine.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

#define WINDOW_MANAGER_SDL

#ifdef WINDOW_MANAGER_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif

namespace DenOfIz
{
    struct DZ_API KeyModifiers
    {
        bool None       = true;
        bool LeftShift  = false;
        bool RightShift = false;
        bool LeftCtrl   = false;
        bool RightCtrl  = false;
        bool LeftAlt    = false;
        bool RightAlt   = false;
        bool LeftGui    = false;
        bool RightGui   = false;
        bool NumLock    = false;
        bool CapsLock   = false;
        bool AltGr      = false;

        bool Shift = false;
        bool Ctrl  = false;
        bool Alt   = false;
        bool Gui   = false;

#ifdef WINDOW_MANAGER_SDL
        [[nodiscard]] uint16_t ToSDLKeymod( ) const
        {
            uint16_t mod = 0;
            if ( LeftShift )
            {
                mod |= KMOD_LSHIFT;
            }
            if ( RightShift )
            {
                mod |= KMOD_RSHIFT;
            }
            if ( LeftCtrl )
            {
                mod |= KMOD_LCTRL;
            }
            if ( RightCtrl )
            {
                mod |= KMOD_RCTRL;
            }
            if ( LeftAlt )
            {
                mod |= KMOD_LALT;
            }
            if ( RightAlt )
            {
                mod |= KMOD_RALT;
            }
            if ( LeftGui )
            {
                mod |= KMOD_LGUI;
            }
            if ( RightGui )
            {
                mod |= KMOD_RGUI;
            }
            if ( NumLock )
            {
                mod |= KMOD_NUM;
            }
            if ( CapsLock )
            {
                mod |= KMOD_CAPS;
            }
            if ( AltGr )
            {
                mod |= KMOD_MODE;
            }
            return mod;
        }

        void FromSDLKeymod( const uint16_t mod )
        {
            None       = ( mod == 0 );
            LeftShift  = ( mod & KMOD_LSHIFT ) != 0;
            RightShift = ( mod & KMOD_RSHIFT ) != 0;
            LeftCtrl   = ( mod & KMOD_LCTRL ) != 0;
            RightCtrl  = ( mod & KMOD_RCTRL ) != 0;
            LeftAlt    = ( mod & KMOD_LALT ) != 0;
            RightAlt   = ( mod & KMOD_RALT ) != 0;
            LeftGui    = ( mod & KMOD_LGUI ) != 0;
            RightGui   = ( mod & KMOD_RGUI ) != 0;
            NumLock    = ( mod & KMOD_NUM ) != 0;
            CapsLock   = ( mod & KMOD_CAPS ) != 0;
            AltGr      = ( mod & KMOD_MODE ) != 0;

            Shift = LeftShift || RightShift;
            Ctrl  = LeftCtrl || RightCtrl;
            Alt   = LeftAlt || RightAlt;
            Gui   = LeftGui || RightGui;
        }
#endif
    };

    class DZ_API InputSystem
    {
        bool        m_initialized;
        static bool s_sdlInitialized;

        Controller m_controllers[ 4 ];
        bool       m_controllerInitialized[ 4 ];

    public:
        InputSystem( );
        ~InputSystem( );

        void Initialize( );
        void Shutdown( );

        // Event handling
        static bool PollEvent( Event &outEvent );
        static bool WaitEvent( Event &outEvent );
        static bool WaitEventTimeout( Event &outEvent, int timeout );
        static void PumpEvents( );
        static void FlushEvents( uint32_t minType, uint32_t maxType );
        static void PushEvent( const Event &event );

        // Keyboard functions
        static KeyState       GetKeyState( KeyCode key );
        static const uint8_t *GetKeyboardState( );
        static bool                  IsKeyPressed( KeyCode key );
        static KeyModifiers   GetModState( );
        static void           SetModState( const KeyModifiers &modifiers );
        KeyCode        GetKeyFromName( const InteropString &name );
        InteropString  GetKeyName( KeyCode key );
        InteropString  GetScancodeName( uint32_t scancode );

        // Mouse functions
        uint32_t GetMouseState( int *x, int *y );
        uint32_t GetGlobalMouseState( int *x, int *y );
        uint32_t GetMouseButtons( );
        uint32_t GetRelativeMouseState( int *x, int *y );
        void     WarpMouseInWindow( const Window &window, int x, int y );
        void     WarpMouseGlobal( int x, int y );
        bool     GetRelativeMouseMode( );
        void     SetRelativeMouseMode( bool enabled );
        void     CaptureMouse( bool enabled );
        bool     GetMouseFocus( uint32_t windowID );

        // Cursor functions
        void ShowCursor( bool show );
        bool IsCursorShown( );

        // Controller functions
        [[nodiscard]] InteropArray<int> GetConnectedControllerIndices( ) const;
        [[nodiscard]] int               GetNumControllers( ) const;
        bool                            OpenController( int playerIndex, int controllerIndex );
        void                            CloseController( int playerIndex );
        [[nodiscard]] bool              IsControllerConnected( int playerIndex ) const;
        [[nodiscard]] Controller       *GetController( int playerIndex );
        [[nodiscard]] const Controller *GetController( int playerIndex ) const;
        [[nodiscard]] bool              IsControllerButtonPressed( int playerIndex, ControllerButton button ) const;
        [[nodiscard]] int16_t           GetControllerAxisValue( int playerIndex, ControllerAxis axis ) const;
        [[nodiscard]] InteropString     GetControllerName( int playerIndex ) const;
        bool                            SetControllerRumble( int playerIndex, uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs );

        [[nodiscard]] bool IsInitialized( ) const;

    private:
        static void InitializeSDL( );

#ifdef WINDOW_MANAGER_SDL
        static void ConvertSDLEventToEvent( const SDL_Event &sdlEvent, Event &outEvent );
#endif
    };

} // namespace DenOfIz
