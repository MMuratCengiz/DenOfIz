using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class BindGroupLayout : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        private BindingDesc[]? _cachedBindings;
        private BindGroupLayoutDesc? _cachedDesc;

        public static implicit operator ulong(BindGroupLayout wrapper) => wrapper?.Handle ?? 0;

        internal BindGroupLayout(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public ReadOnlySpan<BindingDesc> Bindings
        {
            get
            {
                ThrowIfDisposed();
                EnsureCached();
                return _cachedBindings;
            }
        }

        public uint RegisterSpace
        {
            get
            {
                ThrowIfDisposed();
                EnsureCached();
                return _cachedDesc!.Value.RegisterSpace;
            }
        }

        private void EnsureCached()
        {
            if (_cachedDesc.HasValue)
            {
                return;
            }

            unsafe
            {
                var nativeDescPtr = Methods.DenOfIz_BindGroupLayout_GetDesc(Handle);
                if (nativeDescPtr == IntPtr.Zero)
                {
                    throw new InvalidOperationException("Failed to get BindGroupLayout descriptor");
                }

                var nativeDesc = *(BindGroupLayoutDesc*)nativeDescPtr.ToPointer();

                if (nativeDesc.Bindings.NumElements > 0 && nativeDesc.Bindings.Elements != IntPtr.Zero)
                {
                    _cachedBindings = new BindingDesc[(int)nativeDesc.Bindings.NumElements];
                    var srcPtr = (BindingDesc*)nativeDesc.Bindings.Elements.ToPointer();
                    for (int i = 0; i < (int)nativeDesc.Bindings.NumElements; i++)
                    {
                        _cachedBindings[i] = srcPtr[i];
                    }
                }
                else
                {
                    _cachedBindings = Array.Empty<BindingDesc>();
                }

                _cachedDesc = nativeDesc;
            }
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            _cachedBindings = null;
            _cachedDesc = null;

            if (_ownsHandle)
            {
                Methods.DenOfIz_BindGroupLayout_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(BindGroupLayout));
            }
        }
    }
}
