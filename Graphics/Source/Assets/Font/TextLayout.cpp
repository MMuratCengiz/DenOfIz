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

#include "DenOfIzGraphics/Assets/Font/TextLayout.h"
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>
#include <unordered_map>
#include "DenOfIzGraphicsInternal/Assets/Font/FontImpl.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

TextLayout::TextLayout( const TextLayoutDesc &desc ) : m_desc( desc )
{
    m_font = desc.Font;
}

void TextLayout::SetFont( Font *font )
{
    m_font           = font;
    m_lastShapedText = "";
    m_lastFontSize   = 0;
    m_lastDirection  = TextDirection::Auto;
    m_lastScriptTag  = { 0, 0, 0, 0 };
    m_shapedGlyphs.clear( );
    m_totalWidth  = 0.0f;
    m_totalHeight = 0.0f;
}

Font *TextLayout::GetFont( ) const
{
    return m_font;
}

void TextLayout::ShapeText( const ShapeTextDesc &shapeDesc )
{
    if ( shapeDesc.Text.IsEmpty( ) || !m_font )
    {
        return;
    }

    if ( m_lastShapedText.Equals( shapeDesc.Text ) && m_lastFontSize == shapeDesc.FontSize && m_lastDirection == shapeDesc.Direction &&
         m_lastScriptTag.X == shapeDesc.HbScriptTag.X && m_lastScriptTag.Y == shapeDesc.HbScriptTag.Y && m_lastScriptTag.Z == shapeDesc.HbScriptTag.Z &&
         m_lastScriptTag.W == shapeDesc.HbScriptTag.W )
    {
        return;
    }

    m_lastShapedText = shapeDesc.Text;
    m_lastFontSize   = shapeDesc.FontSize;
    m_lastDirection  = shapeDesc.Direction;
    m_lastScriptTag  = shapeDesc.HbScriptTag;

    const std::string    utf8Text  = shapeDesc.Text.Get( );
    const std::u32string utf32Text = Utf8ToUtf32( utf8Text );
    hb_font_t           *hbFont    = m_font->GetHBFont( shapeDesc.FontSize );
    if ( !hbFont )
    {
        spdlog::error( "HarfBuzz font not available for size {}", shapeDesc.FontSize );
        return;
    }

    hb_buffer_t *buffer = hb_buffer_create( );
    if ( !hb_buffer_allocation_successful( buffer ) )
    {
        spdlog::error( "Failed to allocate HarfBuzz buffer" );
        return;
    }

    hb_buffer_reset( buffer );

    hb_direction_t hbDirection;
    switch ( shapeDesc.Direction )
    {
    case TextDirection::LeftToRight:
        hbDirection = HB_DIRECTION_LTR;
        break;
    case TextDirection::RightToLeft:
        hbDirection = HB_DIRECTION_RTL;
        break;
    case TextDirection::Auto:
    default:
        hbDirection = HB_DIRECTION_LTR;
        break;
    }

    const UInt32_4 scriptTag = shapeDesc.HbScriptTag;
    hb_buffer_set_direction( buffer, hbDirection );
    hb_buffer_set_script( buffer, static_cast<hb_script_t>( HB_TAG( scriptTag.X, scriptTag.Y, scriptTag.Z, scriptTag.W ) ) );
    hb_buffer_set_language( buffer, hb_language_from_string( "en", -1 ) );

    const char *text_ptr = utf8Text.c_str( );
    const int   text_len = utf8Text.length( );
    hb_buffer_add_utf8( buffer, text_ptr, text_len, 0, text_len );

    constexpr hb_feature_t features[] = {
        { HB_TAG( 'k', 'e', 'r', 'n' ), 1, 0, UINT_MAX }, // Enable kerning
        { HB_TAG( 'l', 'i', 'g', 'a' ), 1, 0, UINT_MAX }  // Enable standard ligatures
    };

    hb_shape( hbFont, buffer, features, std::size( features ) );

    unsigned int               glyphCount = 0;
    const hb_glyph_info_t     *glyphInfo  = hb_buffer_get_glyph_infos( buffer, &glyphCount );
    const hb_glyph_position_t *glyphPos   = hb_buffer_get_glyph_positions( buffer, &glyphCount );

    if ( !glyphPos || glyphCount == 0 )
    {
        spdlog::error( "No glyph positions returned from HarfBuzz shaping" );
        hb_buffer_destroy( buffer );
        return;
    }

    constexpr float posScale = 1.0f / 64.0f; // HarfBuzz units to pixels, HarfBuzz uses 26.6 fixed-point format

    m_shapedGlyphs.resize( glyphCount );
    float totalAdvance = 0.0f;

    std::unordered_map<uint32_t, uint32_t> glyphIndexToCodePoint;
    const FT_Face                          face = m_font->GetFTFace( );
    for ( const char32_t codePoint : utf32Text )
    {
        if ( FT_UInt glyphIndex = FT_Get_Char_Index( face, codePoint ) )
        {
            glyphIndexToCodePoint[ glyphIndex ] = codePoint;
        }
    }

    for ( unsigned int i = 0; i < glyphCount; ++i )
    {
        GlyphAdvance &glyphAdvance = m_shapedGlyphs[ i ];
        uint32_t      codepoint    = glyphInfo[ i ].codepoint;
        glyphAdvance.XOffset       = static_cast<float>( glyphPos[ i ].x_offset ) * posScale;
        glyphAdvance.YOffset       = static_cast<float>( glyphPos[ i ].y_offset ) * posScale;
        glyphAdvance.XAdvance      = static_cast<float>( glyphPos[ i ].x_advance ) * posScale;
        glyphAdvance.YAdvance      = static_cast<float>( glyphPos[ i ].y_advance ) * posScale;

        if ( glyphIndexToCodePoint.contains( codepoint ) )
        {
            glyphAdvance.CodePoint = glyphIndexToCodePoint[ codepoint ];
        }
        if ( glyphAdvance.CodePoint == 0 )
        {
            glyphAdvance.CodePoint = ' ';
        }
        totalAdvance += glyphAdvance.XAdvance;
    }

    m_totalWidth = totalAdvance;

    const float baseSize       = static_cast<float>( m_font->Asset( )->InitialFontSize );
    const float targetSize     = static_cast<float>( shapeDesc.FontSize );
    const float effectiveScale = targetSize / baseSize;
    const auto &metrics        = m_font->Asset( )->Metrics;
    m_totalHeight              = static_cast<float>( metrics.Ascent + metrics.Descent ) * effectiveScale;
    hb_buffer_destroy( buffer );
}

void TextLayout::GenerateTextVertices( const GenerateTextVerticesDesc &generateDesc ) const
{
    if ( !m_font || m_shapedGlyphs.empty( ) )
    {
        spdlog::error( "No glyphs to generate vertices for, call ShapeText first." );
        return;
    }

    if ( !generateDesc.OutVertices || !generateDesc.OutIndices )
    {
        spdlog::error( "Output vertex or index buffer is null" );
        return;
    }

    float          x             = generateDesc.StartPosition.X;
    float          y             = generateDesc.StartPosition.Y;
    const Float_4 &color         = generateDesc.Color;
    GlyphVertex   *outVertices   = generateDesc.OutVertices;
    uint32_t      *outIndices    = generateDesc.OutIndices;
    const float    scale         = generateDesc.Scale;
    const float    letterSpacing = generateDesc.LetterSpacing;
    const uint32_t baseVertex    = generateDesc.BaseVertexIndex;

    const FontAsset *fontAsset   = m_font->Asset( );
    uint32_t         vertexIndex = 0;
    uint32_t         indexIndex  = 0;

    for ( const auto &shapedGlyph : m_shapedGlyphs )
    {
        const FontGlyph *metrics = m_font->GetGlyph( shapedGlyph.CodePoint );
        if ( !metrics || metrics->Width == 0 || metrics->Height == 0 )
        {
            x += shapedGlyph.XAdvance;
            continue;
        }

        const float x0 = x + shapedGlyph.XOffset + metrics->BearingX * scale;
        const float x1 = x0 + metrics->Width * scale;
        const float y0 = y - metrics->BearingY * scale + shapedGlyph.YOffset;
        const float y1 = y0 + metrics->Height * scale;

        x += shapedGlyph.XAdvance + letterSpacing;
        y += shapedGlyph.YAdvance;

        const float u0 = static_cast<float>( metrics->AtlasX ) / static_cast<float>( fontAsset->AtlasWidth );
        const float v0 = static_cast<float>( metrics->AtlasY ) / static_cast<float>( fontAsset->AtlasHeight );
        const float u1 = static_cast<float>( metrics->AtlasX + metrics->Width ) / static_cast<float>( fontAsset->AtlasWidth );
        const float v1 = static_cast<float>( metrics->AtlasY + metrics->Height ) / static_cast<float>( fontAsset->AtlasHeight );

        // Top-left
        outVertices[ vertexIndex + 0 ] = GlyphVertex{ Float_2{ x0, y0 }, Float_2{ u0, v0 }, color };
        // Top-right
        outVertices[ vertexIndex + 1 ] = GlyphVertex{ Float_2{ x1, y0 }, Float_2{ u1, v0 }, color };
        // Bottom-left
        outVertices[ vertexIndex + 2 ] = GlyphVertex{ Float_2{ x0, y1 }, Float_2{ u0, v1 }, color };
        // Bottom-right
        outVertices[ vertexIndex + 3 ] = GlyphVertex{ Float_2{ x1, y1 }, Float_2{ u1, v1 }, color };

        const uint32_t currentBaseVertex = baseVertex + vertexIndex;
        outIndices[ indexIndex + 0 ]     = currentBaseVertex + 0;
        outIndices[ indexIndex + 1 ]     = currentBaseVertex + 1;
        outIndices[ indexIndex + 2 ]     = currentBaseVertex + 2;
        outIndices[ indexIndex + 3 ]     = currentBaseVertex + 1;
        outIndices[ indexIndex + 4 ]     = currentBaseVertex + 3;
        outIndices[ indexIndex + 5 ]     = currentBaseVertex + 2;

        vertexIndex += 4;
        indexIndex += 6;
    }
}

std::u32string TextLayout::Utf8ToUtf32( const std::string &utf8Text )
{
    std::u32string result;
    result.reserve( utf8Text.size( ) );
    // Process each byte in the UTF-8 string
    for ( size_t i = 0; i < utf8Text.size( ); )
    {
        char32_t   codePoint = 0;
        const auto firstByte = static_cast<uint8_t>( utf8Text[ i++ ] );

        // Determine the number of bytes for this code point
        int numBytes = 0;
        if ( ( firstByte & 0x80 ) == 0 ) // 0xxxxxxx
        {
            codePoint = firstByte;
            numBytes  = 0;
        }
        else if ( ( firstByte & 0xE0 ) == 0xC0 ) // 110xxxxx
        {
            codePoint = firstByte & 0x1F;
            numBytes  = 1;
        }
        else if ( ( firstByte & 0xF0 ) == 0xE0 ) // 1110xxxx
        {
            codePoint = firstByte & 0x0F;
            numBytes  = 2;
        }
        else if ( ( firstByte & 0xF8 ) == 0xF0 ) // 11110xxx
        {
            codePoint = firstByte & 0x07;
            numBytes  = 3;
        }
        else
        {
            // Invalid UTF-8 sequence, skip it
            continue;
        }

        // Read the continuation bytes
        for ( int j = 0; j < numBytes && i < utf8Text.size( ); ++j )
        {
            const auto continuationByte = static_cast<uint8_t>( utf8Text[ i++ ] );
            // Check if it's a valid continuation byte (10xxxxxx)
            if ( ( continuationByte & 0xC0 ) != 0x80 )
            {
                // Invalid continuation byte, abort this code point
                codePoint = 0;
                break;
            }

            codePoint = codePoint << 6 | continuationByte & 0x3F;
        }
        if ( codePoint != 0 )
        {
            result.push_back( codePoint );
        }
    }

    return result;
}

TextVertexAllocationInfo TextLayout::GetVertexAllocationInfo( ) const
{
    TextVertexAllocationInfo info;
    if ( !m_font || m_shapedGlyphs.empty( ) )
    {
        return info; // Return zeros
    }
    uint32_t visibleGlyphCount = 0;
    for ( const auto &shapedGlyph : m_shapedGlyphs )
    {
        const FontGlyph *metrics = m_font->GetGlyph( shapedGlyph.CodePoint );
        if ( metrics && metrics->Width > 0 && metrics->Height > 0 )
        {
            visibleGlyphCount++;
        }
    }
    info.VertexCount = visibleGlyphCount * 4;
    info.IndexCount  = visibleGlyphCount * 6;
    return info;
}

Float_2 TextLayout::GetTextSize( ) const
{
    return Float_2{ m_totalWidth, m_totalHeight };
}

float TextLayout::GetTextWidth( ) const
{
    return m_totalWidth;
}

float TextLayout::GetTextHeight( ) const
{
    return m_totalHeight;
}
