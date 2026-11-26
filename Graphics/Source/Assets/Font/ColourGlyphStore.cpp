/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/ColourGlyphStore.h"

#include <algorithm>
#include <cmath>
#include <harfbuzz/hb-ot.h>
#include <iterator>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"

using namespace DenOfIz;

namespace
{
    // Pixels per em a glyph is painted at, the smallest one covering the requested size. Larger sizes scale the largest bucket up.
    constexpr uint32_t Buckets[] = { 16, 20, 24, 32, 40, 48, 64, 96, 128, 192, 256, 384, 512 };
    // The key of a bitmap glyph drawn above its largest strike, which is kept at the strike's own size.
    constexpr uint32_t StrikeBucket = std::size( Buckets );
    // 16 MiB of texels, and the frames a bitmap is kept without being drawn once they are passed.
    constexpr size_t   PixelBudget = 4u << 20;
    constexpr uint64_t MaxAge      = 300;

    uint32_t BucketOf( const float pixelsPerEm )
    {
        uint32_t bucket = 0;
        while ( bucket + 1 < std::size( Buckets ) && pixelsPerEm > static_cast<float>( Buckets[ bucket ] ) )
        {
            ++bucket;
        }
        return bucket;
    }

    bool HasColour( const Font &font, const uint32_t glyphId )
    {
        hb_face_t *face = hb_font_get_face( font.HbFont( ) );
        if ( hb_ot_color_glyph_has_paint( face, glyphId ) || hb_ot_color_glyph_get_layers( face, glyphId, 0, nullptr, nullptr ) > 0 )
        {
            return true;
        }
        hb_blob_t *png    = hb_ot_color_glyph_reference_png( font.HbFont( ), glyphId );
        const bool bitmap = hb_blob_get_length( png ) > 0;
        hb_blob_destroy( png );
        return bitmap;
    }

    // Pixels per em of the glyph's largest bitmap strike, from its PNG width and the extents HarfBuzz scales it to. COLR glyphs have no limit.
    uint32_t LargestStrike( const Font &font, const uint32_t glyphId )
    {
        hb_blob_t         *png    = hb_ot_color_glyph_reference_png( font.HbFont( ), glyphId );
        unsigned int       length = 0;
        const auto        *data   = reinterpret_cast<const uint8_t *>( hb_blob_get_data( png, &length ) );
        hb_glyph_extents_t extents{ };
        uint32_t           ppem = UINT32_MAX;
        // The width is big endian in the IHDR chunk, after the 8 byte signature and the chunk's length and type.
        if ( length >= 24 && hb_font_get_glyph_extents( font.HbFont( ), glyphId, &extents ) && extents.width > 0 )
        {
            const uint32_t width = static_cast<uint32_t>( data[ 16 ] ) << 24 | static_cast<uint32_t>( data[ 17 ] ) << 16 | static_cast<uint32_t>( data[ 18 ] ) << 8 | data[ 19 ];
            ppem                 = static_cast<uint32_t>( std::max( 1l, std::lround( static_cast<double>( width ) * font.Upem( ) / extents.width ) ) );
        }
        hb_blob_destroy( png );
        return ppem;
    }
} // namespace

uint32_t ColourGlyphStore::Find( const Font *font, const uint32_t glyphId )
{
    if ( !font->IsColour( ) )
    {
        return None;
    }
    const auto [ it, inserted ] = m_sourceLookup.try_emplace( static_cast<uint64_t>( font->Id( ) ) << 32 | glyphId, None );
    if ( inserted && HasColour( *font, glyphId ) )
    {
        it->second = static_cast<uint32_t>( m_sources.size( ) );
        m_sources.push_back( Source{ font, glyphId, LargestStrike( *font, glyphId ), false } );
    }
    return it->second;
}

const ColourGlyph &ColourGlyphStore::Get( const uint32_t source, const float pixelsPerEm, const uint32_t foreground )
{
    Source  &glyph  = m_sources[ source ];
    uint32_t bucket = BucketOf( pixelsPerEm );
    uint32_t ppem   = Buckets[ bucket ];
    if ( ppem > glyph.LargestStrike )
    {
        bucket = StrikeBucket;
        ppem   = glyph.LargestStrike;
    }
    const uint32_t opaque = foreground | 0xFF;
    auto           key    = [ & ]( const uint32_t colour ) { return static_cast<uint64_t>( source ) << 36 | static_cast<uint64_t>( bucket ) << 32 | colour; };
    auto [ it, inserted ] = m_glyphs.try_emplace( key( glyph.UsesForeground ? opaque : 0 ) );
    it->second.LastUsed   = m_frame;
    if ( !inserted )
    {
        return it->second;
    }

    // The text colour's alpha is applied when drawing, foreground layers are painted opaque.
    ColourBitmap bitmap;
    if ( RasteriseColourGlyph( *glyph.FontPtr, glyph.GlyphId, ppem, HB_COLOR( opaque >> 8 & 0xFF, opaque >> 16 & 0xFF, opaque >> 24, 0xFF ), bitmap ) && bitmap.UsesForeground &&
         !glyph.UsesForeground )
    {
        // Only known after the first paint: from now on every colour gets its own bitmap.
        glyph.UsesForeground = true;
        m_glyphs.erase( it );
        it = m_glyphs.try_emplace( key( opaque ) ).first;
    }
    ColourGlyph &entry = it->second;
    entry.Ppem         = ppem;
    entry.LastUsed     = m_frame;
    if ( bitmap.Width > 0 )
    {
        entry = ColourGlyph{ bitmap.Left, bitmap.Top, bitmap.Width, bitmap.Height, static_cast<uint32_t>( m_pixels.size( ) ), ppem, m_frame };
        m_pixels.push_back( bitmap.Width | bitmap.Height << 16 );
        m_pixels.insert( m_pixels.end( ), bitmap.Pixels.begin( ), bitmap.Pixels.end( ) );
    }
    return entry;
}

// Packs only when it at least halves the stream, so a stream mostly in use just grows to the next threshold.
void ColourGlyphStore::BeginFrame( )
{
    ++m_frame;
    if ( m_pixels.size( ) <= std::max( m_packAt, PixelBudget ) )
    {
        return;
    }
    const auto texels = []( const ColourGlyph &glyph ) { return glyph.Width > 0 ? 1 + static_cast<size_t>( glyph.Width ) * glyph.Height : 0; };
    const auto stale  = [ this ]( const ColourGlyph &glyph ) { return glyph.LastUsed + MaxAge < m_frame; };
    size_t     live   = 0;
    for ( const auto &[ key, glyph ] : m_glyphs )
    {
        live += stale( glyph ) ? 0 : texels( glyph );
    }
    if ( live * 2 > m_pixels.size( ) )
    {
        m_packAt = m_pixels.size( ) * 2;
        return;
    }

    std::vector<uint32_t> pixels;
    pixels.reserve( live );
    for ( auto it = m_glyphs.begin( ); it != m_glyphs.end( ); )
    {
        ColourGlyph &glyph = it->second;
        if ( stale( glyph ) )
        {
            it = m_glyphs.erase( it );
            continue;
        }
        const auto first = m_pixels.begin( ) + glyph.Offset;
        glyph.Offset     = static_cast<uint32_t>( pixels.size( ) );
        pixels.insert( pixels.end( ), first, first + static_cast<ptrdiff_t>( texels( glyph ) ) );
        ++it;
    }
    m_pixels   = std::move( pixels );
    m_uploaded = 0;
    m_packAt   = m_pixels.size( ) * 2;
    ++m_generation;
}

uint32_t ColourGlyphStore::Generation( ) const
{
    return m_generation;
}

const std::vector<uint32_t> &ColourGlyphStore::GetPixels( ) const
{
    return m_pixels;
}

size_t ColourGlyphStore::GetUploadMark( ) const
{
    return m_uploaded;
}

bool ColourGlyphStore::HasPendingUpload( ) const
{
    return m_uploaded != m_pixels.size( );
}

void ColourGlyphStore::MarkUploaded( )
{
    m_uploaded = m_pixels.size( );
}
