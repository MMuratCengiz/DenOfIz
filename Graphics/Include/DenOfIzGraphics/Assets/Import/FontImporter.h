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
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_UnicodeRange
    {
        uint32_t Start;
        uint32_t End;
    } DenOfIz_UnicodeRange;

    typedef struct DenOfIz_UnicodeRangeArray
    {
        const DenOfIz_UnicodeRange *Elements;
        size_t                      NumElements;
    } DenOfIz_UnicodeRangeArray;

    typedef struct DenOfIz_FontImportDesc
    {
        DenOfIz_StringView        SourceFilePath;
        DenOfIz_StringView        TargetDirectory;
        DenOfIz_StringView        AssetNamePrefix;
        uint32_t                  InitialFontSize; // ie. 36
        uint32_t                  AtlasWidth;      // ie. 512
        uint32_t                  AtlasHeight;     // ie. 512
        DenOfIz_UnicodeRangeArray CustomRanges;
        DenOfIz_BinaryContainer   TargetContainer;
    } DenOfIz_FontImportDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FontImporter )

    DZ_API DenOfIz_FontImporter    DenOfIz_FontImporter_Create( );
    DZ_API void                    DenOfIz_FontImporter_Destroy( DenOfIz_FontImporter importer );
    DZ_API DenOfIz_StringView      DenOfIz_FontImporter_GetName( DenOfIz_FontImporter importer );
    DZ_API DenOfIz_StringViewArray DenOfIz_FontImporter_GetSupportedExtensions( DenOfIz_FontImporter importer );
    DZ_API bool                    DenOfIz_FontImporter_CanProcessFileExtension( DenOfIz_FontImporter importer, DenOfIz_StringView extension );
    DZ_API bool                    DenOfIz_FontImporter_ValidateFile( DenOfIz_FontImporter importer, DenOfIz_StringView filePath );
    DZ_API DenOfIz_ImporterResult  DenOfIz_FontImporter_Import( DenOfIz_FontImporter importer, const DenOfIz_FontImportDesc *desc );

#ifdef __cplusplus
}
#endif
