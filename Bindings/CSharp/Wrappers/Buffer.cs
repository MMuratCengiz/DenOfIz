using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Buffer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Buffer wrapper) => wrapper?.Handle ?? 0;

        internal Buffer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public IntPtr MapMemory()
        {
            ThrowIfDisposed();
            IntPtr outMappedMemory = default;
            Methods.DenOfIz_Buffer_MapMemory(Handle, out outMappedMemory);
            return outMappedMemory;
        }

        public void UnmapMemory()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Buffer_UnmapMemory(Handle);
        }

        public ulong NumBytes()
        {
            ThrowIfDisposed();
            ulong outNumBytes = default;
            Methods.DenOfIz_Buffer_NumBytes(Handle, out outNumBytes);
            return outNumBytes;
        }

        public IntPtr Data()
        {
            ThrowIfDisposed();
            IntPtr outData = default;
            Methods.DenOfIz_Buffer_Data(Handle, out outData);
            return outData;
        }

        public ByteArray GetData()
        {
            ThrowIfDisposed();
            ByteArray outData = default;
            Methods.DenOfIz_Buffer_GetData(Handle, out outData);
            return outData;
        }

        public void SetData(in ByteArrayView data, bool keepMapped)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Buffer_SetData(Handle, in data, keepMapped);
        }

        public unsafe void SetData(ReadOnlySpan<byte> data, bool keepMapped = false)
        {
            ThrowIfDisposed();
            fixed (byte* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)data.Length;
                Methods.DenOfIz_Buffer_SetData(Handle, in view, keepMapped);
            }
        }

        public unsafe void SetData<T>(in T data, bool keepMapped = false) where T : unmanaged
        {
            ThrowIfDisposed();
            fixed (T* ptr = &data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)sizeof(T);
                Methods.DenOfIz_Buffer_SetData(Handle, in view, keepMapped);
            }
        }

        public unsafe void SetData<T>(ReadOnlySpan<T> data, bool keepMapped = false) where T : unmanaged
        {
            ThrowIfDisposed();
            fixed (T* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)(data.Length * sizeof(T));
                Methods.DenOfIz_Buffer_SetData(Handle, in view, keepMapped);
            }
        }

        public void WriteData(in ByteArrayView data, uint bufferOffset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Buffer_WriteData(Handle, in data, bufferOffset);
        }

        public unsafe void WriteData(ReadOnlySpan<byte> data, uint bufferOffset = 0)
        {
            ThrowIfDisposed();
            fixed (byte* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)data.Length;
                Methods.DenOfIz_Buffer_WriteData(Handle, in view, bufferOffset);
            }
        }

        public unsafe void WriteData<T>(in T data, uint bufferOffset = 0) where T : unmanaged
        {
            ThrowIfDisposed();
            fixed (T* ptr = &data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)sizeof(T);
                Methods.DenOfIz_Buffer_WriteData(Handle, in view, bufferOffset);
            }
        }

        public unsafe void WriteData<T>(ReadOnlySpan<T> data, uint bufferOffset = 0) where T : unmanaged
        {
            ThrowIfDisposed();
            fixed (T* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)(data.Length * sizeof(T));
                Methods.DenOfIz_Buffer_WriteData(Handle, in view, bufferOffset);
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
                Methods.DenOfIz_Buffer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Buffer));
            }
        }
    }
}
