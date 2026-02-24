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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Font )

    typedef struct DenOfIz_FontDesc
    {
        DenOfIz_FontAsset FontAsset;
    } DenOfIz_FontDesc;

#define DENOFIZ_FONT_MSDF_PIXEL_RANGE 6.0f

    DZ_API DenOfIz_FontAsset  DenOfIz_Font_Asset( DenOfIz_Font font );
    DZ_API DenOfIz_FontGlyph *DenOfIz_Font_GetGlyph( DenOfIz_Font font, uint32_t codePoint );
    DZ_API float              DenOfIz_Font_MsdfPixelRange( );

#ifdef __cplusplus
}
#endif
