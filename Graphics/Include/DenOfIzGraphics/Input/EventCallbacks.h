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

// ReSharper disable CppClassCanBeFinal
#pragma once

#include <DenOfIzGraphics/Input/Event.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    class DZ_API EventCallback{ public : virtual ~EventCallback( ){} virtual void Execute( const Event &event ){} };
    class DZ_API KeyboardEventCallback{ public : virtual ~KeyboardEventCallback( ){} virtual void Execute( const KeyboardEventData &eventData ){} };
    class DZ_API MouseMotionEventCallback{ public : virtual ~MouseMotionEventCallback( ){} virtual void Execute( const MouseMotionEventData &eventData ){} };
    class DZ_API MouseButtonEventCallback{ public : virtual ~MouseButtonEventCallback( ){} virtual void Execute( const MouseButtonEventData &eventData ){} };
    class DZ_API MouseWheelEventCallback{ public : virtual ~MouseWheelEventCallback( ){} virtual void Execute( const MouseWheelEventData &eventData ){} };
    class DZ_API WindowEventCallback{ public : virtual ~WindowEventCallback( ){} virtual void Execute( const WindowEventData &eventData ){} };
    class DZ_API ControllerAxisEventCallback{ public : virtual ~ControllerAxisEventCallback( ){} virtual void Execute( const ControllerAxisEventData &eventData ){} };
    class DZ_API ControllerButtonEventCallback{ public : virtual ~ControllerButtonEventCallback( ){} virtual void Execute( const ControllerButtonEventData &eventData ){} };
    class DZ_API QuitEventCallback{ public : virtual ~QuitEventCallback( ){} virtual void Execute( const QuitEventData &eventData ){} };

    struct DZ_API EventHandlerCallbacks
    {
        KeyboardEventCallback         *OnKeyDown              = nullptr;
        KeyboardEventCallback         *OnKeyUp                = nullptr;
        MouseMotionEventCallback      *OnMouseMotion          = nullptr;
        MouseButtonEventCallback      *OnMouseButtonDown      = nullptr;
        MouseButtonEventCallback      *OnMouseButtonUp        = nullptr;
        MouseWheelEventCallback       *OnMouseWheel           = nullptr;
        WindowEventCallback           *OnWindowEvent          = nullptr;
        ControllerAxisEventCallback   *OnControllerAxisMotion = nullptr;
        ControllerButtonEventCallback *OnControllerButtonDown = nullptr;
        ControllerButtonEventCallback *OnControllerButtonUp   = nullptr;
        QuitEventCallback             *OnQuit                 = nullptr;
        EventCallback                 *OnGenericEvent         = nullptr;

        ~EventHandlerCallbacks( )
        {
        }
    };

} // namespace DenOfIz
