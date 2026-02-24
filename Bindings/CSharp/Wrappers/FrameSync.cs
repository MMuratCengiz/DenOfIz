// Manual wrapper - parent caches children
using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FrameSync : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;
        private Fence?[]? _frameFences;
        private CommandList?[]? _commandLists;

        public static implicit operator ulong(FrameSync wrapper) => wrapper?.Handle ?? 0;

        public FrameSync(in FrameSyncDesc desc)
        {
            Methods.DenOfIz_FrameSync_Create(in desc, out var outFrameSync);
            Handle = outFrameSync;
            _ownsHandle = true;
        }

        internal FrameSync(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public uint NextFrame()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_NextFrame(Handle, out var outFrameIndex);
            return outFrameIndex;
        }

        public Fence GetFrameFence(uint frame)
        {
            ThrowIfDisposed();

            _frameFences ??= new Fence?[8];

            if (frame < _frameFences.Length && _frameFences[frame] != null)
            {
                return _frameFences[frame]!;
            }

            Methods.DenOfIz_FrameSync_GetFrameFence(Handle, frame, out var outFence);
            var fence = new Fence(outFence, ownsHandle: false);

            if (frame < _frameFences.Length)
            {
                _frameFences[frame] = fence;
            }

            return fence;
        }

        public CommandList GetCommandList(uint frame)
        {
            ThrowIfDisposed();

            _commandLists ??= new CommandList?[8];

            if (frame < _commandLists.Length && _commandLists[frame] != null)
            {
                return _commandLists[frame]!;
            }

            Methods.DenOfIz_FrameSync_GetCommandList(Handle, frame, out var outCommandList);
            var commandList = new CommandList(outCommandList, ownsHandle: false);

            if (frame < _commandLists.Length)
            {
                _commandLists[frame] = commandList;
            }

            return commandList;
        }

        public void ExecuteCommandList(uint frame, in SemaphoreArray additionalSemaphores)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_ExecuteCommandList(Handle, frame, in additionalSemaphores);
        }

        public void ExecuteCommandListWithSemaphores(uint frame, in SemaphoreArray waitSemaphores, in SemaphoreArray signalSemaphores)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_ExecuteCommandListWithSemaphores(Handle, frame, in waitSemaphores, in signalSemaphores);
        }

        public uint AcquireNextImage()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_AcquireNextImage(Handle, out var outImageIndex);
            return outImageIndex;
        }

        public PresentResult Present(uint imageIndex)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_Present(Handle, imageIndex, out var outResult);
            return outResult;
        }

        public void WaitIdle()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FrameSync_WaitIdle(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_frameFences != null)
            {
                foreach (var fence in _frameFences)
                {
                    fence?.Invalidate();
                }
                _frameFences = null;
            }

            if (_commandLists != null)
            {
                foreach (var cmd in _commandLists)
                {
                    cmd?.Invalidate();
                }
                _commandLists = null;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FrameSync_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FrameSync));
            }
        }
    }
}
