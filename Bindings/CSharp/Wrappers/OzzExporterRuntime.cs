using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class OzzExporterRuntime : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(OzzExporterRuntime wrapper) => wrapper?.Handle ?? 0;

        public OzzExporterRuntime()
        {
            Handle = Methods.DenOfIz_OzzExporterRuntime_Create();
            _ownsHandle = true;
        }

        internal OzzExporterRuntime(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool Load(StringView gltfFilePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_Load(Handle, gltfFilePath);
        }

        public bool Load(string gltfFilePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_Load(Handle, StringView.Intern(gltfFilePath));
        }

        public bool LoadFromMemory(ByteArrayView gltfBytes, StringView basePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_LoadFromMemory(Handle, gltfBytes, basePath);
        }

        public bool LoadFromMemory(byte[] gltfBytes, string basePath = "")
        {
            ThrowIfDisposed();
            unsafe
            {
                fixed (byte* ptr = gltfBytes)
                {
                    var byteView = new ByteArrayView
                    {
                        Elements = (IntPtr)ptr,
                        NumElements = (ulong)gltfBytes.Length
                    };
                    return Methods.DenOfIz_OzzExporterRuntime_LoadFromMemory(Handle, byteView, StringView.Intern(basePath));
                }
            }
        }

        public bool LoadReference(StringView referenceGltfPath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_LoadReference(Handle, referenceGltfPath);
        }

        public bool LoadReference(string referenceGltfPath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_LoadReference(Handle, StringView.Intern(referenceGltfPath));
        }

        public bool LoadReferenceFromMemory(ByteArrayView gltfBytes, StringView basePath)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_LoadReferenceFromMemory(Handle, gltfBytes, basePath);
        }

        public bool LoadReferenceFromMemory(byte[] gltfBytes, string basePath = "")
        {
            ThrowIfDisposed();
            unsafe
            {
                fixed (byte* ptr = gltfBytes)
                {
                    var byteView = new ByteArrayView
                    {
                        Elements = (IntPtr)ptr,
                        NumElements = (ulong)gltfBytes.Length
                    };
                    return Methods.DenOfIz_OzzExporterRuntime_LoadReferenceFromMemory(Handle, byteView, StringView.Intern(basePath));
                }
            }
        }

        public void SetSanitizeTransforms(bool sanitize)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_OzzExporterRuntime_SetSanitizeTransforms(Handle, sanitize);
        }

        public OzzGltfDesc GetDesc()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_OzzExporterRuntime_GetDesc(Handle);
        }

        public bool HasSkeleton
        {
            get
            {
                ThrowIfDisposed();
                return GetDesc().HasSkeleton;
            }
        }

        public uint NumAnimations
        {
            get
            {
                ThrowIfDisposed();
                return GetDesc().NumAnimations;
            }
        }

        public IReadOnlyList<string> GetAnimationNames()
        {
            ThrowIfDisposed();
            var desc = GetDesc();
            var names = new List<string>((int)desc.NumAnimations);
            var animNames = desc.AnimationNames.ToArray();

            foreach (var name in animNames)
            {
                names.Add(name.ToString());
            }

            return names;
        }

        public BinaryContainer? BuildSkeleton()
        {
            ThrowIfDisposed();
            var handle = Methods.DenOfIz_OzzExporterRuntime_BuildSkeleton(Handle);
            if (handle == 0)
            {
                return null;
            }

            return new BinaryContainer(handle, ownsHandle: true);
        }

        public BinaryContainer? BuildAnimation(OzzSkeleton ozzAnimation, uint animationIndex)
        {
            ThrowIfDisposed();
            var handle = Methods.DenOfIz_OzzExporterRuntime_BuildAnimation(Handle, ozzAnimation, animationIndex);
            if (handle == 0)
            {
                return null;
            }

            return new BinaryContainer(handle, ownsHandle: true);
        }

        public BinaryContainer? BuildAnimation(OzzSkeleton ozzAnimation, StringView animationName)
        {
            ThrowIfDisposed();
            var handle = Methods.DenOfIz_OzzExporterRuntime_BuildAnimationByName(Handle, ozzAnimation, animationName);
            if (handle == 0)
            {
                return null;
            }

            return new BinaryContainer(handle, ownsHandle: true);
        }

        public BinaryContainer? BuildAnimation(OzzSkeleton ozzAnimation, string animationName)
        {
            ThrowIfDisposed();
            var handle = Methods.DenOfIz_OzzExporterRuntime_BuildAnimationByName(Handle, ozzAnimation, StringView.Intern(animationName));
            if (handle == 0)
            {
                return null;
            }

            return new BinaryContainer(handle, ownsHandle: true);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_OzzExporterRuntime_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(OzzExporterRuntime));
            }
        }
    }
}
