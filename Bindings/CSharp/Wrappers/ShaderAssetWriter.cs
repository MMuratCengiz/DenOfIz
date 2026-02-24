using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderAssetWriter : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderAssetWriter wrapper) => wrapper?.Handle ?? 0;

        public ShaderAssetWriter(in ShaderAssetWriterDesc desc)
        {
            Handle = Methods.DenOfIz_ShaderAssetWriter_Create(in desc);
            _ownsHandle = true;
        }

        internal ShaderAssetWriter(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Write(ShaderAsset? shaderAsset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAssetWriter_Write(Handle, shaderAsset);
        }

        public void End()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAssetWriter_End(Handle);
        }

        public static ShaderAsset CreateFromCompiledShader(in CompiledShader compiledShader)
        {
            return new ShaderAsset(Methods.DenOfIz_ShaderAssetWriter_CreateFromCompiledShader(in compiledShader), ownsHandle: false);
        }

        public static ulong NumRequiredArenaBytes(in CompiledShader compiledShader)
        {
            return Methods.DenOfIz_ShaderAssetWriter_NumRequiredArenaBytes(in compiledShader);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderAssetWriter_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderAssetWriter));
            }
        }
    }
}
