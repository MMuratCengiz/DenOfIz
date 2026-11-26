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
        private Texture?[]? _textures;

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

        public Texture? BackBuffer { get; private set; }
        public Texture? DepthBuffer { get; private set; }
        public CommandList? CommandList { get; private set; }

        public Result BeginFrame(out Frame frame)
        {
            ThrowIfDisposed();
            var result = Methods.DenOfIz_FrameSync_BeginFrame(Handle, out frame);
            if (result != Result.Success)
            {
                BackBuffer = null;
                DepthBuffer = null;
                CommandList = null;
                return result;
            }

            BackBuffer = WrapTexture(frame.BackBuffer);
            DepthBuffer = frame.DepthBuffer == 0 ? null : WrapTexture(frame.DepthBuffer);
            CommandList = GetCommandList(frame.Index);
            return result;
        }

        public Result EndFrame()
        {
            ThrowIfDisposed();
            var noSemaphores = new SemaphoreArray();
            return Methods.DenOfIz_FrameSync_EndFrame(Handle, in noSemaphores);
        }

        public Result EndFrame(in SemaphoreArray waitSemaphores)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FrameSync_EndFrame(Handle, in waitSemaphores);
        }

        private Texture WrapTexture(ulong handle)
        {
            _textures ??= new Texture?[8];
            var free = -1;
            for (var i = 0; i < _textures.Length; i++)
            {
                var texture = _textures[i];
                if (texture == null)
                {
                    free = free < 0 ? i : free;
                }
                else if (texture.Handle == handle)
                {
                    return texture;
                }
            }

            var wrapper = new Texture(handle, ownsHandle: false);
            _textures[free < 0 ? 0 : free] = wrapper;
            return wrapper;
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
