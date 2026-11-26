/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderIncludeHandler )

    DZ_API DenOfIz_ShaderIncludeHandler DenOfIz_ShaderIncludeHandler_Create( );
    DZ_API void DenOfIz_ShaderIncludeHandler_AddFile( DenOfIz_ShaderIncludeHandler handler, const DenOfIz_StringView *path, const DenOfIz_ByteArrayView *data );
    DZ_API void DenOfIz_ShaderIncludeHandler_Destroy( DenOfIz_ShaderIncludeHandler handler );

#ifdef __cplusplus
}
#endif
