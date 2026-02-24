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

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureResource_Destroy(Handle);
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
