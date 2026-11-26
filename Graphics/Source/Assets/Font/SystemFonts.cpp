/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"

#include <algorithm>
#include <harfbuzz/hb-ot.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace
{
    struct SystemFonts
    {
        std::mutex                                             Mutex;
        std::unordered_map<std::string, std::unique_ptr<Font>> Files;
        std::unordered_map<std::string, Font *>                Clusters;
        std::unordered_map<uint32_t, std::vector<Font *>>      Scripts;
        std::unordered_map<std::string, std::vector<Font *>>   Replacements;
    };

    constexpr uint32_t EmojiScript = UINT32_MAX;

    SystemFonts &Instance( )
    {
        static SystemFonts fonts;
        return fonts;
    }

    uint32_t FaceOf( hb_blob_t *blob, const SystemFontFile &file )
    {
        const unsigned int count = file.PostScriptName.empty( ) ? 0 : hb_face_count( blob );
        for ( unsigned int i = 0; count > 1 && i < count; ++i )
        {
            hb_face_t   *face = hb_face_create( blob, i );
            char         name[ 256 ];
            unsigned int size = sizeof( name );
            hb_ot_name_get_utf8( face, HB_OT_NAME_ID_POSTSCRIPT_NAME, HB_LANGUAGE_INVALID, &size, name );
            hb_face_destroy( face );
            if ( file.PostScriptName == name )
            {
                return i;
            }
        }
        return file.FaceIndex;
    }

    std::string Key( const SystemFontFile &file )
    {
        return file.Path + '\n' + file.PostScriptName + '\n' + std::to_string( file.FaceIndex );
    }

    bool Drawable( const Font &font )
    {
        hb_face_t *face  = hb_font_get_face( font.HbFont( ) );
        auto       table = [ face ]( const hb_tag_t tag )
        {
            hb_blob_t *blob   = hb_face_reference_table( face, tag );
            const bool exists = hb_blob_get_length( blob ) > 0;
            hb_blob_destroy( blob );
            return exists;
        };
        return font.IsColour( ) || table( HB_TAG( 'g', 'l', 'y', 'f' ) ) || table( HB_TAG( 'C', 'F', 'F', ' ' ) ) || table( HB_TAG( 'C', 'F', 'F', '2' ) );
    }

    Font *Load( SystemFonts &fonts, const SystemFontFile &file )
    {
        auto [ it, inserted ] = fonts.Files.try_emplace( Key( file ) );
        if ( !inserted )
        {
            return it->second.get( );
        }
        hb_blob_t *blob = hb_blob_create_from_file_or_fail( file.Path.c_str( ) );
        if ( blob == nullptr )
        {
            return nullptr;
        }
        const uint32_t faceIndex = FaceOf( blob, file );
        auto           font      = std::make_unique<Font>( blob, faceIndex );
        if ( font->IsValid( ) && Drawable( *font ) )
        {
            spdlog::info( "Fallback font: {} face {}", file.Path, faceIndex );
            it->second = std::move( font );
        }
        return it->second.get( );
    }
} // namespace

Font *DenOfIz::SystemFallbackFont( const char *text, const uint32_t length, const hb_script_t script, const bool colour )
{
    SystemFonts          &fonts = Instance( );
    const std::lock_guard lock( fonts.Mutex );
    auto [ cluster, inserted ] = fonts.Clusters.try_emplace( std::string( text, length ) + static_cast<char>( colour ), nullptr );
    if ( !inserted )
    {
        return cluster->second;
    }

    std::vector<Font *> &scriptFonts = fonts.Scripts[ colour ? EmojiScript : script ];
    for ( Font *font : scriptFonts )
    {
        if ( font->Covers( text, length ) )
        {
            return cluster->second = font;
        }
    }
    SystemFontFile file;
    if ( !FindSystemFont( text, length, file ) )
    {
        return nullptr;
    }
    Font *font = Load( fonts, file );
    if ( font == nullptr || !font->Covers( text, length ) )
    {
        font                              = nullptr;
        std::vector<Font *> &replacements = fonts.Replacements[ Key( file ) ];
        const auto           covering     = std::ranges::find_if( replacements, [ & ]( const Font *replacement ) { return replacement->Covers( text, length ); } );
        if ( covering != replacements.end( ) )
        {
            font = *covering;
        }
        else
        {
            std::vector<SystemFontFile> candidates;
            FindSystemFonts( text, length, candidates );
            for ( const SystemFontFile &candidate : candidates )
            {
                Font *loaded = Load( fonts, candidate );
                if ( loaded != nullptr && loaded->Covers( text, length ) )
                {
                    replacements.push_back( font = loaded );
                    break;
                }
            }
        }
    }
    if ( font == nullptr )
    {
        return nullptr;
    }
    if ( font->IsColour( ) != colour )
    {
        std::vector<SystemFontFile> candidates;
        FindSystemFonts( text, length, candidates );
        for ( const SystemFontFile &candidate : candidates )
        {
            Font *loaded = Load( fonts, candidate );
            if ( loaded != nullptr && loaded->IsColour( ) == colour && loaded->Covers( text, length ) )
            {
                font = loaded;
                break;
            }
        }
    }
    if ( font->IsColour( ) == colour )
    {
        scriptFonts.push_back( font );
    }
    return cluster->second = font;
}
