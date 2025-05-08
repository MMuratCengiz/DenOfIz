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

#include <DenOfIzGraphics/Input/EventHandler.h>
#include <DenOfIzGraphics/Utilities/Common_Macro.h>

using namespace DenOfIz;

EventHandler::EventHandler( InputSystem *inputSystem ) : m_inputSystem( inputSystem ), m_shouldQuit( false )
{
    DZ_NOT_NULL( inputSystem );
}

EventHandler::~EventHandler( )
{
    // We don't delete the callbacks here as they are owned by the user
    m_callbacks = EventHandlerCallbacks( );
}

bool EventHandler::ProcessEvent( const Event &event )
{
    if ( !m_inputSystem )
    {
        return false;
    }

    // Always call the generic event handler if it's set
    if ( m_callbacks.OnGenericEvent )
    {
        m_callbacks.OnGenericEvent->Execute( event );
    }

    switch ( static_cast<EventType>( event.Type ) )
    {
    case EventType::KeyDown:
        if ( m_callbacks.OnKeyDown )
        {
            m_callbacks.OnKeyDown->Execute( event.Key );
        }
        break;
    case EventType::KeyUp:
        if ( m_callbacks.OnKeyUp )
        {
            m_callbacks.OnKeyUp->Execute( event.Key );
        }
        break;
    case EventType::MouseMotion:
        if ( m_callbacks.OnMouseMotion )
        {
            m_callbacks.OnMouseMotion->Execute( event.Motion );
        }
        break;
    case EventType::MouseButtonDown:
        if ( m_callbacks.OnMouseButtonDown )
        {
            m_callbacks.OnMouseButtonDown->Execute( event.Button );
        }
        break;
    case EventType::MouseButtonUp:
        if ( m_callbacks.OnMouseButtonUp )
        {
            m_callbacks.OnMouseButtonUp->Execute( event.Button );
        }
        break;
    case EventType::MouseWheel:
        if ( m_callbacks.OnMouseWheel )
        {
            m_callbacks.OnMouseWheel->Execute( event.Wheel );
        }
        break;
    case EventType::WindowEvent:
        if ( m_callbacks.OnWindowEvent )
        {
            m_callbacks.OnWindowEvent->Execute( event.Window );
        }

        if ( static_cast<WindowEventType>( event.Window.Event ) == WindowEventType::Close )
        {
            m_shouldQuit = true;
        }
        break;
    case EventType::ControllerAxisMotion:
        if ( m_callbacks.OnControllerAxisMotion )
        {
            m_callbacks.OnControllerAxisMotion->Execute( event.ControllerAxis );
        }
        break;
    case EventType::ControllerButtonDown:
        if ( m_callbacks.OnControllerButtonDown )
        {
            m_callbacks.OnControllerButtonDown->Execute( event.ControllerButton );
        }
        break;
    case EventType::ControllerButtonUp:
        if ( m_callbacks.OnControllerButtonUp )
        {
            m_callbacks.OnControllerButtonUp->Execute( event.ControllerButton );
        }
        break;
    case EventType::Quit:
        if ( m_callbacks.OnQuit )
        {
            m_callbacks.OnQuit->Execute( event.Quit );
        }
        m_shouldQuit = true;
        break;
    default:
        return false;
    }

    return true;
}

void EventHandler::ProcessEvents( )
{
    if ( !m_inputSystem )
    {
        return;
    }

    Event event{ };
    while ( m_inputSystem->PollEvent( event ) )
    {
        ProcessEvent( event );
    }
}

bool EventHandler::ProcessEventsWithTimeout( const int timeout )
{
    if ( !m_inputSystem )
    {
        return false;
    }

    Event event{ };
    if ( m_inputSystem->WaitEventTimeout( event, timeout ) )
    {
        ProcessEvent( event );
        while ( m_inputSystem->PollEvent( event ) )
        {
            ProcessEvent( event );
        }

        return true;
    }

    return false;
}

void EventHandler::SetOnKeyDown( KeyboardEventCallback *callback )
{
    m_callbacks.OnKeyDown = callback;
}

void EventHandler::SetOnKeyUp( KeyboardEventCallback *callback )
{
    m_callbacks.OnKeyUp = callback;
}

void EventHandler::SetOnMouseMotion( MouseMotionEventCallback *callback )
{
    m_callbacks.OnMouseMotion = callback;
}

void EventHandler::SetOnMouseButtonDown( MouseButtonEventCallback *callback )
{
    m_callbacks.OnMouseButtonDown = callback;
}

void EventHandler::SetOnMouseButtonUp( MouseButtonEventCallback *callback )
{
    m_callbacks.OnMouseButtonUp = callback;
}

void EventHandler::SetOnMouseWheel( MouseWheelEventCallback *callback )
{
    m_callbacks.OnMouseWheel = callback;
}

void EventHandler::SetOnWindowEvent( WindowEventCallback *callback )
{
    m_callbacks.OnWindowEvent = callback;
}

void EventHandler::SetOnControllerAxisMotion( ControllerAxisEventCallback *callback )
{
    m_callbacks.OnControllerAxisMotion = callback;
}

void EventHandler::SetOnControllerButtonDown( ControllerButtonEventCallback *callback )
{
    m_callbacks.OnControllerButtonDown = callback;
}

void EventHandler::SetOnControllerButtonUp( ControllerButtonEventCallback *callback )
{
    m_callbacks.OnControllerButtonUp = callback;
}

void EventHandler::SetOnQuit( QuitEventCallback *callback )
{
    m_callbacks.OnQuit = callback;
}

void EventHandler::SetOnGenericEvent( EventCallback *callback )
{
    m_callbacks.OnGenericEvent = callback;
}

bool EventHandler::ShouldQuit( ) const
{
    return m_shouldQuit;
}

void EventHandler::SetShouldQuit( const bool quit )
{
    m_shouldQuit = quit;
}
