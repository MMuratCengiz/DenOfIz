using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextureData : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextureData? wrapper) => wrapper?.Handle ?? 0;

        internal TextureData(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public static TextureData CreateFromPath(in TextureCreateFromPathDesc desc)
        {
            return new TextureData(Methods.DenOfIz_TextureData_CreateFromPath(in desc), ownsHandle: true);
        }

        public static TextureData CreateFromData(in TextureCreateFromDataDesc desc)
        {
            return new TextureData(Methods.DenOfIz_TextureData_CreateFromData(in desc), ownsHandle: true);
        }

        public static TextureExtension IdentifyTextureFormat(in ByteArrayView data)
        {
            return Methods.DenOfIz_TextureData_IdentifyTextureFormat(in data);
        }

        public TextureMipArray ReadMipData()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_ReadMipData(Handle);
        }

        public uint GetWidth()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetWidth(Handle);
        }

        public uint GetHeight()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetHeight(Handle);
        }

        public uint GetDepth()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetDepth(Handle);
        }

        public uint GetMipLevels()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetMipLevels(Handle);
        }

        public uint GetArraySize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetArraySize(Handle);
        }

        public uint GetBitsPerPixel()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetBitsPerPixel(Handle);
        }

        public uint GetBlockSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetBlockSize(Handle);
        }

        public uint GetRowPitch()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetRowPitch(Handle);
        }

        public uint GetNumRows()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetNumRows(Handle);
        }

        public uint GetSlicePitch()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetSlicePitch(Handle);
        }

        public Format GetFormat()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetFormat(Handle);
        }

        public TextureDimension GetDimension()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetDimension(Handle);
        }

        public TextureExtension GetExtension()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetExtension(Handle);
        }

        public ByteArrayView GetData()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureData_GetData(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureData_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextureData));
            }
        }
    }
}
