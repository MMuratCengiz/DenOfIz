using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FontAssetReader : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FontAssetReader wrapper) => wrapper?.Handle ?? 0;

        public FontAssetReader(in FontAssetReaderDesc desc)
        {
            Handle = Methods.DenOfIz_FontAssetReader_Create(in desc);
            _ownsHandle = true;
        }

        internal FontAssetReader(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public FontAsset Read()
        {
            ThrowIfDisposed();
            return new FontAsset(Methods.DenOfIz_FontAssetReader_Read(Handle), ownsHandle: false);
        }

        public static void LoadAtlasIntoGpuTexture(FontAsset? fontAsset, in LoadAtlasIntoGpuTextureDesc desc)
        {
            Methods.DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture(fontAsset, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FontAssetReader_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FontAssetReader));
            }
        }
    }
}
