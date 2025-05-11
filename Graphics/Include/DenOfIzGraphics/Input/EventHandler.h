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
    class EventHandler
    {
        InputSystem          *m_inputSystem;
        EventHandlerCallbacks m_callbacks;
        bool                  m_shouldQuit;

    public:
        DZ_API explicit EventHandler( InputSystem *inputSystem );
        DZ_API ~EventHandler( );

        DZ_API bool ProcessEvent( const Event &event );
        DZ_API void ProcessEvents( );
        DZ_API bool ProcessEventsWithTimeout( int timeout );

        DZ_API void SetOnKeyDown( KeyboardEventCallback *callback );
        DZ_API void SetOnKeyUp( KeyboardEventCallback *callback );
        DZ_API void SetOnMouseMotion( MouseMotionEventCallback *callback );
        DZ_API void SetOnMouseButtonDown( MouseButtonEventCallback *callback );
        DZ_API void SetOnMouseButtonUp( MouseButtonEventCallback *callback );
        DZ_API void SetOnMouseWheel( MouseWheelEventCallback *callback );
        DZ_API void SetOnWindowEvent( WindowEventCallback *callback );
        DZ_API void SetOnControllerAxisMotion( ControllerAxisEventCallback *callback );
        DZ_API void SetOnControllerButtonDown( ControllerButtonEventCallback *callback );
        DZ_API void SetOnControllerButtonUp( ControllerButtonEventCallback *callback );
        DZ_API void SetOnQuit( QuitEventCallback *callback );
        DZ_API void SetOnGenericEvent( EventCallback *callback );

        DZ_API [[nodiscard]] bool ShouldQuit( ) const;
        DZ_API void               SetShouldQuit( bool quit );
    };

} // namespace DenOfIz
