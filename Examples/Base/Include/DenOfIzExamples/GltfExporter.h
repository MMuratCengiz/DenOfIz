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

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

typedef enum DenOfIz_GltfExportFormat
{
    DENOFIZ_GLTF_EXPORT_FORMAT_GLB,
    DENOFIZ_GLTF_EXPORT_FORMAT_GLTF_BIN
} DenOfIz_GltfExportFormat;

typedef enum DenOfIz_GltfExportResultCode
{
    DENOFIZ_GLTF_EXPORT_SUCCESS,
    DENOFIZ_GLTF_EXPORT_FILE_NOT_FOUND,
    DENOFIZ_GLTF_EXPORT_UNSUPPORTED_FORMAT,
    DENOFIZ_GLTF_EXPORT_IMPORT_FAILED,
    DENOFIZ_GLTF_EXPORT_WRITE_FAILED,
    DENOFIZ_GLTF_EXPORT_INVALID_PARAMETERS,
    DENOFIZ_GLTF_EXPORT_RESOURCE_UNAVAILABLE
} DenOfIz_GltfExportResultCode;

/**
 * @brief GLTF Export configuration.
 *
 * Output Specification:
 *   - Always outputs conformant GLTF/GLB (right-handed, Y-up, column-major)
 *   - No coordinate system conversion options - use GltfLoader for RH->LH conversion on load
 */
typedef struct DenOfIz_GltfExportDesc
{
    DenOfIz_StringView SourceFilePath;
    DenOfIz_StringView TargetDirectory;
    DenOfIz_StringView AssetNamePrefix;

    DenOfIz_GltfExportFormat OutputFormat;

    bool     EmbedTextures;
    bool     OverwriteExisting;
    bool     OptimizeMeshes;
    float    ScaleFactor;
    bool     JoinIdenticalVertices;
    bool     PreTransformVertices;
    bool     LimitBoneWeights;
    uint32_t MaxBoneWeightsPerVertex;
    bool     RemoveRedundantMaterials;
    bool     MergeMeshes;
    bool     OptimizeGraph;
    bool     GenerateNormals;
    bool     SmoothNormals;
    float    SmoothNormalsAngle;
    bool     TriangulateMeshes;
    bool     PreservePivots;
    bool     DropNormals;
    bool     CalculateTangentSpace;
    bool     FixInfacingNormals;
} DenOfIz_GltfExportDesc;

typedef struct DenOfIz_GltfExportResult
{
    DenOfIz_GltfExportResultCode ResultCode;
    DenOfIz_StringView           ErrorMessage;
    DenOfIz_StringView           GltfFilePath;
} DenOfIz_GltfExportResult;

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_GltfExporter )

    DZ_API DenOfIz_GltfExporter     DenOfIz_GltfExporter_Create( );
    DZ_API void                     DenOfIz_GltfExporter_Destroy( DenOfIz_GltfExporter exporter );
    DZ_API DenOfIz_StringView       DenOfIz_GltfExporter_GetName( DenOfIz_GltfExporter exporter );
    DZ_API DenOfIz_StringViewArray  DenOfIz_GltfExporter_GetSupportedExtensions( DenOfIz_GltfExporter exporter );
    DZ_API bool                     DenOfIz_GltfExporter_CanProcessFileExtension( DenOfIz_GltfExporter exporter, DenOfIz_StringView extension );
    DZ_API bool                     DenOfIz_GltfExporter_ValidateFile( DenOfIz_GltfExporter exporter, DenOfIz_StringView filePath );
    DZ_API DenOfIz_GltfExportResult DenOfIz_GltfExporter_Export( DenOfIz_GltfExporter exporter, const DenOfIz_GltfExportDesc *desc );
    DZ_API void                     DenOfIz_GltfExportResult_Destroy( DenOfIz_GltfExportResult *result );

#ifdef __cplusplus
}
#endif
