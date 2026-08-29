using System;
using System.Numerics;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BinaryReader : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BinaryReader wrapper) => wrapper?.Handle ?? 0;

        internal BinaryReader(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public int Read(ByteArray buffer, uint offset, uint count)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_Read(Handle, buffer, offset, count);
        }

        public ByteArray ReadAllBytes()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadAllBytes(Handle);
        }

        public ByteArray ReadBytes(uint count)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadBytes(Handle, count);
        }

        public ushort ReadUInt16()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt16(Handle);
        }

        public uint ReadUInt32()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt32(Handle);
        }

        public ulong ReadUInt64()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt64(Handle);
        }

        public short ReadInt16()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt16(Handle);
        }

        public int ReadInt32()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt32(Handle);
        }

        public long ReadInt64()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt64(Handle);
        }

        public float ReadFloat()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadFloat(Handle);
        }

        public double ReadDouble()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadDouble(Handle);
        }

        public StringView ReadString()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadString(Handle);
        }

        public UShort2 ReadUInt16_2()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt16_2(Handle);
        }

        public UShort3 ReadUInt16_3()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt16_3(Handle);
        }

        public UShort4 ReadUInt16_4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt16_4(Handle);
        }

        public Short2 ReadInt16_2()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt16_2(Handle);
        }

        public Short3 ReadInt16_3()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt16_3(Handle);
        }

        public Short4 ReadInt16_4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt16_4(Handle);
        }

        public UInt2 ReadUInt32_2()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt32_2(Handle);
        }

        public UInt3 ReadUInt32_3()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt32_3(Handle);
        }

        public UInt4 ReadUInt32_4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadUInt32_4(Handle);
        }

        public Int2 ReadInt32_2()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt32_2(Handle);
        }

        public Int3 ReadInt32_3()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt32_3(Handle);
        }

        public Int4 ReadInt32_4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadInt32_4(Handle);
        }

        public Vector2 ReadFloat_2()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadFloat_2(Handle);
        }

        public Vector3 ReadFloat_3()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadFloat_3(Handle);
        }

        public Vector4 ReadFloat_4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadFloat_4(Handle);
        }

        public Matrix4x4 ReadFloat_4x4()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_ReadFloat_4x4(Handle);
        }

        public ulong Position()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_BinaryReader_Position(Handle);
        }

        public void Seek(ulong position)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryReader_Seek(Handle, position);
        }

        public void Skip(ulong count)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BinaryReader_Skip(Handle, count);
        }

        public static BinaryReader CreateFromContainer(BinaryContainer? container, in BinaryReaderDesc desc)
        {
            return new BinaryReader(Methods.DenOfIz_BinaryReader_CreateFromContainer(container, in desc), ownsHandle: true);
        }

        public static BinaryReader CreateFromFile(StringView filePath, in BinaryReaderDesc desc)
        {
            return new BinaryReader(Methods.DenOfIz_BinaryReader_CreateFromFile(filePath, in desc), ownsHandle: true);
        }

        public static BinaryReader CreateFromData(ByteArrayView data, in BinaryReaderDesc desc)
        {
            return new BinaryReader(Methods.DenOfIz_BinaryReader_CreateFromData(data, in desc), ownsHandle: true);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_BinaryReader_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BinaryReader));
            }
        }
    }
}
