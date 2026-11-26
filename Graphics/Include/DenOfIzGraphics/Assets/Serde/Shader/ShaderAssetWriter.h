/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderAssetWriter )

    typedef struct DenOfIz_ShaderAssetWriterDesc
    {
        DenOfIz_BinaryWriter Writer;
    } DenOfIz_ShaderAssetWriterDesc;

    DZ_API DenOfIz_ShaderAssetWriter DenOfIz_ShaderAssetWriter_Create( const DenOfIz_ShaderAssetWriterDesc *desc );
    DZ_API void                      DenOfIz_ShaderAssetWriter_Destroy( DenOfIz_ShaderAssetWriter writer );
    DZ_API void                      DenOfIz_ShaderAssetWriter_Write( DenOfIz_ShaderAssetWriter writer, DenOfIz_ShaderAsset shaderAsset );
    DZ_API void                      DenOfIz_ShaderAssetWriter_End( DenOfIz_ShaderAssetWriter writer );
    DZ_API DenOfIz_ShaderAsset       DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( const DenOfIz_CompiledShader *compiledShader );
    DZ_API size_t                    DenOfIz_ShaderAssetWriter_NumRequiredArenaBytes( const DenOfIz_CompiledShader *compiledShader );

#ifdef __cplusplus
}
#endif
