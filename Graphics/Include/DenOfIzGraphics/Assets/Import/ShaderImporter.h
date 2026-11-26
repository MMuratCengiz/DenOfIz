/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
