using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderLocalData : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderLocalData wrapper) => wrapper?.Handle ?? 0;

        internal ShaderLocalData(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Begin()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_Begin(Handle);
        }

        public void CbvBuffer(uint binding, Buffer? bufferResource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_CbvBuffer(Handle, binding, bufferResource);
        }

        public void CbvData(uint binding, in ByteArrayView data)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_CbvData(Handle, binding, in data);
        }

        public void SrvBuffer(uint binding, Buffer? bufferResource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_SrvBuffer(Handle, binding, bufferResource);
        }

        public void SrvTexture(uint binding, Texture? textureResource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_SrvTexture(Handle, binding, textureResource);
        }

        public void UavBuffer(uint binding, Buffer? bufferResource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_UavBuffer(Handle, binding, bufferResource);
        }

        public void UavTexture(uint binding, Texture? textureResource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_UavTexture(Handle, binding, textureResource);
        }

        public void Sampler(uint binding, Sampler? sampler)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_Sampler(Handle, binding, sampler);
        }

        public void End()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderLocalData_End(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderLocalData_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderLocalData));
            }
        }
    }
}
