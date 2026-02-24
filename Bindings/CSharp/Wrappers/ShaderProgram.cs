using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderProgram : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderProgram wrapper) => wrapper?.Handle ?? 0;

        public ShaderProgram(in ShaderProgramDesc desc)
        {
            Handle = Methods.DenOfIz_ShaderProgram_Create(in desc);
            _ownsHandle = true;
        }

        internal ShaderProgram(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public CompiledShaderStageArray CompiledShaders()
        {
            ThrowIfDisposed();
            CompiledShaderStageArray outCompiledShaders = default;
            Methods.DenOfIz_ShaderProgram_CompiledShaders(Handle, out outCompiledShaders);
            return outCompiledShaders;
        }

        public ShaderReflectDesc Reflect()
        {
            ThrowIfDisposed();
            ShaderReflectDesc outReflectDesc = default;
            Methods.DenOfIz_ShaderProgram_Reflect(Handle, out outReflectDesc);
            return outReflectDesc;
        }

        public ShaderProgramDesc Desc()
        {
            ThrowIfDisposed();
            ShaderProgramDesc outDesc = default;
            Methods.DenOfIz_ShaderProgram_Desc(Handle, out outDesc);
            return outDesc;
        }

        public static ShaderProgram CreateFromAsset(ShaderAssetReader? reader)
        {
            return new ShaderProgram(Methods.DenOfIz_ShaderProgram_CreateFromAsset(reader), ownsHandle: false);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderProgram_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderProgram));
            }
        }
    }
}
