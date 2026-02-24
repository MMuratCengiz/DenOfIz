using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Semaphore : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Semaphore wrapper) => wrapper?.Handle ?? 0;

        internal Semaphore(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Notify()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Semaphore_Notify(Handle);
        }

        public bool IsCompleted()
        {
            ThrowIfDisposed();
            bool outCompleted = default;
            Methods.DenOfIz_Semaphore_IsCompleted(Handle, out outCompleted);
            return outCompleted;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Semaphore_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Semaphore));
            }
        }
    }
}
