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

#include "DenOfIzGraphics/Animation/Ozz.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

/**
 * @file OzzExporterRuntime.h
 * @brief Runtime API for loading GLTF files and building ozz skeleton/animation data in memory.
 *
 * This API allows loading GLTF files and extracting skeleton/animation data as ozz binary format
 * in memory, without writing to disk. The returned BinaryContainer objects can be used directly
 * with OzzAnimation_CreateFromBinaryContainer and OzzAnimation_LoadAnimationFromBinaryContainer.
 */

typedef struct DenOfIz_OzzGltfDesc
{
    bool                    HasSkeleton;
    uint32_t                NumAnimations;
    DenOfIz_StringViewArray AnimationNames;
} DenOfIz_OzzGltfDesc;

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_OzzExporterRuntime )

    DZ_API DenOfIz_OzzExporterRuntime DenOfIz_OzzExporterRuntime_Create( );
    DZ_API void                       DenOfIz_OzzExporterRuntime_Destroy( DenOfIz_OzzExporterRuntime runtime );

    DZ_API bool DenOfIz_OzzExporterRuntime_Load( DenOfIz_OzzExporterRuntime runtime, DenOfIz_StringView gltfFilePath );
    DZ_API bool DenOfIz_OzzExporterRuntime_LoadFromMemory( DenOfIz_OzzExporterRuntime runtime, DenOfIz_ByteArrayView gltfBytes, DenOfIz_StringView basePath );

    DZ_API DenOfIz_OzzGltfDesc DenOfIz_OzzExporterRuntime_GetDesc( DenOfIz_OzzExporterRuntime runtime );

    DZ_API bool DenOfIz_OzzExporterRuntime_LoadReference( DenOfIz_OzzExporterRuntime runtime, DenOfIz_StringView referenceGltfPath );
    DZ_API bool DenOfIz_OzzExporterRuntime_LoadReferenceFromMemory( DenOfIz_OzzExporterRuntime runtime, DenOfIz_ByteArrayView gltfBytes, DenOfIz_StringView basePath );
    DZ_API void DenOfIz_OzzExporterRuntime_SetSanitizeTransforms( DenOfIz_OzzExporterRuntime runtime, bool sanitize );

    DZ_API DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildSkeleton( DenOfIz_OzzExporterRuntime runtime );
    DZ_API DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildAnimation( DenOfIz_OzzExporterRuntime runtime, DenOfIz_OzzSkeleton ozzAnimation, uint32_t animationIndex );
    DZ_API DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildAnimationByName( DenOfIz_OzzExporterRuntime runtime, DenOfIz_OzzSkeleton ozzAnimation, DenOfIz_StringView animationName );

#ifdef __cplusplus
}
#endif
