using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class GeometryData : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(GeometryData wrapper) => wrapper?.Handle ?? 0;

        internal GeometryData(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public uint GetVertexCount()
        {
            ThrowIfDisposed();
            uint outCount = default;
            Methods.DenOfIz_GeometryData_GetVertexCount(Handle, out outCount);
            return outCount;
        }

        public uint GetIndexCount()
        {
            ThrowIfDisposed();
            uint outCount = default;
            Methods.DenOfIz_GeometryData_GetIndexCount(Handle, out outCount);
            return outCount;
        }

        public void GetVertexData(byte[] outData)
        {
            ThrowIfDisposed();
            var __pin_outData = GCHandle.Alloc(outData, GCHandleType.Pinned);
            try
            {
                Methods.DenOfIz_GeometryData_GetVertexData(Handle, __pin_outData.AddrOfPinnedObject());
            }
            finally
            {
                __pin_outData.Free();
            }
        }

        public void GetIndexData(byte[] outData)
        {
            ThrowIfDisposed();
            var __pin_outData = GCHandle.Alloc(outData, GCHandleType.Pinned);
            try
            {
                Methods.DenOfIz_GeometryData_GetIndexData(Handle, __pin_outData.AddrOfPinnedObject());
            }
            finally
            {
                __pin_outData.Free();
            }
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_GeometryData_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(GeometryData));
            }
        }
    }
}
