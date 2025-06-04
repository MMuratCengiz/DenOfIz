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

#include <memory>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

// Forward declarations for FreeType and HarfBuzz types
struct FT_FaceRec_;
typedef struct FT_FaceRec_ *FT_Face;
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_ *FT_Library;
struct hb_font_t;

namespace DenOfIz
{
    struct FontImpl;
    struct DZ_API FontDesc
    {
        FontAsset *FontAsset;
    };

    class DZ_API Font
    {
        std::unique_ptr<FontImpl>               m_impl;
        FontDesc                                m_desc;
        std::unordered_map<uint32_t, FontGlyph> m_glyphs;

        friend class FontLibrary;
        friend class TextLayout;

        Font( FT_Library ftLibrary, const FontDesc &desc );
        [[nodiscard]] FT_Face    GetFTFace( ) const;
        [[nodiscard]] hb_font_t *GetHBFont( ) const;

    public:
        static constexpr float MsdfPixelRange = 12.0f;

        [[nodiscard]] FontAsset *Asset( ) const;
        ~Font( );
        FontGlyph *GetGlyph( uint32_t codePoint );
    };
} // namespace DenOfIz
