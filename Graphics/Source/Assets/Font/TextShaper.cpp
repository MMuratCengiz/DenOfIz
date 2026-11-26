/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"

#include <SheenBidi/SheenBidi.h>
#include <algorithm>
#include <graphemebreak.h>
#include <linebreak.h>
#include <utility>
#include "DenOfIzGraphics/Assets/Font/Text.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Emoji.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"

using namespace DenOfIz;

namespace
{
    constexpr float TabSpaces = 4.0f;

    SBCodepoint CodepointAt( const char *text, const uint32_t length, const uint32_t offset )
    {
        SBUInteger index = offset;
        return SBCodepointDecodeNextFromUTF8( reinterpret_cast<const SBUInt8 *>( text ), length, &index );
    }

    bool IsControl( const SBCodepoint codePoint )
    {
        return codePoint < 0x20 || ( codePoint >= 0x7F && codePoint < 0xA0 );
    }

    uint8_t BidiFlags( const SBBidiType type )
    {
        switch ( type )
        {
        case SBBidiTypeS:
        case SBBidiTypeB:
            return TextGlyphFlags::BidiSeparator;
        case SBBidiTypeWS:
        case SBBidiTypeBN:
        case SBBidiTypeLRI:
        case SBBidiTypeRLI:
        case SBBidiTypeFSI:
        case SBBidiTypePDI:
        case SBBidiTypeLRE:
        case SBBidiTypeRLE:
        case SBBidiTypeLRO:
        case SBBidiTypeRLO:
        case SBBidiTypePDF:
            return TextGlyphFlags::BidiTrailing;
        default:
            return 0;
        }
    }

    // The first font of the chain covering every code point of the cluster, else the system font covering it, else the first covering its base
    // character, else the primary font. Emoji presentation clusters prefer colour fonts and VS15 text presentation ones the others, a font of the
    // other kind only takes them when no font of their kind covers them. System fonts are appended after the chain, the cache behind
    // SystemFallbackFont picks them per script.
    uint8_t SelectFont( std::vector<Font *> &fonts, const std::span<const uint8_t> chain, const char *text, const uint32_t length, const uint32_t start, const uint32_t end,
                        const SBScript script )
    {
        const SBCodepoint      base         = CodepointAt( text, length, start );
        const TextPresentation presentation = ClusterPresentation( text + start, end - start );
        auto                   preferred    = [ presentation ]( const Font *font )
        { return presentation == TextPresentation::Default || font->IsColour( ) == ( presentation == TextPresentation::Emoji ); };
        int covering = -1;
        int other    = -1;
        for ( const uint8_t f : chain )
        {
            if ( fonts[ f ]->Covers( text + start, end - start ) )
            {
                if ( preferred( fonts[ f ] ) )
                {
                    return f;
                }
                other = other < 0 ? f : other;
            }
            if ( covering < 0 && fonts[ f ]->HasCodePoint( base ) )
            {
                covering = f;
            }
        }
        Font *system = IsControl( base ) ? nullptr
                                         : SystemFallbackFont( text + start, end - start, hb_script_from_iso15924_tag( SBScriptGetUnicodeTag( script ) ),
                                                               presentation == TextPresentation::Emoji );
        if ( system != nullptr && ( other < 0 || preferred( system ) ) )
        {
            const auto it = std::ranges::find( fonts, system );
            if ( it != fonts.end( ) )
            {
                return static_cast<uint8_t>( it - fonts.begin( ) );
            }
            if ( fonts.size( ) <= UINT8_MAX )
            {
                fonts.push_back( system );
                return static_cast<uint8_t>( fonts.size( ) - 1 );
            }
        }
        return static_cast<uint8_t>( other >= 0 ? other : covering >= 0 ? covering : chain.empty( ) ? 0 : chain[ 0 ] );
    }

    bool AllIgnorable( const char *text, const uint32_t start, const uint32_t end )
    {
        for ( SBUInteger i = start; i < end; )
        {
            if ( !TextIsDefaultIgnorable( SBCodepointDecodeNextFromUTF8( reinterpret_cast<const SBUInt8 *>( text ), end, &i ) ) )
            {
                return false;
            }
        }
        return true;
    }

    // The bytes of the sequence at i: a whole character when wellFormed, else the maximal subpart of an ill-formed one, at least one byte.
    uint32_t Sequence( const uint8_t *bytes, const uint32_t length, const uint32_t i, bool &wellFormed )
    {
        const uint8_t lead = bytes[ i ];
        uint32_t      need = 0;
        uint8_t       low  = 0x80;
        uint8_t       high = 0xBF;
        if ( lead < 0x80 )
        {
            wellFormed = true;
            return 1;
        }
        if ( lead >= 0xC2 && lead <= 0xDF )
        {
            need = 1;
        }
        else if ( lead >= 0xE0 && lead <= 0xEF )
        {
            need = 2;
            low  = lead == 0xE0 ? 0xA0 : 0x80;
            high = lead == 0xED ? 0x9F : 0xBF;
        }
        else if ( lead >= 0xF0 && lead <= 0xF4 )
        {
            need = 3;
            low  = lead == 0xF0 ? 0x90 : 0x80;
            high = lead == 0xF4 ? 0x8F : 0xBF;
        }
        uint32_t n = 1;
        while ( n <= need && i + n < length && bytes[ i + n ] >= low && bytes[ i + n ] <= high )
        {
            ++n;
            low  = 0x80;
            high = 0xBF;
        }
        wellFormed = need > 0 && n == need + 1;
        return n;
    }

    void GraphemeStarts( const char *text, const uint32_t length, std::vector<char> &breaks, std::vector<uint8_t> &starts )
    {
        starts.assign( length + 1, 1 );
        breaks.resize( length );
        set_graphemebreaks_utf8( reinterpret_cast<const utf8_t *>( text ), length, nullptr, breaks.data( ) );
        for ( uint32_t i = 0; i + 1 < length; ++i )
        {
            starts[ i + 1 ] = breaks[ i ] == GRAPHEMEBREAK_BREAK;
        }
    }

    // One per byte of text plus one for the end, non zero where a grapheme cluster starts. A maximal subpart of ill-formed UTF-8 is one cluster.
    const std::vector<uint8_t> &Graphemes( const char *text, const uint32_t length )
    {
        thread_local std::string           valid;
        thread_local std::vector<uint32_t> origins;
        thread_local std::vector<char>     breaks;
        thread_local std::vector<uint8_t>  starts;
        thread_local std::vector<uint8_t>  mapped;
        if ( !TextReplaceIllFormed( text, length, valid, origins ) )
        {
            GraphemeStarts( text, length, breaks, starts );
            return starts;
        }
        GraphemeStarts( valid.data( ), static_cast<uint32_t>( valid.size( ) ), breaks, starts );
        mapped.assign( length + 1, 0 );
        for ( uint32_t b = 0; b <= valid.size( ); ++b )
        {
            mapped[ origins[ b ] ] |= starts[ b ];
        }
        return mapped;
    }
} // namespace

TextShaper::TextShaper( ) : m_buffer( hb_buffer_create( ) )
{
}

TextShaper::~TextShaper( )
{
    hb_buffer_destroy( m_buffer );
}

void TextShaper::Shape( const char *text, const uint32_t length, const std::span<Font *const> fonts, const uint8_t baseLevel, ShapedText &out )
{
    m_chain.resize( std::min<size_t>( fonts.size( ), UINT8_MAX + 1 ) );
    for ( size_t f = 0; f < m_chain.size( ); ++f )
    {
        m_chain[ f ] = static_cast<uint8_t>( f );
    }
    const TextSegment whole{ UINT32_MAX, 0, static_cast<uint32_t>( m_chain.size( ) ), false, 0.0f };
    Shape( text, length, fonts, std::span( &whole, 1 ), m_chain, baseLevel, out );
}

void TextShaper::Shape( const char *text, const uint32_t length, const std::span<Font *const> fonts, const std::span<const TextSegment> segments,
                        const std::span<const uint8_t> chains, const uint8_t baseLevel, ShapedText &out )
{
    if ( !TextReplaceIllFormed( text, length, m_valid, m_origins ) )
    {
        ShapeWellFormed( text, length, fonts, segments, chains, baseLevel, out );
        return;
    }
    // Segment ends move with the replacement, onto the first byte that came from at or after them.
    m_segments.assign( segments.begin( ), segments.end( ) );
    for ( TextSegment &segment : m_segments )
    {
        if ( segment.ByteEnd < length )
        {
            segment.ByteEnd = static_cast<uint32_t>( std::lower_bound( m_origins.begin( ), m_origins.end( ), segment.ByteEnd ) - m_origins.begin( ) );
        }
    }
    ShapeWellFormed( m_valid.data( ), static_cast<uint32_t>( m_valid.size( ) ), fonts, m_segments, chains, baseLevel, out );
    for ( TextGlyph &glyph : out.Glyphs )
    {
        glyph.Cluster = m_origins[ glyph.Cluster ];
    }
    for ( TextParagraph &paragraph : out.Paragraphs )
    {
        paragraph.ByteStart = m_origins[ paragraph.ByteStart ];
        paragraph.ByteEnd   = m_origins[ paragraph.ByteEnd ];
    }
    for ( TextItem &item : out.Items )
    {
        item.ByteEnd = m_origins[ item.ByteEnd ];
    }
    std::vector<uint8_t> starts( length + 1, 0 );
    for ( uint32_t b = 0; b < out.GraphemeStarts.size( ); ++b )
    {
        starts[ m_origins[ b ] ] |= out.GraphemeStarts[ b ];
    }
    out.GraphemeStarts.swap( starts );
}

void TextShaper::ShapeWellFormed( const char *text, const uint32_t length, const std::span<Font *const> fonts, const std::span<const TextSegment> segments,
                                  const std::span<const uint8_t> chains, const uint8_t baseLevel, ShapedText &out )
{
    out.Fonts.assign( fonts.begin( ), fonts.end( ) );
    out.Glyphs.clear( );
    out.Paragraphs.clear( );
    out.Items.clear( );
    if ( length == 0 || fonts.empty( ) || segments.empty( ) )
    {
        out.GraphemeStarts.assign( length + 1, 1 );
        out.Paragraphs.push_back( TextParagraph{ 0, 0, 0, 0, static_cast<uint8_t>( baseLevel == 1 ? 1 : 0 ) } );
        return;
    }

    const auto *bytes = reinterpret_cast<const utf8_t *>( text );
    GraphemeStarts( text, length, m_breaks, out.GraphemeStarts );
    set_linebreaks_utf8( bytes, length, nullptr, m_breaks.data( ) );

    const SBCodepointSequence sequence{ SBStringEncodingUTF8, text, length };
    const SBAlgorithmRef      algorithm = SBAlgorithmCreate( &sequence );
    const SBScriptLocatorRef  locator   = SBScriptLocatorCreate( );
    SBScriptLocatorLoadCodepoints( locator, &sequence );
    const SBScriptAgent *agent    = SBScriptLocatorGetAgent( locator );
    bool                 hasAgent = SBScriptLocatorMoveNext( locator );
    auto                 scriptAt = [ & ]( const uint32_t offset )
    {
        while ( hasAgent && offset >= agent->offset + agent->length )
        {
            hasAgent = SBScriptLocatorMoveNext( locator );
        }
        return hasAgent ? agent->script : static_cast<SBScript>( SBScriptZYYY );
    };

    uint32_t segment   = 0;
    auto     segmentAt = [ & ]( const uint32_t offset )
    {
        while ( segment + 1 < segments.size( ) && offset >= segments[ segment ].ByteEnd )
        {
            ++segment;
        }
        return segment;
    };
    auto chainOf = [ & ]( const uint32_t s )
    {
        const TextSegment &seg = segments[ s ];
        return seg.ChainStart + seg.ChainCount <= chains.size( ) ? chains.subspan( seg.ChainStart, seg.ChainCount ) : std::span<const uint8_t>( );
    };

    SBUInteger separatorLength = 0;
    for ( uint32_t start = 0; start < length; )
    {
        SBUInteger paragraphLength = 0;
        SBAlgorithmGetParagraphBoundary( algorithm, start, length - start, &paragraphLength, &separatorLength );
        const SBParagraphRef paragraph  = SBAlgorithmCreateParagraph( algorithm, start, paragraphLength, baseLevel );
        const SBLevel       *levels     = SBParagraphGetLevelsPtr( paragraph );
        const uint32_t       end        = start + static_cast<uint32_t>( paragraphLength );
        const uint32_t       contentEnd = end - static_cast<uint32_t>( separatorLength );
        TextParagraph        info{ start, contentEnd, static_cast<uint32_t>( out.Glyphs.size( ) ), 0, SBParagraphGetBaseLevel( paragraph ) };

        // Items are maximal runs of grapheme clusters with the same level, script, font and segment, shaped with the whole paragraph as context. A
        // cluster of default ignorables keeps the font before it, so a joiner or bidi control never splits an item by font.
        uint32_t itemStart   = start;
        uint8_t  level       = levels[ 0 ];
        SBScript script      = scriptAt( start );
        uint8_t  font        = 0;
        uint32_t itemSegment = segmentAt( start );
        auto     shapeItem   = [ & ]( const uint32_t itemEnd )
        {
            const hb_script_t hbScript = hb_script_from_iso15924_tag( SBScriptGetUnicodeTag( script ) );
            out.Items.push_back( TextItem{ static_cast<uint32_t>( out.Glyphs.size( ) ), itemEnd, static_cast<uint32_t>( out.Paragraphs.size( ) ), hbScript } );
            if ( segments[ itemSegment ].Object )
            {
                for ( uint32_t g = itemStart; g < itemEnd; ++g )
                {
                    if ( out.GraphemeStarts[ g ] )
                    {
                        const float advance = g == itemStart ? segments[ itemSegment ].ObjectAdvance : 0.0f;
                        out.Glyphs.push_back( TextGlyph{ TextHiddenGlyph, g, advance, 0.0f, 0.0f, font, level, 0 } );
                    }
                }
                return;
            }
            ShapeItem( text + start, end - start, start, itemStart, itemEnd, level, hbScript, out.Fonts[ font ], font, out.Glyphs );
        };
        for ( uint32_t g = start; g < end; )
        {
            uint32_t next = g + 1;
            while ( next < end && !out.GraphemeStarts[ next ] )
            {
                ++next;
            }
            const uint8_t  graphemeLevel   = levels[ g - start ];
            const SBScript graphemeScript  = scriptAt( g );
            const uint32_t graphemeSegment = segmentAt( g );
            const auto     chain           = chainOf( graphemeSegment );
            uint8_t        graphemeFont    = font;
            if ( segments[ graphemeSegment ].Object )
            {
                graphemeFont = chain.empty( ) ? 0 : chain[ 0 ];
            }
            else if ( g == start || graphemeSegment != itemSegment || !AllIgnorable( text, g, next ) )
            {
                graphemeFont = SelectFont( out.Fonts, chain, text, length, g, next, graphemeScript );
            }
            if ( g == start )
            {
                font = graphemeFont;
            }
            else if ( graphemeLevel != level || graphemeScript != script || graphemeFont != font || graphemeSegment != itemSegment )
            {
                shapeItem( g );
                itemStart   = g;
                level       = graphemeLevel;
                script      = graphemeScript;
                font        = graphemeFont;
                itemSegment = graphemeSegment;
            }
            g = next;
        }
        shapeItem( end );
        SBParagraphRelease( paragraph );

        const auto count = static_cast<uint32_t>( out.Glyphs.size( ) );
        info.GlyphEnd    = count;
        while ( info.GlyphEnd > info.GlyphStart && out.Glyphs[ info.GlyphEnd - 1 ].Cluster >= contentEnd )
        {
            --info.GlyphEnd;
        }
        for ( uint32_t i = info.GlyphStart; i < count; ++i )
        {
            TextGlyph        &glyph     = out.Glyphs[ i ];
            const SBCodepoint codePoint = CodepointAt( text, length, glyph.Cluster );
            if ( i >= info.GlyphEnd )
            {
                glyph.Flags   = TextGlyphFlags::BidiSeparator | ( i == info.GlyphEnd ? TextGlyphFlags::Newline : 0 );
                glyph.GlyphId = TextHiddenGlyph;
                glyph.Advance = 0.0f;
                continue;
            }
            const SBBidiType type = SBCodepointGetBidiType( codePoint );
            glyph.Flags           = ( glyph.Flags & TextGlyphFlags::UnsafeToBreak ) | BidiFlags( type );
            if ( type == SBBidiTypeWS || type == SBBidiTypeS )
            {
                glyph.Flags |= TextGlyphFlags::Space;
                glyph.GlyphId = TextHiddenGlyph;
            }
            if ( codePoint == '\t' )
            {
                const Font    *tabFont    = out.Fonts[ glyph.Font ];
                hb_codepoint_t spaceGlyph = 0;
                hb_font_get_nominal_glyph( tabFont->HbFont( ), ' ', &spaceGlyph );
                glyph.Advance = static_cast<float>( hb_font_get_glyph_h_advance( tabFont->HbFont( ), spaceGlyph ) ) * TabSpaces / static_cast<float>( tabFont->Upem( ) );
            }
            else if ( IsControl( codePoint ) )
            {
                glyph.GlyphId = TextHiddenGlyph;
                glyph.Advance = 0.0f;
            }
            if ( i + 1 < count && out.Glyphs[ i + 1 ].Cluster == glyph.Cluster )
            {
                continue;
            }
            // Break data sits on the last byte of each character, so the last byte of the cluster tells whether a line may end after it.
            const uint32_t clusterEnd = i + 1 < count ? out.Glyphs[ i + 1 ].Cluster : length;
            const char     lineBreak  = m_breaks[ clusterEnd - 1 ];
            glyph.Flags |= lineBreak == LINEBREAK_ALLOWBREAK ? TextGlyphFlags::BreakAfter : lineBreak == LINEBREAK_MUSTBREAK ? TextGlyphFlags::MustBreak : 0;
        }
        out.Paragraphs.push_back( info );
        start = end;
    }
    if ( separatorLength > 0 )
    {
        const auto count = static_cast<uint32_t>( out.Glyphs.size( ) );
        out.Paragraphs.push_back( TextParagraph{ length, length, count, count, out.Paragraphs.back( ).Level } );
    }
    SBScriptLocatorRelease( locator );
    SBAlgorithmRelease( algorithm );
}

void TextShaper::ShapeItem( const char *paragraph, const uint32_t paragraphLength, const uint32_t paragraphStart, const uint32_t start, const uint32_t end, const uint8_t level,
                            const hb_script_t script, const Font *font, const uint8_t fontIndex, std::vector<TextGlyph> &out )
{
    const bool rtl   = ( level & 1 ) != 0;
    const int  flags = ( start == paragraphStart ? HB_BUFFER_FLAG_BOT : 0 ) | ( end == paragraphStart + paragraphLength ? HB_BUFFER_FLAG_EOT : 0 );
    hb_buffer_clear_contents( m_buffer );
    hb_buffer_set_flags( m_buffer, static_cast<hb_buffer_flags_t>( flags ) );
    hb_buffer_set_direction( m_buffer, rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR );
    hb_buffer_set_script( m_buffer, script );
    hb_buffer_set_language( m_buffer, hb_language_get_default( ) );
    hb_buffer_add_utf8( m_buffer, paragraph, static_cast<int>( paragraphLength ), start - paragraphStart, static_cast<int>( end - start ) );
    hb_buffer_guess_segment_properties( m_buffer );
    hb_shape( font->HbFont( ), m_buffer, nullptr, 0 );
    // HarfBuzz returns right to left runs in visual order, reversing them keeps every glyph array in logical order.
    if ( rtl )
    {
        hb_buffer_reverse( m_buffer );
    }

    unsigned int               count     = 0;
    const hb_glyph_info_t     *infos     = hb_buffer_get_glyph_infos( m_buffer, &count );
    const hb_glyph_position_t *positions = hb_buffer_get_glyph_positions( m_buffer, &count );
    const float                em        = 1.0f / static_cast<float>( font->Upem( ) );
    for ( unsigned int i = 0; i < count; ++i )
    {
        const uint8_t flags = hb_glyph_info_get_glyph_flags( &infos[ i ] ) & HB_GLYPH_FLAG_UNSAFE_TO_BREAK ? TextGlyphFlags::UnsafeToBreak : 0;
        out.push_back( TextGlyph{ infos[ i ].codepoint, paragraphStart + infos[ i ].cluster, static_cast<float>( positions[ i ].x_advance ) * em,
                                  static_cast<float>( positions[ i ].x_offset ) * em, static_cast<float>( positions[ i ].y_offset ) * em, fontIndex, level, flags } );
    }
}

void TextShaper::Reshape( const char *text, const uint32_t length, const ShapedText &shaped, const uint32_t start, const uint32_t end, const std::span<const uint32_t> cuts,
                          std::vector<TextGlyph> &out )
{
    const auto    &glyphs = shaped.Glyphs;
    const auto     item = std::upper_bound( shaped.Items.begin( ), shaped.Items.end( ), start, []( const uint32_t glyph, const TextItem &i ) { return glyph < i.GlyphStart; } ) - 1;
    const uint32_t itemEnd        = item + 1 != shaped.Items.end( ) ? ( item + 1 )->GlyphStart : static_cast<uint32_t>( glyphs.size( ) );
    const uint32_t paragraph      = item->Paragraph;
    const uint32_t paragraphStart = shaped.Paragraphs[ paragraph ].ByteStart;
    const uint32_t paragraphEnd   = paragraph + 1 < shaped.Paragraphs.size( ) ? shaped.Paragraphs[ paragraph + 1 ].ByteStart : length;
    const TextGlyph &first        = glyphs[ start ];
    auto             byteAt       = [ & ]( const uint32_t glyph ) { return glyph < itemEnd ? glyphs[ glyph ].Cluster : item->ByteEnd; };

    // HarfBuzz sees the paragraph as Shape gave it, ill-formed sequences replaced. Offsets below are relative to the paragraph.
    const bool     remap    = TextReplaceIllFormed( text + paragraphStart, paragraphEnd - paragraphStart, m_valid, m_origins );
    const char    *source   = remap ? m_valid.data( ) : text + paragraphStart;
    const uint32_t size     = remap ? static_cast<uint32_t>( m_valid.size( ) ) : paragraphEnd - paragraphStart;
    auto           toSource = [ & ]( const uint32_t byte )
    {
        const uint32_t offset = byte - paragraphStart;
        return remap ? static_cast<uint32_t>( std::lower_bound( m_origins.begin( ), m_origins.end( ), offset ) - m_origins.begin( ) ) : offset;
    };

    const size_t firstNew = out.size( );
    uint32_t     piece    = start;
    for ( size_t c = 0; c <= cuts.size( ); ++c )
    {
        const uint32_t pieceEnd = c < cuts.size( ) ? cuts[ c ] : end;
        ShapeItem( source, size, 0, toSource( byteAt( piece ) ), toSource( byteAt( pieceEnd ) ), first.Level, item->Script, shaped.Fonts[ first.Font ], first.Font, out );
        piece = pieceEnd;
    }
    for ( size_t i = firstNew; i < out.size( ); ++i )
    {
        out[ i ].Cluster = paragraphStart + ( remap ? m_origins[ out[ i ].Cluster ] : out[ i ].Cluster );
    }

    // The glyph replaced is the last one of the cluster holding the new glyph's first byte. Break flags stay where a cluster still ends.
    for ( size_t i = firstNew; i < out.size( ); ++i )
    {
        TextGlyph &glyph = out[ i ];
        const auto it = std::upper_bound( glyphs.begin( ) + start, glyphs.begin( ) + end, glyph.Cluster, []( const uint32_t b, const TextGlyph &g ) { return b < g.Cluster; } ) - 1;
        const TextGlyph  &replaced    = *it;
        const uint32_t    replacedEnd = byteAt( static_cast<uint32_t>( it - glyphs.begin( ) ) + 1 );
        const uint32_t    clusterEnd  = i + 1 < out.size( ) ? out[ i + 1 ].Cluster : byteAt( end );
        constexpr uint8_t breaks      = TextGlyphFlags::BreakAfter | TextGlyphFlags::MustBreak;
        glyph.Flags                   = ( glyph.Flags & TextGlyphFlags::UnsafeToBreak ) | ( replaced.Flags & ~( breaks | TextGlyphFlags::UnsafeToBreak ) ) |
                      ( clusterEnd != glyph.Cluster && clusterEnd >= replacedEnd ? replaced.Flags & breaks : 0 );
        if ( replaced.GlyphId == TextHiddenGlyph )
        {
            glyph.GlyphId = TextHiddenGlyph;
            glyph.Advance = replaced.Advance;
        }
    }
}

void DenOfIz::TextVisualOrder( const TextGlyph *glyphs, const uint32_t count, const uint8_t paragraphLevel, uint8_t *levels, uint32_t *order )
{
    bool trailing = true;
    for ( uint32_t i = count; i-- > 0; )
    {
        const uint8_t flags = glyphs[ i ].Flags;
        if ( flags & TextGlyphFlags::BidiSeparator )
        {
            levels[ i ] = paragraphLevel;
            trailing    = true;
        }
        else if ( trailing && ( flags & TextGlyphFlags::BidiTrailing ) )
        {
            levels[ i ] = paragraphLevel;
        }
        else
        {
            levels[ i ] = glyphs[ i ].Level;
            trailing    = false;
        }
    }

    int highest   = 0;
    int lowestOdd = INT32_MAX;
    for ( uint32_t i = 0; i < count; ++i )
    {
        order[ i ] = i;
        highest    = std::max<int>( highest, levels[ i ] );
        if ( levels[ i ] & 1 )
        {
            lowestOdd = std::min<int>( lowestOdd, levels[ i ] );
        }
    }
    // L2: from the highest level down to the lowest odd one, reverse every run at that level or above.
    for ( int level = highest; level >= lowestOdd; --level )
    {
        for ( uint32_t i = 0; i < count; )
        {
            if ( levels[ order[ i ] ] < level )
            {
                ++i;
                continue;
            }
            uint32_t j = i;
            while ( j < count && levels[ order[ j ] ] >= level )
            {
                ++j;
            }
            std::reverse( order + i, order + j );
            i = j;
        }
    }
}

uint32_t DenOfIz::TextNextGrapheme( const char *text, const uint32_t length, const uint32_t offset )
{
    if ( offset >= length )
    {
        return length;
    }
    const std::vector<uint8_t> &starts = Graphemes( text, length );
    uint32_t                    next   = offset + 1;
    while ( next < length && !starts[ next ] )
    {
        ++next;
    }
    return next;
}

uint32_t DenOfIz::TextPreviousGrapheme( const char *text, const uint32_t length, const uint32_t offset )
{
    const uint32_t end = std::min( offset, length );
    if ( end == 0 )
    {
        return 0;
    }
    // Boundaries before the offset only depend on the text before it.
    const std::vector<uint8_t> &starts   = Graphemes( text, end );
    uint32_t                    previous = end - 1;
    while ( previous > 0 && !starts[ previous ] )
    {
        --previous;
    }
    return previous;
}

bool DenOfIz::TextIsDefaultIgnorable( const uint32_t codePoint )
{
    constexpr std::pair<uint32_t, uint32_t> ranges[] = { { 0x00AD, 0x00AD }, { 0x034F, 0x034F },   { 0x061C, 0x061C },  { 0x17B4, 0x17B5 }, { 0x180B, 0x180F },
                                                         { 0x200B, 0x200F }, { 0x202A, 0x202E },   { 0x2060, 0x206F },  { 0xFE00, 0xFE0F }, { 0xFEFF, 0xFEFF },
                                                         { 0xFFF0, 0xFFF8 }, { 0x1D173, 0x1D17A }, { 0xE0000, 0xE0FFF } };
    return std::ranges::any_of( ranges, [ codePoint ]( const auto &range ) { return codePoint >= range.first && codePoint <= range.second; } );
}

bool DenOfIz::TextReplaceIllFormed( const char *text, const uint32_t length, std::string &out, std::vector<uint32_t> &origins )
{
    const auto *bytes      = reinterpret_cast<const uint8_t *>( text );
    bool        wellFormed = true;
    uint32_t    i          = 0;
    while ( i < length )
    {
        const uint32_t n = Sequence( bytes, length, i, wellFormed );
        if ( !wellFormed )
        {
            break;
        }
        i += n;
    }
    if ( wellFormed )
    {
        return false;
    }
    out.assign( text, i );
    origins.resize( i );
    for ( uint32_t b = 0; b < i; ++b )
    {
        origins[ b ] = b;
    }
    while ( i < length )
    {
        const uint32_t n = Sequence( bytes, length, i, wellFormed );
        if ( wellFormed )
        {
            for ( uint32_t b = i; b < i + n; ++b )
            {
                out.push_back( text[ b ] );
                origins.push_back( b );
            }
        }
        else
        {
            out.append( "\xEF\xBF\xBD" );
            origins.insert( origins.end( ), 3, i );
        }
        i += n;
    }
    origins.push_back( length );
    return true;
}

extern "C"
{
    uint32_t DenOfIz_Text_NextGrapheme( const DenOfIz_StringView text, const uint32_t byteOffset )
    {
        return text.Chars != nullptr ? TextNextGrapheme( text.Chars, static_cast<uint32_t>( text.NumChars ), byteOffset ) : 0;
    }

    uint32_t DenOfIz_Text_PreviousGrapheme( const DenOfIz_StringView text, const uint32_t byteOffset )
    {
        return text.Chars != nullptr ? TextPreviousGrapheme( text.Chars, static_cast<uint32_t>( text.NumChars ), byteOffset ) : 0;
    }
}
