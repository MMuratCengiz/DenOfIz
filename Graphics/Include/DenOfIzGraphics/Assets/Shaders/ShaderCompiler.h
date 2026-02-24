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

#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderCompiler )

    typedef struct
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

    typedef struct
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
    DZ_API void                   DenOfIz_ShaderCompiler_Destroy( DenOfIz_ShaderCompiler compiler );

#ifdef __cplusplus
}
#endif
