using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FontAssetWriter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FontAssetWriter wrapper) => wrapper?.Handle ?? 0;

        public FontAssetWriter(in FontAssetWriterDesc desc)
        {
            Handle = Methods.DenOfIz_FontAssetWriter_Create(in desc);
            _ownsHandle = true;
        }

        internal FontAssetWriter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Write(FontAsset? fontAsset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAssetWriter_Write(Handle, fontAsset);
        }

        public void End()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAssetWriter_End(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FontAssetWriter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FontAssetWriter));
            }
        }
    }
}
