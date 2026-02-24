using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FontLibrary : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FontLibrary wrapper) => wrapper?.Handle ?? 0;

        public FontLibrary()
        {
            Handle = Methods.DenOfIz_FontLibrary_Create();
            _ownsHandle = true;
        }

        internal FontLibrary(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public Font LoadFontFromDesc(in FontDesc desc)
        {
            ThrowIfDisposed();
            return new Font(Methods.DenOfIz_FontLibrary_LoadFontFromDesc(Handle, in desc), ownsHandle: false);
        }

        public Font LoadFontFromPath(StringView ttfPath)
        {
            ThrowIfDisposed();
            return new Font(Methods.DenOfIz_FontLibrary_LoadFontFromPath(Handle, ttfPath), ownsHandle: false);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FontLibrary_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FontLibrary));
            }
        }
    }
}
