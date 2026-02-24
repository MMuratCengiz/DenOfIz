using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TopLevelAS : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TopLevelAS wrapper) => wrapper?.Handle ?? 0;

        internal TopLevelAS(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void UpdateInstanceTransforms(in UpdateTransformsDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TopLevelAS_UpdateInstanceTransforms(Handle, in desc);
        }

        public ulong BuildNumBytes()
        {
            ThrowIfDisposed();
            ulong outNumBytes = default;
            Methods.DenOfIz_TopLevelAS_BuildNumBytes(Handle, out outNumBytes);
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
                Methods.DenOfIz_TopLevelAS_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TopLevelAS));
            }
        }
    }
}
