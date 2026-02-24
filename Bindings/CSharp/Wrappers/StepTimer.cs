using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class StepTimer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(StepTimer wrapper) => wrapper?.Handle ?? 0;

        public StepTimer()
        {
            Handle = Methods.DenOfIz_StepTimer_Create();
            _ownsHandle = true;
        }

        internal StepTimer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public double GetDeltaTime()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetDeltaTime(Handle);
        }

        public ulong GetElapsedTicks()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetElapsedTicks(Handle);
        }

        public double GetElapsedSeconds()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetElapsedSeconds(Handle);
        }

        public ulong GetTotalTicks()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetTotalTicks(Handle);
        }

        public double GetTotalSeconds()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetTotalSeconds(Handle);
        }

        public uint GetFrameCount()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetFrameCount(Handle);
        }

        public uint GetFramesPerSecond()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_StepTimer_GetFramesPerSecond(Handle);
        }

        public void SetFixedTimeStep(bool isFixedTimestep)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_StepTimer_SetFixedTimeStep(Handle, isFixedTimestep);
        }

        public void SetTargetElapsedTicks(ulong targetElapsed)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_StepTimer_SetTargetElapsedTicks(Handle, targetElapsed);
        }

        public void SetTargetElapsedSeconds(double targetElapsed)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_StepTimer_SetTargetElapsedSeconds(Handle, targetElapsed);
        }

        public void ResetElapsedTime()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_StepTimer_ResetElapsedTime(Handle);
        }

        public void Tick()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_StepTimer_Tick(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_StepTimer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(StepTimer));
            }
        }
    }
}
