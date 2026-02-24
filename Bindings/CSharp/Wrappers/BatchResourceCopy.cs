using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BatchResourceCopy : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BatchResourceCopy wrapper) => wrapper?.Handle ?? 0;

        internal BatchResourceCopy(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Begin()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_Begin(Handle);
        }

        public void CopyToGPUBuffer(in CopyToGpuBufferDesc copyDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_CopyToGPUBuffer(Handle, in copyDesc);
        }

        public void CopyBufferRegion(in CopyBufferRegionDesc copyDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_CopyBufferRegion(Handle, in copyDesc);
        }

        public void CopyTextureRegion(in CopyTextureRegionDesc copyDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_CopyTextureRegion(Handle, in copyDesc);
        }

        public void CopyDataToTexture(in CopyDataToTextureDesc copyDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_CopyDataToTexture(Handle, in copyDesc);
        }

        public Texture CreateAndLoadTexture(StringView file)
        {
            ThrowIfDisposed();
            return new Texture(Methods.DenOfIz_BatchResourceCopy_CreateAndLoadTexture(Handle, file), ownsHandle: false);
        }

        public void LoadTexture(in LoadTextureDesc loadDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_LoadTexture(Handle, in loadDesc);
        }

        public void LoadTextureFromData(in LoadTextureFromDataDesc loadDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_LoadTextureFromData(Handle, in loadDesc);
        }

        public Buffer CreateUniformBuffer(in ByteArrayView data, uint numBytes)
        {
            ThrowIfDisposed();
            return new Buffer(Methods.DenOfIz_BatchResourceCopy_CreateUniformBuffer(Handle, in data, numBytes), ownsHandle: false);
        }

        public Buffer CreateGeometryVertexBuffer(GeometryData geometryData)
        {
            ThrowIfDisposed();
            ulong geometryHandle = geometryData.Handle;
            return new Buffer(Methods.DenOfIz_BatchResourceCopy_CreateGeometryVertexBuffer(Handle, in geometryHandle), ownsHandle: false);
        }

        public Buffer CreateGeometryIndexBuffer(GeometryData geometryData)
        {
            ThrowIfDisposed();
            ulong geometryHandle = geometryData.Handle;
            return new Buffer(Methods.DenOfIz_BatchResourceCopy_CreateGeometryIndexBuffer(Handle, in geometryHandle), ownsHandle: false);
        }

        public void Submit(Semaphore? notify)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BatchResourceCopy_Submit(Handle, notify);
        }

        public BatchResourceCopy(in BatchResourceCopyDesc desc)
        {
            ulong outBatchResourceCopy = 0;
            Methods.DenOfIz_BatchResourceCopy_Create(in desc, out outBatchResourceCopy);
            Handle = outBatchResourceCopy;
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
                Methods.DenOfIz_BatchResourceCopy_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BatchResourceCopy));
            }
        }
    }
}
