using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderImporter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderImporter wrapper) => wrapper?.Handle ?? 0;

        public ShaderImporter()
        {
            Handle = Methods.DenOfIz_ShaderImporter_Create();
            _ownsHandle = true;
        }

        internal ShaderImporter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public StringView GetName()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderImporter_GetName(Handle);
        }

        public StringViewArray GetSupportedExtensions()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderImporter_GetSupportedExtensions(Handle);
        }

        public bool CanProcessFileExtension(StringView extension)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderImporter_CanProcessFileExtension(Handle, extension);
        }

        public bool ValidateFile(StringView filePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderImporter_ValidateFile(Handle, filePath);
        }

        public ImporterResult Import(in ShaderImportDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderImporter_Import(Handle, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderImporter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderImporter));
            }
        }
    }
}
