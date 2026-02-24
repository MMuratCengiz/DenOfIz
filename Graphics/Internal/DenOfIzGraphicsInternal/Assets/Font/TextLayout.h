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

#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"

namespace DenOfIz
{
    struct TextLayoutDesc
    {
        class Font *Font;
    };

    struct GlyphVertex
    {
        DenOfIz_Float2 Position{ };
        DenOfIz_Float2 UV{ };
        DenOfIz_Float4 Color{ };

        GlyphVertex( const DenOfIz_Float2 &position, const DenOfIz_Float2 &uv, const DenOfIz_Float4 &color ) : Position( position ), UV( uv ), Color( color )
        {
        }

        GlyphVertex( ) = default;
    };

    struct GlyphVertexArray
    {
        GlyphVertex *Elements;
        uint32_t     NumElements;
    };

    struct ShapeTextDesc
    {
        DenOfIz_StringView    Text;
        DenOfIz_UInt4         HbScriptTag{ 'L', 'a', 't', 'n' };
        DenOfIz_TextDirection Direction = DENOFIZ_TEXT_DIRECTION_LEFT_TO_RIGHT;
        uint32_t              FontSize  = 36;
    };

    struct GenerateTextVerticesDesc
    {
        DenOfIz_Float2 StartPosition{ 0.0f, 0.0f };
        DenOfIz_Float4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        GlyphVertex *OutVertices;
        uint32_t    *OutIndices;
        uint32_t     BaseVertexIndex = 0;
        uint32_t     BaseIndexOffset = 0;
        float        Scale           = 1.0f;
        uint16_t     LetterSpacing   = 0;
        uint16_t     LineHeight      = 0;
    };

    struct TextVertexAllocationInfo
    {
        uint32_t VertexCount = 0;
        uint32_t IndexCount  = 0;
    };

    class TextLayout
    {
        Font          *m_font;
        TextLayoutDesc m_desc;

        struct GlyphAdvance
        {
            uint32_t CodePoint = 0;
            float    XOffset   = 0;
            float    YOffset   = 0;
            float    XAdvance  = 0;
            float    YAdvance  = 0;
        };

        std::vector<GlyphAdvance> m_shapedGlyphs;
        float                     m_totalWidth  = 0;
        float                     m_totalHeight = 0;

        std::string           m_lastShapedText;
        uint32_t              m_lastFontSize  = 0;
        DenOfIz_TextDirection m_lastDirection = DENOFIZ_TEXT_DIRECTION_AUTO;
        DenOfIz_UInt4         m_lastScriptTag{ 0, 0, 0, 0 };

    public:
        DZ_API explicit TextLayout( const TextLayoutDesc &desc );
        DZ_API void                     SetFont( Font *font );
        DZ_API Font                    *GetFont( ) const;
        DZ_API void                     ShapeText( const ShapeTextDesc &shapeDesc );
        DZ_API void                     GenerateTextVertices( const GenerateTextVerticesDesc &generateDesc ) const;
        DZ_API TextVertexAllocationInfo GetVertexAllocationInfo( ) const;

        DZ_API DenOfIz_Float2 GetTextSize( ) const;
        DZ_API float          GetTextWidth( ) const;
        DZ_API float          GetTextHeight( ) const;

    private:
        static std::u32string Utf8ToUtf32( const std::string &utf8Text );
    };
} // namespace DenOfIz
