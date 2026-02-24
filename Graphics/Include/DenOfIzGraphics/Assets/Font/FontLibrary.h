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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FontLibrary )

    DZ_API DenOfIz_FontLibrary DenOfIz_FontLibrary_Create( );
    DZ_API void                DenOfIz_FontLibrary_Destroy( DenOfIz_FontLibrary fontLibrary );
    DZ_API DenOfIz_Font        DenOfIz_FontLibrary_LoadFontFromDesc( DenOfIz_FontLibrary fontLibrary, const DenOfIz_FontDesc *desc );
#ifndef __EMSCRIPTEN__
    DZ_API DenOfIz_Font        DenOfIz_FontLibrary_LoadFontFromPath( DenOfIz_FontLibrary fontLibrary, DenOfIz_StringView ttfPath );
#endif

#ifdef __cplusplus
}
#endif
