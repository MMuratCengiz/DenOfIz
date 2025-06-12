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

#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include <freetype/freetype.h>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"

using namespace DenOfIz;

FontLibrary::FontLibrary( )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ) )
    {
        spdlog::error( "Failed to initialize FreeType library: {}", FT_Error_String( error ) );
    }
}

FontLibrary::~FontLibrary( )
{
    // Clear fonts before destroying FreeType library
    m_fonts.clear( );
    m_fontStore.clear( );

    if ( m_ftLibrary )
    {
        FT_Done_FreeType( m_ftLibrary );
    }
}

// Note keep DenOfIz::Font here to disambiguate in Linux
DenOfIz::Font *FontLibrary::LoadFont( const FontDesc &desc )
{
    std::lock_guard lock( m_mutex );
    if ( desc.FontAsset->Uri.Path.IsEmpty( ) )
    {
        return m_fontStore.emplace_back( std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) ) ).get( );
    }

    auto uriPath = desc.FontAsset->Uri.Path.Get( );
    if ( m_fonts.contains( uriPath ) )
    {
        return m_fonts[ uriPath ].get( );
    }
    return m_fonts.emplace( uriPath, std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) ) ).first->second.get( );
}

// Note keep DenOfIz::Font here to disambiguate in Linux
DenOfIz::Font *FontLibrary::LoadFont( const InteropString &ttf )
{
    BinaryContainer targetContainer{ };

    FontImportDesc fontImport{ };
    fontImport.TargetContainer = &targetContainer;

    ImportJobDesc importJob{ };
    importJob.SourceFilePath = ttf;
    importJob.Desc           = &fontImport;
    m_fontImporter.Import( importJob );

    BinaryReader    reader( targetContainer );
    FontAssetReader fontReader( { &reader } );
    auto           &asset = m_assets.emplace_back( std::unique_ptr<FontAsset>( fontReader.Read( ) ) );

    FontDesc desc{ };
    desc.FontAsset = asset.get( );
    auto font      = std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) );

    m_fonts.emplace( ttf.Get( ), std::move( font ) );
    return m_fonts[ ttf.Get( ) ].get( );
}
