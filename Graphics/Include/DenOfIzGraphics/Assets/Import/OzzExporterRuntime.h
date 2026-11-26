/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Animation/Ozz.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

/**
 * Builds ozz skeletons and animations from glTF in memory, without writing files. The Build functions return a new container the caller
 * destroys with DenOfIz_BinaryContainer_Destroy, ready for DenOfIz_Ozz_CreateSkeletonFromBinaryContainer and
 * DenOfIz_Ozz_LoadAnimationFromBinaryContainer, or NULL on failure.
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
