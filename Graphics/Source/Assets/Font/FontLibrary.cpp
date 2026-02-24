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

#include "DenOfIzGraphicsInternal/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

FontLibrary::FontLibrary( ) : m_fontImporter( DenOfIz_FontImporter_Create( ) )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ) )
    {
        spdlog::error( "Failed to initialize FreeType library: {}", FT_Error_String( error ) );
    }
}

FontLibrary::~FontLibrary( )
{
    m_fonts.clear( );
    m_fontStore.clear( );

    if ( DENOFIZ_HANDLE_IS_VALID( m_fontImporter ) )
    {
        DenOfIz_FontImporter_Destroy( m_fontImporter );
    }

    if ( m_ftLibrary )
    {
        FT_Done_FreeType( m_ftLibrary );
    }
}

Font *FontLibrary::LoadFont( const DenOfIz_FontDesc &desc )
{
    std::lock_guard lock( m_mutex );
    if ( DenOfIz_FontAsset_Path( desc.FontAsset ).NumChars == 0 )
    {
        return m_fontStore.emplace_back( std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) ) ).get( );
    }

    const auto uriPath    = DenOfIz_FontAsset_Path( desc.FontAsset );
    auto       uriPathStr = std::string( uriPath.Chars, uriPath.NumChars );
    if ( m_fonts.contains( uriPathStr ) )
    {
        return m_fonts[ uriPathStr ].get( );
    }
    return m_fonts.emplace( uriPathStr, std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) ) ).first->second.get( );
}

Font *FontLibrary::LoadFont( const DenOfIz_StringView &ttf )
{
    DenOfIz_BinaryContainer targetContainer = DenOfIz_BinaryContainer_Create( );

    DenOfIz_FontImportDesc fontImport{ };
    fontImport.InitialFontSize = 36;
    fontImport.AtlasWidth      = 512;
    fontImport.AtlasHeight     = 512;
    fontImport.SourceFilePath  = ttf;
    fontImport.TargetContainer = targetContainer;

    DenOfIz_FontImporter_Import( m_fontImporter, &fontImport );

    DenOfIz_BinaryReaderDesc binaryReaderDesc{ };
    binaryReaderDesc.NumBytes   = 0;
    DenOfIz_BinaryReader reader = DenOfIz_BinaryReader_CreateFromContainer( targetContainer, &binaryReaderDesc );

    DenOfIz_FontAssetReaderDesc fontReaderDesc{ };
    fontReaderDesc.Reader              = reader;
    DenOfIz_FontAssetReader fontReader = DenOfIz_FontAssetReader_Create( &fontReaderDesc );
    DenOfIz_FontAsset       asset      = DenOfIz_FontAssetReader_Read( fontReader );
    DenOfIz_FontAssetReader_Destroy( fontReader );
    m_assets.push_back( asset );

    DenOfIz_BinaryContainer_Destroy( targetContainer );

    DenOfIz_FontDesc desc{ };
    desc.FontAsset = asset;
    auto font      = std::unique_ptr<Font>( new Font( m_ftLibrary, desc ) );

    std::string ttfStr( ttf.Chars, ttf.NumChars );
    m_fonts.emplace( ttfStr, std::move( font ) );
    return m_fonts[ ttfStr ].get( );
}

extern "C"
{
    DenOfIz_FontLibrary DenOfIz_FontLibrary_Create( )
    {
        auto *fontLibrary = new FontLibrary( );
        return DENOFIZ_TO_HANDLE( fontLibrary );
    }

    void DenOfIz_FontLibrary_Destroy( DenOfIz_FontLibrary fontLibrary )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontLibrary ) )
        {
            return;
        }
        delete FONT_LIBRARY_IMPL( fontLibrary );
    }

    DenOfIz_Font DenOfIz_FontLibrary_LoadFontFromDesc( DenOfIz_FontLibrary fontLibrary, const DenOfIz_FontDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontLibrary ) || desc == nullptr )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        Font *font = FONT_LIBRARY_IMPL( fontLibrary )->LoadFont( *desc );
        return DENOFIZ_TO_HANDLE( font );
    }

    DenOfIz_Font DenOfIz_FontLibrary_LoadFontFromPath( DenOfIz_FontLibrary fontLibrary, DenOfIz_StringView ttfPath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontLibrary ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        Font *font = FONT_LIBRARY_IMPL( fontLibrary )->LoadFont( ttfPath );
        return DENOFIZ_TO_HANDLE( font );
    }
}
