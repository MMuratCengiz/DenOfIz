/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderCompiler )

    typedef struct DenOfIz_CompileDesc
    {
        DenOfIz_StringView           Path;
        DenOfIz_CodePage             CodePage;
        DenOfIz_ByteArray            Data;
        DenOfIz_StringView           EntryPoint;
        DenOfIz_ShaderStageFlags     Stage;
        DenOfIz_TargetIL             TargetIL;
        DenOfIz_StringViewArray      Defines;
        bool                         SupportNonZeroBaseInstance;
        DenOfIz_ShaderIncludeHandler IncludeHandler;
    } DenOfIz_CompileDesc;

    typedef struct DenOfIz_CompileResult
    {
        DenOfIz_ByteArray Code;
        DenOfIz_ByteArray Reflection;
    } DenOfIz_CompileResult;

#define DENOFIZ_VK_SHIFT_CBV 100
#define DENOFIZ_VK_SHIFT_SRV 200
#define DENOFIZ_VK_SHIFT_UAV 300
#define DENOFIZ_VK_SHIFT_SAMPLER 400

    DZ_API DenOfIz_ShaderCompiler DenOfIz_ShaderCompiler_Create( );
    DZ_API void                   DenOfIz_ShaderCompiler_CompileHLSL( DenOfIz_ShaderCompiler compiler, const DenOfIz_CompileDesc *compileDesc, DenOfIz_CompileResult *outResult );
    /** The compiler diagnostics of the last compilation on the calling thread, empty when it succeeded. Valid until the next compilation on that thread. */
    DZ_API DenOfIz_StringView     DenOfIz_ShaderCompiler_GetLastError( );
    DZ_API void                   DenOfIz_ShaderCompiler_Destroy( DenOfIz_ShaderCompiler compiler );

#ifdef __cplusplus
}
#endif
