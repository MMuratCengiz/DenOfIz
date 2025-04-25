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

#include <DenOfIzGraphics/Utilities/Interop.h>
#include <freetype/freetype.h>
#include <unordered_map>

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"

namespace DenOfIz
{
    // This is pretty much a FreeType container
    struct FontDesc
    {
        // Specify either FontPath or FontData
        FontAsset *FontAsset;
    };

    // This class is generally not DZ_API friendly due to heavily relying on 3rd party libraries
    class Font
    {
        FT_Library m_ftLibrary{ };
        FT_Face    m_face{ };
        FontDesc   m_desc;
        std::unordered_map<uint32_t, FontGlyph> m_glyphs;

        friend class FontLibrary;
        friend class TextLayout;

        Font( FT_Library library, const FontDesc &desc );
        [[nodiscard]] FT_Face FTFace( ) const;

    public:
        [[nodiscard]] FontAsset *Asset( ) const;
        ~Font( );
        FontGlyph *GetGlyph( uint32_t codePoint );
    };
} // namespace DenOfIz
