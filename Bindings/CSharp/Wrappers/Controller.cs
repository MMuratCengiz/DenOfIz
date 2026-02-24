using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Controller : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Controller wrapper) => wrapper?.Handle ?? 0;

        public Controller()
        {
            Handle = Methods.DenOfIz_Controller_Create();
            _ownsHandle = true;
        }

        internal Controller(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool Open(int controllerIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_Open(Handle, controllerIndex);
        }

        public void Close()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Controller_Close(Handle);
        }

        public bool IsButtonPressed(ControllerButton button)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_IsButtonPressed(Handle, button);
        }

        public short GetAxisValue(ControllerAxis axis)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetAxisValue(Handle, axis);
        }

        public bool HasRumble()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_HasRumble(Handle);
        }

        public bool SetRumble(ushort lowFrequencyRumble, ushort highFrequencyRumble, uint durationMs)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_SetRumble(Handle, lowFrequencyRumble, highFrequencyRumble, durationMs);
        }

        public bool SetTriggerRumble(bool leftTrigger, bool rightTrigger, ushort strength, uint durationMs)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_SetTriggerRumble(Handle, leftTrigger, rightTrigger, strength, durationMs);
        }

        public StringView GetButtonName(ControllerButton button)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetButtonName(Handle, button);
        }

        public StringView GetAxisName(ControllerAxis axis)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetAxisName(Handle, axis);
        }

        public bool HasButton(ControllerButton button)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_HasButton(Handle, button);
        }

        public bool HasAxis(ControllerAxis axis)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_HasAxis(Handle, axis);
        }

        public StringView GetMapping()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetMapping(Handle);
        }

        public bool IsConnected()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_IsConnected(Handle);
        }

        public StringView GetName()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetName(Handle);
        }

        public uint GetInstanceID()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetInstanceID(Handle);
        }

        public ControllerDeviceInfo GetDeviceInfo()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetDeviceInfo(Handle);
        }

        public bool SetPlayerIndex(int playerIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_SetPlayerIndex(Handle, playerIndex);
        }

        public int GetPlayerIndex()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Controller_GetPlayerIndex(Handle);
        }

        public static Controller CreateWithIndex(int controllerIndex)
        {
            return new Controller(Methods.DenOfIz_Controller_CreateWithIndex(controllerIndex), ownsHandle: false);
        }

        public static void InitializeSDL()
        {
            Methods.DenOfIz_Controller_InitializeSDL();
        }

        public static Int32Array GetConnectedControllerIndices()
        {
            return Methods.DenOfIz_Controller_GetConnectedControllerIndices();
        }

        public static bool IsGameController(int joystickIndex)
        {
            return Methods.DenOfIz_Controller_IsGameController(joystickIndex);
        }

        public static int GetControllerCount()
        {
            return Methods.DenOfIz_Controller_GetControllerCount();
        }

        public static StringView GetControllerNameForIndex(int joystickIndex)
        {
            return Methods.DenOfIz_Controller_GetControllerNameForIndex(joystickIndex);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Controller_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Controller));
            }
        }
    }
}
