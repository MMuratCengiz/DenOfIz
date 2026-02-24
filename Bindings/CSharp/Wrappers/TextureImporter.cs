using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextureImporter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextureImporter wrapper) => wrapper?.Handle ?? 0;

        public TextureImporter()
        {
            Handle = Methods.DenOfIz_TextureImporter_Create();
            _ownsHandle = true;
        }

        internal TextureImporter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public StringView GetName()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureImporter_GetName(Handle);
        }

        public StringViewArray GetSupportedExtensions()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureImporter_GetSupportedExtensions(Handle);
        }

        public bool CanProcessFileExtension(StringView extension)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureImporter_CanProcessFileExtension(Handle, extension);
        }

        public bool ValidateFile(StringView filePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureImporter_ValidateFile(Handle, filePath);
        }

        public ImporterResult Import(in TextureImportDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextureImporter_Import(Handle, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextureImporter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextureImporter));
            }
        }
    }
}
