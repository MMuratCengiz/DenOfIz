// Manual wrapper - parent caches children
using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class CommandListPool : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;
        private CommandList[]? _commandLists;

        public static implicit operator ulong(CommandListPool wrapper) => wrapper?.Handle ?? 0;

        internal CommandListPool(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ReadOnlySpan<CommandList> GetCommandLists()
        {
            ThrowIfDisposed();

            if (_commandLists != null)
            {
                return _commandLists;
            }

            Methods.DenOfIz_CommandListPool_GetCommandLists(Handle, out var array);

            _commandLists = new CommandList[(int)array.NumElements];
            unsafe
            {
                var elements = (ulong*)array.Elements.ToPointer();
                for (var i = 0; i < (int)array.NumElements; i++)
                {
                    _commandLists[i] = new CommandList(elements[i], ownsHandle: false);
                }
            }

            return _commandLists;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_commandLists != null)
            {
                foreach (var cmd in _commandLists)
                {
                    cmd.Invalidate();
                }
                _commandLists = null;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_CommandListPool_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(CommandListPool));
            }
        }
    }
}
