using System;
using System.Numerics;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BinaryWriter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BinaryWriter wrapper) => wrapper?.Handle ?? 0;

        internal BinaryWriter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void WriteByte(byte value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteByte(Handle, value);
        }

        public void Write(ByteArrayView buffer, uint offset, uint count)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_Write(Handle, buffer, offset, count);
        }

        public void WriteBytes(ByteArrayView buffer)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteBytes(Handle, buffer);
        }

        public void WriteUInt16(ushort value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt16(Handle, value);
        }

        public void WriteUInt32(uint value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt32(Handle, value);
        }

        public void WriteUInt64(ulong value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt64(Handle, value);
        }

        public void WriteInt16(short value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt16(Handle, value);
        }

        public void WriteInt32(int value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt32(Handle, value);
        }

        public void WriteInt64(long value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt64(Handle, value);
        }

        public void WriteFloat(float value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteFloat(Handle, value);
        }

        public void WriteDouble(double value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteDouble(Handle, value);
        }

        public void WriteString(StringView value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteString(Handle, value);
        }

        public void WriteUInt16_2(UShort2 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt16_2(Handle, value);
        }

        public void WriteUInt16_3(UShort3 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt16_3(Handle, value);
        }

        public void WriteUInt16_4(UShort4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt16_4(Handle, value);
        }

        public void WriteInt16_2(Short2 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt16_2(Handle, value);
        }

        public void WriteInt16_3(Short3 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt16_3(Handle, value);
        }

        public void WriteInt16_4(Short4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt16_4(Handle, value);
        }

        public void WriteUInt32_2(UInt2 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt32_2(Handle, value);
        }

        public void WriteUInt32_3(UInt3 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt32_3(Handle, value);
        }

        public void WriteUInt32_4(UInt4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteUInt32_4(Handle, value);
        }

        public void WriteInt32_2(Int2 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt32_2(Handle, value);
        }

        public void WriteInt32_3(Int3 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt32_3(Handle, value);
        }

        public void WriteInt32_4(Int4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteInt32_4(Handle, value);
        }

        public void WriteFloat_2(Vector2 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteFloat_2(Handle, value);
        }

        public void WriteFloat_3(Vector3 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteFloat_3(Handle, value);
        }

        public void WriteFloat_4(Vector4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteFloat_4(Handle, value);
        }

        public void WriteFloat_4x4(Matrix4x4 value)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_WriteFloat_4x4(Handle, value);
        }

        public ulong Position()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryWriter_Position(Handle);
        }

        public void Seek(ulong position)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_Seek(Handle, position);
        }

        public void Flush()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryWriter_Flush(Handle);
        }

        public static BinaryWriter CreateFromContainer(BinaryContainer? container)
        {
            return new BinaryWriter(Methods.DenOfIz_BinaryWriter_CreateFromContainer(container), ownsHandle: false);
        }

        public static BinaryWriter CreateFromFile(StringView filePath)
        {
            return new BinaryWriter(Methods.DenOfIz_BinaryWriter_CreateFromFile(filePath), ownsHandle: false);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_BinaryWriter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BinaryWriter));
            }
        }
    }
}
