using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Texture : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Texture? wrapper) => wrapper?.Handle ?? 0;

        internal Texture(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        internal void Invalidate()
        {
            Handle = 0;
        }

        public Format GetFormat()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Texture_GetFormat(Handle, out var outFormat);
            return outFormat;
        }

        public TextureDesc GetDesc()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Texture_GetDesc(Handle, out var outDesc);
            return outDesc;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Texture_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Texture));
            }
        }
    }
}
