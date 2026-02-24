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
