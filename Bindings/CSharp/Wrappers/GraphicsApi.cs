using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class GraphicsApi : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(GraphicsApi wrapper) => wrapper?.Handle ?? 0;

        public GraphicsApi(in APIPreference preference)
        {
            Handle = Methods.DenOfIz_GraphicsApi_Create(in preference);
            _ownsHandle = true;
        }

        internal GraphicsApi(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public LogicalDevice CreateLogicalDevice(in LogicalDeviceDesc desc)
        {
            ThrowIfDisposed();
            return new LogicalDevice(Methods.DenOfIz_GraphicsApi_CreateLogicalDevice(Handle, in desc), ownsHandle: false);
        }

        public void LogDeviceCapabilities(in PhysicalDevice gpuDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_GraphicsApi_LogDeviceCapabilities(Handle, in gpuDesc);
        }

        public LogicalDevice CreateAndLoadOptimalLogicalDevice(in LogicalDeviceDesc desc)
        {
            ThrowIfDisposed();
            return new LogicalDevice(Methods.DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice(Handle, in desc), ownsHandle: false);
        }

        public StringView ActiveAPI()
        {
            ThrowIfDisposed();
            StringView outActiveAPI = default;
            Methods.DenOfIz_GraphicsApi_ActiveAPI(Handle, out outActiveAPI);
            return outActiveAPI;
        }

        public static void ReportLiveObjects()
        {
            Methods.DenOfIz_GraphicsApi_ReportLiveObjects();
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_GraphicsApi_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(GraphicsApi));
            }
        }
    }
}
