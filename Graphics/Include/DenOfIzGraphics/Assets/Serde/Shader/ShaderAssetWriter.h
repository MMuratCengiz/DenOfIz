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
