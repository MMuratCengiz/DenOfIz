/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstdint>
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DZ_API float DenOfIz_OS_GetDisplayScale( int display );
    DZ_API float DenOfIz_OS_GetWindowSizeScaleFactor( int display );

#ifdef __cplusplus
}
#endif
