using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FontAsset : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FontAsset wrapper) => wrapper?.Handle ?? 0;

        public FontAsset()
        {
            Handle = Methods.DenOfIz_FontAsset_Create();
            _ownsHandle = true;
        }

        internal FontAsset(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ulong Magic()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_Magic(Handle);
        }

        public uint Version()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_Version(Handle);
        }

        public ulong NumBytes()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_NumBytes(Handle);
        }

        public StringView Path()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_Path(Handle);
        }

        public ByteArray Data()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_Data(Handle);
        }

        public uint InitialFontSize()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_InitialFontSize(Handle);
        }

        public AntiAliasingMode GetAntiAliasingMode()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_GetAntiAliasingMode(Handle);
        }

        public uint AtlasWidth()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_AtlasWidth(Handle);
        }

        public uint AtlasHeight()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_AtlasHeight(Handle);
        }

        public FontMetrics GetMetrics()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_GetMetrics(Handle);
        }

        public FontGlyphArray Glyphs()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_Glyphs(Handle);
        }

        public ulong NumGlyphs()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_NumGlyphs(Handle);
        }

        public ByteArray AtlasData()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontAsset_AtlasData(Handle);
        }

        public void SetVersion(uint version)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetVersion(Handle, version);
        }

        public void SetNumBytes(ulong numBytes)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetNumBytes(Handle, numBytes);
        }

        public void SetPath(StringView path)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetPath(Handle, path);
        }

        public void SetData(ref byte data, ulong numBytes)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetData(Handle, ref data, numBytes);
        }

        public void SetInitialFontSize(uint fontSize)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetInitialFontSize(Handle, fontSize);
        }

        public void SetAntiAliasingMode(AntiAliasingMode mode)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetAntiAliasingMode(Handle, mode);
        }

        public void SetAtlasWidth(uint width)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetAtlasWidth(Handle, width);
        }

        public void SetAtlasHeight(uint height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetAtlasHeight(Handle, height);
        }

        public void SetMetrics(in FontMetrics metrics)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetMetrics(Handle, in metrics);
        }

        public void SetAtlasData(ref byte data, ulong numBytes)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetAtlasData(Handle, ref data, numBytes);
        }

        public void AddGlyph(in FontGlyph glyph)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_AddGlyph(Handle, in glyph);
        }

        public void SetGlyphs(in FontGlyph glyphs, ulong count)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_SetGlyphs(Handle, in glyphs, count);
        }

        public void ReserveGlyphs(ulong capacity)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_ReserveGlyphs(Handle, capacity);
        }

        public void ClearGlyphs()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_FontAsset_ClearGlyphs(Handle);
        }

        public static StringView Extension()
        {
            return Methods.DenOfIz_FontAsset_Extension();
        }

        public static FontAsset GetJetbrainsMono()
        {
            return new FontAsset(Methods.DenOfIz_FontAsset_GetJetbrainsMono(), ownsHandle: false);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FontAsset_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FontAsset));
            }
        }
    }
}
