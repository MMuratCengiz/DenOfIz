using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ResourceTracking : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ResourceTracking wrapper) => wrapper?.Handle ?? 0;

        internal ResourceTracking(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void TrackBuffer(Buffer? buffer, QueueType queueType)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_TrackBuffer(Handle, buffer, queueType);
        }

        public void TrackTexture(Texture? texture, QueueType queueType)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_TrackTexture(Handle, texture, queueType);
        }

        public void UntrackBuffer(Buffer? buffer)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_UntrackBuffer(Handle, buffer);
        }

        public void UntrackTexture(Texture? texture)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_UntrackTexture(Handle, texture);
        }

        public void TransitionBuffer(CommandList? commandList, Buffer? buffer, uint newUsage, QueueType queueType)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_TransitionBuffer(Handle, commandList, buffer, newUsage, queueType);
        }

        public void TransitionTexture(CommandList? commandList, Texture? texture, uint newUsage, QueueType queueType)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_TransitionTexture(Handle, commandList, texture, newUsage, queueType);
        }

        public void BatchTransition(CommandList? commandList, in BatchTransitionDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_BatchTransition(Handle, commandList, in desc);
        }

        public void NotifyTextureTransition(Texture? texture, uint newUsage)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_NotifyTextureTransition(Handle, texture, newUsage);
        }

        public void NotifyBufferTransition(Buffer? buffer, uint newUsage)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ResourceTracking_NotifyBufferTransition(Handle, buffer, newUsage);
        }

        public ResourceTracking()
        {
            ulong outTracking = 0;
            Methods.DenOfIz_ResourceTracking_Create(out outTracking);
            Handle = outTracking;
            _ownsHandle = true;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ResourceTracking_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ResourceTracking));
            }
        }
    }
}
