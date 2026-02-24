using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextureAsset : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextureAsset wrapper) => wrapper?.Handle ?? 0;

        public TextureAsset()
        {
            Handle = Methods.DenOfIz_TextureAsset_Create();
            _ownsHandle = true;
        }

        internal TextureAsset(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ulong Magic()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Magic(Handle);
        }

        public uint Version()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Version(Handle);
        }

        public ulong NumBytes()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_NumBytes(Handle);
        }

        public StringView Path()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Path(Handle);
        }

        public StringView Name()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Name(Handle);
        }

        public StringView SourcePath()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_SourcePath(Handle);
        }

        public uint Width()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Width(Handle);
        }

        public uint Height()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Height(Handle);
        }

        public uint Depth()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Depth(Handle);
        }

        public Format GetFormat()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_GetFormat(Handle);
        }

        public TextureDimension GetDimension()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_GetDimension(Handle);
        }

        public uint MipLevels()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_MipLevels(Handle);
        }

        public uint ArraySize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_ArraySize(Handle);
        }

        public uint BitsPerPixel()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_BitsPerPixel(Handle);
        }

        public uint BlockSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_BlockSize(Handle);
        }

        public uint RowPitch()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_RowPitch(Handle);
        }

        public uint NumRows()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_NumRows(Handle);
        }

        public uint SlicePitch()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_SlicePitch(Handle);
        }

        public TextureMipArray Mips()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Mips(Handle);
        }

        public ulong NumMips()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_NumMips(Handle);
        }

        public AssetDataStream Data()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureAsset_Data(Handle);
        }

        public void SetVersion(uint version)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetVersion(Handle, version);
        }

        public void SetNumBytes(ulong numBytes)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetNumBytes(Handle, numBytes);
        }

        public void SetPath(StringView path)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetPath(Handle, path);
        }

        public void SetName(StringView name)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetName(Handle, name);
        }

        public void SetSourcePath(StringView sourcePath)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetSourcePath(Handle, sourcePath);
        }

        public void SetWidth(uint width)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetWidth(Handle, width);
        }

        public void SetHeight(uint height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetHeight(Handle, height);
        }

        public void SetDepth(uint depth)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetDepth(Handle, depth);
        }

        public void SetFormat(Format format)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetFormat(Handle, format);
        }

        public void SetDimension(TextureDimension dimension)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetDimension(Handle, dimension);
        }

        public void SetMipLevels(uint mipLevels)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetMipLevels(Handle, mipLevels);
        }

        public void SetArraySize(uint arraySize)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetArraySize(Handle, arraySize);
        }

        public void SetBitsPerPixel(uint bitsPerPixel)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetBitsPerPixel(Handle, bitsPerPixel);
        }

        public void SetBlockSize(uint blockSize)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetBlockSize(Handle, blockSize);
        }

        public void SetRowPitch(uint rowPitch)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetRowPitch(Handle, rowPitch);
        }

        public void SetNumRows(uint numRows)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetNumRows(Handle, numRows);
        }

        public void SetSlicePitch(uint slicePitch)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetSlicePitch(Handle, slicePitch);
        }

        public void SetData(AssetDataStream data)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetData(Handle, data);
        }

        public void AddMip(in TextureMip mip)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_AddMip(Handle, in mip);
        }

        public void SetMips(in TextureMip mips, ulong count)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_SetMips(Handle, in mips, count);
        }

        public void ReserveMips(ulong capacity)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_ReserveMips(Handle, capacity);
        }

        public void ClearMips()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextureAsset_ClearMips(Handle);
        }

        public static StringView Extension()
        {
            return Methods.DenOfIz_TextureAsset_Extension();
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureAsset_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextureAsset));
            }
        }
    }
}
