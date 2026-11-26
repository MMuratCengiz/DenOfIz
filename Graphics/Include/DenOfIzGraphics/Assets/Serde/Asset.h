/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdint.h>
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_AssetHeader
    {
        uint64_t           Magic;
        uint32_t           Version;
        uint64_t           NumBytes;
        DenOfIz_StringView Path;
    } DenOfIz_AssetHeader;

    typedef struct DenOfIz_AssetDataStream
    {
        uint64_t Offset;
        uint64_t NumBytes;
    } DenOfIz_AssetDataStream;

#ifdef __cplusplus
}
#endif
