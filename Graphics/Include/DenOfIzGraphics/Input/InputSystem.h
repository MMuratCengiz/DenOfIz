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

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Input/Controller.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Utilities/Engine.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include <memory>

namespace DenOfIz
{
    struct DZ_API MouseCoords
    {
        int X;
        int Y;
    };

    class DZ_API InputSystem
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        InputSystem( );
        ~InputSystem( );
        
        InputSystem( const InputSystem& ) = delete;
        InputSystem& operator=( const InputSystem& ) = delete;
        
        InputSystem( InputSystem&& other ) noexcept;
        InputSystem& operator=( InputSystem&& other ) noexcept;

        static bool PollEvent( Event &outEvent );
        static bool WaitEvent( Event &outEvent );
        static bool WaitEventTimeout( Event &outEvent, int timeout );
        static void PumpEvents( );
        static void FlushEvents( EventType minType, EventType maxType );
        static void PushEvent( const Event &event );

        // Keyboard
        static KeyState       GetKeyState( KeyCode key );
        static bool           IsKeyPressed( KeyCode key );
        static BitSet<KeyMod> GetModState( );
        static void           SetModState( const BitSet<KeyMod> &modifiers );
        static KeyCode        GetKeyFromName( const InteropString &name );
        static InteropString  GetKeyName( KeyCode key );
        static InteropString  GetScancodeName( uint32_t scancode );

        // Mouse
        static MouseCoords GetMouseState( );
        static MouseCoords GetGlobalMouseState( );
        static uint32_t    GetMouseButtons( );
        static MouseCoords GetRelativeMouseState( );
        static void        WarpMouseInWindow( const Window &window, int x, int y );
        static void        WarpMouseGlobal( int x, int y );
        static bool        GetRelativeMouseMode( );
        static void        SetRelativeMouseMode( bool enabled );
        static void        CaptureMouse( bool enabled );
        static bool        GetMouseFocus( uint32_t windowID );

        // Cursor
        static void ShowCursor( bool show );
        static bool IsCursorShown( );

        // Controller
        [[nodiscard]] static InteropArray<int> GetConnectedControllerIndices( );
        [[nodiscard]] static int               GetNumControllers( );
        bool                                   OpenController( int playerIndex, int controllerIndex );
        void                                   CloseController( int playerIndex );
        [[nodiscard]] bool                     IsControllerConnected( int playerIndex ) const;
        [[nodiscard]] Controller              *GetController( int playerIndex );
        [[nodiscard]] const Controller        *GetController( int playerIndex ) const;
        [[nodiscard]] bool                     IsControllerButtonPressed( int playerIndex, ControllerButton button ) const;
        [[nodiscard]] int16_t                  GetControllerAxisValue( int playerIndex, ControllerAxis axis ) const;
        [[nodiscard]] InteropString            GetControllerName( int playerIndex ) const;
        [[nodiscard]] bool                     SetControllerRumble( int playerIndex, uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs ) const;
    };

} // namespace DenOfIz
