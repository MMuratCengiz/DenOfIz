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
