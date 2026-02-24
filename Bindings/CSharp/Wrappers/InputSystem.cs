using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class InputSystem : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(InputSystem wrapper) => wrapper?.Handle ?? 0;

        public InputSystem()
        {
            Handle = Methods.DenOfIz_InputSystem_Create();
            _ownsHandle = true;
        }

        internal InputSystem(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool OpenController(int playerIndex, int controllerIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_OpenController(Handle, playerIndex, controllerIndex);
        }

        public void CloseController(int playerIndex)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_InputSystem_CloseController(Handle, playerIndex);
        }

        public bool IsControllerConnected(int playerIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_IsControllerConnected(Handle, playerIndex);
        }

        public Controller GetController(int playerIndex)
        {
            ThrowIfDisposed();
            return new Controller(Methods.DenOfIz_InputSystem_GetController(Handle, playerIndex), ownsHandle: false);
        }

        public bool IsControllerButtonPressed(int playerIndex, ControllerButton button)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_IsControllerButtonPressed(Handle, playerIndex, button);
        }

        public short GetControllerAxisValue(int playerIndex, ControllerAxis axis)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_GetControllerAxisValue(Handle, playerIndex, axis);
        }

        public StringView GetControllerName(int playerIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_GetControllerName(Handle, playerIndex);
        }

        public bool SetControllerRumble(int playerIndex, ushort lowFrequency, ushort highFrequency, uint durationMs)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_InputSystem_SetControllerRumble(Handle, playerIndex, lowFrequency, highFrequency, durationMs);
        }

        public static bool PollEvent(out Event outEvent)
        {
            return Methods.DenOfIz_InputSystem_PollEvent(out outEvent);
        }

        public static bool WaitEvent(out Event outEvent)
        {
            return Methods.DenOfIz_InputSystem_WaitEvent(out outEvent);
        }

        public static bool WaitEventTimeout(out Event outEvent, int timeout)
        {
            return Methods.DenOfIz_InputSystem_WaitEventTimeout(out outEvent, timeout);
        }

        public static void PumpEvents()
        {
            Methods.DenOfIz_InputSystem_PumpEvents();
        }

        public static void FlushEvents(EventType minType, EventType maxType)
        {
            Methods.DenOfIz_InputSystem_FlushEvents(minType, maxType);
        }

        public static void PushEvent(in Event ev)
        {
            Methods.DenOfIz_InputSystem_PushEvent(in ev);
        }

        public static KeyState GetKeyState(KeyCode key)
        {
            return Methods.DenOfIz_InputSystem_GetKeyState(key);
        }

        public static bool IsKeyPressed(KeyCode key)
        {
            return Methods.DenOfIz_InputSystem_IsKeyPressed(key);
        }

        public static uint GetModState()
        {
            return Methods.DenOfIz_InputSystem_GetModState();
        }

        public static void SetModState(uint modifiers)
        {
            Methods.DenOfIz_InputSystem_SetModState(modifiers);
        }

        public static KeyCode GetKeyFromName(StringView name)
        {
            return Methods.DenOfIz_InputSystem_GetKeyFromName(name);
        }

        public static StringView GetKeyName(KeyCode key)
        {
            return Methods.DenOfIz_InputSystem_GetKeyName(key);
        }

        public static StringView GetScancodeName(uint scancode)
        {
            return Methods.DenOfIz_InputSystem_GetScancodeName(scancode);
        }

        public static MouseCoords GetMouseState()
        {
            return Methods.DenOfIz_InputSystem_GetMouseState();
        }

        public static MouseCoords GetGlobalMouseState()
        {
            return Methods.DenOfIz_InputSystem_GetGlobalMouseState();
        }

        public static MouseButtonState GetMouseButtonState()
        {
            return Methods.DenOfIz_InputSystem_GetMouseButtonState();
        }

        public static bool IsMouseButtonPressed(MouseButton button)
        {
            return Methods.DenOfIz_InputSystem_IsMouseButtonPressed(button);
        }

        public static uint GetMouseButtons()
        {
            return Methods.DenOfIz_InputSystem_GetMouseButtons();
        }

        public static MouseCoords GetRelativeMouseState()
        {
            return Methods.DenOfIz_InputSystem_GetRelativeMouseState();
        }

        public static void WarpMouseInWindow(Window? window, float x, float y)
        {
            Methods.DenOfIz_InputSystem_WarpMouseInWindow(window, x, y);
        }

        public static bool GetRelativeMouseMode(uint windowId)
        {
            return Methods.DenOfIz_InputSystem_GetRelativeMouseMode(windowId);
        }

        public static void SetRelativeMouseMode(uint windowId, bool enabled)
        {
            Methods.DenOfIz_InputSystem_SetRelativeMouseMode(windowId, enabled);
        }

        public static void CaptureMouse(bool enabled)
        {
            Methods.DenOfIz_InputSystem_CaptureMouse(enabled);
        }

        public static bool GetMouseFocus(uint windowID)
        {
            return Methods.DenOfIz_InputSystem_GetMouseFocus(windowID);
        }

        public static void ShowCursor(bool show)
        {
            Methods.DenOfIz_InputSystem_ShowCursor(show);
        }

        public static bool IsCursorShown()
        {
            return Methods.DenOfIz_InputSystem_IsCursorShown();
        }

        public static void SetCursor(SystemCursor cursor)
        {
            Methods.DenOfIz_InputSystem_SetCursor(cursor);
        }

        public static void ResetCursor()
        {
            Methods.DenOfIz_InputSystem_ResetCursor();
        }

        public static void StartTextInput()
        {
            Methods.DenOfIz_InputSystem_StartTextInput();
        }

        public static void StopTextInput()
        {
            Methods.DenOfIz_InputSystem_StopTextInput();
        }

        public static bool IsTextInputActive()
        {
            return Methods.DenOfIz_InputSystem_IsTextInputActive();
        }

        public static void SetTextInputRect(Window? window, int x, int y, int w, int h)
        {
            Methods.DenOfIz_InputSystem_SetTextInputRect(window, x, y, w, h);
        }

        public static Int32Array GetConnectedControllerIndices()
        {
            return Methods.DenOfIz_InputSystem_GetConnectedControllerIndices();
        }

        public static int GetNumControllers()
        {
            return Methods.DenOfIz_InputSystem_GetNumControllers();
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_InputSystem_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(InputSystem));
            }
        }
    }
}
