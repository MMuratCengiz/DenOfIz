using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class FontImporter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(FontImporter wrapper) => wrapper?.Handle ?? 0;

        public FontImporter()
        {
            Handle = Methods.DenOfIz_FontImporter_Create();
            _ownsHandle = true;
        }

        internal FontImporter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public StringView GetName()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontImporter_GetName(Handle);
        }

        public StringViewArray GetSupportedExtensions()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontImporter_GetSupportedExtensions(Handle);
        }

        public bool CanProcessFileExtension(StringView extension)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontImporter_CanProcessFileExtension(Handle, extension);
        }

        public bool ValidateFile(StringView filePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontImporter_ValidateFile(Handle, filePath);
        }

        public ImporterResult Import(in FontImportDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_FontImporter_Import(Handle, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_FontImporter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(FontImporter));
            }
        }
    }
}
