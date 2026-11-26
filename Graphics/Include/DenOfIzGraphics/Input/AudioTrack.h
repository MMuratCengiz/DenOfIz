/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/AudioMixer.h"
#include "DenOfIzGraphics/Input/AudioSource.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_AudioPlayDesc
    {
        int32_t Loops;
        int32_t FadeInMs;
        int64_t StartMs;
        int64_t MaxDurationMs;
    } DenOfIz_AudioPlayDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_AudioTrack )

    DZ_API DenOfIz_AudioTrack DenOfIz_AudioTrack_Create( DenOfIz_AudioMixer mixer );
    DZ_API void               DenOfIz_AudioTrack_Destroy( DenOfIz_AudioTrack track );

    DZ_API bool DenOfIz_AudioTrack_SetAudio( DenOfIz_AudioTrack track, DenOfIz_AudioSource source );

    DZ_API bool DenOfIz_AudioTrack_Play( DenOfIz_AudioTrack track, const DenOfIz_AudioPlayDesc *desc );
    DZ_API bool DenOfIz_AudioTrack_Stop( DenOfIz_AudioTrack track, int32_t fadeOutMs );
    DZ_API bool DenOfIz_AudioTrack_Pause( DenOfIz_AudioTrack track );
    DZ_API bool DenOfIz_AudioTrack_Resume( DenOfIz_AudioTrack track );

    DZ_API bool DenOfIz_AudioTrack_IsPlaying( DenOfIz_AudioTrack track );
    DZ_API bool DenOfIz_AudioTrack_IsPaused( DenOfIz_AudioTrack track );

    DZ_API void  DenOfIz_AudioTrack_SetGain( DenOfIz_AudioTrack track, float gain );
    DZ_API float DenOfIz_AudioTrack_GetGain( DenOfIz_AudioTrack track );

    DZ_API void    DenOfIz_AudioTrack_SetLoops( DenOfIz_AudioTrack track, int32_t loops );
    DZ_API int32_t DenOfIz_AudioTrack_GetLoops( DenOfIz_AudioTrack track );

    DZ_API bool    DenOfIz_AudioTrack_SetPositionMs( DenOfIz_AudioTrack track, int64_t ms );
    DZ_API int64_t DenOfIz_AudioTrack_GetPositionMs( DenOfIz_AudioTrack track );
    DZ_API int64_t DenOfIz_AudioTrack_GetRemainingMs( DenOfIz_AudioTrack track );

    DZ_API bool DenOfIz_AudioTrack_Tag( DenOfIz_AudioTrack track, DenOfIz_StringView tag );
    DZ_API void DenOfIz_AudioTrack_Untag( DenOfIz_AudioTrack track, DenOfIz_StringView tag );

#ifdef __cplusplus
}
#endif
