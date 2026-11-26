/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_StepTimer )

    DZ_API DenOfIz_StepTimer DenOfIz_StepTimer_Create( );
    DZ_API void              DenOfIz_StepTimer_Destroy( DenOfIz_StepTimer timer );

    DZ_API double   DenOfIz_StepTimer_GetDeltaTime( DenOfIz_StepTimer timer );
    DZ_API uint64_t DenOfIz_StepTimer_GetElapsedTicks( DenOfIz_StepTimer timer );
    DZ_API double   DenOfIz_StepTimer_GetElapsedSeconds( DenOfIz_StepTimer timer );
    DZ_API uint64_t DenOfIz_StepTimer_GetTotalTicks( DenOfIz_StepTimer timer );
    DZ_API double   DenOfIz_StepTimer_GetTotalSeconds( DenOfIz_StepTimer timer );
    DZ_API uint32_t DenOfIz_StepTimer_GetFrameCount( DenOfIz_StepTimer timer );
    DZ_API uint32_t DenOfIz_StepTimer_GetFramesPerSecond( DenOfIz_StepTimer timer );

    DZ_API void DenOfIz_StepTimer_SetFixedTimeStep( DenOfIz_StepTimer timer, bool isFixedTimestep );
    DZ_API void DenOfIz_StepTimer_SetTargetElapsedTicks( DenOfIz_StepTimer timer, uint64_t targetElapsed );
    DZ_API void DenOfIz_StepTimer_SetTargetElapsedSeconds( DenOfIz_StepTimer timer, double targetElapsed );

    DZ_API void DenOfIz_StepTimer_ResetElapsedTime( DenOfIz_StepTimer timer );
    DZ_API void DenOfIz_StepTimer_Tick( DenOfIz_StepTimer timer );

#ifdef __cplusplus
}
#endif
