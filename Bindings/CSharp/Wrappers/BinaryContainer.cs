using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BinaryContainer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BinaryContainer wrapper) => wrapper?.Handle ?? 0;

        public BinaryContainer()
        {
            Handle = Methods.DenOfIz_BinaryContainer_Create();
            _ownsHandle = true;
        }

        internal BinaryContainer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ByteArrayView GetData()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryContainer_GetData(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_BinaryContainer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BinaryContainer));
            }
        }
    }
}
