/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdbool.h>
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DZ_API void DenOfIz_Clipboard_SetText( DenOfIz_StringView text );
    /** The text stays valid until the next GetText. */
    DZ_API DenOfIz_StringView DenOfIz_Clipboard_GetText( );
    DZ_API bool               DenOfIz_Clipboard_HasText( );
    DZ_API void               DenOfIz_Clipboard_Clear( );

#ifdef __cplusplus
}
#endif
