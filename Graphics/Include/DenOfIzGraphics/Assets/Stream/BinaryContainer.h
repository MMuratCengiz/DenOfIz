/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_BinaryContainer )

    DZ_API DenOfIz_BinaryContainer DenOfIz_BinaryContainer_Create( );
    DZ_API void                    DenOfIz_BinaryContainer_Destroy( DenOfIz_BinaryContainer container );
    DZ_API DenOfIz_ByteArrayView   DenOfIz_BinaryContainer_GetData( DenOfIz_BinaryContainer container );

#ifdef __cplusplus
}
#endif
