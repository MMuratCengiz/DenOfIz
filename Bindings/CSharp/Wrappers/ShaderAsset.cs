using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderAsset : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderAsset wrapper) => wrapper?.Handle ?? 0;

        public ShaderAsset()
        {
            Handle = Methods.DenOfIz_ShaderAsset_Create();
            _ownsHandle = true;
        }

        internal ShaderAsset(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public AssetHeader Header()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_Header(Handle);
        }

        public ulong Magic()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_Magic(Handle);
        }

        public uint Version()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_Version(Handle);
        }

        public ulong NumBytes()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_NumBytes(Handle);
        }

        public StringView Path()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_Path(Handle);
        }

        public ulong NumStages()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_NumStages(Handle);
        }

        public ShaderStageAsset GetStage(ulong index)
        {
            ThrowIfDisposed();
            return Marshal.PtrToStructure<ShaderStageAsset>(Methods.DenOfIz_ShaderAsset_GetStage(Handle, index));
        }

        public ShaderStageAsset AddStage()
        {
            ThrowIfDisposed();
            return Marshal.PtrToStructure<ShaderStageAsset>(Methods.DenOfIz_ShaderAsset_AddStage(Handle));
        }

        public void SetStageEntryPoint(ulong index, StringView entryPoint)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_SetStageEntryPoint(Handle, index, entryPoint);
        }

        public void ReserveStages(ulong capacity)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_ReserveStages(Handle, capacity);
        }

        public void ClearStages()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_ClearStages(Handle);
        }

        public ShaderReflectDesc GetReflectDesc()
        {
            ThrowIfDisposed();
            return Marshal.PtrToStructure<ShaderReflectDesc>(Methods.DenOfIz_ShaderAsset_GetReflectDesc(Handle));
        }

        public void SetReflectDesc(in ShaderReflectDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_SetReflectDesc(Handle, in desc);
        }

        public ShaderRayTracingDesc GetRayTracing()
        {
            ThrowIfDisposed();
            return Marshal.PtrToStructure<ShaderRayTracingDesc>(Methods.DenOfIz_ShaderAsset_GetRayTracing(Handle));
        }

        public void SetVersion(uint version)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_SetVersion(Handle, version);
        }

        public void SetNumBytes(ulong numBytes)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_SetNumBytes(Handle, numBytes);
        }

        public void SetPath(StringView path)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderAsset_SetPath(Handle, path);
        }

        public StringView StoreString(StringView str)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_ShaderAsset_StoreString(Handle, str);
        }

        public static StringView Extension()
        {
            return Methods.DenOfIz_ShaderAsset_Extension();
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderAsset_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderAsset));
            }
        }
    }
}
