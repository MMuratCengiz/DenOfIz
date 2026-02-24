using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Window : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Window wrapper) => wrapper?.Handle ?? 0;

        public Window(in WindowDesc desc)
        {
            Handle = Methods.DenOfIz_Window_Create(in desc);
            _ownsHandle = true;
        }

        internal Window(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Show()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Show(Handle);
        }

        public void Hide()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Hide(Handle);
        }

        public void Minimize()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Minimize(Handle);
        }

        public void Maximize()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Maximize(Handle);
        }

        public void Raise()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Raise(Handle);
        }

        public void Restore()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Restore(Handle);
        }

        public GraphicsWindowHandle GetGraphicsWindowHandle()
        {
            ThrowIfDisposed();
            return new GraphicsWindowHandle(Methods.DenOfIz_Window_GetGraphicsWindowHandle(Handle), ownsHandle: false);
        }

        public uint GetWindowID()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetWindowID(Handle);
        }

        public DisplaySize GetSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetSize(Handle);
        }

        public DisplaySize GetSizeInPixels()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetSizeInPixels(Handle);
        }

        public void SetSize(int width, int height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetSize(Handle, width, height);
        }

        public float GetPixelDensity()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetPixelDensity(Handle);
        }

        public float GetDisplayScale()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetDisplayScale(Handle);
        }

        public StringView GetTitle()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetTitle(Handle);
        }

        public void SetTitle(StringView title)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetTitle(Handle, title);
        }

        public bool GetFullscreen()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetFullscreen(Handle);
        }

        public void SetFullscreen(bool fullscreen)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetFullscreen(Handle, fullscreen);
        }

        public void SetPosition(int x, int y)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetPosition(Handle, x, y);
        }

        public int GetPositionX()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetPositionX(Handle);
        }

        public int GetPositionY()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetPositionY(Handle);
        }

        public void SetResizable(bool resizable)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetResizable(Handle, resizable);
        }

        public void SetBordered(bool bordered)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetBordered(Handle, bordered);
        }

        public void SetMinimumSize(int minWidth, int minHeight)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetMinimumSize(Handle, minWidth, minHeight);
        }

        public void SetMaximumSize(int maxWidth, int maxHeight)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetMaximumSize(Handle, maxWidth, maxHeight);
        }

        public bool IsShown()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_IsShown(Handle);
        }

        public bool IsMinimized()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_IsMinimized(Handle);
        }

        public bool IsMaximized()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_IsMaximized(Handle);
        }

        public uint GetFlags()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetFlags(Handle);
        }

        public void Sync()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Sync(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Window_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Window));
            }
        }
    }
}
