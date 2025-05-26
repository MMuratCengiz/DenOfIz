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

#include <DenOfIzGraphics/Assets/Font/TextLayout.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>
#include <unordered_map>

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

    // Check if we can use cached results
    if ( m_lastShapedText.Equals( shapeDesc.Text ) && m_lastFontSize == shapeDesc.FontSize && m_lastDirection == shapeDesc.Direction &&
         m_lastScriptTag.X == shapeDesc.HbScriptTag.X && m_lastScriptTag.Y == shapeDesc.HbScriptTag.Y && m_lastScriptTag.Z == shapeDesc.HbScriptTag.Z &&
         m_lastScriptTag.W == shapeDesc.HbScriptTag.W )
    {
        return;
    }

    // Update cache keys
    m_lastShapedText = shapeDesc.Text;
    m_lastFontSize   = shapeDesc.FontSize;
    m_lastDirection  = shapeDesc.Direction;
    m_lastScriptTag  = shapeDesc.HbScriptTag;

    const std::string    utf8Text  = shapeDesc.Text.Get( );
    const std::u32string utf32Text = Utf8ToUtf32( utf8Text );
    const FT_Face        face      = m_font->FTFace( );

    if ( const FT_Error error = FT_Set_Char_Size( face, 0, shapeDesc.FontSize * 64, 0, 0 ) )
    {
        LOG( ERROR ) << "Failed to set font size: " << FT_Error_String( error );
        return;
    }
    hb_font_t *hbFont = hb_ft_font_create_referenced( face );
    if ( !hbFont )
    {
        LOG( ERROR ) << "Failed to create HarfBuzz font";
    }

    hb_buffer_t *buffer = hb_buffer_create( );
    if ( !hb_buffer_allocation_successful( buffer ) )
    {
        hb_font_destroy( hbFont );
        LOG( ERROR ) << "Failed to allocate HarfBuzz buffer";
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
        LOG( ERROR ) << "No glyph positions returned from HarfBuzz shaping";
        hb_buffer_destroy( buffer );
        hb_font_destroy( hbFont );
    }

    constexpr float posScale = 1.0f / 64.0f; // HarfBuzz units to pixels, HarfBuzz uses 26.6 fixed-point format

    m_shapedGlyphs.resize( glyphCount );
    float totalAdvance = 0.0f;

    std::unordered_map<uint32_t, uint32_t> glyphIndexToCodePoint;
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
    const auto &metrics        = m_font->m_desc.FontAsset->Metrics;
    m_totalHeight              = static_cast<float>( metrics.Ascent + metrics.Descent ) * effectiveScale;
    hb_buffer_destroy( buffer );
    hb_font_destroy( hbFont );
}

void TextLayout::GenerateTextVertices( const GenerateTextVerticesDesc &generateDesc ) const
{
    if ( m_shapedGlyphs.empty( ) )
    {
        LOG( ERROR ) << "No glyphs to generate vertices for, call ShapeText first.";
        return;
    }

    float                      x             = generateDesc.StartPosition.X;
    float                      y             = generateDesc.StartPosition.Y;
    const Float_4             &color         = generateDesc.Color;
    InteropArray<GlyphVertex> *outVertices   = generateDesc.OutVertices;
    InteropArray<uint32_t>    *outIndices    = generateDesc.OutIndices;
    const float                scale         = generateDesc.Scale;
    const float                letterSpacing = generateDesc.LetterSpacing;

    const FontAsset *fontAsset  = m_font->Asset( );
    uint32_t         baseVertex = outVertices->NumElements( );
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
        GlyphVertex vertex1{ Float_2{ x0, y0 }, Float_2{ u0, v0 }, color };
        // Top-right
        GlyphVertex vertex2{ Float_2{ x1, y0 }, Float_2{ u1, v0 }, color };
        // Bottom-left
        GlyphVertex vertex3{ Float_2{ x0, y1 }, Float_2{ u0, v1 }, color };
        // Bottom-right
        GlyphVertex vertex4{ Float_2{ x1, y1 }, Float_2{ u1, v1 }, color };
        outVertices->AddElement( vertex1 );
        outVertices->AddElement( vertex2 );
        outVertices->AddElement( vertex3 );
        outVertices->AddElement( vertex4 );

        outIndices->AddElement( baseVertex + 0 );
        outIndices->AddElement( baseVertex + 1 );
        outIndices->AddElement( baseVertex + 2 );
        outIndices->AddElement( baseVertex + 1 );
        outIndices->AddElement( baseVertex + 3 );
        outIndices->AddElement( baseVertex + 2 );
        baseVertex += 4;
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
