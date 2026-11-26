/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_GPUBufferView
    {
        DenOfIz_Buffer Buffer;
        size_t         NumBytes;
        size_t         Offset;
    } DenOfIz_GPUBufferView;

#ifdef __cplusplus
}
#endif
