using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BindGroup : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(BindGroup wrapper) => wrapper?.Handle ?? 0;

        internal BindGroup(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void BeginUpdate()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_BeginUpdate(Handle);
        }

        public void Cbv(uint binding, Buffer? resource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_Cbv(Handle, binding, resource);
        }

        public void CbvWithDesc(in BindBufferDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_CbvWithDesc(Handle, in desc);
        }

        public void SrvBuffer(uint binding, Buffer? resource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvBuffer(Handle, binding, resource);
        }

        public void SrvBufferWithDesc(in BindBufferDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvBufferWithDesc(Handle, in desc);
        }

        public void SrvTopLevelAS(uint binding, TopLevelAS? accelerationStructure)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvTopLevelAS(Handle, binding, accelerationStructure);
        }

        public void SrvTexture(uint binding, Texture? resource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvTexture(Handle, binding, resource);
        }

        public void SrvTextureMip(uint binding, Texture? resource, uint mipLevel)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvTextureMip(Handle, binding, resource, mipLevel);
        }

        public void SrvArray(uint binding, in TextureArray resources)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_SrvArray(Handle, binding, in resources);
        }

        public BindGroup SrvArrayIndex(uint binding, uint arrayIndex, Texture? resource)
        {
            ThrowIfDisposed();
            ulong outBindGroup = default;
            Methods.DenOfIz_BindGroup_SrvArrayIndex(Handle, binding, arrayIndex, resource, out outBindGroup);
            return new BindGroup(outBindGroup, ownsHandle: false);
        }

        public void UavBuffer(uint binding, Buffer? resource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_UavBuffer(Handle, binding, resource);
        }

        public void UavBufferWithDesc(in BindBufferDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_UavBufferWithDesc(Handle, in desc);
        }

        public void UavTexture(uint binding, Texture? resource)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_UavTexture(Handle, binding, resource);
        }

        public void UavTextureMip(uint binding, Texture? resource, uint mipLevel)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_UavTextureMip(Handle, binding, resource, mipLevel);
        }

        public void Sampler(uint binding, Sampler? sampler)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_Sampler(Handle, binding, sampler);
        }

        public void EndUpdate()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_BindGroup_EndUpdate(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_BindGroup_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BindGroup));
            }
        }
    }
}
