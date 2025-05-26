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

#include <DenOfIzGraphics/Assets/Font/FontLibrary.h>

#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"

using namespace DenOfIz;

FontLibrary::FontLibrary( )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ) )
    {
        LOG( ERROR ) << "Failed to initialize FreeType library: " << FT_Error_String( error );
    }
}

Font *FontLibrary::LoadFont( const FontDesc &desc )
{
    std::lock_guard lock( m_mutex );
    return new Font( m_ftLibrary, desc );
}

Font *FontLibrary::LoadFont( const InteropString &ttf )
{
    BinaryReader reader( ttf );

    FontAssetReader fontReader( { &reader } );
    FontAsset& asset = m_assets.emplace_back( fontReader.Read( ) );

    FontDesc desc{};
    desc.FontAsset = &asset;
    auto font = std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) );

    m_fonts.emplace( ttf.Get( ), std::move( font ) );
    return m_fonts[  ttf.Get( ) ].get( );
}
