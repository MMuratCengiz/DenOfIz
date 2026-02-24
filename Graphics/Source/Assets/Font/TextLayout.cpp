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

#include "DenOfIzGraphicsInternal/Assets/Font/TextLayout.h"
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <unordered_map>
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
    m_lastDirection  = DENOFIZ_TEXT_DIRECTION_AUTO;
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
    if ( !shapeDesc.Text.Chars || shapeDesc.Text.NumChars == 0 || !m_font )
    {
        return;
    }

    std::string currentText;
    currentText.assign( shapeDesc.Text.Chars, shapeDesc.Text.Chars + shapeDesc.Text.NumChars );

    if ( m_lastShapedText == currentText && m_lastFontSize == shapeDesc.FontSize && m_lastDirection == shapeDesc.Direction && m_lastScriptTag.X == shapeDesc.HbScriptTag.X &&
         m_lastScriptTag.Y == shapeDesc.HbScriptTag.Y && m_lastScriptTag.Z == shapeDesc.HbScriptTag.Z && m_lastScriptTag.W == shapeDesc.HbScriptTag.W )
    {
        return;
    }

    m_lastShapedText = currentText;
    m_lastFontSize   = shapeDesc.FontSize;
    m_lastDirection  = shapeDesc.Direction;
    m_lastScriptTag  = shapeDesc.HbScriptTag;

    const std::string   &utf8Text  = currentText;
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
    case DENOFIZ_TEXT_DIRECTION_LEFT_TO_RIGHT:
        hbDirection = HB_DIRECTION_LTR;
        break;
    case DENOFIZ_TEXT_DIRECTION_RIGHT_TO_LEFT:
        hbDirection = HB_DIRECTION_RTL;
        break;
    case DENOFIZ_TEXT_DIRECTION_AUTO:
    default:
        hbDirection = HB_DIRECTION_LTR;
        break;
    }

    const DenOfIz_UInt4 scriptTag = shapeDesc.HbScriptTag;
    hb_buffer_set_direction( buffer, hbDirection );
    hb_buffer_set_script( buffer, static_cast<hb_script_t>( HB_TAG( scriptTag.X, scriptTag.Y, scriptTag.Z, scriptTag.W ) ) );
    hb_buffer_set_language( buffer, hb_language_from_string( "en", -1 ) );

    const char *text_ptr = utf8Text.c_str( );
    const int   text_len = utf8Text.length( );
    hb_buffer_add_utf8( buffer, text_ptr, text_len, 0, text_len );

    constexpr hb_feature_t features[] = { { HB_TAG( 'k', 'e', 'r', 'n' ), 1, 0, UINT_MAX }, { HB_TAG( 'l', 'i', 'g', 'a' ), 0, 0, UINT_MAX } };

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

    constexpr float posScale = 1.0f / 64.0f;

    m_shapedGlyphs.resize( glyphCount );
    float totalAdvance = 0.0f;

    for ( unsigned int i = 0; i < glyphCount; ++i )
    {
        GlyphAdvance &glyphAdvance = m_shapedGlyphs[ i ];
        uint32_t      cluster      = glyphInfo[ i ].cluster;
        glyphAdvance.XOffset       = static_cast<float>( glyphPos[ i ].x_offset ) * posScale;
        glyphAdvance.YOffset       = static_cast<float>( glyphPos[ i ].y_offset ) * posScale;
        glyphAdvance.XAdvance      = static_cast<float>( glyphPos[ i ].x_advance ) * posScale;
        glyphAdvance.YAdvance      = static_cast<float>( glyphPos[ i ].y_advance ) * posScale;

        if ( cluster < utf32Text.size( ) )
        {
            glyphAdvance.CodePoint = utf32Text[ cluster ];
        }
        else
        {
            glyphAdvance.CodePoint = ' ';
        }
        totalAdvance += glyphAdvance.XAdvance;
    }

    m_totalWidth = totalAdvance;

    const float baseSize       = static_cast<float>( DenOfIz_FontAsset_InitialFontSize( m_font->Asset( ) ) );
    const float targetSize     = static_cast<float>( shapeDesc.FontSize );
    const float effectiveScale = targetSize / baseSize;
    const auto  metrics        = DenOfIz_FontAsset_GetMetrics( m_font->Asset( ) );
    m_totalHeight              = ( metrics.Ascent + metrics.Descent ) * effectiveScale;
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

    float                 x             = generateDesc.StartPosition.X;
    float                 y             = generateDesc.StartPosition.Y;
    const DenOfIz_Float4 &color         = generateDesc.Color;
    GlyphVertex          *outVertices   = generateDesc.OutVertices;
    uint32_t             *outIndices    = generateDesc.OutIndices;
    const float           scale         = generateDesc.Scale;
    const float           letterSpacing = generateDesc.LetterSpacing;
    const uint32_t        baseVertex    = generateDesc.BaseVertexIndex;

    const DenOfIz_FontAsset fontAsset   = m_font->Asset( );
    uint32_t                vertexIndex = 0;
    uint32_t                indexIndex  = 0;

    for ( const auto &shapedGlyph : m_shapedGlyphs )
    {
        const DenOfIz_FontGlyph *metrics = m_font->GetGlyph( shapedGlyph.CodePoint );
        if ( !metrics || metrics->Width == 0 || metrics->Height == 0 )
        {
            x += shapedGlyph.XAdvance;
            continue;
        }

        const float bearingX    = metrics->BearingX * scale;
        const float bearingY    = metrics->BearingY * scale;
        const float glyphWidth  = static_cast<float>( metrics->Width ) * scale;
        const float glyphHeight = static_cast<float>( metrics->Height ) * scale;

        const float x0 = x + shapedGlyph.XOffset + bearingX;
        const float x1 = x0 + glyphWidth;
        const float y0 = y - bearingY + shapedGlyph.YOffset;
        const float y1 = y0 + glyphHeight;

        x += shapedGlyph.XAdvance + letterSpacing;
        y += shapedGlyph.YAdvance;

        const float u0 = static_cast<float>( metrics->AtlasX ) / static_cast<float>( DenOfIz_FontAsset_AtlasWidth( fontAsset ) );
        const float v0 = static_cast<float>( metrics->AtlasY ) / static_cast<float>( DenOfIz_FontAsset_AtlasHeight( fontAsset ) );
        const float u1 = static_cast<float>( metrics->AtlasX + metrics->Width ) / static_cast<float>( DenOfIz_FontAsset_AtlasWidth( fontAsset ) );
        const float v1 = static_cast<float>( metrics->AtlasY + metrics->Height ) / static_cast<float>( DenOfIz_FontAsset_AtlasHeight( fontAsset ) );

        outVertices[ vertexIndex + 0 ] = GlyphVertex{ DenOfIz_Float2{ x0, y0 }, DenOfIz_Float2{ u0, v0 }, color };
        outVertices[ vertexIndex + 1 ] = GlyphVertex{ DenOfIz_Float2{ x1, y0 }, DenOfIz_Float2{ u1, v0 }, color };
        outVertices[ vertexIndex + 2 ] = GlyphVertex{ DenOfIz_Float2{ x0, y1 }, DenOfIz_Float2{ u0, v1 }, color };
        outVertices[ vertexIndex + 3 ] = GlyphVertex{ DenOfIz_Float2{ x1, y1 }, DenOfIz_Float2{ u1, v1 }, color };

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
    for ( size_t i = 0; i < utf8Text.size( ); )
    {
        char32_t   codePoint = 0;
        const auto firstByte = static_cast<uint8_t>( utf8Text[ i++ ] );

        int numBytes = 0;
        if ( ( firstByte & 0x80 ) == 0 )
        {
            codePoint = firstByte;
            numBytes  = 0;
        }
        else if ( ( firstByte & 0xE0 ) == 0xC0 )
        {
            codePoint = firstByte & 0x1F;
            numBytes  = 1;
        }
        else if ( ( firstByte & 0xF0 ) == 0xE0 )
        {
            codePoint = firstByte & 0x0F;
            numBytes  = 2;
        }
        else if ( ( firstByte & 0xF8 ) == 0xF0 )
        {
            codePoint = firstByte & 0x07;
            numBytes  = 3;
        }
        else
        {
            continue;
        }

        for ( int j = 0; j < numBytes && i < utf8Text.size( ); ++j )
        {
            const auto continuationByte = static_cast<uint8_t>( utf8Text[ i++ ] );
            if ( ( continuationByte & 0xC0 ) != 0x80 )
            {
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
        return info;
    }
    uint32_t visibleGlyphCount = 0;
    for ( const auto &shapedGlyph : m_shapedGlyphs )
    {
        const DenOfIz_FontGlyph *metrics = m_font->GetGlyph( shapedGlyph.CodePoint );
        if ( metrics && metrics->Width > 0 && metrics->Height > 0 )
        {
            visibleGlyphCount++;
        }
    }
    info.VertexCount = visibleGlyphCount * 4;
    info.IndexCount  = visibleGlyphCount * 6;
    return info;
}

DenOfIz_Float2 TextLayout::GetTextSize( ) const
{
    return DenOfIz_Float2{ m_totalWidth, m_totalHeight };
}

float TextLayout::GetTextWidth( ) const
{
    return m_totalWidth;
}

float TextLayout::GetTextHeight( ) const
{
    return m_totalHeight;
}
