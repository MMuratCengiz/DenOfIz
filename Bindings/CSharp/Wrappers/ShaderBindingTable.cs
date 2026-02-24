using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderBindingTable : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderBindingTable wrapper) => wrapper?.Handle ?? 0;

        internal ShaderBindingTable(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void Resize(in SBTSizeDesc resizeDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderBindingTable_Resize(Handle, in resizeDesc);
        }

        public void BindRayGenerationShader(in RayGenerationBindingDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderBindingTable_BindRayGenerationShader(Handle, in desc);
        }

        public void BindHitGroup(in HitGroupBindingDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderBindingTable_BindHitGroup(Handle, in desc);
        }

        public void BindMissShader(in MissBindingDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderBindingTable_BindMissShader(Handle, in desc);
        }

        public void Build()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderBindingTable_Build(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderBindingTable_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderBindingTable));
            }
        }
    }
}
