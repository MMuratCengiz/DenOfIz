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
        private Texture?[]? _renderTargets;

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

        public uint AcquireNextImage()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_AcquireNextImage(Handle, out var outImageIndex);
            return outImageIndex;
        }

        public PresentResult Present(uint imageIndex)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_SwapChain_Present(Handle, imageIndex);
        }

        public void Resize(uint width, uint height)
        {
            ThrowIfDisposed();

            if (_renderTargets != null)
            {
                foreach (var rt in _renderTargets)
                {
                    rt?.Invalidate();
                }
                _renderTargets = null;
            }

            Methods.DenOfIz_SwapChain_Resize(Handle, width, height);
        }

        public Texture GetRenderTarget(uint image)
        {
            ThrowIfDisposed();

            _renderTargets ??= new Texture?[8];

            if (image < _renderTargets.Length && _renderTargets[image] != null)
            {
                return _renderTargets[image]!;
            }

            Methods.DenOfIz_SwapChain_GetRenderTarget(Handle, image, out var outRenderTarget);
            var texture = new Texture(outRenderTarget, ownsHandle: false);

            if (image < _renderTargets.Length)
            {
                _renderTargets[image] = texture;
            }

            return texture;
        }

        public Viewport GetViewport()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_SwapChain_GetViewport(Handle, out var outViewport);
            return Marshal.PtrToStructure<Viewport>(outViewport);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_renderTargets != null)
            {
                foreach (var rt in _renderTargets)
                {
                    rt?.Invalidate();
                }
                _renderTargets = null;
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
