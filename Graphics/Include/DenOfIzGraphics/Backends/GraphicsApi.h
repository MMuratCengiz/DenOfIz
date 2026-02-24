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
        DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN,
        DENOFIZ_API_PREFERENCE_WINDOWS_WEBGPU_NATIVE
    } DenOfIz_APIPreferenceWindows;

    typedef enum DenOfIz_APIPreferenceOSX
    {
        DENOFIZ_API_PREFERENCE_OSX_METAL,
        DENOFIZ_API_PREFERENCE_OSX_VULKAN,
        DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE
    } DenOfIz_APIPreferenceOSX;

    typedef enum DenOfIz_APIPreferenceLinux
    {
        DENOFIZ_API_PREFERENCE_LINUX_VULKAN,
        DENOFIZ_API_PREFERENCE_LINUX_WEBGPU_NATIVE
    } DenOfIz_APIPreferenceLinux;

    typedef enum DenOfIz_APIPreferenceWeb
    {
        DENOFIZ_API_PREFERENCE_WEB_WEBGPU
    } DenOfIz_APIPreferenceWeb;

    typedef struct DenOfIz_APIPreference
    {
        DenOfIz_APIPreferenceWindows Windows;
        DenOfIz_APIPreferenceOSX     OSX;
        DenOfIz_APIPreferenceLinux   Linux;
        DenOfIz_APIPreferenceWeb     Web;
    } DenOfIz_APIPreference;

    DZ_API DenOfIz_GraphicsApi   DenOfIz_GraphicsApi_Create( const DenOfIz_APIPreference *preference );
    DZ_API DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc );
    DZ_API void                  DenOfIz_GraphicsApi_LogDeviceCapabilities( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_PhysicalDevice *gpuDesc );
    DZ_API DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc );
    DZ_API void                  DenOfIz_GraphicsApi_ActiveAPI( DenOfIz_GraphicsApi graphicsApi, DenOfIz_StringView *outActiveAPI );
    DZ_API void                  DenOfIz_GraphicsApi_ReportLiveObjects( );
    DZ_API void                  DenOfIz_GraphicsApi_Destroy( DenOfIz_GraphicsApi graphicsApi );

#ifdef __cplusplus
}
#endif
