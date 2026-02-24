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
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_ShaderImportDesc
    {
        DenOfIz_StringView        TargetDirectory;
        DenOfIz_ShaderProgramDesc ProgramDesc;
        DenOfIz_StringView        OutputShaderName;
    } DenOfIz_ShaderImportDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderImporter )

    DZ_API DenOfIz_ShaderImporter  DenOfIz_ShaderImporter_Create( );
    DZ_API void                    DenOfIz_ShaderImporter_Destroy( DenOfIz_ShaderImporter importer );
    DZ_API DenOfIz_StringView      DenOfIz_ShaderImporter_GetName( DenOfIz_ShaderImporter importer );
    DZ_API DenOfIz_StringViewArray DenOfIz_ShaderImporter_GetSupportedExtensions( DenOfIz_ShaderImporter importer );
    DZ_API bool                    DenOfIz_ShaderImporter_CanProcessFileExtension( DenOfIz_ShaderImporter importer, DenOfIz_StringView extension );
    DZ_API bool                    DenOfIz_ShaderImporter_ValidateFile( DenOfIz_ShaderImporter importer, DenOfIz_StringView filePath );
    DZ_API DenOfIz_ImporterResult  DenOfIz_ShaderImporter_Import( DenOfIz_ShaderImporter importer, const DenOfIz_ShaderImportDesc *desc );

#ifdef __cplusplus
}
#endif
