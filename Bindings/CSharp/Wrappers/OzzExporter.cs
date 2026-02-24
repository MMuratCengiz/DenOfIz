using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class OzzExporter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(OzzExporter wrapper) => wrapper?.Handle ?? 0;

        public OzzExporter()
        {
            Handle = Methods.DenOfIz_OzzExporter_Create();
            _ownsHandle = true;
        }

        internal OzzExporter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool ValidateGltf(StringView filePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporter_ValidateGltf(Handle, filePath);
        }

        public bool ValidateGltfFromMemory(ByteArrayView gltfData)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporter_ValidateGltfFromMemory(Handle, gltfData);
        }

        public bool ValidateGltfFromMemory(byte[] gltfData)
        {
            ThrowIfDisposed();
            unsafe
            {
                fixed (byte* ptr = gltfData)
                {
                    var byteView = new ByteArrayView
                    {
                        Elements = (IntPtr)ptr,
                        NumElements = (ulong)gltfData.Length
                    };
                    return Methods.DenOfIz_OzzExporter_ValidateGltfFromMemory(Handle, byteView);
                }
            }
        }

        public OzzExportResult Export(in OzzExportDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporter_Export(Handle, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_OzzExporter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(OzzExporter));
            }
        }
    }
}
