/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/UI/UIText.h"

#include <harfbuzz/hb.h>
#include <algorithm>
#include <bit>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <limits>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace
{
    constexpr float DefaultFontSize = 14.0f;
    constexpr float WrapEpsilon     = 0.01f;

    uint64_t Mix64( uint64_t h )
    {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }

    uint64_t HashParams( const UITextParams &p )
    {
        uint64_t h = p.FontId;
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.FontSize ) );
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.LetterSpacing ) );
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.LineHeight ) );
        h          = Mix64( h ^ ( static_cast<uint64_t>( p.Wrap ) | static_cast<uint64_t>( p.Overflow ) << 8 | static_cast<uint64_t>( p.MaxLines ) << 16 ) );
        return h;
    }

    // Letter spacing goes after every glyph that advances, so marks and joiners stay attached.
    float GlyphWidth( const TextGlyph &glyph, const float scale, const float spacing, const float spaceAdvance, const bool newlineAsSpace )
    {
        if ( glyph.Flags & TextGlyphFlags::Newline )
        {
            return newlineAsSpace ? spaceAdvance * scale + spacing : 0.0f;
        }
        return glyph.Advance != 0.0f ? glyph.Advance * scale + spacing : 0.0f;
    }

    uint32_t ClusterStart( const std::span<const TextGlyph> glyphs, uint32_t glyph )
    {
        while ( glyph > 0 && glyphs[ glyph - 1 ].Cluster == glyphs[ glyph ].Cluster )
        {
            --glyph;
        }
        return glyph;
    }

    // The glyphs [start, end) shaped again when a line is cut before glyph cut: from the nearest boundary before the cut that is safe to break at to the
    // nearest one after it, within the cut's item and paragraph. False when the cut is safe, at an item boundary or after a mandatory break.
    bool ReshapeSpan( const ShapedText &shaped, const uint32_t cut, uint32_t &start, uint32_t &end )
    {
        const auto &glyphs = shaped.Glyphs;
        const auto  count  = static_cast<uint32_t>( glyphs.size( ) );
        auto        unsafe = [ & ]( const uint32_t glyph ) { return ( glyphs[ glyph ].Flags & TextGlyphFlags::UnsafeToBreak ) != 0; };
        if ( cut == 0 || cut >= count || !unsafe( cut ) || ( glyphs[ cut - 1 ].Flags & TextGlyphFlags::MustBreak ) )
        {
            return false;
        }
        const auto item = std::upper_bound( shaped.Items.begin( ), shaped.Items.end( ), cut, []( const uint32_t glyph, const TextItem &i ) { return glyph < i.GlyphStart; } ) - 1;
        if ( item->GlyphStart == cut )
        {
            return false;
        }
        const uint32_t itemEnd     = std::min( item + 1 != shaped.Items.end( ) ? ( item + 1 )->GlyphStart : count, shaped.Paragraphs[ item->Paragraph ].GlyphEnd );
        auto           nextCluster = [ & ]( uint32_t glyph )
        {
            const uint32_t cluster = glyphs[ glyph ].Cluster;
            while ( ++glyph < itemEnd && glyphs[ glyph ].Cluster == cluster )
            {
            }
            return glyph;
        };
        start = ClusterStart( glyphs, cut - 1 );
        while ( start > item->GlyphStart && unsafe( start ) )
        {
            start = ClusterStart( glyphs, start - 1 );
        }
        end = nextCluster( cut );
        while ( end < itemEnd && unsafe( end ) )
        {
            end = nextCluster( end );
        }
        return true;
    }

    bool SameColor( const DenOfIz_UIColor &a, const DenOfIz_UIColor &b )
    {
        return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A;
    }

    uint32_t PackColor( const DenOfIz_UIColor &c )
    {
        return static_cast<uint32_t>( c.R ) << 24 | static_cast<uint32_t>( c.G ) << 16 | static_cast<uint32_t>( c.B ) << 8 | c.A;
    }

    uint32_t GraphemesIn( const std::vector<uint8_t> &starts, const uint32_t start, const uint32_t end )
    {
        uint32_t count = 0;
        for ( uint32_t b = start + 1; b <= end; ++b )
        {
            count += starts[ b ] != 0;
        }
        return count;
    }
} // namespace

bool UIRunStyle::operator==( const UIRunStyle &other ) const
{
    return ByteStart == other.ByteStart && ByteEnd == other.ByteEnd && Id == other.Id && FontId == other.FontId && Size == other.Size && Shift == other.Shift &&
           SameColor( Color, other.Color ) && SameColor( Background, other.Background ) && SameColor( DecorationColor, other.DecorationColor ) &&
           Decoration == other.Decoration && Object == other.Object && ObjectSize.X == other.ObjectSize.X && ObjectSize.Y == other.ObjectSize.Y && Image == other.Image;
}

uint32_t UIShapedRun::StyleAt( const uint32_t byte ) const
{
    const auto it = std::upper_bound( Styles.begin( ), Styles.end( ), byte, []( const uint32_t b, const UIRunStyle &style ) { return b < style.ByteEnd; } );
    return static_cast<uint32_t>( std::min<ptrdiff_t>( it - Styles.begin( ), static_cast<ptrdiff_t>( Styles.size( ) ) - 1 ) );
}

bool UITextParams::operator==( const UITextParams &other ) const
{
    return FontId == other.FontId && FontSize == other.FontSize && LetterSpacing == other.LetterSpacing && LineHeight == other.LineHeight && Wrap == other.Wrap &&
           Overflow == other.Overflow && MaxLines == other.MaxLines;
}

UITextParams UITextParams::From( const DenOfIz_UITextDesc &desc )
{
    UITextParams params;
    params.FontId        = desc.FontId;
    params.FontSize      = desc.FontSize > 0.0f ? desc.FontSize : DefaultFontSize;
    params.LetterSpacing = desc.LetterSpacing;
    params.LineHeight    = desc.LineHeight > 0.0f ? desc.LineHeight : 0.0f;
    params.Wrap          = desc.Wrap;
    params.Overflow      = desc.Overflow;
    params.MaxLines      = desc.MaxLines;
    return params;
}

bool UITextLayoutKey::operator==( const UITextLayoutKey &other ) const
{
    return Run == other.Run && MaxWidth == other.MaxWidth && Params == other.Params;
}

size_t UITextLayoutKeyHash::operator( )( const UITextLayoutKey &key ) const
{
    return Mix64( reinterpret_cast<uintptr_t>( key.Run ) ^ HashParams( key.Params ) ^ static_cast<uint64_t>( std::bit_cast<uint32_t>( key.MaxWidth ) ) << 32 );
}

bool UIRunKey::operator==( const UIRunKey &other ) const
{
    return Hash == other.Hash && Length == other.Length && FontId == other.FontId && Generation == other.Generation;
}

size_t UIRunKeyHash::operator( )( const UIRunKey &key ) const
{
    return Mix64( key.Hash ^ static_cast<uint64_t>( key.Length ) << 20 ^ static_cast<uint64_t>( key.FontId ) << 48 ^ key.Generation );
}

const std::vector<TextGlyph> &UITextLayout::Glyphs( ) const
{
    return Reshaped.empty( ) ? Run->Shaped.Glyphs : Reshaped;
}

uint32_t UITextLayout::LineAt( const float y ) const
{
    uint32_t line = 0;
    while ( line + 1 < Lines.size( ) && y >= Lines[ line ].Top + Lines[ line ].Height )
    {
        ++line;
    }
    return line;
}

float UITextLayout::AlignOffset( const uint32_t line, const float boxWidth, const DenOfIz_UITextAlign align ) const
{
    const float free = line < Lines.size( ) ? boxWidth - Lines[ line ].Width : 0.0f;
    if ( free <= 0.0f )
    {
        return 0.0f;
    }
    switch ( align )
    {
    case DENOFIZ_UI_TEXT_ALIGN_START:
        return Lines[ line ].Rtl ? free : 0.0f;
    case DENOFIZ_UI_TEXT_ALIGN_END:
        return Lines[ line ].Rtl ? 0.0f : free;
    case DENOFIZ_UI_TEXT_ALIGN_CENTER:
        return std::floor( free * 0.5f * 64.0f ) / 64.0f;
    case DENOFIZ_UI_TEXT_ALIGN_RIGHT:
        return free;
    default:
        return 0.0f;
    }
}

float UITextLayout::GlyphScale( const TextGlyph &glyph ) const
{
    return Run->Styles.empty( ) ? Scale : Run->Styles[ Run->StyleAt( glyph.Cluster ) ].Size;
}

float UITextLayout::GlyphWidth( const uint32_t glyph ) const
{
    const TextGlyph &g = Glyphs( )[ glyph ];
    return ::GlyphWidth( g, GlyphScale( g ), Params.LetterSpacing, Face->SpaceAdvance, Params.Wrap == DENOFIZ_UI_TEXT_WRAP_NONE );
}

uint32_t UITextLayout::StyleAtPoint( const float x, const float y, const float boxWidth, const DenOfIz_UITextAlign align ) const
{
    if ( Run->Styles.empty( ) || Lines.empty( ) || y < 0.0f || y >= Height )
    {
        return UINT32_MAX;
    }
    const uint32_t line = LineAt( y );
    const auto    &l    = Lines[ line ];
    const float    lx   = x - AlignOffset( line, boxWidth, align );
    for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; )
    {
        const UITextCluster cluster = ClusterAt( l, i );
        if ( lx >= cluster.Left && lx < cluster.Right )
        {
            return Run->StyleAt( cluster.ByteStart );
        }
        i = cluster.Last + 1;
    }
    return UINT32_MAX;
}

UITextCluster UITextLayout::ClusterAt( const UITextLineLayout &line, const uint32_t glyph ) const
{
    const auto   &glyphs = Glyphs( );
    UITextCluster cluster{ glyph, glyph, glyphs[ glyph ].Cluster, 0, FLT_MAX, -FLT_MAX, false };
    while ( cluster.First > line.GlyphStart && glyphs[ cluster.First - 1 ].Cluster == cluster.ByteStart )
    {
        --cluster.First;
    }
    while ( cluster.Last + 1 < line.GlyphEnd && glyphs[ cluster.Last + 1 ].Cluster == cluster.ByteStart )
    {
        ++cluster.Last;
    }
    cluster.ByteEnd = cluster.Last + 1 < glyphs.size( ) ? glyphs[ cluster.Last + 1 ].Cluster : static_cast<uint32_t>( Run->Text.size( ) );
    cluster.Rtl     = ( Levels[ cluster.First ] & 1 ) != 0;
    for ( uint32_t i = cluster.First; i <= cluster.Last; ++i )
    {
        cluster.Left  = std::min( cluster.Left, GlyphX[ i ] );
        cluster.Right = std::max( cluster.Right, GlyphX[ i ] + GlyphWidth( i ) );
    }
    return cluster;
}

// A ligature covering several grapheme clusters gets evenly spaced caret stops, from its left edge for LTR and its right edge for RTL.
float UITextLayout::ClusterCaret( const UITextCluster &cluster, const uint32_t byteOffset ) const
{
    const auto    &starts = Run->Shaped.GraphemeStarts;
    const uint32_t total  = GraphemesIn( starts, cluster.ByteStart, cluster.ByteEnd );
    const uint32_t before = byteOffset > cluster.ByteStart ? GraphemesIn( starts, cluster.ByteStart, std::min( byteOffset, cluster.ByteEnd ) ) : 0;
    const float    t      = total > 0 ? static_cast<float>( before ) / static_cast<float>( total ) : 0.0f;
    return cluster.Rtl ? cluster.Right - t * ( cluster.Right - cluster.Left ) : cluster.Left + t * ( cluster.Right - cluster.Left );
}

float UITextLayout::CaretX( const uint32_t line, const uint32_t byteOffset ) const
{
    if ( line >= Lines.size( ) || Lines[ line ].GlyphEnd == Lines[ line ].GlyphStart )
    {
        return 0.0f;
    }
    const UITextLineLayout &l      = Lines[ line ];
    const auto             &glyphs = Glyphs( );
    const auto              first  = glyphs.begin( ) + l.GlyphStart;
    const auto     it = std::upper_bound( first, glyphs.begin( ) + l.GlyphEnd, byteOffset, []( const uint32_t offset, const TextGlyph &glyph ) { return offset < glyph.Cluster; } );
    const uint32_t glyph = it == first ? l.GlyphStart : static_cast<uint32_t>( it - glyphs.begin( ) ) - 1;
    return ClusterCaret( ClusterAt( l, glyph ), byteOffset );
}

uint32_t UITextLayout::LineOfOffset( const uint32_t byteOffset ) const
{
    for ( uint32_t i = 0; i + 1 < Lines.size( ); ++i )
    {
        if ( byteOffset < Lines[ i + 1 ].ByteStart )
        {
            return i;
        }
    }
    return Lines.empty( ) ? 0 : static_cast<uint32_t>( Lines.size( ) - 1 );
}

uint32_t UITextLayout::OffsetAt( const float x, const float y, const float boxWidth, const DenOfIz_UITextAlign align ) const
{
    if ( Lines.empty( ) )
    {
        return 0;
    }
    const uint32_t line = LineAt( y );
    const auto    &l    = Lines[ line ];
    if ( l.GlyphEnd == l.GlyphStart )
    {
        return l.ByteStart;
    }
    const float   lx = x - AlignOffset( line, boxWidth, align );
    UITextCluster best{ };
    float         bestDistance = FLT_MAX;
    for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; )
    {
        const UITextCluster cluster  = ClusterAt( l, i );
        const float         distance = lx < cluster.Left ? cluster.Left - lx : lx > cluster.Right ? lx - cluster.Right : 0.0f;
        if ( distance < bestDistance )
        {
            best         = cluster;
            bestDistance = distance;
        }
        i = cluster.Last + 1;
    }

    const auto    &starts = Run->Shaped.GraphemeStarts;
    const float    span   = best.Right - best.Left;
    const float    t      = span > 0.0f ? std::clamp( ( lx - best.Left ) / span, 0.0f, 1.0f ) : 0.0f;
    const uint32_t target = static_cast<uint32_t>( std::lround( ( best.Rtl ? 1.0f - t : t ) * static_cast<float>( GraphemesIn( starts, best.ByteStart, best.ByteEnd ) ) ) );
    uint32_t       offset = best.ByteStart;
    for ( uint32_t b = best.ByteStart + 1, seen = 0; b <= best.ByteEnd && seen < target; ++b )
    {
        if ( starts[ b ] && ++seen == target )
        {
            offset = b;
        }
    }
    // The end of a soft wrapped line is the start of the next one, so the caret stays before the space the line was broken at.
    const bool softBreak = line + 1 < Lines.size( ) && Lines[ line + 1 ].ByteStart == l.ByteEnd;
    return softBreak && offset >= l.ByteEnd ? Glyphs( )[ l.GlyphEnd - 1 ].Cluster : offset;
}

void UITextLayout::SelectionSpans( const uint32_t line, const uint32_t start, const uint32_t end, const float newlineWidth, std::vector<DenOfIz_Float2> &out ) const
{
    out.clear( );
    if ( line >= Lines.size( ) )
    {
        return;
    }
    const UITextLineLayout &l     = Lines[ line ];
    float                   left  = l.GlyphEnd > l.GlyphStart ? FLT_MAX : 0.0f;
    float                   right = l.GlyphEnd > l.GlyphStart ? -FLT_MAX : 0.0f;
    for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; )
    {
        const UITextCluster cluster = ClusterAt( l, i );
        left                        = std::min( left, cluster.Left );
        right                       = std::max( right, cluster.Right );
        i                           = cluster.Last + 1;
        const uint32_t from         = std::max( start, cluster.ByteStart );
        const uint32_t to           = std::min( end, cluster.ByteEnd );
        if ( from < to )
        {
            const float a = ClusterCaret( cluster, from );
            const float b = ClusterCaret( cluster, to );
            out.push_back( DenOfIz_Float2{ std::min( a, b ), std::max( a, b ) } );
        }
    }
    if ( end > l.ByteEnd )
    {
        out.push_back( l.Rtl ? DenOfIz_Float2{ left - newlineWidth, left } : DenOfIz_Float2{ right, right + newlineWidth } );
    }
    std::sort( out.begin( ), out.end( ), []( const DenOfIz_Float2 &a, const DenOfIz_Float2 &b ) { return a.X < b.X; } );
    size_t merged = 0;
    for ( size_t i = 1; i < out.size( ); ++i )
    {
        if ( out[ i ].X <= out[ merged ].Y + WrapEpsilon )
        {
            out[ merged ].Y = std::max( out[ merged ].Y, out[ i ].Y );
        }
        else
        {
            out[ ++merged ] = out[ i ];
        }
    }
    out.resize( out.empty( ) ? 0 : merged + 1 );
}

const std::vector<UIGlyphQuad> &UITextLayout::GetQuads( SlugGlyphStore &slugGlyphs, ColourGlyphStore &colourGlyphs ) const
{
    if ( QuadsBuilt )
    {
        return Quads;
    }
    QuadsBuilt = true;
    Quads.clear( );
    const auto &glyphs = Glyphs( );
    Quads.reserve( glyphs.size( ) );

    auto addGlyph = [ & ]( const TextGlyph &glyph, const Font *font, const float penX, const uint32_t line, const float em, const float shift, const uint32_t style )
    {
        if ( glyph.GlyphId == TextHiddenGlyph )
        {
            return;
        }
        const float x = penX + glyph.OffsetX * em;
        const float y = -glyph.OffsetY * em - shift;
        if ( const uint32_t source = colourGlyphs.Find( font, glyph.GlyphId ); source != ColourGlyphStore::None )
        {
            Quads.push_back( UIGlyphQuad{ x, y, x, y, 0.0f, 0.0f, 0.0f, 0.0f, source, line, true, em, style } );
            return;
        }
        const uint32_t        record = slugGlyphs.Get( font->Id( ), font->HbFont( ), font->Upem( ), glyph.GlyphId );
        const DenOfIz_Float4 &bounds = slugGlyphs.GetGlyph( record ).Bounds;
        if ( bounds.Z <= bounds.X || bounds.W <= bounds.Y )
        {
            return;
        }
        Quads.push_back(
            UIGlyphQuad{ x + bounds.X * em, y - bounds.W * em, x + bounds.Z * em, y - bounds.Y * em, bounds.X, bounds.W, bounds.Z, bounds.Y, record, line, false, em, style } );
    };

    const auto &styles = Run->Styles;
    for ( uint32_t line = 0; line < Lines.size( ); ++line )
    {
        const UITextLineLayout &l = Lines[ line ];
        for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; ++i )
        {
            const TextGlyph &glyph = glyphs[ i ];
            if ( styles.empty( ) )
            {
                addGlyph( glyph, Run->Shaped.Fonts[ glyph.Font ], GlyphX[ i ], line, Scale, 0.0f, UINT32_MAX );
                continue;
            }
            const uint32_t    index = Run->StyleAt( glyph.Cluster );
            const UIRunStyle &style = styles[ index ];
            addGlyph( glyph, Run->Shaped.Fonts[ glyph.Font ], GlyphX[ i ], line, style.Size, style.Shift, PackColor( style.Color ) != 0 ? index : UINT32_MAX );
        }
        if ( l.Ellipsis )
        {
            float pen = l.Rtl ? 0.0f : l.Width - Face->EllipsisAdvance * Scale - Params.LetterSpacing * static_cast<float>( Face->Ellipsis.Glyphs.size( ) );
            for ( const TextGlyph &glyph : Face->Ellipsis.Glyphs )
            {
                addGlyph( glyph, Face->Ellipsis.Fonts[ glyph.Font ], pen, line, Scale, 0.0f, UINT32_MAX );
                pen += glyph.Advance * Scale + Params.LetterSpacing;
            }
        }
    }
    return Quads;
}

UIText::UIText( )
{
    AddFont( 0, Font::Default( ) );
}

UIText::~UIText( )
{
    for ( UIFontFace &face : m_faces )
    {
        ReleaseFace( face );
    }
}

void UIText::AddFont( const uint16_t fontId, Font *font )
{
    if ( fontId >= m_faces.size( ) )
    {
        spdlog::error( "UI: font id {} is out of range, at most {} fonts are supported", fontId, m_faces.size( ) );
        return;
    }
    if ( font == nullptr )
    {
        spdlog::error( "UI: AddFont called with a null font" );
        return;
    }
    // Fonts listing this id as a fallback were shaped without it even when the id was empty.
    Invalidate( fontId );
    LoadFace( m_faces[ fontId ], font );
}

void UIText::RemoveFont( const uint16_t fontId )
{
    if ( fontId >= m_faces.size( ) || m_faces[ fontId ].FontPtr == nullptr )
    {
        return;
    }
    Invalidate( fontId );
    ReleaseFace( m_faces[ fontId ] );
}

void UIText::SetFallbacks( const uint16_t fontId, const std::span<const uint16_t> fallbacks )
{
    if ( fontId >= m_faces.size( ) )
    {
        spdlog::error( "UI: font id {} is out of range, at most {} fonts are supported", fontId, m_faces.size( ) );
        return;
    }
    m_fallbacks[ fontId ].assign( fallbacks.begin( ), fallbacks.end( ) );
    Invalidate( fontId );
}

void UIText::Invalidate( const uint16_t fontId )
{
    std::erase_if( m_layouts, [ this, fontId ]( const auto &entry ) { return Uses( *entry.first.Run, fontId ); } );
    std::erase_if( m_runs, [ this, fontId ]( const auto &entry ) { return Uses( *entry.second, fontId ); } );
    ++m_epoch;
}

bool UIText::Uses( const UIShapedRun &run, const uint16_t fontId ) const
{
    auto uses = [ this, fontId ]( const uint16_t id ) { return id == fontId || std::ranges::find( m_fallbacks[ id ], fontId ) != m_fallbacks[ id ].end( ); };
    return uses( run.FontId ) || std::ranges::any_of( run.Styles, [ &uses ]( const UIRunStyle &style ) { return uses( style.FontId ); } );
}

const UIFontFace *UIText::GetFace( const uint16_t fontId ) const
{
    return &m_faces[ ResolveFontId( fontId ) ];
}

SlugGlyphStore &UIText::Glyphs( )
{
    return m_glyphs;
}

ColourGlyphStore &UIText::ColourGlyphs( )
{
    return m_colourGlyphs;
}

uint16_t UIText::ResolveFontId( const uint16_t fontId ) const
{
    if ( fontId < m_faces.size( ) && m_faces[ fontId ].FontPtr != nullptr )
    {
        return fontId;
    }
    return 0;
}

// Eight bytes per step, equal hashes are always confirmed by comparing the text.
uint64_t UIText::Hash( const char *chars, const size_t length )
{
    uint64_t hash = 0x9E3779B97F4A7C15ULL ^ length;
    size_t   i    = 0;
    for ( ; i + 8 <= length; i += 8 )
    {
        uint64_t word;
        std::memcpy( &word, chars + i, 8 );
        hash = ( hash ^ word ) * 0xFF51AFD7ED558CCDULL;
        hash ^= hash >> 29;
    }
    uint64_t tail = 0;
    if ( i < length )
    {
        std::memcpy( &tail, chars + i, length - i );
    }
    return Mix64( hash ^ tail );
}

void UIText::LoadFace( UIFontFace &face, Font *font )
{
    face.FontPtr    = font;
    face.Generation = font->Id( );

    const DenOfIz_FontMetrics &metrics = font->Metrics( );
    face.Ascent                        = metrics.Ascent;
    face.Descent                       = metrics.Descent;
    face.LineHeight                    = metrics.LineHeight;

    hb_codepoint_t spaceGlyph = 0;
    face.SpaceAdvance         = hb_font_get_nominal_glyph( font->HbFont( ), ' ', &spaceGlyph )
                                    ? static_cast<float>( hb_font_get_glyph_h_advance( font->HbFont( ), spaceGlyph ) ) / static_cast<float>( font->Upem( ) )
                                    : 0.3f;

    Font *const fonts[] = { font };
    m_shaper.Shape( "\xE2\x80\xA6", 3, fonts, 0, face.Ellipsis );
    if ( face.Ellipsis.Glyphs.empty( ) || face.Ellipsis.Glyphs[ 0 ].GlyphId == 0 )
    {
        m_shaper.Shape( "...", 3, fonts, 0, face.Ellipsis );
    }
    face.EllipsisAdvance = 0.0f;
    for ( const TextGlyph &glyph : face.Ellipsis.Glyphs )
    {
        face.EllipsisAdvance += glyph.Advance;
    }
}

void UIText::ReleaseFace( UIFontFace &face )
{
    face = UIFontFace{ };
}

const UIShapedRun *UIText::Shape( const uint16_t fontId, const char *chars, const uint32_t length, const uint64_t hash )
{
    const uint16_t    resolved = ResolveFontId( fontId );
    const UIFontFace &face     = m_faces[ resolved ];
    UIRunKey          key{ hash, length, resolved, face.Generation };

    for ( auto it = m_runs.find( key ); it != m_runs.end( ); it = m_runs.find( key ) )
    {
        if ( it->second->Styles.empty( ) && ( length == 0 || std::memcmp( it->second->Text.data( ), chars, length ) == 0 ) )
        {
            it->second->LastUsedFrame = m_frame;
            return it->second.get( );
        }
        key.Hash = Mix64( key.Hash + 0x9E3779B97F4A7C15ULL );
    }

    auto run           = std::make_unique<UIShapedRun>( );
    run->FontId        = resolved;
    run->Generation    = face.Generation;
    run->Hash          = hash;
    run->LastUsedFrame = m_frame;
    run->Text.assign( chars, length );
    m_chain.assign( 1, face.FontPtr );
    for ( const uint16_t fallback : m_fallbacks[ resolved ] )
    {
        Font *font = fallback < m_faces.size( ) ? m_faces[ fallback ].FontPtr : nullptr;
        if ( font != nullptr && std::ranges::find( m_chain, font ) == m_chain.end( ) )
        {
            m_chain.push_back( font );
        }
    }
    m_shaper.Shape( run->Text.data( ), length, m_chain, TextAutoLevel, run->Shaped );
    MeasureWords( *run );

    const UIShapedRun *result = run.get( );
    m_runs.emplace( key, std::move( run ) );
    return result;
}

uint64_t UIText::HashStyles( const uint64_t textHash, const std::span<const UIRunStyle> styles )
{
    uint64_t h = Mix64( textHash ^ 0xA0761D6478BD642FULL ^ styles.size( ) );
    for ( const UIRunStyle &style : styles )
    {
        h = Mix64( h ^ style.ByteEnd ^ static_cast<uint64_t>( style.FontId ) << 32 ^ static_cast<uint64_t>( style.Id ) << 40 );
        h = Mix64( h ^ std::bit_cast<uint32_t>( style.Size ) ^ static_cast<uint64_t>( std::bit_cast<uint32_t>( style.Shift ) ) << 32 );
        h = Mix64( h ^ PackColor( style.Color ) ^ static_cast<uint64_t>( PackColor( style.Background ) ) << 32 );
        h = Mix64( h ^ PackColor( style.DecorationColor ) ^ static_cast<uint64_t>( style.Decoration ) << 32 ^ static_cast<uint64_t>( style.Object ) << 40 );
        h = Mix64( h ^ std::bit_cast<uint32_t>( style.ObjectSize.X ) ^ static_cast<uint64_t>( std::bit_cast<uint32_t>( style.ObjectSize.Y ) ) << 32 ^ style.Image );
    }
    return h;
}

// Consecutive runs with the same font and size share a segment, so only a change of font or size cuts shaping. A segment's chain is its font and the
// font's fallbacks.
const UIShapedRun *UIText::ShapeRich( const uint16_t fontId, const char *chars, const uint32_t length, const std::span<const UIRunStyle> styles, const uint64_t hash )
{
    const uint16_t resolved = ResolveFontId( fontId );
    UIRunKey       key{ hash, length, resolved, m_faces[ resolved ].Generation };
    for ( auto it = m_runs.find( key ); it != m_runs.end( ); it = m_runs.find( key ) )
    {
        const UIShapedRun &existing = *it->second;
        if ( std::ranges::equal( existing.Styles, styles ) && ( length == 0 || std::memcmp( existing.Text.data( ), chars, length ) == 0 ) )
        {
            existing.LastUsedFrame = m_frame;
            return it->second.get( );
        }
        key.Hash = Mix64( key.Hash + 0x9E3779B97F4A7C15ULL );
    }

    auto run           = std::make_unique<UIShapedRun>( );
    run->FontId        = resolved;
    run->Generation    = m_faces[ resolved ].Generation;
    run->Hash          = hash;
    run->LastUsedFrame = m_frame;
    run->Text.assign( chars, length );
    run->Styles.assign( styles.begin( ), styles.end( ) );

    m_chain.clear( );
    m_segments.clear( );
    m_segmentChains.clear( );
    auto addFont = [ this ]( const uint16_t id )
    {
        Font *font = id < m_faces.size( ) ? m_faces[ id ].FontPtr : nullptr;
        if ( font == nullptr )
        {
            return;
        }
        auto it = std::ranges::find( m_chain, font );
        if ( it == m_chain.end( ) )
        {
            if ( m_chain.size( ) > UINT8_MAX )
            {
                return;
            }
            m_chain.push_back( font );
            it = m_chain.end( ) - 1;
        }
        m_segmentChains.push_back( static_cast<uint8_t>( it - m_chain.begin( ) ) );
    };
    for ( size_t i = 0; i < styles.size( ); )
    {
        const UIRunStyle &style = styles[ i ];
        size_t            next  = i + 1;
        while ( !style.Object && next < styles.size( ) && !styles[ next ].Object && styles[ next ].FontId == style.FontId && styles[ next ].Size == style.Size )
        {
            ++next;
        }
        TextSegment segment{ styles[ next - 1 ].ByteEnd, static_cast<uint32_t>( m_segmentChains.size( ) ), 0, style.Object,
                             style.Object && style.Size > 0.0f ? style.ObjectSize.X / style.Size : 0.0f };
        addFont( style.FontId );
        for ( const uint16_t fallback : style.Object ? std::span<const uint16_t>( ) : std::span( m_fallbacks[ style.FontId ] ) )
        {
            addFont( fallback );
        }
        segment.ChainCount = static_cast<uint32_t>( m_segmentChains.size( ) ) - segment.ChainStart;
        m_segments.push_back( segment );
        i = next;
    }
    m_shaper.Shape( run->Text.data( ), length, m_chain, m_segments, m_segmentChains, TextAutoLevel, run->Shaped );
    MeasureWords( *run );

    const UIShapedRun *result = run.get( );
    m_runs.emplace( key, std::move( run ) );
    return result;
}

// The longest run of glyphs a line cannot break inside, in em for plain text and in points for rich text.
void UIText::MeasureWords( UIShapedRun &run ) const
{
    run.LongestWord       = 0.0f;
    run.LongestWordGlyphs = 0;
    float    word         = 0.0f;
    uint32_t wordGlyphs   = 0;
    for ( const TextGlyph &glyph : run.Shaped.Glyphs )
    {
        if ( glyph.Flags & ( TextGlyphFlags::Space | TextGlyphFlags::Newline ) )
        {
            word       = 0.0f;
            wordGlyphs = 0;
            continue;
        }
        word += run.Styles.empty( ) ? glyph.Advance : glyph.Advance * run.Styles[ run.StyleAt( glyph.Cluster ) ].Size;
        wordGlyphs += glyph.Advance != 0.0f;
        if ( word > run.LongestWord )
        {
            run.LongestWord       = word;
            run.LongestWordGlyphs = wordGlyphs;
        }
        if ( glyph.Flags & ( TextGlyphFlags::BreakAfter | TextGlyphFlags::MustBreak ) )
        {
            word       = 0.0f;
            wordGlyphs = 0;
        }
    }
}

const UITextLayout *UIText::Layout( const UIShapedRun *run, const UITextParams &params, const float maxWidth )
{
    const float width = maxWidth >= 0.0f && std::isfinite( maxWidth ) ? maxWidth : -1.0f;
    if ( width >= 0.0f )
    {
        const UITextLayout *natural = Layout( run, params, -1.0f );
        if ( natural->Width <= width + WrapEpsilon )
        {
            return natural;
        }
    }
    const UITextLayoutKey key{ run, params, width };
    run->LastUsedFrame = m_frame;
    if ( const auto it = m_layouts.find( key ); it != m_layouts.end( ) )
    {
        it->second->LastUsedFrame = m_frame;
        return it->second.get( );
    }

    auto layout           = std::make_unique<UITextLayout>( );
    layout->Run           = run;
    layout->Face          = &m_faces[ run->FontId ];
    layout->Params        = params;
    layout->MaxWidth      = width;
    layout->LastUsedFrame = m_frame;
    BuildLayout( *layout );

    const UITextLayout *result = layout.get( );
    m_layouts[ key ]           = std::move( layout );
    return result;
}

void UIText::BuildLayout( UITextLayout &layout )
{
    const UIShapedRun  &run            = *layout.Run;
    const ShapedText   &shaped         = run.Shaped;
    const UIFontFace   &face           = *layout.Face;
    const UITextParams &params         = layout.Params;
    const float         scale          = params.FontSize;
    const float         spacing        = params.LetterSpacing;
    const bool          newlineAsSpace = params.Wrap == DENOFIZ_UI_TEXT_WRAP_NONE;
    const bool          canWrap        = ( params.Wrap == DENOFIZ_UI_TEXT_WRAP_WORDS || params.Wrap == DENOFIZ_UI_TEXT_WRAP_ANYWHERE ) && layout.MaxWidth >= 0.0f;
    const float         limit          = layout.MaxWidth + WrapEpsilon;

    layout.Scale = scale;
    layout.Reshaped.clear( );

    std::span<const TextGlyph> glyphs = shaped.Glyphs;
    const auto                 count  = static_cast<uint32_t>( glyphs.size( ) );
    const auto                 bytes  = static_cast<uint32_t>( run.Text.size( ) );
    const auto                 width  = [ & ]( const uint32_t glyph )
    { return GlyphWidth( glyphs[ glyph ], layout.GlyphScale( glyphs[ glyph ] ), spacing, face.SpaceAdvance, newlineAsSpace ); };
    const auto                 byteAt = [ & ]( const uint32_t glyph ) { return glyph < glyphs.size( ) ? glyphs[ glyph ].Cluster : bytes; };
    const uint32_t             spaces = TextGlyphFlags::Space | TextGlyphFlags::Newline;
    // GlyphX holds logical x from the start of each line until the lines are known, then the visual x.
    layout.GlyphX.assign( count, 0.0f );
    layout.Levels.assign( count, 0 );
    layout.Lines.clear( );

    if ( newlineAsSpace )
    {
        float x          = 0.0f;
        float visibleEnd = 0.0f;
        for ( uint32_t i = 0; i < count; ++i )
        {
            layout.GlyphX[ i ] = x;
            x += width( i );
            visibleEnd = glyphs[ i ].Flags & spaces ? visibleEnd : x;
        }
        layout.Lines.push_back( UITextLineLayout{ 0, count, 0, bytes, visibleEnd, false, ( shaped.Paragraphs[ 0 ].Level & 1 ) != 0 } );
    }
    for ( const TextParagraph &paragraph : newlineAsSpace ? std::span<const TextParagraph>( ) : std::span( shaped.Paragraphs ) )
    {
        const bool rtl  = ( paragraph.Level & 1 ) != 0;
        auto       emit = [ & ]( const uint32_t start, const uint32_t end, const float lineWidth )
        {
            layout.Lines.push_back( UITextLineLayout{ start, end, start == paragraph.GlyphStart ? paragraph.ByteStart : byteAt( start ),
                                                      end == paragraph.GlyphEnd ? paragraph.ByteEnd : byteAt( end ), lineWidth, false, rtl } );
        };

        uint32_t lineStart    = paragraph.GlyphStart;
        float    x            = 0.0f;
        float    visibleEnd   = 0.0f;
        int64_t  breakAt      = -1;
        float    breakVisible = 0.0f;
        float    breakX       = 0.0f;
        for ( uint32_t i = paragraph.GlyphStart; i < paragraph.GlyphEnd; )
        {
            const TextGlyph &glyph = glyphs[ i ];
            const float      w     = width( i );
            const bool       space = ( glyph.Flags & TextGlyphFlags::Space ) != 0;
            if ( canWrap && !space && i > lineStart && w > 0.0f && glyph.Cluster != glyphs[ i - 1 ].Cluster && x + w > limit )
            {
                if ( breakAt >= static_cast<int64_t>( lineStart ) )
                {
                    const auto next = static_cast<uint32_t>( breakAt + 1 );
                    emit( lineStart, next, breakVisible );
                    for ( uint32_t j = next; j < i; ++j )
                    {
                        layout.GlyphX[ j ] -= breakX;
                    }
                    x -= breakX;
                    visibleEnd = i > next ? visibleEnd - breakX : 0.0f;
                    lineStart  = next;
                    breakAt    = -1;
                    continue;
                }
                // The glyphs before a cut HarfBuzz marks unsafe are shaped again, which can widen the line: cut a cluster earlier while it overflows.
                uint32_t cut = i;
                while ( ClusterStart( glyphs, cut - 1 ) > lineStart && ReshapedEnd( layout, lineStart, cut ) > limit )
                {
                    cut = ClusterStart( glyphs, cut - 1 );
                }
                emit( lineStart, cut, cut == i ? visibleEnd : layout.GlyphX[ cut - 1 ] + width( cut - 1 ) );
                lineStart  = cut;
                i          = cut;
                x          = 0.0f;
                visibleEnd = 0.0f;
                breakAt    = -1;
                continue;
            }

            layout.GlyphX[ i ] = x;
            x += w;
            if ( !space )
            {
                visibleEnd = x;
            }
            if ( glyph.Flags & TextGlyphFlags::BreakAfter )
            {
                breakAt      = i;
                breakVisible = visibleEnd;
                breakX       = x;
            }
            ++i;
            if ( glyph.Flags & TextGlyphFlags::MustBreak )
            {
                emit( lineStart, i, visibleEnd );
                lineStart  = i;
                x          = 0.0f;
                visibleEnd = 0.0f;
                breakAt    = -1;
            }
        }
        emit( lineStart, paragraph.GlyphEnd, visibleEnd );
    }

    bool truncated = false;
    if ( params.MaxLines > 0 && layout.Lines.size( ) > params.MaxLines )
    {
        layout.Lines.resize( params.MaxLines );
        truncated = true;
    }

    if ( canWrap && ReshapeCuts( layout ) )
    {
        glyphs = layout.Reshaped;
        layout.GlyphX.assign( glyphs.size( ), 0.0f );
        layout.Levels.assign( glyphs.size( ), 0 );
        for ( UITextLineLayout &line : layout.Lines )
        {
            float x    = 0.0f;
            line.Width = 0.0f;
            for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
            {
                layout.GlyphX[ i ] = x;
                x += width( i );
                line.Width = glyphs[ i ].Flags & TextGlyphFlags::Space ? line.Width : x;
            }
        }
    }

    // Lines are cut in logical order, which matches the visual end of the line unless the line mixes directions.
    const float ellipsis = face.EllipsisAdvance * scale + spacing * static_cast<float>( face.Ellipsis.Glyphs.size( ) );
    if ( params.Overflow == DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS )
    {
        for ( size_t lineIndex = 0; lineIndex < layout.Lines.size( ); ++lineIndex )
        {
            UITextLineLayout &line      = layout.Lines[ lineIndex ];
            const bool        isLast    = lineIndex + 1 == layout.Lines.size( );
            const bool        overflows = layout.MaxWidth >= 0.0f && line.Width > limit;
            if ( !overflows && !( truncated && isLast ) )
            {
                continue;
            }
            const float fit       = layout.MaxWidth >= 0.0f ? layout.MaxWidth - ellipsis : std::numeric_limits<float>::max( );
            float       lineWidth = line.Width;
            while ( line.GlyphEnd > line.GlyphStart )
            {
                const uint32_t last = line.GlyphEnd - 1;
                lineWidth           = layout.GlyphX[ last ] + width( last );
                if ( !( glyphs[ last ].Flags & spaces ) && lineWidth <= fit )
                {
                    break;
                }
                --line.GlyphEnd;
            }
            if ( line.GlyphEnd == line.GlyphStart )
            {
                lineWidth = 0.0f;
            }
            line.ByteEnd  = byteAt( line.GlyphEnd );
            line.Width    = lineWidth + ellipsis;
            line.Ellipsis = true;
        }
    }

    // Visual order per line and paragraph. Trailing spaces hang past the end of the line, which is the left edge for right to left paragraphs.
    size_t paragraphIndex = 0;
    for ( const UITextLineLayout &line : layout.Lines )
    {
        float x = line.Ellipsis && line.Rtl ? ellipsis : 0.0f;
        for ( uint32_t start = line.GlyphStart; start < line.GlyphEnd; )
        {
            // Paragraphs are found by bytes, reshaped lines moved their glyph indices.
            while ( paragraphIndex + 1 < shaped.Paragraphs.size( ) && shaped.Paragraphs[ paragraphIndex + 1 ].ByteStart <= byteAt( start ) )
            {
                ++paragraphIndex;
            }
            const uint8_t level = shaped.Paragraphs[ paragraphIndex ].Level;
            uint32_t      end   = line.GlyphEnd;
            if ( paragraphIndex + 1 < shaped.Paragraphs.size( ) )
            {
                end = static_cast<uint32_t>( std::lower_bound( glyphs.begin( ) + start, glyphs.begin( ) + line.GlyphEnd, shaped.Paragraphs[ paragraphIndex + 1 ].ByteStart,
                                                               []( const TextGlyph &glyph, const uint32_t byte ) { return glyph.Cluster < byte; } ) -
                                             glyphs.begin( ) );
            }
            m_order.resize( end - start );
            TextVisualOrder( &glyphs[ start ], end - start, level, &layout.Levels[ start ], m_order.data( ) );
            if ( ( level & 1 ) && end == line.GlyphEnd )
            {
                for ( uint32_t i = end; i > start && ( glyphs[ i - 1 ].Flags & TextGlyphFlags::Space ); --i )
                {
                    x -= width( i - 1 );
                }
            }
            for ( const uint32_t i : m_order )
            {
                layout.GlyphX[ start + i ] = x;
                x += width( start + i );
            }
            start = end;
        }
    }

    // CSS line-height normal: a line is as tall as the tallest of the fonts drawing on it and the primary font, each with its line gap split around it.
    // A fixed line height is the strut of rich text, runs taller than it still grow the line. Shifted runs and inline objects reach further.
    const auto &styles   = run.Styles;
    const bool  rich     = !styles.empty( );
    const bool  fixed    = params.LineHeight > 0.0f;
    const float strutTop = fixed ? ( params.LineHeight - ( face.Ascent + face.Descent ) * scale ) * 0.5f + face.Ascent * scale
                                 : ( face.Ascent + ( face.LineHeight - face.Ascent - face.Descent ) * 0.5f ) * scale;
    const float strutBottom = ( fixed ? params.LineHeight : face.LineHeight * scale ) - strutTop;
    float       top         = 0.0f;
    layout.Width            = 0.0f;
    for ( UITextLineLayout &line : layout.Lines )
    {
        line.Top    = top;
        float above = strutTop;
        float below = strutBottom;
        if ( rich || !fixed )
        {
            uint32_t font  = 0;
            uint32_t style = UINT32_MAX;
            for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
            {
                const TextGlyph &glyph = glyphs[ i ];
                const uint32_t   index = rich ? run.StyleAt( glyph.Cluster ) : UINT32_MAX;
                if ( rich && styles[ index ].Object )
                {
                    above = std::max( above, styles[ index ].ObjectSize.Y + styles[ index ].Shift );
                    below = std::max( below, -styles[ index ].Shift );
                    continue;
                }
                if ( glyph.GlyphId == TextHiddenGlyph || ( glyph.Font == font && index == style ) )
                {
                    continue;
                }
                font                         = glyph.Font;
                style                        = index;
                const DenOfIz_FontMetrics &m = shaped.Fonts[ font ]->Metrics( );
                const float                gap   = fixed ? 0.0f : m.LineGap * 0.5f;
                const float                size  = rich ? styles[ index ].Size : scale;
                const float                shift = rich ? styles[ index ].Shift : 0.0f;
                above                            = std::max( above, ( m.Ascent + gap ) * size + shift );
                below                            = std::max( below, ( m.Descent + gap ) * size - shift );
            }
        }
        line.Height   = above + below;
        line.Baseline = top + above;
        top += line.Height;
        layout.Width = std::max( layout.Width, line.Width );
    }
    layout.Height   = top;
    layout.Baseline = layout.Lines.empty( ) ? 0.0f : layout.Lines[ 0 ].Baseline;
}

// Lines cut inside an item where HarfBuzz marks the cut unsafe get the glyphs around the cut shaped again on each side, with the paragraph as context:
// kerning, ligatures and contextual alternates no longer reach across the cut while joining forms stay, as browsers do (CSS Text 3, shaping across
// intra-word breaks). The span shaped again grows to the nearest safe cluster boundaries, close cuts share one span.
bool UIText::ReshapeCuts( UITextLayout &layout )
{
    struct Span
    {
        uint32_t Start;
        uint32_t End;
        uint32_t FirstCut;
        uint32_t Cuts;
    };

    const UIShapedRun    &run    = *layout.Run;
    const ShapedText     &shaped = run.Shaped;
    const auto           &glyphs = shaped.Glyphs;
    const auto            count  = static_cast<uint32_t>( glyphs.size( ) );
    std::vector<Span>     spans;
    std::vector<uint32_t> cuts;
    for ( size_t l = 0; l + 1 < layout.Lines.size( ); ++l )
    {
        const uint32_t cut   = layout.Lines[ l ].GlyphEnd;
        uint32_t       start = 0;
        uint32_t       end   = 0;
        if ( cut != layout.Lines[ l + 1 ].GlyphStart || !ReshapeSpan( shaped, cut, start, end ) )
        {
            continue;
        }
        if ( !spans.empty( ) && spans.back( ).End > start )
        {
            spans.back( ).End = std::max( spans.back( ).End, end );
            ++spans.back( ).Cuts;
        }
        else
        {
            spans.push_back( Span{ start, end, static_cast<uint32_t>( cuts.size( ) ), 1 } );
        }
        cuts.push_back( cut );
    }
    if ( spans.empty( ) )
    {
        return false;
    }

    // Line ranges move to the new glyph indices, a cut to the first glyph shaped after it.
    std::vector<uint32_t>   remap( count + 1 );
    std::vector<TextGlyph> &out = layout.Reshaped;
    out.reserve( count + 16 );
    uint32_t copied = 0;
    for ( const Span &span : spans )
    {
        for ( ; copied < span.Start; ++copied )
        {
            remap[ copied ] = static_cast<uint32_t>( out.size( ) );
            out.push_back( glyphs[ copied ] );
        }
        const size_t first = out.size( );
        m_shaper.Reshape( run.Text.data( ), static_cast<uint32_t>( run.Text.size( ) ), shaped, span.Start, span.End, std::span( cuts ).subspan( span.FirstCut, span.Cuts ), out );
        for ( uint32_t i = span.Start; i < span.End; ++i )
        {
            const auto it = std::lower_bound( out.begin( ) + static_cast<ptrdiff_t>( first ), out.end( ), glyphs[ i ].Cluster,
                                              []( const TextGlyph &glyph, const uint32_t cluster ) { return glyph.Cluster < cluster; } );
            remap[ i ]    = static_cast<uint32_t>( it - out.begin( ) );
        }
        copied = span.End;
    }
    for ( ; copied < count; ++copied )
    {
        remap[ copied ] = static_cast<uint32_t>( out.size( ) );
        out.push_back( glyphs[ copied ] );
    }
    remap[ count ] = static_cast<uint32_t>( out.size( ) );
    for ( UITextLineLayout &line : layout.Lines )
    {
        line.GlyphStart = remap[ line.GlyphStart ];
        line.GlyphEnd   = remap[ line.GlyphEnd ];
    }
    return true;
}

// The logical width of the line [lineStart, cut) once the glyphs before an unsafe cut are shaped again, GlyphX still holding logical x.
float UIText::ReshapedEnd( const UITextLayout &layout, const uint32_t lineStart, const uint32_t cut )
{
    const ShapedText &shaped = layout.Run->Shaped;
    uint32_t          start  = 0;
    uint32_t          end    = 0;
    if ( !ReshapeSpan( shaped, cut, start, end ) )
    {
        return layout.GlyphX[ cut - 1 ] + ::GlyphWidth( shaped.Glyphs[ cut - 1 ], layout.GlyphScale( shaped.Glyphs[ cut - 1 ] ), layout.Params.LetterSpacing, layout.Face->SpaceAdvance, false );
    }
    start = std::max( start, lineStart );
    m_pieces.clear( );
    m_shaper.Reshape( layout.Run->Text.data( ), static_cast<uint32_t>( layout.Run->Text.size( ) ), shaped, start, cut, { }, m_pieces );
    float width = start > lineStart ? layout.GlyphX[ start ] : 0.0f;
    for ( const TextGlyph &glyph : m_pieces )
    {
        width += ::GlyphWidth( glyph, layout.GlyphScale( glyph ), layout.Params.LetterSpacing, layout.Face->SpaceAdvance, false );
    }
    return width;
}

float UIText::MinContentWidth( const UIShapedRun *run, const UITextParams &params ) const
{
    if ( params.Overflow == DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS || params.Wrap == DENOFIZ_UI_TEXT_WRAP_ANYWHERE )
    {
        return 0.0f;
    }
    if ( params.Wrap == DENOFIZ_UI_TEXT_WRAP_WORDS )
    {
        const float scale = run->Styles.empty( ) ? params.FontSize : 1.0f;
        return run->LongestWord * scale + params.LetterSpacing * static_cast<float>( run->LongestWordGlyphs );
    }
    return -1.0f;
}

DenOfIz_Float2 UIText::Measure( const char *chars, const uint32_t length, const DenOfIz_UITextDesc &desc, const float maxWidth )
{
    const UITextParams  params = UITextParams::From( desc );
    const UIShapedRun  *run    = Shape( params.FontId, chars, length, Hash( chars, length ) );
    const UITextLayout *layout = Layout( run, params, maxWidth > 0.0f ? maxWidth : -1.0f );
    return DenOfIz_Float2{ layout->Width, layout->Height };
}

void UIText::BeginFrame( const uint64_t frame )
{
    m_frame = frame;
}

void UIText::Evict( const uint64_t maxAge )
{
    const uint64_t frame   = m_frame;
    const uint64_t layoutAge = std::max<uint64_t>( 2, maxAge / 4 );
    const size_t   removed = std::erase_if( m_layouts, [ frame, layoutAge ]( const auto &entry ) { return frame - entry.second->LastUsedFrame > layoutAge; } ) +
                           std::erase_if( m_runs, [ frame, maxAge ]( const auto &entry ) { return frame - entry.second->LastUsedFrame > maxAge; } );
    if ( removed > 0 )
    {
        ++m_epoch;
    }
}

uint64_t UIText::Epoch( ) const
{
    return m_epoch;
}

uint64_t UIText::Frame( ) const
{
    return m_frame;
}
