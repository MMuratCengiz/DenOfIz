/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_TextureImportDesc
    {
        DenOfIz_StringView SourceFilePath;
        DenOfIz_StringView TargetDirectory;
        DenOfIz_StringView AssetNamePrefix;
        bool               GenerateMips;
        bool               NormalizeNormalMaps;
        bool               FlipY;
    } DenOfIz_TextureImportDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextureImporter )

    DZ_API DenOfIz_TextureImporter DenOfIz_TextureImporter_Create( );
    DZ_API void                    DenOfIz_TextureImporter_Destroy( DenOfIz_TextureImporter importer );
    DZ_API DenOfIz_StringView      DenOfIz_TextureImporter_GetName( DenOfIz_TextureImporter importer );
    DZ_API DenOfIz_StringViewArray DenOfIz_TextureImporter_GetSupportedExtensions( DenOfIz_TextureImporter importer );
    DZ_API bool                    DenOfIz_TextureImporter_CanProcessFileExtension( DenOfIz_TextureImporter importer, DenOfIz_StringView extension );
    DZ_API bool                    DenOfIz_TextureImporter_ValidateFile( DenOfIz_TextureImporter importer, DenOfIz_StringView filePath );
    DZ_API DenOfIz_ImporterResult  DenOfIz_TextureImporter_Import( DenOfIz_TextureImporter importer, const DenOfIz_TextureImportDesc *desc );

#ifdef __cplusplus
}
#endif
