using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BottomLevelAS : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BottomLevelAS wrapper) => wrapper?.Handle ?? 0;

        internal BottomLevelAS(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ulong BuildNumBytes()
        {
            ThrowIfDisposed();
            ulong outNumBytes = default;
            Methods.DenOfIz_BottomLevelAS_BuildNumBytes(Handle, out outNumBytes);
            return outNumBytes;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_BottomLevelAS_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BottomLevelAS));
            }
        }
    }
}
