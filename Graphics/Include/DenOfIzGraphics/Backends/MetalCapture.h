/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Starts capturing the device's GPU work into a timestamped .gputrace file in the system temp directory, for Xcode. Requires
     * DenOfIz_LogicalDeviceDesc::EnableValidationLayers. Returns false when the capture did not start, always on non Metal builds.
     */
    DZ_API bool DenOfIz_Metal_BeginGpuCapture( DenOfIz_LogicalDevice device );
    /** Stops the capture and logs the file path. */
    DZ_API void DenOfIz_Metal_EndGpuCapture( DenOfIz_LogicalDevice device );
    DZ_API bool DenOfIz_Metal_IsCapturing( DenOfIz_LogicalDevice device );

#ifdef __cplusplus
}
#endif
