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

std::shared_ptr<FontCache> FontManager::LoadFont( const std::string &fontPath, const uint32_t pixelSize, const bool antiAliasing )
{
    const std::string cacheKey = fontPath + "_" + std::to_string( pixelSize );
    if ( const auto it = m_fontCache.find( cacheKey ); it != m_fontCache.end( ) )
    {
        return it->second;
    }

    const auto fontAsset    = std::make_shared<FontAsset>( );
    fontAsset->FontPath     = fontPath.c_str( );
    fontAsset->PixelSize    = pixelSize;
    fontAsset->AntiAliasing = antiAliasing;

    auto fontCache = std::make_shared<FontCache>( *fontAsset );
    fontCache->InitializeAtlasBitmap( );

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
        LoadGlyph( fontCache, c, face );
    }

    FT_Done_Face( face );
    m_fontCache[ cacheKey ] = fontCache;

    return fontCache;
}

std::shared_ptr<FontCache> FontManager::GetFont( const std::string &fontPath, const uint32_t pixelSize )
{
    const std::string cacheKey = fontPath + "_" + std::to_string( pixelSize );

    if ( const auto it = m_fontCache.find( cacheKey ); it != m_fontCache.end( ) )
    {
        return it->second;
    }

    return nullptr;
}

bool FontManager::EnsureGlyphsLoaded( const std::shared_ptr<FontCache> &fontCache, const std::u32string &text )
{
    if ( !fontCache )
    {
        return false;
    }

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

    for ( const char32_t c : text )
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

bool FontManager::LoadGlyph( const std::shared_ptr<FontCache> &fontCache, const uint32_t codePoint, const FT_Face face ) // NOLINT
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

void FontManager::GenerateTextVertices( const std::shared_ptr<FontCache> &fontCache, const std::u32string &text, std::vector<float> &vertices, std::vector<uint32_t> &indices,
                                        const float x, const float y, const XMFLOAT4 &color, const float scale )
{
    if ( !fontCache || text.empty( ) )
    {
        return;
    }

    EnsureGlyphsLoaded( fontCache, text );

    float penX = x;
    float penY = y;

    const FontAsset &fontAsset = fontCache->GetFontAsset( );

    uint32_t    baseVertex = vertices.size( ) / 8; // 8 floats per vertex (pos.xy, uv.xy, color.rgba)
    const float lineHeight = static_cast<float>( fontAsset.Metrics.LineHeight ) * scale;

    for ( const char32_t c : text )
    {
        if ( c == '\n' )
        {
            penX = x;
            penY += lineHeight;
            continue;
        }

        const GlyphMetrics *metrics = fontCache->GetGlyphMetrics( c );
        if ( !metrics )
        {
            // Skip characters we couldn't load
            continue;
        }

        // Skip invisible glyphs
        if ( metrics->Width == 0 || metrics->Height == 0 )
        {
            penX += static_cast<float>( metrics->Advance ) * scale;
            continue;
        }

        // Calculate vertex positions and UVs
        float x0 = penX + static_cast<float>( metrics->BearingX ) * scale;
        float y0 = penY - static_cast<float>( metrics->BearingY ) * scale;
        float x1 = x0 + static_cast<float>( metrics->Width ) * scale;
        float y1 = y0 + static_cast<float>( metrics->Height ) * scale;

        // Calculate UVs
        float u0 = static_cast<float>( metrics->AtlasX ) / static_cast<float>( fontAsset.AtlasWidth );
        float v0 = static_cast<float>( metrics->AtlasY ) / static_cast<float>( fontAsset.AtlasHeight );
        float u1 = static_cast<float>( metrics->AtlasX + metrics->Width ) / static_cast<float>( fontAsset.AtlasWidth );
        float v1 = static_cast<float>( metrics->AtlasY + metrics->Height ) / static_cast<float>( fontAsset.AtlasHeight );

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

        penX += static_cast<float>( metrics->Advance ) * scale;
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
