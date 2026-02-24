using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextureAssetReader : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextureAssetReader wrapper) => wrapper?.Handle ?? 0;

        public TextureAssetReader(in TextureAssetReaderDesc desc)
        {
            Handle = Methods.DenOfIz_TextureAssetReader_Create(in desc);
            _ownsHandle = true;
        }

        internal TextureAssetReader(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public TextureAsset Read()
        {
            ThrowIfDisposed();
            return new TextureAsset(Methods.DenOfIz_TextureAssetReader_Read(Handle), ownsHandle: false);
        }

        public void LoadIntoGpuTexture(in LoadIntoGpuTextureDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAssetReader_LoadIntoGpuTexture(Handle, in desc);
        }

        public ByteArray ReadRaw(uint mipLevel, uint arrayLayer)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAssetReader_ReadRaw(Handle, mipLevel, arrayLayer);
        }

        public ulong AlignedTotalNumBytes(in DeviceConstants constants)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAssetReader_AlignedTotalNumBytes(Handle, in constants);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureAssetReader_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextureAssetReader));
            }
        }
    }
}
