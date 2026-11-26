using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Font : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(Font wrapper) => wrapper?.Handle ?? 0;

        public Font(in FontDesc desc)
        {
            Handle = Methods.DenOfIz_Font_Create(in desc);
            _ownsHandle = true;
        }

        internal Font(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public static unsafe Font Create(ReadOnlySpan<byte> data, uint faceIndex = 0)
        {
            fixed (byte* ptr = data)
            {
                var desc = new FontDesc
                {
                    Data = new ByteArrayView { Elements = (IntPtr)ptr, NumElements = (ulong)data.Length },
                    FaceIndex = faceIndex
                };
                return new Font(in desc);
            }
        }

        public static Font CreateSynthetic(Font font, in FontSynthesisDesc desc)
        {
            return new Font(Methods.DenOfIz_Font_CreateSynthetic(font, in desc), ownsHandle: true);
        }

        public static Font CreateFromPath(StringView path, uint faceIndex = 0)
        {
            return new Font(Methods.DenOfIz_Font_CreateFromPath(path, faceIndex), ownsHandle: true);
        }

        public static Font GetDefault()
        {
            return new Font(Methods.DenOfIz_Font_GetDefault(), ownsHandle: false);
        }

        public FontMetrics GetMetrics()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Font_GetMetrics(Handle);
        }

        public bool HasCodePoint(uint codePoint)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Font_HasCodePoint(Handle, codePoint);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Font_Destroy(Handle);
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
