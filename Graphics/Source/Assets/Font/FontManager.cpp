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
// ReSharper disable CppMemberFunctionMayBeStatic
#include <DenOfIzGraphics/Assets/Font/FontManager.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <codecvt>
#include <ft2build.h>
#include <locale>
#include <ranges>
#include <unordered_set>

#include FT_FREETYPE_H
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>

using namespace DenOfIz;

FontManager::FontManager( )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ); error != 0 )
    {
        LOG( FATAL ) << "Failed to initialize FreeType library: " << FT_Error_String( error );
    }
}

FontManager::~FontManager( )
{
    for ( const auto &hbFont : m_hbFonts | std::views::values )
    {
        if ( hbFont )
        {
            hb_font_destroy( hbFont );
        }
    }
    m_hbFonts.clear( );

    FT_Done_FreeType( m_ftLibrary );
}

FontCache *FontManager::LoadFont( const InteropString &fontPath, const uint32_t pixelSize, const bool antiAliasing )
{
    const std::string &path     = fontPath.Get( );
    const std::string  cacheKey = path + "_" + std::to_string( pixelSize );
    if ( const auto it = m_fontCache.find( cacheKey ); it != m_fontCache.end( ) )
    {
        return it->second.get( );
    }

    const auto fontAsset    = std::make_shared<FontAsset>( );
    fontAsset->FontPath     = path.c_str( );
    fontAsset->PixelSize    = pixelSize;
    fontAsset->AntiAliasing = antiAliasing;

    const auto fontCache = std::make_shared<FontCache>( *fontAsset );
    fontCache->InitializeAtlasBitmap( );

    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( path );
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        LOG( ERROR ) << "Failed to load font: " << path;
        return nullptr;
    }

    error = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    if ( error )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to set font size";
        return nullptr;
    }

    auto &asset              = const_cast<FontAsset &>( fontCache->GetFontAsset( ) );
    asset.Metrics.Ascent     = static_cast<uint32_t>( face->size->metrics.ascender >> 6 );
    asset.Metrics.Descent    = static_cast<uint32_t>( abs( face->size->metrics.descender ) >> 6 );
    asset.Metrics.LineGap    = static_cast<uint32_t>( ( face->size->metrics.height - ( face->size->metrics.ascender - face->size->metrics.descender ) ) >> 6 );
    asset.Metrics.LineHeight = static_cast<uint32_t>( face->size->metrics.height >> 6 );

    if ( FT_IS_SCALABLE( face ) )
    {
        asset.Metrics.UnderlinePos       = static_cast<uint32_t>( abs( face->underline_position ) >> 6 );
        asset.Metrics.UnderlineThickness = static_cast<uint32_t>( face->underline_thickness >> 6 );
    }
    else
    {
        asset.Metrics.UnderlinePos       = asset.Metrics.Descent / 2;
        asset.Metrics.UnderlineThickness = asset.Metrics.Ascent / 20;
    }

    // Load ASCII glyphs
    for ( uint32_t c = 32; c < 127; c++ )
    {
        LoadGlyph( fontCache.get( ), c, face );
    }

    FT_Done_Face( face );
    m_fontCache[ cacheKey ] = fontCache;

    return fontCache.get( );
}

FontCache *FontManager::GetFont( const InteropString &fontPath, const uint32_t pixelSize )
{
    const std::string &path     = fontPath.Get( );
    const std::string  cacheKey = path + "_" + std::to_string( pixelSize );

    if ( const auto it = m_fontCache.find( cacheKey ); it != m_fontCache.end( ) )
    {
        return it->second.get( );
    }

    return nullptr;
}

bool FontManager::EnsureGlyphsLoaded( FontCache *fontCache, const InteropString &interopText )
{
    bool allLoaded         = true;
    bool anyNewGlyphLoaded = false;

    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( fontCache->GetFontAsset( ).FontPath.Get( ) );
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        LOG( ERROR ) << "Failed to load font face";
        return false;
    }

    error = FT_Set_Pixel_Sizes( face, 0, fontCache->GetFontAsset( ).PixelSize );
    if ( error )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to set font size";
        return false;
    }

    for ( const std::u32string text = Utf8ToUtf32( interopText.Get( ) ); const char32_t c : text )
    {
        if ( !fontCache->HasGlyph( c ) )
        {
            if ( !LoadGlyph( fontCache, c, face ) )
            {
                allLoaded = false;
            }
            else
            {
                anyNewGlyphLoaded = true;
            }
        }
    }

    FT_Done_Face( face );
    return allLoaded && anyNewGlyphLoaded;
}

bool FontManager::LoadGlyph( FontCache *fontCache, const uint32_t codePoint, const FT_Face face ) // NOLINT
{
    static const std::unordered_set<uint32_t> ignoredGlyphs = { '\n' };
    if ( ignoredGlyphs.contains( codePoint ) )
    {
        return true;
    }

    if ( fontCache->HasGlyph( codePoint ) )
    {
        return true;
    }

    const FT_UInt glyphIndex = FT_Get_Char_Index( face, codePoint );
    if ( glyphIndex == 0 )
    {
        LOG( WARNING ) << "Glyph not found for code point: " << codePoint;
        return false;
    }

    FT_Int32 loadFlags = FT_LOAD_DEFAULT;
    if ( fontCache->GetFontAsset( ).AntiAliasing )
    {
        loadFlags |= FT_LOAD_RENDER;
    }
    else
    {
        loadFlags |= FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
    }

    if ( const FT_Error error = FT_Load_Glyph( face, glyphIndex, loadFlags ) )
    {
        LOG( ERROR ) << "Failed to load glyph: " << FT_Error_String( error );
        return false;
    }

    const FT_GlyphSlot slot   = face->glyph; // NOLINT
    const FT_Bitmap    bitmap = slot->bitmap;

    InteropArray<Byte> data( bitmap.pitch * bitmap.rows );
    data.MemCpy( bitmap.buffer, bitmap.pitch * bitmap.rows );

    AddGlyphDesc addGlyphDesc{ };
    addGlyphDesc.CodePoint    = codePoint;
    addGlyphDesc.Width        = bitmap.width;
    addGlyphDesc.Height       = bitmap.rows;
    addGlyphDesc.BearingX     = slot->bitmap_left;
    addGlyphDesc.BearingY     = slot->bitmap_top;
    addGlyphDesc.Advance      = slot->advance.x >> 6; // Convert from 26.6 fixed-point format
    addGlyphDesc.BitmapData   = data;
    addGlyphDesc.BitmapPitch  = bitmap.pitch;
    addGlyphDesc.IsMonochrome = bitmap.pixel_mode == FT_PIXEL_MODE_MONO;

    fontCache->AddGlyph( addGlyphDesc );
    return true;
}

std::u32string FontManager::Utf8ToUtf32( const std::string &utf8Text )
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

std::string FontManager::Utf32ToUtf8( const std::u32string &utf32Text )
{
    std::string result;
    result.reserve( utf32Text.size( ) * 4 );

    for ( const char32_t codePoint : utf32Text )
    {
        if ( codePoint < 0x80 )
        {
            result.push_back( static_cast<char>( codePoint ) );
        }
        else if ( codePoint < 0x800 )
        {
            result.push_back( static_cast<char>( 0xC0 | codePoint >> 6 ) );
            result.push_back( static_cast<char>( 0x80 | codePoint & 0x3F ) );
        }
        else if ( codePoint < 0x10000 )
        {
            result.push_back( static_cast<char>( 0xE0 | codePoint >> 12 ) );
            result.push_back( static_cast<char>( 0x80 | codePoint >> 6 & 0x3F ) );
            result.push_back( static_cast<char>( 0x80 | codePoint & 0x3F ) );
        }
        else if ( codePoint < 0x110000 )
        {
            result.push_back( static_cast<char>( 0xF0 | codePoint >> 18 ) );
            result.push_back( static_cast<char>( 0x80 | codePoint >> 12 & 0x3F ) );
            result.push_back( static_cast<char>( 0x80 | codePoint >> 6 & 0x3F ) );
            result.push_back( static_cast<char>( 0x80 | codePoint & 0x3F ) );
        }
    }

    return result;
}

hb_font_t *FontManager::GetHarfBuzzFont( const std::string &fontPath, const uint32_t numPixels, FT_Face face )
{
    const std::string cacheKey = fontPath + "_" + std::to_string( numPixels );
    if ( const auto it = m_hbFonts.find( cacheKey ); it != m_hbFonts.end( ) )
    {
        return it->second;
    }

    hb_font_t *hbFont = hb_ft_font_create_referenced( face );
    if ( !hbFont )
    {
        LOG( ERROR ) << "Failed to create HarfBuzz font for: " << fontPath;
        return nullptr;
    }

    hb_font_set_scale( hbFont, numPixels * 64, numPixels * 64 );
    m_hbFonts[ cacheKey ] = hbFont;
    return hbFont;
}

void FontManager::DestroyHarfBuzzFont( const std::string &fontCacheKey )
{
    if ( const auto it = m_hbFonts.find( fontCacheKey ); it != m_hbFonts.end( ) )
    {
        hb_font_destroy( it->second );
        m_hbFonts.erase( it );
    }
}

TextLayout FontManager::ShapeText( FontCache *fontCache, const InteropString &interopText, const TextDirection direction )
{
    TextLayout layout;
    if ( !fontCache || interopText.NumChars( ) == 0 )
    {
        return layout;
    }

    EnsureGlyphsLoaded( fontCache, interopText );
    const std::string    utf8Text  = interopText.Get( );
    const std::u32string utf32Text = Utf8ToUtf32( interopText.Get( ) );

    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( fontCache->GetFontAsset( ).FontPath.Get( ) );
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        LOG( ERROR ) << "Failed to load font face for text shaping";
        return layout;
    }

    error = FT_Set_Pixel_Sizes( face, 0, fontCache->GetFontAsset( ).PixelSize );
    if ( error )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to set font size for text shaping";
        return layout;
    }

    hb_font_t *hbFont = hb_ft_font_create( face, nullptr );
    if ( !hbFont )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to create HarfBuzz font";
        return layout;
    }

    hb_buffer_t *buffer = hb_buffer_create( );
    if ( !hb_buffer_allocation_successful( buffer ) )
    {
        hb_font_destroy( hbFont );
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to allocate HarfBuzz buffer";
        return layout;
    }

    hb_buffer_reset( buffer );

    hb_direction_t hbDirection;
    switch ( direction )
    {
    case TextDirection::LeftToRight:
        hbDirection = HB_DIRECTION_LTR;
        break;
    case TextDirection::RightToLeft:
        hbDirection = HB_DIRECTION_RTL;
        break;
    case TextDirection::Auto:
    default:
        hbDirection = HB_DIRECTION_LTR; // Default to LTR instead of INVALID
        break;
    }
    hb_buffer_set_direction( buffer, hbDirection );
    hb_buffer_set_script( buffer, HB_SCRIPT_LATIN );
    hb_buffer_set_language( buffer, hb_language_from_string( "en", -1 ) );

    // Add text to buffer
    const char *text_ptr = utf8Text.c_str( );
    const int   text_len = utf8Text.length( );
    hb_buffer_add_utf8( buffer, text_ptr, text_len, 0, text_len );

    // Define shaping features (kerning and ligatures)
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
        FT_Done_Face( face );
        return layout;
    }

    constexpr float posScale = 1.0f / 64.0f; // HarfBuzz units to pixels, HarfBuzz uses 26.6 fixed-point format

    layout.ShapedGlyphs.Resize( glyphCount );
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
        ShapedGlyph &shapedGlyph = layout.ShapedGlyphs.GetElement( i );

        shapedGlyph.GlyphId      = glyphInfo[ i ].codepoint;
        shapedGlyph.ClusterIndex = glyphInfo[ i ].cluster;

        shapedGlyph.XOffset  = glyphPos[ i ].x_offset * posScale;
        shapedGlyph.YOffset  = glyphPos[ i ].y_offset * posScale;
        shapedGlyph.XAdvance = glyphPos[ i ].x_advance * posScale;
        shapedGlyph.YAdvance = glyphPos[ i ].y_advance * posScale;

        if ( glyphIndexToCodePoint.contains( shapedGlyph.GlyphId ) )
        {
            shapedGlyph.CodePoint = glyphIndexToCodePoint[ shapedGlyph.GlyphId ];
        }
        else
        {
            // Try to get code point from the cluster index
            if ( const unsigned int clusterIndex = shapedGlyph.ClusterIndex; clusterIndex < utf8Text.length( ) )
            {
                unsigned int charIndex = 0;
                for ( size_t j = 0; j < utf32Text.size( ) && charIndex <= clusterIndex; ++j )
                {
                    size_t charBytes = 0;
                    if ( const char32_t cp = utf32Text[ j ]; cp < 0x80 )
                    {
                        charBytes = 1;
                    }
                    else if ( cp < 0x800 )
                    {
                        charBytes = 2;
                    }
                    else if ( cp < 0x10000 )
                    {
                        charBytes = 3;
                    }
                    else
                    {
                        charBytes = 4;
                    }

                    if ( charIndex + charBytes > clusterIndex )
                    {
                        shapedGlyph.CodePoint = utf32Text[ j ];
                        break;
                    }

                    charIndex += charBytes;
                }
            }
            // Default to space(Most white space characters do not have a CodePoint)
            if ( shapedGlyph.CodePoint == 0 )
            {
                shapedGlyph.CodePoint = ' ';
                LOG( WARNING ) << "Could not determine code point for glyph ID: " << shapedGlyph.GlyphId;
            }
        }

        totalAdvance += shapedGlyph.XAdvance;
    }

    layout.TotalWidth  = totalAdvance;
    layout.TotalHeight = static_cast<float>( fontCache->GetFontAsset( ).Metrics.LineHeight );
    hb_buffer_destroy( buffer );
    FT_Done_Face( face );
    return layout;
}

void FontManager::GenerateTextVertices( const GenerateTextVerticesDesc &desc, InteropArray<float> &outVertices, InteropArray<uint32_t> &outIndices )
{
    const FontCache  *fontCache = desc.FontCache;
    const TextLayout &layout    = desc.Layout;
    const float       x         = desc.X;
    const float       y         = desc.Y;
    const Float_4    &color     = desc.Color;
    const float       scale     = desc.Scale;
    if ( !fontCache || layout.ShapedGlyphs.NumElements( ) == 0 )
    {
        return;
    }

    const FontAsset &fontAsset  = fontCache->GetFontAsset( );
    uint32_t         baseVertex = outVertices.NumElements( ) / 8; // 8 floats per vertex (pos.xy, uv.xy, color.rgba)

    std::vector<float> absXPositions( layout.ShapedGlyphs.NumElements( ) );
    float              currentX = x;
    for ( size_t i = 0; i < layout.ShapedGlyphs.NumElements( ); i++ )
    {
        absXPositions[ i ] = currentX;
        currentX += layout.ShapedGlyphs.GetElement( i ).XAdvance * scale;
    }

    for ( size_t i = 0; i < layout.ShapedGlyphs.NumElements( ); i++ )
    {
        const auto &shapedGlyph = layout.ShapedGlyphs.GetElement( i );

        const GlyphMetrics *metrics = fontCache->GetGlyphMetrics( shapedGlyph.CodePoint );
        if ( !metrics )
        {
            continue;
        }

        if ( metrics->Width == 0 || metrics->Height == 0 )
        {
            continue;
        }

        const float glyph_x = absXPositions[ i ];

        const float xPos = glyph_x + shapedGlyph.XOffset * scale;
        const float yPos = y + shapedGlyph.YOffset * scale;

        float x0 = xPos + static_cast<float>( metrics->BearingX ) * scale;
        float y0 = yPos - static_cast<float>( metrics->BearingY ) * scale;
        float x1 = x0 + static_cast<float>( metrics->Width ) * scale;
        float y1 = y0 + static_cast<float>( metrics->Height ) * scale;

        const float u0 = static_cast<float>( metrics->AtlasX ) / static_cast<float>( fontAsset.AtlasWidth );
        const float v0 = static_cast<float>( metrics->AtlasY ) / static_cast<float>( fontAsset.AtlasHeight );
        const float u1 = static_cast<float>( metrics->AtlasX + metrics->Width ) / static_cast<float>( fontAsset.AtlasWidth );
        const float v1 = static_cast<float>( metrics->AtlasY + metrics->Height ) / static_cast<float>( fontAsset.AtlasHeight );

        // Top-left
        outVertices.AddElement( x0 );
        outVertices.AddElement( y0 );
        outVertices.AddElement( u0 );
        outVertices.AddElement( v0 );
        outVertices.AddElement( color.X );
        outVertices.AddElement( color.Y );
        outVertices.AddElement( color.Z );
        outVertices.AddElement( color.W );

        // Top-right
        outVertices.AddElement( x1 );
        outVertices.AddElement( y0 );
        outVertices.AddElement( u1 );
        outVertices.AddElement( v0 );
        outVertices.AddElement( color.X );
        outVertices.AddElement( color.Y );
        outVertices.AddElement( color.Z );
        outVertices.AddElement( color.W );

        // Bottom-left
        outVertices.AddElement( x0 );
        outVertices.AddElement( y1 );
        outVertices.AddElement( u0 );
        outVertices.AddElement( v1 );
        outVertices.AddElement( color.X );
        outVertices.AddElement( color.Y );
        outVertices.AddElement( color.Z );
        outVertices.AddElement( color.W );

        // Bottom-right
        outVertices.AddElement( x1 );
        outVertices.AddElement( y1 );
        outVertices.AddElement( u1 );
        outVertices.AddElement( v1 );
        outVertices.AddElement( color.X );
        outVertices.AddElement( color.Y );
        outVertices.AddElement( color.Z );
        outVertices.AddElement( color.W );

        // Add indices for the quad (making a triangle strip)
        outIndices.AddElement( baseVertex + 0 );
        outIndices.AddElement( baseVertex + 1 );
        outIndices.AddElement( baseVertex + 2 );
        outIndices.AddElement( baseVertex + 1 );
        outIndices.AddElement( baseVertex + 3 );
        outIndices.AddElement( baseVertex + 2 );

        baseVertex += 4;
    }
}
