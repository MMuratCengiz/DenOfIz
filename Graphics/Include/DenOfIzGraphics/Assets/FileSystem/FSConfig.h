/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_FSDesc
    {
        DenOfIz_StringView Name;
        DenOfIz_StringView AssetPath;
    } DenOfIz_FSDesc;

    DZ_API void               DenOfIz_FSConfig_InitDefaults( );
    DZ_API void               DenOfIz_FSConfig_Init( const DenOfIz_FSDesc *config );
    DZ_API DenOfIz_StringView DenOfIz_FSConfig_AssetPath( );
    DZ_API DenOfIz_StringView DenOfIz_FSConfig_BundleResourcePath( );

#ifdef __cplusplus
}
#endif
