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

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Begins capturing Metal GPU workload for debugging.
     *
     * Starts a GPU trace capture that can be opened in Xcode. The .gputrace file
     * is saved to the system temp directory with a timestamped filename.
     *
     * This requires EnableValidationLayers=true in DenOfIz_LogicalDeviceDesc.
     * On non-Metal backends, this is a no-op and returns false.
     *
     * @param device Valid logical device handle.
     * @return true if capture started successfully, false otherwise.
     *
     * @see DenOfIz_Metal_EndGpuCapture
     * @see DenOfIz_Metal_IsCapturing
     */
    DZ_API bool DenOfIz_Metal_BeginGpuCapture( DenOfIz_LogicalDevice device );

    /**
     * @brief Ends an active Metal GPU capture.
     *
     * Stops the current GPU trace capture and finalizes the .gputrace file.
     * The capture file path is logged when the capture ends.
     *
     * On non-Metal backends, this is a no-op.
     *
     * @param device Valid logical device handle.
     *
     * @see DenOfIz_Metal_BeginGpuCapture
     */
    DZ_API void DenOfIz_Metal_EndGpuCapture( DenOfIz_LogicalDevice device );

    /**
     * @brief Checks if a Metal GPU capture is currently in progress.
     *
     * @param device Valid logical device handle.
     * @return true if capture is active, false otherwise.
     *
     * @see DenOfIz_Metal_BeginGpuCapture
     */
    DZ_API bool DenOfIz_Metal_IsCapturing( DenOfIz_LogicalDevice device );

#ifdef __cplusplus
}
#endif
