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

#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Font/FontCache.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAsset.h>
#include <DenOfIzGraphics/Utilities/Common.h>

#include <freetype/freetype.h>
#include <hb.h>

namespace msdfgen
{
    class FreetypeHandle;
    class FontHandle;
    class Shape;
} // namespace msdfgen

using namespace DirectX;

namespace DenOfIz
{
    struct DZ_API ShapedGlyphBounds
    {
        float XMin;
        float YMin;
        float XMax;
        float YMax;
    };

    struct ShapedGlyph
    {
        uint32_t          GlyphId;
        uint32_t          ClusterIndex;
        float             XOffset;
        float             YOffset;
        float             XAdvance;
        float             YAdvance;
        ShapedGlyphBounds Bounds;
        uint32_t          CodePoint;
    };
    template class DZ_API InteropArray<ShapedGlyph>;

    enum class TextDirection
    {
        LeftToRight,
        RightToLeft,
        Auto
    };

    enum class TextScript
    {
    };

    struct TextLayout
    {
        InteropArray<ShapedGlyph> ShapedGlyphs;
        float                     TotalWidth  = 0.0f;
        float                     TotalHeight = 0.0f;
    };

    struct ShapeTextDesc
    {
    };

    struct GenerateTextVerticesDesc
    {
        FontCache    *FontCache;
        TextLayout    Layout;
        InteropString Text;
        float         X;
        float         Y;
        Float_4       Color;
        float         Scale;
    };

    class FontManager
    {
        FT_Library                                                  m_ftLibrary{ };
        msdfgen::FreetypeHandle                                    *m_msdfFtHandle{ };
        std::unordered_map<std::string, std::shared_ptr<FontCache>> m_fontCache;
        std::unordered_map<std::string, hb_font_t *>                m_hbFonts;
        static constexpr float                                      MSDF_PIXEL_RANGE             = 4.0f;
        float                                                       m_msdfPixelRange             = MSDF_PIXEL_RANGE;
        float                                                       m_edgeColoringAngleThreshold = 3.0f;

    public:
        FontManager( );
        ~FontManager( );

        FontCache *LoadFont( const InteropString &fontPath, uint32_t pixelSize = 24, bool antiAliasing = true );
        FontCache *GetFont( const InteropString &fontPath, uint32_t pixelSize = 24 );
        TextLayout ShapeText( FontCache *fontCache, const InteropString &interopText, TextDirection direction = TextDirection::Auto );
        void       GenerateTextVertices( const GenerateTextVerticesDesc &desc, InteropArray<float> &outVertices, InteropArray<uint32_t> &outIndices );

    private:
        std::u32string Utf8ToUtf32( const std::string &utf8Text );
        bool           EnsureGlyphsLoaded( FontCache *fontCache, const InteropString &interopText );
        bool           LoadGlyph( FontCache *fontCache, uint32_t codePoint, FT_Face face );
        bool           GenerateMsdfForGlyph( AddGlyphDesc &glyphDesc, msdfgen::FontHandle *msdfFont, uint32_t codePoint, uint32_t pixelSize ) const;
        hb_font_t     *GetHarfBuzzFont( const std::string &fontPath, uint32_t numPixels, FT_Face face );
        void           DestroyHarfBuzzFont( const std::string &fontCacheKey );
    };

} // namespace DenOfIz
