/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Result.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_GraphicsBackendType
    {
        DENOFIZ_GRAPHICS_BACKEND_UNKNOWN,
        DENOFIZ_GRAPHICS_BACKEND_DIRECTX12,
        DENOFIZ_GRAPHICS_BACKEND_VULKAN,
        DENOFIZ_GRAPHICS_BACKEND_METAL,
        DENOFIZ_GRAPHICS_BACKEND_WEBGPU
    } DenOfIz_GraphicsBackendType;

    typedef struct DenOfIz_NativeDeviceHandles
    {
        DenOfIz_GraphicsBackendType Backend;

        void *DX12Device;
        void *DX12Adapter;

        void *VkDevice;
        void *VkPhysicalDevice;
        void *VkInstance;
        void *VkGetInstanceProcAddr;
        void *VkGetDeviceProcAddr;

        void *MTLDevice;
    } DenOfIz_NativeDeviceHandles;

    typedef struct DenOfIz_NativeQueueHandles
    {
        DenOfIz_GraphicsBackendType Backend;

        void *DX12CommandQueue;

        void    *VkQueue;
        uint32_t VkQueueFamilyIndex;

        void *MTLCommandQueue;
    } DenOfIz_NativeQueueHandles;

    typedef struct DenOfIz_NativeTextureHandles
    {
        DenOfIz_GraphicsBackendType Backend;

        void    *DX12Resource;
        uint64_t DX12RtvDescriptorHandle;
        uint64_t DX12DsvDescriptorHandle;

        void    *VkImage;
        void    *VkImageView;
        uint32_t VkImageLayout;
        uint32_t VkFormat;
        uint32_t VkImageAspect;

        void *MTLTexture;
    } DenOfIz_NativeTextureHandles;

    typedef struct DenOfIz_ExternalTextureDescDX12
    {
        void               *Resource;
        uint64_t            RtvDescriptorHandle;
        DenOfIz_TextureDesc TextureDesc;
    } DenOfIz_ExternalTextureDescDX12;

    typedef struct DenOfIz_ExternalTextureDescVulkan
    {
        void                *VkImage;
        void                *VkImageView;
        uint32_t             VkFormat;
        uint32_t             VkImageAspect;
        DenOfIz_TextureDesc  TextureDesc;
    } DenOfIz_ExternalTextureDescVulkan;

    typedef struct DenOfIz_ExternalTextureDescMetal
    {
        void                *MTLTexture;
        DenOfIz_TextureDesc  TextureDesc;
    } DenOfIz_ExternalTextureDescMetal;

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_GetBackendType( DenOfIz_LogicalDevice device, DenOfIz_GraphicsBackendType *outBackend );

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_GetNativeDeviceHandles( DenOfIz_LogicalDevice device, DenOfIz_NativeDeviceHandles *outHandles );

    DZ_API DenOfIz_Result DenOfIz_CommandQueue_GetNativeHandles( DenOfIz_CommandQueue queue, DenOfIz_NativeQueueHandles *outHandles );

    DZ_API DenOfIz_Result DenOfIz_Texture_GetNativeHandles( DenOfIz_Texture texture, DenOfIz_NativeTextureHandles *outHandles );

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureDX12( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescDX12 *desc, DenOfIz_Texture *outTexture );

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureVulkan( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescVulkan *desc, DenOfIz_Texture *outTexture );

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureMetal( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescMetal *desc, DenOfIz_Texture *outTexture );

#ifdef __cplusplus
}
#endif
