/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Backends/MetalCapture.h"

extern "C"
{

    bool DenOfIz_Metal_BeginGpuCapture( DenOfIz_LogicalDevice device )
    {
        (void)device;
        return false;
    }

    void DenOfIz_Metal_EndGpuCapture( DenOfIz_LogicalDevice device )
    {
        (void)device;
    }

    bool DenOfIz_Metal_IsCapturing( DenOfIz_LogicalDevice device )
    {
        (void)device;
        return false;
    }
}
