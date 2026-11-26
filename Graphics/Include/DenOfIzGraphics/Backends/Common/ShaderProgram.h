/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
     * The shader stages of one pipeline, compiled and reflected together: the reflection gives the input layout, bind group layouts, root
     * constants and local root signatures of the whole pipeline. Create copies the desc. Arrays and descs returned by the getters are owned
     * by the program and valid until it is destroyed.
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
