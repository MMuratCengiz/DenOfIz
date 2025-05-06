%module(directors="1") DenOfIzGraphics

%feature("director") DenOfIz::EventCallback;
%feature("director") DenOfIz::KeyboardEventCallback;
%feature("director") DenOfIz::MouseMotionEventCallback;
%feature("director") DenOfIz::MouseButtonEventCallback;
%feature("director") DenOfIz::MouseWheelEventCallback;
%feature("director") DenOfIz::WindowEventCallback;
%feature("director") DenOfIz::ControllerAxisEventCallback;
%feature("director") DenOfIz::ControllerButtonEventCallback;
%feature("director") DenOfIz::QuitEventCallback;

%include <DenOfIzGraphics/Input/InputData.h>
%include <DenOfIzGraphics/Input/Event.h>
%include <DenOfIzGraphics/Input/EventCallbacks.h>
%include <DenOfIzGraphics/Input/Window.h>
%include <DenOfIzGraphics/Input/Controller.h>
%include <DenOfIzGraphics/Input/InputSystem.h>
%include <DenOfIzGraphics/Input/EventHandler.h>

typedef DenOfIz::InteropArray<int> IntArray;
%template(IntArray) DenOfIz::InteropArray<int>;