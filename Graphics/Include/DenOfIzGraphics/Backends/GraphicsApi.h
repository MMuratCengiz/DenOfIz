/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_GraphicsApi )

    typedef enum DenOfIz_APIPreferenceWindows
    {
        DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12,
        DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN
    } DenOfIz_APIPreferenceWindows;

    typedef enum DenOfIz_APIPreferenceOSX
    {
        DENOFIZ_API_PREFERENCE_OSX_METAL,
        DENOFIZ_API_PREFERENCE_OSX_VULKAN
    } DenOfIz_APIPreferenceOSX;

    typedef enum DenOfIz_APIPreferenceLinux
    {
        DENOFIZ_API_PREFERENCE_LINUX_VULKAN
    } DenOfIz_APIPreferenceLinux;

    /** Only the field of the platform the program runs on is read. macOS builds have no Vulkan backend: OSX VULKAN creates no device. */
    typedef struct DenOfIz_APIPreference
    {
        DenOfIz_APIPreferenceWindows Windows;
        DenOfIz_APIPreferenceOSX     OSX;
        DenOfIz_APIPreferenceLinux   Linux;
    } DenOfIz_APIPreference;

    DZ_API DenOfIz_GraphicsApi DenOfIz_GraphicsApi_Create( const DenOfIz_APIPreference *preference );
    /**
     * Creates a device of the preferred backend with no physical device loaded yet, see DenOfIz_LogicalDevice_LoadPhysicalDevice. A NULL
     * desc is a zeroed one. NULL when the backend is not in this build. The caller destroys it with DenOfIz_LogicalDevice_Destroy.
     */
    DZ_API DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc );
    DZ_API void                  DenOfIz_GraphicsApi_LogDeviceCapabilities( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_PhysicalDevice *gpuDesc );
    /**
     * Like CreateLogicalDevice, then loads the best physical device: dedicated GPUs first, then by memory, ray tracing and mesh shader
     * support. NULL when no GPU could be loaded.
     */
    DZ_API DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc );
    /** A static string: "Vulkan", "DirectX12", "Metal" or "Unknown". */
    DZ_API void DenOfIz_GraphicsApi_ActiveAPI( DenOfIz_GraphicsApi graphicsApi, DenOfIz_StringView *outActiveAPI );
    /** Logs live DXGI objects on DX12, does nothing elsewhere. Also runs at exit once a DenOfIz_GraphicsApi was created. */
    DZ_API void DenOfIz_GraphicsApi_ReportLiveObjects( );
    /** Logical devices created from it are not destroyed with it. */
    DZ_API void DenOfIz_GraphicsApi_Destroy( DenOfIz_GraphicsApi graphicsApi );

#ifdef __cplusplus
}
#endif
