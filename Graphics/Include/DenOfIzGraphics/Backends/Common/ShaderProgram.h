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

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief A collection of shaders that will be compiled and run at the same time, you can compile shaders one by one using ShaderCompiler, however most backends require root
     * signature for the whole pipeline, therefore, it is a good idea to compile them together.
     *
     * ShaderProgram also provides the ability to reflect the compiled shader code, you can use DenOfIz_ShaderProgramDesc to peek into the shader, or create
     * RootSignature/InputLayout objects.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderProgram )

    typedef struct DenOfIz_CompiledShader
    {
        DenOfIz_CompiledShaderStageArray Stages;
        DenOfIz_ShaderReflectDesc        ReflectDesc;
        DenOfIz_ShaderRayTracingDesc     RayTracing;
    } DenOfIz_CompiledShader;

    typedef struct DenOfIz_CompiledShaderArray
    {
        DenOfIz_CompiledShader *Elements;
        uint32_t                NumElements;
    } DenOfIz_CompiledShaderArray;

    DZ_API DenOfIz_ShaderProgram DenOfIz_ShaderProgram_Create( const DenOfIz_ShaderProgramDesc *desc );
    DZ_API DenOfIz_ShaderProgram DenOfIz_ShaderProgram_CreateFromAsset( DenOfIz_ShaderAssetReader reader );
    DZ_API void                  DenOfIz_ShaderProgram_CompiledShaders( DenOfIz_ShaderProgram shaderProgram, DenOfIz_CompiledShaderStageArray *outCompiledShaders );
    DZ_API void                  DenOfIz_ShaderProgram_Reflect( DenOfIz_ShaderProgram shaderProgram, DenOfIz_ShaderReflectDesc *outReflectDesc );
    DZ_API void                  DenOfIz_ShaderProgram_Desc( DenOfIz_ShaderProgram shaderProgram, DenOfIz_ShaderProgramDesc *outDesc );
    DZ_API void                  DenOfIz_ShaderProgram_Destroy( DenOfIz_ShaderProgram shaderProgram );

#ifdef __cplusplus
}
#endif
