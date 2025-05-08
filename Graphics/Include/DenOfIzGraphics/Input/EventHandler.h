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

#include <DenOfIzGraphics/Input/Event.h>
#include <DenOfIzGraphics/Input/EventCallbacks.h>
#include <DenOfIzGraphics/Input/InputSystem.h>
#include <DenOfIzGraphics/Utilities/Engine.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    class DZ_API EventHandler
    {
        InputSystem          *m_inputSystem;
        EventHandlerCallbacks m_callbacks;
        bool                  m_shouldQuit;

    public:
        explicit EventHandler( InputSystem *inputSystem );
        ~EventHandler( );

        bool ProcessEvent( const Event &event );
        void ProcessEvents( );
        bool ProcessEventsWithTimeout( int timeout );

        void SetOnKeyDown( KeyboardEventCallback *callback );
        void SetOnKeyUp( KeyboardEventCallback *callback );
        void SetOnMouseMotion( MouseMotionEventCallback *callback );
        void SetOnMouseButtonDown( MouseButtonEventCallback *callback );
        void SetOnMouseButtonUp( MouseButtonEventCallback *callback );
        void SetOnMouseWheel( MouseWheelEventCallback *callback );
        void SetOnWindowEvent( WindowEventCallback *callback );
        void SetOnControllerAxisMotion( ControllerAxisEventCallback *callback );
        void SetOnControllerButtonDown( ControllerButtonEventCallback *callback );
        void SetOnControllerButtonUp( ControllerButtonEventCallback *callback );
        void SetOnQuit( QuitEventCallback *callback );
        void SetOnGenericEvent( EventCallback *callback );

        [[nodiscard]] bool ShouldQuit( ) const;
        void               SetShouldQuit( bool quit );
    };

} // namespace DenOfIz
