# Compiling Shaders in DenOfIz

Shaders are written in HLSL for every backend and compiled with Microsoft's DirectX Shader Compiler (DXC): DXIL for DirectX 12, SPIR-V for Vulkan. Metal libraries are converted
from the DXIL with Apple's Metal Shader Converter, using the root signature the reflection describes.

## Shader Programs

A `DenOfIz_ShaderProgram` holds the stages of one pipeline, compiled and reflected together. Creating one compiles every stage to DXIL and SPIR-V, and to Metal on Windows x64 and
macOS builds:

```cpp
DenOfIz_StringView defines[] = { DENOFIZ_STRING( "USE_NORMAL_MAPPING" ), DENOFIZ_STRING( "MAX_LIGHTS=4" ) };

DenOfIz_ShaderStageDesc stages[ 2 ]{ };
stages[ 0 ].Stage      = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
stages[ 0 ].Path       = DENOFIZ_STRING( "Assets/Shaders/FullscreenQuad.vs.hlsl" );
stages[ 0 ].EntryPoint = DENOFIZ_STRING( "main" );
stages[ 1 ].Stage      = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
stages[ 1 ].Data       = DenOfIz_InteropUtilities_StringToBytes( pixelShaderSource ); // Source in memory instead of Path
stages[ 1 ].EntryPoint = DENOFIZ_STRING( "PSMain" );
stages[ 1 ].Defines    = { defines, 2 };

DenOfIz_ShaderProgramDesc programDesc{ };
programDesc.ShaderStages.Elements    = stages;
programDesc.ShaderStages.NumElements = 2;
DenOfIz_ShaderProgram program        = DenOfIz_ShaderProgram_Create( &programDesc );

DenOfIz_ShaderReflectDesc reflectDesc{ };
DenOfIz_ShaderProgram_Reflect( program, &reflectDesc );
```

The reflection gives the input layout, bind group layouts, root constants, ray tracing local root signatures and thread group sizes, see [Resource Binding](ResourceBinding.md).
`DenOfIz_ShaderProgramDesc::IncludeHandler` resolves `#include`s from memory: create it with `DenOfIz_ShaderIncludeHandler_Create` and add files with
`DenOfIz_ShaderIncludeHandler_AddFile`.

## Compiler Settings

- Shader model 6.6, ray tracing stages are compiled as libraries exporting only their entry point
- HLSL 2021 (`-HV 2021`), row major matrices (`-Zpr`), `-all_resources_bound`
- Debug builds embed debug information in DXIL (`-Zi`)
- SPIR-V targets Vulkan 1.1, and Vulkan 1.3 with the ray tracing and ray query extensions for ray tracing, mesh and hull stages
- SPIR-V uses the DirectX memory layout (`-fvk-use-dx-layout`) and `SV_Position.w` convention (`-fvk-use-dx-position-w`)
- Registers are shifted per type for Vulkan, see [Vulkan Register Shifts](ResourceBinding.md#vulkan-register-shifts)
- Shader programs pass `-fvk-support-nonzero-base-instance`: `SV_InstanceID` starts at 0 whatever the first instance of the draw is, as on DirectX 12

## Vulkan and Metal Notes

- Root constants need `[[vk::push_constant]]` and register space 31, one block per shader:
  ```hlsl
  [[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);
  ```
- Metal compiles every stage of a program against one root signature built from the program's reflection, so programs are converted as a whole.
- On Metal the shaders of AABBS hit groups, intersection shaders included, need `DenOfIz_RayTracingShaderDesc::HitGroupType` set to `DENOFIZ_HIT_GROUP_TYPE_AABBS`.

## Compiling Directly

`DenOfIz_ShaderCompiler` compiles one stage to DXIL or SPIR-V without a program. It cannot produce Metal, which needs the whole program:

```cpp
DenOfIz_ShaderCompiler compiler = DenOfIz_ShaderCompiler_Create( );

DenOfIz_CompileDesc compileDesc{ };
compileDesc.Path       = DENOFIZ_STRING( "Assets/Shaders/MyShader.ps.hlsl" );
compileDesc.Stage      = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
compileDesc.EntryPoint = DENOFIZ_STRING( "main" );
compileDesc.TargetIL   = DENOFIZ_TARGET_IL_SPIRV;

DenOfIz_CompileResult result{ };
DenOfIz_ShaderCompiler_CompileHLSL( compiler, &compileDesc, &result ); // result.Code, result.Reflection
DenOfIz_ShaderCompiler_Destroy( compiler );
```

## Precompiled Shaders

To skip compilation at run time, `DenOfIz_ShaderImporter_Import` compiles a `DenOfIz_ShaderProgramDesc` into a shader asset file in `TargetDirectory`. Load it with
`DenOfIz_BinaryReader_CreateFromFile`, `DenOfIz_ShaderAssetReader_Create` and `DenOfIz_ShaderProgram_CreateFromAsset`. Produce assets meant for Metal on Windows x64 or macOS,
other builds have no Metal Shader Converter.
