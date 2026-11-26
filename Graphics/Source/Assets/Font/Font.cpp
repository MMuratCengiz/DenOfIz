/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"

#include <SheenBidi/SheenBidi.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <harfbuzz/hb-ot.h>
#include <miniz/miniz.h>
#include <string>
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace
{
#include "DenOfIzGraphicsInternal/Assets/Font/Embedded/DefaultFontCompressed.inl"

    std::atomic<uint32_t> g_nextFontId{ 1 };

    hb_blob_t *DecompressDefaultFont( )
    {
        uint32_t size = 0;
        std::memcpy( &size, g_DefaultFontCompressed, sizeof( uint32_t ) );
        auto    *data   = static_cast<unsigned char *>( std::malloc( size ) );
        mz_ulong length = size;
        if ( mz_uncompress( data, &length, g_DefaultFontCompressed + sizeof( uint32_t ), g_DefaultFontCompressedSize - sizeof( uint32_t ) ) != MZ_OK || length != size )
        {
            spdlog::critical( "Failed to decompress the embedded default font" );
            length = 0;
        }
        return hb_blob_create( reinterpret_cast<const char *>( data ), static_cast<unsigned int>( length ), HB_MEMORY_MODE_WRITABLE, data, std::free );
    }

    DenOfIz_Font NewFont( hb_blob_t *blob, const uint32_t faceIndex )
    {
        if ( blob == nullptr )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *font = new Font( blob, faceIndex );
        if ( !font->IsValid( ) )
        {
            spdlog::error( "Font data does not contain a font face at index {}", faceIndex );
            delete font;
            return DENOFIZ_NULL_HANDLE;
        }
        return DENOFIZ_TO_HANDLE( font );
    }
} // namespace

Font::Font( hb_blob_t *blob, const uint32_t faceIndex ) :
    m_blob( blob ), m_face( hb_face_create( blob, faceIndex ) ), m_font( hb_font_create( m_face ) ), m_id( g_nextFontId++ ), m_upem( std::max( 1u, hb_face_get_upem( m_face ) ) ),
    m_colour( hb_ot_color_has_layers( m_face ) || hb_ot_color_has_paint( m_face ) || hb_ot_color_has_png( m_face ) )
{
    hb_font_set_scale( m_font, static_cast<int>( m_upem ), static_cast<int>( m_upem ) );
    hb_font_make_immutable( m_font );

    const float       em = 1.0f / static_cast<float>( m_upem );
    hb_font_extents_t extents{ };
    hb_font_get_h_extents( m_font, &extents );
    m_metrics.Ascent     = static_cast<float>( extents.ascender ) * em;
    m_metrics.Descent    = static_cast<float>( -extents.descender ) * em;
    m_metrics.LineGap    = static_cast<float>( std::max( 0, extents.line_gap ) ) * em;
    m_metrics.LineHeight = m_metrics.Ascent + m_metrics.Descent + m_metrics.LineGap;

    hb_position_t value = 0;
    hb_ot_metrics_get_position_with_fallback( m_font, HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &value );
    m_metrics.UnderlinePos = static_cast<float>( -value ) * em;
    hb_ot_metrics_get_position_with_fallback( m_font, HB_OT_METRICS_TAG_UNDERLINE_SIZE, &value );
    m_metrics.UnderlineThickness = static_cast<float>( value ) * em;
    hb_ot_metrics_get_position_with_fallback( m_font, HB_OT_METRICS_TAG_CAP_HEIGHT, &value );
    m_metrics.CapHeight = static_cast<float>( value ) * em;
}

Font::~Font( )
{
    hb_font_destroy( m_font );
    hb_face_destroy( m_face );
    hb_blob_destroy( m_blob );
}

bool Font::IsValid( ) const
{
    return hb_face_get_glyph_count( m_face ) > 0;
}

uint32_t Font::Id( ) const
{
    return m_id;
}

uint32_t Font::Upem( ) const
{
    return m_upem;
}

hb_font_t *Font::HbFont( ) const
{
    return m_font;
}

const DenOfIz_FontMetrics &Font::Metrics( ) const
{
    return m_metrics;
}

bool Font::IsColour( ) const
{
    return m_colour;
}

bool Font::HasCodePoint( const uint32_t codePoint ) const
{
    hb_codepoint_t glyph = 0;
    return hb_font_get_nominal_glyph( m_font, codePoint, &glyph );
}

bool Font::Covers( const char *text, const uint32_t length ) const
{
    for ( SBUInteger i = 0; i < length; )
    {
        const SBCodepoint codePoint = SBCodepointDecodeNextFromUTF8( reinterpret_cast<const SBUInt8 *>( text ), length, &i );
        // HarfBuzz hides default ignorables a font lacks, but a tag sequence only shows as a flag in a font that has the tags.
        const bool ignored = TextIsDefaultIgnorable( codePoint ) && ( codePoint < 0xE0020 || codePoint > 0xE007F );
        if ( !ignored && !HasCodePoint( codePoint ) )
        {
            return false;
        }
    }
    return true;
}

Font *Font::Default( )
{
    static Font font( DecompressDefaultFont( ), 0 );
    return &font;
}

extern "C"
{
    DenOfIz_Font DenOfIz_Font_Create( const DenOfIz_FontDesc *desc )
    {
        if ( desc == nullptr || desc->Data.Elements == nullptr || desc->Data.NumElements == 0 )
        {
            spdlog::error( "DenOfIz_Font_Create: font data is empty" );
            return DENOFIZ_NULL_HANDLE;
        }
        const auto *data = reinterpret_cast<const char *>( desc->Data.Elements );
        return NewFont( hb_blob_create_or_fail( data, static_cast<unsigned int>( desc->Data.NumElements ), HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr ), desc->FaceIndex );
    }

    DenOfIz_Font DenOfIz_Font_CreateFromPath( const DenOfIz_StringView path, const uint32_t faceIndex )
    {
        const std::string file = FileIO::GetResourcePath( path );
        hb_blob_t        *blob = hb_blob_create_from_file_or_fail( file.c_str( ) );
        if ( blob == nullptr )
        {
            spdlog::error( "DenOfIz_Font_CreateFromPath: cannot read {}", file );
            return DENOFIZ_NULL_HANDLE;
        }
        return NewFont( blob, faceIndex );
    }

    void DenOfIz_Font_Destroy( const DenOfIz_Font font )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( font ) && FONT_IMPL( font ) != Font::Default( ) )
        {
            delete FONT_IMPL( font );
        }
    }

    DenOfIz_Font DenOfIz_Font_GetDefault( )
    {
        return DENOFIZ_TO_HANDLE( Font::Default( ) );
    }

    DenOfIz_FontMetrics DenOfIz_Font_GetMetrics( const DenOfIz_Font font )
    {
        return DENOFIZ_HANDLE_IS_VALID( font ) ? FONT_IMPL( font )->Metrics( ) : DenOfIz_FontMetrics{ };
    }

    bool DenOfIz_Font_HasCodePoint( const DenOfIz_Font font, const uint32_t codePoint )
    {
        return DENOFIZ_HANDLE_IS_VALID( font ) && FONT_IMPL( font )->HasCodePoint( codePoint );
    }
}
