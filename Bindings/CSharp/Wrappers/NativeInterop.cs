using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public static class NativeInterop
    {
        public static GraphicsBackendType GetBackendType(LogicalDevice device)
        {
            if (device == null)
            {
                throw new ArgumentNullException(nameof(device));
            }

            GraphicsBackendType backend = default;
            var result = Methods.DenOfIz_LogicalDevice_GetBackendType(device, out backend);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return backend;
        }

        public static NativeDeviceHandles GetNativeDeviceHandles(LogicalDevice device)
        {
            if (device == null)
            {
                throw new ArgumentNullException(nameof(device));
            }

            NativeDeviceHandles handles = default;
            var result = Methods.DenOfIz_LogicalDevice_GetNativeDeviceHandles(device, out handles);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return handles;
        }

        public static NativeQueueHandles GetNativeQueueHandles(CommandQueue queue)
        {
            if (queue == null)
            {
                throw new ArgumentNullException(nameof(queue));
            }

            NativeQueueHandles handles = default;
            var result = Methods.DenOfIz_CommandQueue_GetNativeHandles(queue, out handles);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return handles;
        }

        public static NativeTextureHandles GetNativeTextureHandles(Texture texture)
        {
            if (texture == null)
            {
                throw new ArgumentNullException(nameof(texture));
            }

            NativeTextureHandles handles = default;
            var result = Methods.DenOfIz_Texture_GetNativeHandles(texture, out handles);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return handles;
        }

        public static Texture ImportExternalTextureDX12(LogicalDevice device, in ExternalTextureDescDX12 desc)
        {
            if (device == null)
            {
                throw new ArgumentNullException(nameof(device));
            }

            ulong outTexture = 0;
            var result = Methods.DenOfIz_LogicalDevice_ImportExternalTextureDX12(device, in desc, out outTexture);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return new Texture(outTexture, ownsHandle: true);
        }

        public static Texture ImportExternalTextureVulkan(LogicalDevice device, in ExternalTextureDescVulkan desc)
        {
            if (device == null)
            {
                throw new ArgumentNullException(nameof(device));
            }

            ulong outTexture = 0;
            var result = Methods.DenOfIz_LogicalDevice_ImportExternalTextureVulkan(device, in desc, out outTexture);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return new Texture(outTexture, ownsHandle: true);
        }

        public static Texture ImportExternalTextureMetal(LogicalDevice device, in ExternalTextureDescMetal desc)
        {
            if (device == null)
            {
                throw new ArgumentNullException(nameof(device));
            }

            ulong outTexture = 0;
            var result = Methods.DenOfIz_LogicalDevice_ImportExternalTextureMetal(device, in desc, out outTexture);
            if (result != Result.Success)
            {
                throw new DenOfIzException(result);
            }

            return new Texture(outTexture, ownsHandle: true);
        }
    }
}
