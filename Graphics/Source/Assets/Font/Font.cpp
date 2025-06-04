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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/FontImpl.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

Font::Font( FT_Library ftLibrary, const FontDesc &desc ) : m_impl( std::make_unique<FontImpl>( ftLibrary ) ), m_desc( desc )
{
    const Byte    *data         = desc.FontAsset->Data.Data( );
    const uint64_t dataNumBytes = desc.FontAsset->DataNumBytes;
    if ( FT_New_Memory_Face( m_impl->m_ftLibrary, data, dataNumBytes, 0, &m_impl->m_face ) )
    {
        LOG( ERROR ) << "Failed to load font: " << desc.FontAsset->Uri.Path.Get( );
    }

    for ( int i = 0; i < m_desc.FontAsset->Glyphs.NumElements( ); i++ )
    {
        const FontGlyph &glyph      = m_desc.FontAsset->Glyphs.GetElement( i );
        m_glyphs[ glyph.CodePoint ] = glyph;
    }

    const uint32_t initialFontSize = m_desc.FontAsset->InitialFontSize * 64;
    if ( const FT_Error error = FT_Set_Char_Size( m_impl->m_face, 0, initialFontSize, 0, 0 ) )
    {
        LOG( ERROR ) << "Failed to set font size: " << FT_Error_String( error );
    }
}

FT_Face Font::GetFTFace( ) const
{
    return m_impl->m_face;
}

hb_font_t *Font::GetHBFont( const uint32_t fontSize ) const
{
    std::lock_guard lock( m_impl->m_hbFontsMutex );
    const auto      it = m_impl->m_hbFonts.find( fontSize );
    if ( it != m_impl->m_hbFonts.end( ) )
    {
        return it->second;
    }
    hb_font_t *hbFont = nullptr;
    {
        std::lock_guard faceLock( m_impl->m_faceMutex );
        const uint32_t              sizeIn26_6 = fontSize * 64;
        if ( const FT_Error error = FT_Set_Char_Size( m_impl->m_face, 0, sizeIn26_6, 0, 0 ) )
        {
            LOG( ERROR ) << "Failed to set font size: " << FT_Error_String( error );
            return nullptr;
        }

        hbFont = hb_ft_font_create_referenced( m_impl->m_face );
    }
    if ( !hbFont )
    {
        LOG( ERROR ) << "Failed to create HarfBuzz font for size " << fontSize;
        return nullptr;
    }
    m_impl->m_hbFonts[ fontSize ] = hbFont;
    return hbFont;
}

FontAsset *Font::Asset( ) const
{
    return m_desc.FontAsset;
}

Font::~Font( ) = default;

FontGlyph *Font::GetGlyph( const uint32_t codePoint )
{
    if ( !m_glyphs.contains( codePoint ) )
    {
        return nullptr;
    }
    return &m_glyphs[ codePoint ];
}
