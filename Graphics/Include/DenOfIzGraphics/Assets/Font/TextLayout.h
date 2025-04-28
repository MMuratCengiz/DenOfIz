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
#include <vector>

#include "Font.h"

namespace DenOfIz
{
    struct DZ_API TextLayoutDesc
    {
        Font *Font;
    };

    struct DZ_API GlyphVertex
    {
        Float_2 Position{ };
        Float_2 UV{ };
        Float_4 Color{ };

        GlyphVertex( const Float_2 &position, const Float_2 &uv, const Float_4 &color ) : Position( position ), UV( uv ), Color( color )
        {
        }

        GlyphVertex( ) = default;
    };

    enum class TextDirection
    {
        LeftToRight,
        RightToLeft,
        Auto
    };

    struct DZ_API ShapeTextDesc
    {
        InteropString Text;                              // UTF-8 encoded string
        UInt32_4      HbScriptTag{ 'L', 'a', 't', 'n' }; // Language identifier, refer to hb_script_t
        TextDirection Direction;
        uint32_t      FontSize = 36;
    };

    struct DZ_API GenerateTextVerticesDesc
    {
        Float_2 StartPosition{ 0.0f, 0.0f };
        Float_4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        InteropArray<GlyphVertex> *OutVertices;
        InteropArray<uint32_t>    *OutIndices;
        float                      Scale = 1.5f;
    };
    class DZ_API TextLayout
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
        InteropString             m_lastShapedText;

    public:
        explicit TextLayout( const TextLayoutDesc &desc );
        void SetFont( Font *font );
        void ShapeText( const ShapeTextDesc &shapeDesc );
        void GenerateTextVertices( const GenerateTextVerticesDesc &generateDesc ) const;

    private:
        static std::u32string Utf8ToUtf32( const std::string &utf8Text );
    };
} // namespace DenOfIz
