using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class PipelineCache : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(PipelineCache wrapper) => wrapper?.Handle ?? 0;

        internal PipelineCache(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ulong GetDataNumBytes()
        {
            ThrowIfDisposed();
            ulong outNumBytes = default;
            Methods.DenOfIz_PipelineCache_GetDataNumBytes(Handle, out outNumBytes);
            return outNumBytes;
        }

        public void GetData(ref ByteArray data)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_PipelineCache_GetData(Handle, ref data);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_PipelineCache_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(PipelineCache));
            }
        }
    }
}
