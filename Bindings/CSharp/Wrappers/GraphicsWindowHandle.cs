using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class GraphicsWindowHandle : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(GraphicsWindowHandle wrapper) => wrapper?.Handle ?? 0;

        public GraphicsWindowHandle()
        {
            Handle = Methods.DenOfIz_GraphicsWindowHandle_Create();
            _ownsHandle = true;
        }

        internal GraphicsWindowHandle(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void CreateFromSDLWindowId(uint windowId)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_GraphicsWindowHandle_CreateFromSDLWindowId(Handle, windowId);
        }

        public uint GetSDLWindowId()
        {
            ThrowIfDisposed();
            uint outWindowId = default;
            Methods.DenOfIz_GraphicsWindowHandle_GetSDLWindowId(Handle, out outWindowId);
            return outWindowId;
        }

        public GraphicsWindowSurface GetSurface()
        {
            ThrowIfDisposed();
            GraphicsWindowSurface outSurface = default;
            Methods.DenOfIz_GraphicsWindowHandle_GetSurface(Handle, out outSurface);
            return outSurface;
        }

        public bool IsValid()
        {
            ThrowIfDisposed();
            bool outIsValid = default;
            Methods.DenOfIz_GraphicsWindowHandle_IsValid(Handle, out outIsValid);
            return outIsValid;
        }

        public void SetFullScreenMode(FullScreenMode mode)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_GraphicsWindowHandle_SetFullScreenMode(Handle, mode);
        }

        public FullScreenMode GetFullScreenMode()
        {
            ThrowIfDisposed();
            FullScreenMode outMode = default;
            Methods.DenOfIz_GraphicsWindowHandle_GetFullScreenMode(Handle, out outMode);
            return outMode;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_GraphicsWindowHandle_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(GraphicsWindowHandle));
            }
        }
    }
}
