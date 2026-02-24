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

#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

Font::Font( FT_Library ftLibrary, const DenOfIz_FontDesc &desc ) : m_ftLibrary( ftLibrary ), m_desc( desc )
{
    const Byte    *data         = DenOfIz_FontAsset_Data( desc.FontAsset ).Elements;
    const uint64_t dataNumBytes = DenOfIz_FontAsset_Data( desc.FontAsset ).NumElements;
    if ( FT_New_Memory_Face( m_ftLibrary, data, dataNumBytes, 0, &m_baseFace ) )
    {
        const auto  path = DenOfIz_FontAsset_Path( desc.FontAsset );
        std::string pathStr( path.Chars, path.NumChars );
        spdlog::error( "Failed to load font: {}", pathStr );
    }

    for ( uint32_t i = 0; i < DenOfIz_FontAsset_Glyphs( m_desc.FontAsset ).NumElements; i++ )
    {
        const DenOfIz_FontGlyph &glyph = DenOfIz_FontAsset_Glyphs( m_desc.FontAsset ).Elements[ i ];
        m_glyphs[ glyph.CodePoint ]    = glyph;
    }

    m_loadFlags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
}

Font::~Font( )
{
    for ( const auto &font : m_hbFonts | std::views::values )
    {
        if ( font )
        {
            hb_font_destroy( font );
        }
    }

    for ( const auto &face : m_sizedFaces | std::views::values )
    {
        if ( face )
        {
            FT_Done_Face( face );
        }
    }

    if ( m_baseFace )
    {
        FT_Done_Face( m_baseFace );
    }
}

FT_Face Font::GetFTFace( ) const
{
    return m_baseFace;
}

const std::unordered_map<uint32_t, uint32_t> &Font::GetGlyphIndexMap( ) const
{
    if ( !m_glyphMapInitialized )
    {
        std::lock_guard lock( m_mutex );
        if ( !m_glyphMapInitialized )
        {
            for ( const auto &codePoint : m_glyphs | std::views::keys )
            {
                if ( FT_UInt glyphIndex = FT_Get_Char_Index( m_baseFace, codePoint ) )
                {
                    m_glyphIndexToCodePoint[ glyphIndex ] = codePoint;
                }
            }
            m_glyphMapInitialized = true;
        }
    }
    return m_glyphIndexToCodePoint;
}

hb_font_t *Font::GetHBFont( const uint32_t fontSize ) const
{
    std::lock_guard lock( m_mutex );

    const auto hbIt = m_hbFonts.find( fontSize );
    if ( hbIt != m_hbFonts.end( ) )
    {
        return hbIt->second;
    }

    const auto faceIt = m_sizedFaces.find( fontSize );
    FT_Face    face;

    if ( faceIt == m_sizedFaces.end( ) )
    {
        const Byte    *data         = DenOfIz_FontAsset_Data( m_desc.FontAsset ).Elements;
        const uint64_t dataNumBytes = DenOfIz_FontAsset_Data( m_desc.FontAsset ).NumElements;

        if ( FT_New_Memory_Face( m_ftLibrary, data, dataNumBytes, 0, &face ) )
        {
            spdlog::error( "Failed to create face for size {}", fontSize );
            return nullptr;
        }

        const uint32_t sizeIn26_6 = fontSize * 64;
        if ( const FT_Error error = FT_Set_Char_Size( face, 0, sizeIn26_6, 0, 0 ) )
        {
            FT_Done_Face( face );
            spdlog::error( "Failed to set font size: {}", FT_Error_String( error ) );
            return nullptr;
        }

        m_sizedFaces[ fontSize ] = face;
    }
    else
    {
        face = faceIt->second;
    }

    hb_font_t *hbFont = hb_ft_font_create_referenced( face );
    if ( !hbFont )
    {
        spdlog::error( "Failed to create HarfBuzz font for size {}", fontSize );
        return nullptr;
    }

    hb_ft_font_set_load_flags( hbFont, m_loadFlags );
    m_hbFonts[ fontSize ] = hbFont;
    return hbFont;
}

DenOfIz_FontAsset Font::Asset( ) const
{
    return m_desc.FontAsset;
}

DenOfIz_FontGlyph *Font::GetGlyph( const uint32_t codePoint )
{
    if ( !m_glyphs.contains( codePoint ) )
    {
        return nullptr;
    }
    return &m_glyphs[ codePoint ];
}

extern "C"
{
    DenOfIz_FontAsset DenOfIz_Font_Asset( DenOfIz_Font font )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return FONT_IMPL( font )->Asset( );
    }

    DenOfIz_FontGlyph *DenOfIz_Font_GetGlyph( DenOfIz_Font font, uint32_t codePoint )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( font ) )
        {
            return nullptr;
        }
        return FONT_IMPL( font )->GetGlyph( codePoint );
    }

    float DenOfIz_Font_MsdfPixelRange( )
    {
        return DENOFIZ_FONT_MSDF_PIXEL_RANGE;
    }
}
