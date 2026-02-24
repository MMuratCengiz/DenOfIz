using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class CommandQueue : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(CommandQueue wrapper) => wrapper?.Handle ?? 0;

        internal CommandQueue(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void WaitIdle()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandQueue_WaitIdle(Handle);
        }

        public void ExecuteCommandLists(in ExecuteCommandListsDesc executeCommandListsDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in executeCommandListsDesc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandList(CommandList commandList, Fence? signalFence = null)
        {
            ThrowIfDisposed();
            ulong handle = commandList.Handle;
            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)(&handle);
            desc.CommandLists.NumElements = 1;
            desc.WaitSemaphores = default;
            desc.SignalSemaphores = default;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandLists(CommandList cmd0, CommandList cmd1, Fence? signalFence = null)
        {
            ThrowIfDisposed();
            ulong* handles = stackalloc ulong[2];
            handles[0] = cmd0.Handle;
            handles[1] = cmd1.Handle;
            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)handles;
            desc.CommandLists.NumElements = 2;
            desc.WaitSemaphores = default;
            desc.SignalSemaphores = default;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandLists(CommandList cmd0, CommandList cmd1, CommandList cmd2, Fence? signalFence = null)
        {
            ThrowIfDisposed();
            ulong* handles = stackalloc ulong[3];
            handles[0] = cmd0.Handle;
            handles[1] = cmd1.Handle;
            handles[2] = cmd2.Handle;
            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)handles;
            desc.CommandLists.NumElements = 3;
            desc.WaitSemaphores = default;
            desc.SignalSemaphores = default;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandLists(CommandList cmd0, CommandList cmd1, CommandList cmd2, CommandList cmd3, Fence? signalFence = null)
        {
            ThrowIfDisposed();
            ulong* handles = stackalloc ulong[4];
            handles[0] = cmd0.Handle;
            handles[1] = cmd1.Handle;
            handles[2] = cmd2.Handle;
            handles[3] = cmd3.Handle;
            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)handles;
            desc.CommandLists.NumElements = 4;
            desc.WaitSemaphores = default;
            desc.SignalSemaphores = default;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandLists(ReadOnlySpan<CommandList> commandLists, Fence? signalFence = null)
        {
            ThrowIfDisposed();
            int count = commandLists.Length;
            ulong* handles = stackalloc ulong[count];
            for (int i = 0; i < count; i++)
            {
                handles[i] = commandLists[i].Handle;
            }

            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)handles;
            desc.CommandLists.NumElements = (ulong)count;
            desc.WaitSemaphores = default;
            desc.SignalSemaphores = default;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void ExecuteCommandLists(
            ReadOnlySpan<CommandList> commandLists,
            Fence? signalFence,
            ReadOnlySpan<Semaphore> waitSemaphores,
            ReadOnlySpan<Semaphore> signalSemaphores)
        {
            ThrowIfDisposed();
            int cmdCount = commandLists.Length;
            int waitCount = waitSemaphores.Length;
            int signalCount = signalSemaphores.Length;

            ulong* cmdHandles = stackalloc ulong[cmdCount];
            ulong* waitHandles = stackalloc ulong[waitCount];
            ulong* signalHandles = stackalloc ulong[signalCount];

            for (int i = 0; i < cmdCount; i++)
            {
                cmdHandles[i] = commandLists[i].Handle;
            }
            for (int i = 0; i < waitCount; i++)
            {
                waitHandles[i] = waitSemaphores[i].Handle;
            }
            for (int i = 0; i < signalCount; i++)
            {
                signalHandles[i] = signalSemaphores[i].Handle;
            }

            ExecuteCommandListsDesc desc;
            desc.Signal = signalFence?.Handle ?? 0;
            desc.CommandLists.Elements = (IntPtr)cmdHandles;
            desc.CommandLists.NumElements = (ulong)cmdCount;
            desc.WaitSemaphores.Elements = (IntPtr)waitHandles;
            desc.WaitSemaphores.NumElements = (ulong)waitCount;
            desc.SignalSemaphores.Elements = (IntPtr)signalHandles;
            desc.SignalSemaphores.NumElements = (ulong)signalCount;
            Methods.DenOfIz_CommandQueue_ExecuteCommandLists(Handle, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_CommandQueue_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(CommandQueue));
            }
        }
    }
}
