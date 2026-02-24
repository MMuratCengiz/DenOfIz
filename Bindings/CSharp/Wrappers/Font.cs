using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Font : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Font wrapper) => wrapper?.Handle ?? 0;

        internal Font(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public FontAsset Asset()
        {
            ThrowIfDisposed();
            return new FontAsset(Methods.DenOfIz_Font_Asset(Handle), ownsHandle: false);
        }

        public FontGlyph GetGlyph(uint codePoint)
        {
            ThrowIfDisposed();
            return Marshal.PtrToStructure<FontGlyph>(Methods.DenOfIz_Font_GetGlyph(Handle, codePoint));
        }

        public static float MsdfPixelRange()
        {
            return Methods.DenOfIz_Font_MsdfPixelRange();
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Font));
            }
        }
    }
}
