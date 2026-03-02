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
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/AudioData.h"
#include "DenOfIzGraphics/Input/AudioMixer.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_AudioSource )

    DZ_API DenOfIz_AudioSource DenOfIz_AudioSource_LoadFromFile( DenOfIz_AudioMixer mixer,
                                                                  DenOfIz_StringView filePath,
                                                                  bool predecode );
    DZ_API void                DenOfIz_AudioSource_Destroy( DenOfIz_AudioSource source );

    DZ_API int64_t            DenOfIz_AudioSource_GetDuration( DenOfIz_AudioSource source );
    DZ_API DenOfIz_AudioSpec  DenOfIz_AudioSource_GetFormat( DenOfIz_AudioSource source );

    DZ_API DenOfIz_StringView DenOfIz_AudioSource_GetTitle( DenOfIz_AudioSource source );
    DZ_API DenOfIz_StringView DenOfIz_AudioSource_GetArtist( DenOfIz_AudioSource source );
    DZ_API DenOfIz_StringView DenOfIz_AudioSource_GetAlbum( DenOfIz_AudioSource source );

#ifdef __cplusplus
}
#endif
