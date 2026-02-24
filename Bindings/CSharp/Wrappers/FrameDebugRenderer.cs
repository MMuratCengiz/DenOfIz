using System;
using System.Numerics;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FrameDebugRenderer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FrameDebugRenderer wrapper) => wrapper?.Handle ?? 0;

        public FrameDebugRenderer(in FrameDebugRendererDesc desc)
        {
            Handle = Methods.DenOfIz_FrameDebugRenderer_Create(in desc);
            _ownsHandle = true;
        }

        internal FrameDebugRenderer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Render(CommandList? commandList)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_Render(Handle, commandList);
        }

        public void SetViewport(in Viewport viewport)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_SetViewport(Handle, in viewport);
        }

        public void SetProjectionMatrix(in Matrix4x4 projectionMatrix)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_SetProjectionMatrix(Handle, in projectionMatrix);
        }

        public void SetViewportSize(uint width, uint height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_SetViewportSize(Handle, width, height);
        }

        public void AddDebugLine(StringView text, in Vector4 color)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_AddDebugLine(Handle, text, in color);
        }

        public void ClearCustomDebugLines()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_ClearCustomDebugLines(Handle);
        }

        public void SetEnabled(bool enabled)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_SetEnabled(Handle, enabled);
        }

        public bool IsEnabled()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FrameDebugRenderer_IsEnabled(Handle);
        }

        public void ToggleVisibility()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameDebugRenderer_ToggleVisibility(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FrameDebugRenderer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FrameDebugRenderer));
            }
        }
    }
}
