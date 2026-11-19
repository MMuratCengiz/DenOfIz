// Manual wrapper - parent caches children
using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class SwapChain : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;
        private Texture?[]? _textures;

        public static implicit operator ulong(SwapChain wrapper) => wrapper?.Handle ?? 0;

        internal SwapChain(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public Format GetPreferredFormat()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_GetPreferredFormat(Handle, out var outFormat);
            return outFormat;
        }

        public Result AcquireNextImage(out Texture? backBuffer)
        {
            ThrowIfDisposed();
            var result = Methods.DenOfIz_SwapChain_AcquireNextImage(Handle, out var outBackBuffer);
            backBuffer = result == Result.Success ? WrapTexture(outBackBuffer) : null;
            return result;
        }

        public Result Present()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_SwapChain_Present(Handle);
        }

        public Texture? GetDepthBuffer()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_GetDepthBuffer(Handle, out var outDepthBuffer);
            return outDepthBuffer == 0 ? null : WrapTexture(outDepthBuffer);
        }

        public SwapChainDesc GetDesc()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_GetDesc(Handle, out var outDesc);
            return outDesc;
        }

        public Viewport GetViewport()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_GetViewport(Handle, out var outViewport);
            return Marshal.PtrToStructure<Viewport>(outViewport);
        }

        internal Texture WrapTexture(ulong handle)
        {
            _textures ??= new Texture?[8];
            var free = -1;
            for (var i = 0; i < _textures.Length; i++)
            {
                var texture = _textures[i];
                if (texture == null)
                {
                    free = free < 0 ? i : free;
                }
                else if (texture.Handle == handle)
                {
                    return texture;
                }
            }

            var wrapper = new Texture(handle, ownsHandle: false);
            _textures[free < 0 ? 0 : free] = wrapper;
            return wrapper;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_textures != null)
            {
                foreach (var texture in _textures)
                {
                    texture?.Invalidate();
                }
                _textures = null;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_SwapChain_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(SwapChain));
            }
        }
    }
}
