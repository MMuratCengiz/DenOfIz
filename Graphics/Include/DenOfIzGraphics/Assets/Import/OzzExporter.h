/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Animation/Ozz.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

/** Exports glTF skeletons and animations to ozz files. Strings of an export result live until DenOfIz_OzzExportResult_Destroy. */

typedef enum
{
    DENOFIZ_OZZ_EXPORT_SUCCESS,
    DENOFIZ_OZZ_EXPORT_FILE_NOT_FOUND,
    DENOFIZ_OZZ_EXPORT_PARSE_ERROR,
    DENOFIZ_OZZ_EXPORT_NO_SKELETON,
    DENOFIZ_OZZ_EXPORT_NO_ANIMATIONS,
    DENOFIZ_OZZ_EXPORT_WRITE_FAILED,
    DENOFIZ_OZZ_EXPORT_INVALID_PARAMETERS
} DenOfIz_OzzExportResultCode;

typedef struct DenOfIz_OzzExportDesc
{
    DenOfIz_StringView    GltfSourcePath;
    DenOfIz_ByteArrayView GltfSourceData;
    DenOfIz_StringView    GltfSourceBasePath;
    DenOfIz_StringView    OutputDirectory;
    DenOfIz_StringView    AssetNamePrefix;
    DenOfIz_OzzSkeleton   ExternalSkeleton;
    DenOfIz_StringView    ReferenceGltfPath;
    DenOfIz_ByteArrayView ReferenceGltfData;
    DenOfIz_StringView    ReferenceGltfBasePath;
    bool                  ExportSkeleton;
    bool                  ExportAnimations;
    bool                  OverwriteExisting;
    bool                  SanitizeTransforms;
} DenOfIz_OzzExportDesc;

typedef struct DenOfIz_OzzExportResult
{
    DenOfIz_OzzExportResultCode ResultCode;
    DenOfIz_StringView          ErrorMessage;
    DenOfIz_StringView          SkeletonFilePath;
    DenOfIz_StringViewArray     AnimationFilePaths;
} DenOfIz_OzzExportResult;

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_OzzExporter )

    DZ_API DenOfIz_OzzExporter     DenOfIz_OzzExporter_Create( );
    DZ_API void                    DenOfIz_OzzExporter_Destroy( DenOfIz_OzzExporter exporter );
    DZ_API bool                    DenOfIz_OzzExporter_ValidateGltf( DenOfIz_OzzExporter exporter, DenOfIz_StringView filePath );
    DZ_API bool                    DenOfIz_OzzExporter_ValidateGltfFromMemory( DenOfIz_OzzExporter exporter, DenOfIz_ByteArrayView gltfData );
    DZ_API DenOfIz_OzzExportResult DenOfIz_OzzExporter_Export( DenOfIz_OzzExporter exporter, const DenOfIz_OzzExportDesc *desc );
    DZ_API void                    DenOfIz_OzzExportResult_Destroy( DenOfIz_OzzExportResult *result );

#ifdef __cplusplus
}
#endif
