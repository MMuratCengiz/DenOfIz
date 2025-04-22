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
#include <ft2build.h>
#include <unordered_set>

#include FT_FREETYPE_H

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
    FT_Done_FreeType( m_ftLibrary );
}

std::shared_ptr<FontAsset> FontManager::LoadFont( const std::string &fontPath, const uint32_t pixelSize, const bool antiAliasing )
{
    const std::string cacheKey = fontPath + "_" + std::to_string( pixelSize );
    if ( const auto it = m_fontCache.find( cacheKey ); it != m_fontCache.end( ) )
    {
        return it->second;
    }

    auto fontAsset          = std::make_shared<FontAsset>( );
    fontAsset->FontPath     = fontPath.c_str( );
    fontAsset->PixelSize    = pixelSize;
    fontAsset->AntiAliasing = antiAliasing;
    fontAsset->ReserveAtlasBitmap( );

    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( fontPath );
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        LOG( ERROR ) << "Failed to load font: " << fontPath;
        return nullptr;
    }

    error = FT_Set_Pixel_Sizes( face, 0, pixelSize );
    if ( error )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to set font size";
        return nullptr;
    }

    // Load ASCII glyphs
    for ( uint32_t c = 32; c < 127; c++ )
    {
        LoadGlyph( fontAsset, c, face );
    }

    FT_Done_Face( face );
    m_fontCache[ cacheKey ] = fontAsset;

    return fontAsset;
}

std::shared_ptr<FontAsset> FontManager::GetFont( const std::string &fontPath, const uint32_t pixelSize )
{
    const std::string cacheKey = fontPath + "_" + std::to_string( pixelSize );
    const auto        it       = m_fontCache.find( cacheKey );

    if ( it != m_fontCache.end( ) )
    {
        return it->second;
    }

    return nullptr;
}

bool FontManager::EnsureGlyphsLoaded( const std::shared_ptr<FontAsset> &font, const std::u32string &text )
{
    if ( !font )
    {
        return false;
    }

    bool allLoaded         = true;
    bool anyNewGlyphLoaded = false;

    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( font->FontPath.Get( ) );
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        LOG( ERROR ) << "Failed to load font face";
        return false;
    }

    error = FT_Set_Pixel_Sizes( face, 0, font->PixelSize );
    if ( error )
    {
        FT_Done_Face( face );
        LOG( ERROR ) << "Failed to set font size";
        return false;
    }

    for ( const char32_t c : text )
    {
        if ( !font->GlyphCache.contains( c ) )
        {
            if ( !LoadGlyph( font, c, face ) )
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

bool FontManager::LoadGlyph( const std::shared_ptr<FontAsset> &font, const uint32_t codePoint, const FT_Face face ) // NOLINT
{
    static const std::unordered_set<uint32_t> ignoredGlyphs = { '\n' };
    if ( ignoredGlyphs.contains( codePoint ) )
    {
        return true;
    }
    if ( font->GlyphCache.contains( codePoint ) )
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
    if ( font->AntiAliasing )
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

    // Skip empty glyphs (like white spaces)
    if ( bitmap.width == 0 || bitmap.rows == 0 )
    {
        GlyphMetrics metrics{ };
        metrics.CodePoint = codePoint;
        metrics.Width     = 0;
        metrics.Height    = 0;
        metrics.BearingX  = slot->bitmap_left;
        metrics.BearingY  = slot->bitmap_top;
        metrics.Advance   = slot->advance.x >> 6; // Convert from 26.6 fixed-point format
        metrics.AtlasX    = 0;
        metrics.AtlasY    = 0;

        font->GlyphCache[ codePoint ] = metrics;
        return true;
    }

    const Rect rect = AllocateSpace( font, bitmap.width, bitmap.rows );
    CopyGlyphToAtlas( font, face, rect );

    GlyphMetrics metrics{ };
    metrics.CodePoint = codePoint;
    metrics.Width     = bitmap.width;
    metrics.Height    = bitmap.rows;
    metrics.BearingX  = slot->bitmap_left;
    metrics.BearingY  = slot->bitmap_top;
    metrics.Advance   = slot->advance.x >> 6; // Convert from 26.6 fixed-point format
    metrics.AtlasX    = rect.X;
    metrics.AtlasY    = rect.Y;

    font->GlyphCache[ codePoint ] = metrics;

    return true;
}

FontManager::Rect FontManager::AllocateSpace( const std::shared_ptr<FontAsset> &font, const uint32_t width, const uint32_t height )
{
    // If we can't fit on the current row, move to the next row
    if ( m_currentAtlasX + width > font->AtlasWidth )
    {
        m_currentAtlasX = 0;
        m_currentAtlasY += m_rowHeight;
        m_rowHeight = 0;
    }

    // Resize if we can't fit in the atlas
    if ( m_currentAtlasY + height > font->AtlasHeight )
    {
        // For now, we'll just clear the atlas and start over we need to instead resize and preserve existing glyphs in the future for performance reasons
        font->AtlasHeight *= 2;
        font->ReserveAtlasBitmap( );
        font->ClearAtlasBitmap( );
        font->GlyphCache.clear( );

        m_currentAtlasX = 0;
        m_currentAtlasY = 0;
        m_rowHeight     = 0;

        LOG( WARNING ) << "Font atlas resized to " << font->AtlasWidth << "x" << font->AtlasHeight;
    }

    Rect rect{ };
    rect.X      = m_currentAtlasX;
    rect.Y      = m_currentAtlasY;
    rect.Width  = width;
    rect.Height = height;

    m_currentAtlasX += width;
    m_rowHeight = std::max( m_rowHeight, height );
    return rect;
}

void FontManager::CopyGlyphToAtlas( const std::shared_ptr<FontAsset> &font, const FT_Face face, const Rect &rect ) //NOLINT
{
    const FT_GlyphSlot slot   = face->glyph; // NOLINT
    const FT_Bitmap    bitmap = slot->bitmap;
    for ( uint32_t y = 0; y < bitmap.rows; y++ )
    {
        for ( uint32_t x = 0; x < bitmap.width; x++ )
        {
            const uint32_t atlasOffset = ( rect.Y + y ) * font->AtlasWidth + ( rect.X + x );
            // Check bounds to prevent buffer overruns
            if ( atlasOffset >= font->AtlasBitmap.size( ) )
            {
                LOG( ERROR ) << "Atlas offset out of bounds: " << atlasOffset << " >= " << font->AtlasBitmap.size( );
                continue;
            }

            if ( bitmap.pixel_mode == FT_PIXEL_MODE_GRAY )
            {
                font->AtlasBitmap[ atlasOffset ] = bitmap.buffer[ y * bitmap.pitch + x ];
            }
            else if ( bitmap.pixel_mode == FT_PIXEL_MODE_MONO )
            {
                // 1-bit monochrome - convert to grayscale
                const uint8_t byte               = bitmap.buffer[ y * bitmap.pitch + x / 8 ];
                const uint8_t bit                = byte >> ( 7 - x % 8 ) & 1;
                font->AtlasBitmap[ atlasOffset ] = bit ? 255 : 0;
            }
        }
    }
}

void FontManager::GenerateTextVertices( const std::shared_ptr<FontAsset> &font, const std::u32string &text, std::vector<float> &vertices, std::vector<uint32_t> &indices,
                                        const float x, const float y, const XMFLOAT4 &color, const float scale )
{
    if ( !font || text.empty( ) )
    {
        return;
    }

    EnsureGlyphsLoaded( font, text );

    float penX = x;
    float penY = y;

    uint32_t    baseVertex = vertices.size( ) / 6; // 6 floats per vertex (pos.xy, uv.xy, color.rgba), TODO, should this be configurable?
    const float lineHeight = static_cast<float>( font->PixelSize ) * scale * 1.2f;
    for ( const char32_t c : text )
    {
        if ( c == '\n' )
        {
            penX = x;
            penY += lineHeight;
            continue;
        }

        auto it = font->GlyphCache.find( c );
        if ( it == font->GlyphCache.end( ) )
        {
            // Skip characters we couldn't load
            continue;
        }

        const GlyphMetrics &metrics = it->second;

        // Skip invisible glyphs
        if ( metrics.Width == 0 || metrics.Height == 0 )
        {
            penX += static_cast<float>( metrics.Advance ) * scale;
            continue;
        }

        // Calculate vertex positions and UVs
        float x0 = penX + static_cast<float>( metrics.BearingX ) * scale;
        float y0 = penY - static_cast<float>( metrics.BearingY ) * scale;
        float x1 = x0 + static_cast<float>( metrics.Width ) * scale;
        float y1 = y0 + static_cast<float>( metrics.Height ) * scale;

        // Calculate UVs
        float u0 = static_cast<float>( metrics.AtlasX ) / static_cast<float>( font->AtlasWidth );
        float v0 = static_cast<float>( metrics.AtlasY ) / static_cast<float>( font->AtlasHeight );
        float u1 = static_cast<float>( metrics.AtlasX + metrics.Width ) / static_cast<float>( font->AtlasWidth );
        float v1 = static_cast<float>( metrics.AtlasY + metrics.Height ) / static_cast<float>( font->AtlasHeight );

        // Top-left
        vertices.push_back( x0 );
        vertices.push_back( y0 );
        vertices.push_back( u0 );
        vertices.push_back( v0 );
        vertices.push_back( color.x );
        vertices.push_back( color.y );
        vertices.push_back( color.z );
        vertices.push_back( color.w );

        // Top-right
        vertices.push_back( x1 );
        vertices.push_back( y0 );
        vertices.push_back( u1 );
        vertices.push_back( v0 );
        vertices.push_back( color.x );
        vertices.push_back( color.y );
        vertices.push_back( color.z );
        vertices.push_back( color.w );

        // Bottom-left
        vertices.push_back( x0 );
        vertices.push_back( y1 );
        vertices.push_back( u0 );
        vertices.push_back( v1 );
        vertices.push_back( color.x );
        vertices.push_back( color.y );
        vertices.push_back( color.z );
        vertices.push_back( color.w );

        // Bottom-right
        vertices.push_back( x1 );
        vertices.push_back( y1 );
        vertices.push_back( u1 );
        vertices.push_back( v1 );
        vertices.push_back( color.x );
        vertices.push_back( color.y );
        vertices.push_back( color.z );
        vertices.push_back( color.w );

        indices.push_back( baseVertex + 0 );
        indices.push_back( baseVertex + 1 );
        indices.push_back( baseVertex + 2 );
        indices.push_back( baseVertex + 1 );
        indices.push_back( baseVertex + 3 );
        indices.push_back( baseVertex + 2 );

        penX += static_cast<float>( metrics.Advance ) * scale;
        baseVertex += 4;
    }
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

        // Add the code point to the result if valid
        if ( codePoint != 0 )
        {
            result.push_back( codePoint );
        }
    }

    return result;
}
