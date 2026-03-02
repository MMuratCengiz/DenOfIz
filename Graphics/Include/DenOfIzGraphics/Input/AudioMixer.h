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
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_AudioMixer )

    DZ_API DenOfIz_AudioMixer  DenOfIz_AudioMixer_Create( const DenOfIz_AudioSpec *spec );
    DZ_API void                DenOfIz_AudioMixer_Destroy( DenOfIz_AudioMixer mixer );

    DZ_API void  DenOfIz_AudioMixer_SetGain( DenOfIz_AudioMixer mixer, float gain );
    DZ_API float DenOfIz_AudioMixer_GetGain( DenOfIz_AudioMixer mixer );

    DZ_API void DenOfIz_AudioMixer_StopAll( DenOfIz_AudioMixer mixer, int32_t fadeOutMs );
    DZ_API void DenOfIz_AudioMixer_PauseAll( DenOfIz_AudioMixer mixer );
    DZ_API void DenOfIz_AudioMixer_ResumeAll( DenOfIz_AudioMixer mixer );

    DZ_API int32_t             DenOfIz_AudioMixer_GetNumDecoders( );
    DZ_API DenOfIz_StringView  DenOfIz_AudioMixer_GetDecoder( int32_t index );

#ifdef __cplusplus
}
#endif
