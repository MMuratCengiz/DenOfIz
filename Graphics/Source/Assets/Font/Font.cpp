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

#include <DenOfIzGraphics/Assets/Font/Font.h>

using namespace DenOfIz;

Font::Font( const FT_Library library, const FontDesc &desc ) : m_ftLibrary( library ), m_desc( desc )
{
    const Byte    *data         = desc.FontAsset->Data.Data( );
    const uint64_t dataNumBytes = desc.FontAsset->DataNumBytes;
    if ( FT_New_Memory_Face( m_ftLibrary, data, dataNumBytes, 0, &m_face ) )
    {
        LOG( ERROR ) << "Failed to load font: " << desc.FontAsset->Uri.Path.Get( );
    }

    for ( int i = 0; i < m_desc.FontAsset->Glyphs.NumElements( ); i++ )
    {
        const FontGlyph &glyph      = m_desc.FontAsset->Glyphs.GetElement( i );
        m_glyphs[ glyph.CodePoint ] = glyph;
    }

    const uint32_t initialFontSize = m_desc.FontAsset->InitialFontSize * 64;
    if ( const FT_Error error = FT_Set_Char_Size( m_face, initialFontSize, initialFontSize, 0, 0 ) )
    {
        LOG( ERROR ) << "Failed to set font size: " << FT_Error_String( error );
    }
}

FT_Face Font::FTFace( ) const
{
    return m_face;
}

FontAsset *Font::Asset( ) const
{
    return m_desc.FontAsset;
}

Font::~Font( )
{
    if ( m_face )
    {
        FT_Done_Face( m_face );
    }
}

FontGlyph *Font::GetGlyph( const uint32_t codePoint )
{
    if ( !m_glyphs.contains( codePoint ) )
    {
        return nullptr;
    }
    return &m_glyphs[ codePoint ];
}
