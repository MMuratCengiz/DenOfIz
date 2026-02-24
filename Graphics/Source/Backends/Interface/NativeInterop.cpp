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

#include "DenOfIzGraphics/Backends/Interface/NativeInterop.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

#ifdef BUILD_DX12
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12LogicalDevice.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12CommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Texture.h"
#endif

#ifdef BUILD_VK
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanLogicalDevice.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTexture.h"
#endif

#ifdef BUILD_METAL
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalLogicalDevice.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalTexture.h"
#endif

#include <cstring>

#define DEVICE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ILogicalDevice, handle )
#define QUEUE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ICommandQueue, handle )
#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, handle )

extern "C"
{
    DenOfIz_Result DenOfIz_LogicalDevice_GetBackendType( DenOfIz_LogicalDevice device, DenOfIz_GraphicsBackendType *outBackend )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) || outBackend == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

        auto *impl = DEVICE_IMPL( device );

#ifdef BUILD_DX12
        if ( dynamic_cast<DenOfIz::DX12LogicalDevice *>( impl ) != nullptr )
        {
            *outBackend = DENOFIZ_GRAPHICS_BACKEND_DIRECTX12;
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_VK
        if ( dynamic_cast<DenOfIz::VulkanLogicalDevice *>( impl ) != nullptr )
        {
            *outBackend = DENOFIZ_GRAPHICS_BACKEND_VULKAN;
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_METAL
        if ( dynamic_cast<DenOfIz::MetalLogicalDevice *>( impl ) != nullptr )
        {
            *outBackend = DENOFIZ_GRAPHICS_BACKEND_METAL;
            return DENOFIZ_SUCCESS;
        }
#endif

        *outBackend = DENOFIZ_GRAPHICS_BACKEND_UNKNOWN;
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_GetNativeDeviceHandles( DenOfIz_LogicalDevice device, DenOfIz_NativeDeviceHandles *outHandles )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) || outHandles == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

        std::memset( outHandles, 0, sizeof( DenOfIz_NativeDeviceHandles ) );

        auto *impl = DEVICE_IMPL( device );

#ifdef BUILD_DX12
        if ( auto *dx12 = dynamic_cast<DenOfIz::DX12LogicalDevice *>( impl ) )
        {
            auto *ctx                = dx12->GetContext( );
            outHandles->Backend      = DENOFIZ_GRAPHICS_BACKEND_DIRECTX12;
            outHandles->DX12Device   = ctx->D3DDevice.get( );
            outHandles->DX12Adapter  = ctx->Adapter.get( );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_VK
        if ( auto *vulkan = dynamic_cast<DenOfIz::VulkanLogicalDevice *>( impl ) )
        {
            auto *ctx                        = vulkan->GetContext( );
            outHandles->Backend              = DENOFIZ_GRAPHICS_BACKEND_VULKAN;
            outHandles->VkDevice             = ctx->LogicalDevice;
            outHandles->VkPhysicalDevice     = ctx->PhysicalDevice;
            outHandles->VkInstance           = ctx->Instance;
            outHandles->VkGetInstanceProcAddr = reinterpret_cast<void *>( vkGetInstanceProcAddr );
            outHandles->VkGetDeviceProcAddr   = reinterpret_cast<void *>( vkGetDeviceProcAddr );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_METAL
        if ( auto *metal = dynamic_cast<DenOfIz::MetalLogicalDevice *>( impl ) )
        {
            auto *ctx            = metal->Context( );
            outHandles->Backend  = DENOFIZ_GRAPHICS_BACKEND_METAL;
            outHandles->MTLDevice = (__bridge void *)ctx->Device;
            return DENOFIZ_SUCCESS;
        }
#endif

        outHandles->Backend = DENOFIZ_GRAPHICS_BACKEND_UNKNOWN;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
    }

    DenOfIz_Result DenOfIz_CommandQueue_GetNativeHandles( DenOfIz_CommandQueue queue, DenOfIz_NativeQueueHandles *outHandles )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queue ) || outHandles == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

        std::memset( outHandles, 0, sizeof( DenOfIz_NativeQueueHandles ) );

        auto *impl = QUEUE_IMPL( queue );

#ifdef BUILD_DX12
        if ( auto *dx12 = dynamic_cast<DenOfIz::DX12CommandQueue *>( impl ) )
        {
            outHandles->Backend          = DENOFIZ_GRAPHICS_BACKEND_DIRECTX12;
            outHandles->DX12CommandQueue = dx12->GetCommandQueue( );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_VK
        if ( auto *vulkan = dynamic_cast<DenOfIz::VulkanCommandQueue *>( impl ) )
        {
            outHandles->Backend            = DENOFIZ_GRAPHICS_BACKEND_VULKAN;
            outHandles->VkQueue            = vulkan->GetQueue( );
            outHandles->VkQueueFamilyIndex = vulkan->GetQueueFamilyIndex( );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_METAL
        if ( auto *metal = dynamic_cast<DenOfIz::MetalCommandQueue *>( impl ) )
        {
            outHandles->Backend         = DENOFIZ_GRAPHICS_BACKEND_METAL;
            outHandles->MTLCommandQueue = (__bridge void *)metal->GetQueue( );
            return DENOFIZ_SUCCESS;
        }
#endif

        outHandles->Backend = DENOFIZ_GRAPHICS_BACKEND_UNKNOWN;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
    }

    DenOfIz_Result DenOfIz_Texture_GetNativeHandles( DenOfIz_Texture texture, DenOfIz_NativeTextureHandles *outHandles )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) || outHandles == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

        std::memset( outHandles, 0, sizeof( DenOfIz_NativeTextureHandles ) );

        auto *impl = TEXTURE_IMPL( texture );

#ifdef BUILD_DX12
        if ( auto *dx12 = dynamic_cast<DenOfIz::DX12Texture *>( impl ) )
        {
            outHandles->Backend      = DENOFIZ_GRAPHICS_BACKEND_DIRECTX12;
            outHandles->DX12Resource = dx12->Resource( );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_VK
        if ( auto *vulkan = dynamic_cast<DenOfIz::VulkanTexture *>( impl ) )
        {
            outHandles->Backend       = DENOFIZ_GRAPHICS_BACKEND_VULKAN;
            outHandles->VkImage       = vulkan->Image( );
            outHandles->VkImageView   = vulkan->ImageView( );
            outHandles->VkImageLayout = static_cast<uint32_t>( vulkan->Layout( ) );
            outHandles->VkImageAspect = static_cast<uint32_t>( vulkan->Aspect( ) );
            return DENOFIZ_SUCCESS;
        }
#endif

#ifdef BUILD_METAL
        if ( auto *metal = dynamic_cast<DenOfIz::MetalTexture *>( impl ) )
        {
            outHandles->Backend    = DENOFIZ_GRAPHICS_BACKEND_METAL;
            outHandles->MTLTexture = (__bridge void *)metal->Instance( );
            return DENOFIZ_SUCCESS;
        }
#endif

        outHandles->Backend = DENOFIZ_GRAPHICS_BACKEND_UNKNOWN;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureDX12( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescDX12 *desc, DenOfIz_Texture *outTexture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) || desc == nullptr || outTexture == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

#ifdef BUILD_DX12
        auto *impl = DEVICE_IMPL( device );
        auto *dx12 = dynamic_cast<DenOfIz::DX12LogicalDevice *>( impl );

        if ( dx12 == nullptr )
        {
            return DENOFIZ_ERROR_NOT_SUPPORTED;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        rtvHandle.ptr = desc->RtvDescriptorHandle;

        auto *texture = new DenOfIz::DX12Texture(
            static_cast<ID3D12Resource2 *>( desc->Resource ),
            rtvHandle
        );

        *outTexture = DENOFIZ_TO_HANDLE( texture );
        return DENOFIZ_SUCCESS;
#else
        ( void )device;
        ( void )desc;
        ( void )outTexture;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
#endif
    }

    DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureVulkan( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescVulkan *desc, DenOfIz_Texture *outTexture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) || desc == nullptr || outTexture == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

#ifdef BUILD_VK
        auto *impl   = DEVICE_IMPL( device );
        auto *vulkan = dynamic_cast<DenOfIz::VulkanLogicalDevice *>( impl );

        if ( vulkan == nullptr )
        {
            return DENOFIZ_ERROR_NOT_SUPPORTED;
        }

        auto *texture = new DenOfIz::VulkanTexture(
            static_cast<VkImage>( desc->VkImage ),
            static_cast<VkImageView>( desc->VkImageView ),
            static_cast<VkFormat>( desc->VkFormat ),
            static_cast<VkImageAspectFlags>( desc->VkImageAspect ),
            desc->TextureDesc
        );

        *outTexture = DENOFIZ_TO_HANDLE( texture );
        return DENOFIZ_SUCCESS;
#else
        ( void )device;
        ( void )desc;
        ( void )outTexture;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
#endif
    }

    DenOfIz_Result DenOfIz_LogicalDevice_ImportExternalTextureMetal( DenOfIz_LogicalDevice device, const DenOfIz_ExternalTextureDescMetal *desc, DenOfIz_Texture *outTexture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) || desc == nullptr || outTexture == nullptr )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }

#ifdef BUILD_METAL
        auto *impl  = DEVICE_IMPL( device );
        auto *metal = dynamic_cast<DenOfIz::MetalLogicalDevice *>( impl );

        if ( metal == nullptr )
        {
            return DENOFIZ_ERROR_NOT_SUPPORTED;
        }

        id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)desc->MTLTexture;

        auto *texture = new DenOfIz::MetalTexture(
            metal->Context( ),
            desc->TextureDesc,
            mtlTexture
        );

        *outTexture = DENOFIZ_TO_HANDLE( texture );
        return DENOFIZ_SUCCESS;
#else
        ( void )device;
        ( void )desc;
        ( void )outTexture;
        return DENOFIZ_ERROR_NOT_SUPPORTED;
#endif
    }
}
