using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextureAssetWriter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextureAssetWriter wrapper) => wrapper?.Handle ?? 0;

        public TextureAssetWriter(in TextureAssetWriterDesc desc)
        {
            Handle = Methods.DenOfIz_TextureAssetWriter_Create(in desc);
            _ownsHandle = true;
        }

        internal TextureAssetWriter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Write(TextureAsset? textureAsset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAssetWriter_Write(Handle, textureAsset);
        }

        public void AddPixelData(in ByteArrayView bytes, uint mipIndex, uint arrayLayer)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAssetWriter_AddPixelData(Handle, in bytes, mipIndex, arrayLayer);
        }

        public void End()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAssetWriter_End(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureAssetWriter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextureAssetWriter));
            }
        }
    }
}
