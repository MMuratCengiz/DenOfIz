/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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
