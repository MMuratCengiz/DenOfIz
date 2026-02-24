using System;
using System.Runtime.InteropServices;
using System.Text;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class ShaderIncludeHandler : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(ShaderIncludeHandler wrapper) => wrapper?.Handle ?? 0;

        public ShaderIncludeHandler()
        {
            Handle = Methods.DenOfIz_ShaderIncludeHandler_Create();
            _ownsHandle = true;
        }

        internal ShaderIncludeHandler(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void AddFile(in StringView path, in ByteArrayView data)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_ShaderIncludeHandler_AddFile(Handle, in path, in data);
        }

        public void AddFile(string path, in ByteArrayView data)
        {
            ThrowIfDisposed();
            var pathView = StringView.Intern(path);
            Methods.DenOfIz_ShaderIncludeHandler_AddFile(Handle, in pathView, in data);
        }

        public unsafe void AddFile(string path, ReadOnlySpan<byte> data)
        {
            ThrowIfDisposed();
            var pathView = StringView.Intern(path);
            fixed (byte* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)data.Length;
                Methods.DenOfIz_ShaderIncludeHandler_AddFile(Handle, in pathView, in view);
            }
        }

        public unsafe void AddFile(string path, byte[] data)
        {
            ThrowIfDisposed();
            var pathView = StringView.Intern(path);
            fixed (byte* ptr = data)
            {
                ByteArrayView view = default;
                view.Elements = (IntPtr)ptr;
                view.NumElements = (ulong)data.Length;
                Methods.DenOfIz_ShaderIncludeHandler_AddFile(Handle, in pathView, in view);
            }
        }

        public void AddFile(string path, string content)
        {
            ThrowIfDisposed();
            byte[] data = Encoding.UTF8.GetBytes(content);
            AddFile(path, data);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_ShaderIncludeHandler_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(ShaderIncludeHandler));
            }
        }
    }
}
