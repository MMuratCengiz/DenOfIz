using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Fence : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Fence wrapper) => wrapper?.Handle ?? 0;

        internal Fence(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        internal void Invalidate()
        {
            Handle = 0;
        }

        public void Wait()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Fence_Wait(Handle);
        }

        public void Reset()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Fence_Reset(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Fence_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Fence));
            }
        }
    }
}
