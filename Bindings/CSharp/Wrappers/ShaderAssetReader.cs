using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderAssetReader : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderAssetReader wrapper) => wrapper?.Handle ?? 0;

        public ShaderAssetReader(in ShaderAssetReaderDesc desc)
        {
            Handle = Methods.DenOfIz_ShaderAssetReader_Create(in desc);
            _ownsHandle = true;
        }

        internal ShaderAssetReader(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ShaderAsset Read()
        {
            ThrowIfDisposed();
            return new ShaderAsset(Methods.DenOfIz_ShaderAssetReader_Read(Handle), ownsHandle: false);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderAssetReader_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderAssetReader));
            }
        }
    }
}
