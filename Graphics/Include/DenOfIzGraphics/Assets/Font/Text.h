/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Byte offset of the grapheme cluster boundary after or before byteOffset in UTF-8 text: the steps of a caret and what Backspace and Delete remove. */
    DZ_API uint32_t DenOfIz_Text_NextGrapheme( DenOfIz_StringView text, uint32_t byteOffset );
    DZ_API uint32_t DenOfIz_Text_PreviousGrapheme( DenOfIz_StringView text, uint32_t byteOffset );

#ifdef __cplusplus
}
#endif
