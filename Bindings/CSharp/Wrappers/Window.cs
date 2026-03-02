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

        public static Window CreatePopup(in PopupWindowDesc desc)
        {
            ulong handle = Methods.DenOfIz_Window_CreatePopup(in desc);
            return new Window(handle, ownsHandle: true);
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

        public void SetFullscreenMode(in DisplayMode mode)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetFullscreenMode(Handle, in mode);
        }

        public DisplayMode GetFullscreenMode()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetFullscreenMode(Handle);
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

        public DisplaySize GetMinimumSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetMinimumSize(Handle);
        }

        public void SetMaximumSize(int maxWidth, int maxHeight)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetMaximumSize(Handle, maxWidth, maxHeight);
        }

        public DisplaySize GetMaximumSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetMaximumSize(Handle);
        }

        public void SetAspectRatio(float minAspect, float maxAspect)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetAspectRatio(Handle, minAspect, maxAspect);
        }

        public AspectRatio GetAspectRatio()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetAspectRatio(Handle);
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

        public void SetIcon(in WindowIconDesc icon)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetIcon(Handle, in icon);
        }

        public void Flash(FlashOperation operation)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_Flash(Handle, operation);
        }

        public void SetOpacity(float opacity)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetOpacity(Handle, opacity);
        }

        public float GetOpacity()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetOpacity(Handle);
        }


        public void SetAlwaysOnTop(bool onTop)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetAlwaysOnTop(Handle, onTop);
        }

        public void SetFocusable(bool focusable)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetFocusable(Handle, focusable);
        }

        public void SetModal(bool modal)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetModal(Handle, modal);
        }


        public Window GetParent()
        {
            ThrowIfDisposed();
            ulong parentHandle = Methods.DenOfIz_Window_GetParent(Handle);
            if (parentHandle == 0)
            {
                return null;
            }
            return new Window(parentHandle, ownsHandle: false);
        }

        public void SetParent(Window parent)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetParent(Handle, parent?.Handle ?? 0);
        }


        public void SetMouseGrab(bool grabbed)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetMouseGrab(Handle, grabbed);
        }

        public bool GetMouseGrab()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetMouseGrab(Handle);
        }

        public void SetKeyboardGrab(bool grabbed)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetKeyboardGrab(Handle, grabbed);
        }

        public bool GetKeyboardGrab()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetKeyboardGrab(Handle);
        }

        public void SetMouseRect(in Rect rect)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetMouseRect(Handle, in rect);
        }

        public Rect GetMouseRect()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetMouseRect(Handle);
        }

        public void WarpMouse(float x, float y)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_WarpMouse(Handle, x, y);
        }


        public WindowBordersSize GetBordersSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetBordersSize(Handle);
        }

        public Rect GetSafeArea()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetSafeArea(Handle);
        }


        public void SetProgressState(ProgressState state)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetProgressState(Handle, state);
        }

        public ProgressState GetProgressState()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetProgressState(Handle);
        }

        public void SetProgressValue(float value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_SetProgressValue(Handle, value);
        }

        public float GetProgressValue()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetProgressValue(Handle);
        }


        public void ShowSystemMenu(int x, int y)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Window_ShowSystemMenu(Handle, x, y);
        }

        public uint GetDisplayID()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetDisplayID(Handle);
        }

        public uint GetPixelFormat()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Window_GetPixelFormat(Handle);
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
