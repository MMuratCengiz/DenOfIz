/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        DENOFIZ_AUDIO_FORMAT_UNKNOWN = 0,
        DENOFIZ_AUDIO_FORMAT_U8,
        DENOFIZ_AUDIO_FORMAT_S8,
        DENOFIZ_AUDIO_FORMAT_S16,
        DENOFIZ_AUDIO_FORMAT_S32,
        DENOFIZ_AUDIO_FORMAT_F32
    } DenOfIz_AudioFormat;

    typedef struct DenOfIz_AudioSpec
    {
        DenOfIz_AudioFormat Format;
        int32_t             Channels;
        int32_t             Freq;
    } DenOfIz_AudioSpec;

#ifdef __cplusplus
}
#endif
