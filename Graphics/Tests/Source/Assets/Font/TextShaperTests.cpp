/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "gtest/gtest.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Font/Text.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/UI/UIText.h"

using namespace DenOfIz;

namespace
{
    const char *ArialUnicodePath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
    const char *MonoPath         = DZ_TEST_DATA_SRC_DIR "/Fonts/JetBrainsMono-Regular.ttf";

    struct FontFile
    {
        DenOfIz_Font Handle = DENOFIZ_NULL_HANDLE;

        explicit FontFile( const char *path )
        {
            if ( std::filesystem::exists( path ) )
            {
                Handle = DenOfIz_Font_CreateFromPath( DenOfIz_StringView{ path, std::strlen( path ) }, 0 );
            }
        }
        ~FontFile( )
        {
            DenOfIz_Font_Destroy( Handle );
        }
        FontFile( const FontFile & )            = delete;
        FontFile &operator=( const FontFile & ) = delete;

        Font *Get( ) const
        {
            return DENOFIZ_HANDLE_IS_VALID( Handle ) ? FONT_IMPL( Handle ) : nullptr;
        }
    };

    class TextShaperTest : public testing::Test
    {
    protected:
        FontFile   m_unicode{ ArialUnicodePath };
        TextShaper m_shaper;
        ShapedText m_shaped;

        void SetUp( ) override
        {
            if ( m_unicode.Get( ) == nullptr )
            {
                GTEST_SKIP( ) << ArialUnicodePath << " is missing";
            }
        }

        void Shape( const std::string &text, const uint8_t baseLevel = TextAutoLevel )
        {
            Font *const fonts[] = { m_unicode.Get( ) };
            m_shaper.Shape( text.data( ), static_cast<uint32_t>( text.size( ) ), fonts, baseLevel, m_shaped );
        }

        // Character indices in visual order for the glyphs [start, end) of the first paragraph, one entry per cluster.
        std::vector<uint32_t> Visual( const std::string &text, uint32_t start = 0, uint32_t end = UINT32_MAX ) const
        {
            std::vector<uint32_t> charOf( text.size( ) + 1, 0 );
            for ( uint32_t i = 0, c = 0; i < text.size( ); ++i )
            {
                c += i > 0 && ( static_cast<uint8_t>( text[ i ] ) & 0xC0 ) != 0x80;
                charOf[ i ] = c;
            }
            const TextParagraph &paragraph = m_shaped.Paragraphs[ 0 ];
            end                            = std::min( end, paragraph.GlyphEnd );
            std::vector<uint8_t>  levels( end - start );
            std::vector<uint32_t> order( end - start );
            TextVisualOrder( m_shaped.Glyphs.data( ) + start, end - start, paragraph.Level, levels.data( ), order.data( ) );
            std::vector<uint32_t> result;
            uint32_t              last = UINT32_MAX;
            for ( const uint32_t i : order )
            {
                const uint32_t cluster = m_shaped.Glyphs[ start + i ].Cluster;
                if ( cluster != last )
                {
                    result.push_back( charOf[ cluster ] );
                }
                last = cluster;
            }
            return result;
        }

        std::vector<uint32_t> BreaksAfter( const std::string &text, const uint8_t flag ) const
        {
            std::vector<uint32_t> result;
            for ( const TextGlyph &glyph : m_shaped.Glyphs )
            {
                if ( glyph.Flags & flag )
                {
                    uint32_t c = 0;
                    for ( uint32_t i = 1; i <= glyph.Cluster; ++i )
                    {
                        c += ( static_cast<uint8_t>( text[ i ] ) & 0xC0 ) != 0x80;
                    }
                    result.push_back( c );
                }
            }
            return result;
        }

        uint32_t NominalGlyph( const uint32_t codePoint ) const
        {
            hb_codepoint_t glyph = 0;
            hb_font_get_nominal_glyph( m_unicode.Get( )->HbFont( ), codePoint, &glyph );
            return glyph;
        }
    };

    using Order = std::vector<uint32_t>;

    uint32_t CodePointAt( const std::string &text, const uint32_t byte )
    {
        const auto lead  = static_cast<uint8_t>( text[ byte ] );
        const int  extra = lead >= 0xF0 ? 3 : lead >= 0xE0 ? 2 : lead >= 0xC0 ? 1 : 0;
        uint32_t   value = extra == 0 ? lead : lead & ( 0x3F >> extra );
        for ( int i = 1; i <= extra; ++i )
        {
            value = value << 6 | ( static_cast<uint8_t>( text[ byte + i ] ) & 0x3F );
        }
        return value;
    }
} // namespace

TEST_F( TextShaperTest, LatinWithHebrewKeepsLtrParagraph )
{
    const std::string text = "abc אבג def";
    Shape( text );
    EXPECT_EQ( m_shaped.Paragraphs[ 0 ].Level, 0 );
    EXPECT_EQ( Visual( text ), ( Order{ 0, 1, 2, 3, 6, 5, 4, 7, 8, 9, 10 } ) );
}

TEST_F( TextShaperTest, ArabicWithNumbersAndLatin )
{
    // The first strong character is Arabic, so the paragraph is RTL; the digits after Arabic become AN and stay left to right.
    const std::string text = "مرحبا 123 world";
    Shape( text );
    EXPECT_EQ( m_shaped.Paragraphs[ 0 ].Level, 1 );
    EXPECT_EQ( Visual( text ), ( Order{ 10, 11, 12, 13, 14, 9, 6, 7, 8, 5, 4, 3, 2, 1, 0 } ) );
}

TEST_F( TextShaperTest, DigitsInHebrew )
{
    const std::string text = "אבג 123 דה";
    Shape( text );
    EXPECT_EQ( Visual( text ), ( Order{ 9, 8, 7, 4, 5, 6, 3, 2, 1, 0 } ) );
}

TEST_F( TextShaperTest, ExplicitBaseLevel )
{
    const std::string text = "abc אבג";
    Shape( text, 1 );
    EXPECT_EQ( m_shaped.Paragraphs[ 0 ].Level, 1 );
    EXPECT_EQ( Visual( text ), ( Order{ 6, 5, 4, 3, 0, 1, 2 } ) );
    Shape( "123 456" );
    EXPECT_EQ( m_shaped.Paragraphs[ 0 ].Level, 0 );
}

TEST_F( TextShaperTest, NestedBracketsInRtl )
{
    // N0: the outer pair encloses R text, the inner one only L text but follows R context, so every bracket is R and gets mirrored.
    const std::string text = "אב (גד [ef] הו) זח";
    Shape( text );
    EXPECT_EQ( Visual( text ), ( Order{ 17, 16, 15, 14, 13, 12, 11, 10, 8, 9, 7, 6, 5, 4, 3, 2, 1, 0 } ) );
    for ( const TextGlyph &glyph : m_shaped.Glyphs )
    {
        if ( text[ glyph.Cluster ] == ')' )
        {
            EXPECT_EQ( glyph.GlyphId, NominalGlyph( '(' ) );
        }
        if ( text[ glyph.Cluster ] == '[' )
        {
            EXPECT_EQ( glyph.GlyphId, NominalGlyph( ']' ) );
        }
    }
}

TEST_F( TextShaperTest, NestedBracketsInLtr )
{
    // The inner pair holds only R text but the preceding strong type is L, so it takes the embedding direction.
    const std::string text = "a (b [אב] c) d";
    Shape( text );
    EXPECT_EQ( Visual( text ), ( Order{ 0, 1, 2, 3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 13 } ) );
}

TEST_F( TextShaperTest, TrailingWhitespaceTakesParagraphLevel )
{
    // Broken after "אב ", the space between two RTL words ends the line, so L1 moves it to the paragraph's end edge.
    const std::string text = "abc אב גד";
    Shape( text );
    EXPECT_EQ( Visual( text, 0, 7 ), ( Order{ 0, 1, 2, 3, 5, 4, 6 } ) );
}

TEST_F( TextShaperTest, LineBreakOpportunities )
{
    std::string text = "say (hi) now";
    Shape( text );
    EXPECT_EQ( BreaksAfter( text, TextGlyphFlags::BreakAfter ), ( Order{ 3, 8 } ) );

    text = "日本語。です";
    Shape( text );
    EXPECT_EQ( BreaksAfter( text, TextGlyphFlags::BreakAfter ), ( Order{ 0, 1, 3, 4 } ) );

    text = "well-known";
    Shape( text );
    EXPECT_EQ( BreaksAfter( text, TextGlyphFlags::BreakAfter ), ( Order{ 4 } ) );

    text = "a\xE2\x80\xA8"
           "b";
    Shape( text );
    EXPECT_EQ( BreaksAfter( text, TextGlyphFlags::MustBreak ), ( Order{ 1 } ) );
}

TEST_F( TextShaperTest, ParagraphsSplitOnHardBreaks )
{
    const std::string text = "abc\r\nאבג\n";
    Shape( text );
    ASSERT_EQ( m_shaped.Paragraphs.size( ), 3u );
    EXPECT_EQ( m_shaped.Paragraphs[ 0 ].ByteEnd, 3u );
    EXPECT_EQ( m_shaped.Paragraphs[ 1 ].ByteStart, 5u );
    EXPECT_EQ( m_shaped.Paragraphs[ 1 ].Level, 1 );
    EXPECT_EQ( m_shaped.Paragraphs[ 2 ].ByteStart, text.size( ) );
    EXPECT_TRUE( m_shaped.Glyphs[ m_shaped.Paragraphs[ 0 ].GlyphEnd ].Flags & TextGlyphFlags::Newline );
}

TEST( TextGrapheme, Boundaries )
{
    auto next     = []( const std::string &text, const uint32_t offset ) { return DenOfIz_Text_NextGrapheme( DenOfIz_StringView{ text.data( ), text.size( ) }, offset ); };
    auto previous = []( const std::string &text, const uint32_t offset ) { return DenOfIz_Text_PreviousGrapheme( DenOfIz_StringView{ text.data( ), text.size( ) }, offset ); };

    const std::string combining = "e\xCC\x81x";
    EXPECT_EQ( next( combining, 0 ), 3u );
    EXPECT_EQ( previous( combining, 3 ), 0u );
    EXPECT_EQ( next( combining, 3 ), 4u );

    const std::string zwj = "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB"
                            "a";
    EXPECT_EQ( next( zwj, 0 ), 11u );
    EXPECT_EQ( previous( zwj, 12 ), 11u );
    EXPECT_EQ( previous( zwj, 11 ), 0u );

    const std::string jamo = "\xE1\x84\x80\xE1\x85\xA1\xE1\x86\xA8\xED\x95\x9C";
    EXPECT_EQ( next( jamo, 0 ), 9u );
    EXPECT_EQ( next( jamo, 9 ), 12u );

    const std::string flags = "\xF0\x9F\x87\xA9\xF0\x9F\x87\xAA\xF0\x9F\x87\xAB\xF0\x9F\x87\xB7";
    EXPECT_EQ( next( flags, 0 ), 8u );
    EXPECT_EQ( previous( flags, 16 ), 8u );

    const std::string crlf = "a\r\nb";
    EXPECT_EQ( next( crlf, 1 ), 3u );
}

TEST_F( TextShaperTest, GraphemeStarts )
{
    const std::string text = "e\xCC\x81\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB";
    Shape( text );
    std::vector<uint32_t> starts;
    for ( uint32_t i = 0; i < m_shaped.GraphemeStarts.size( ); ++i )
    {
        if ( m_shaped.GraphemeStarts[ i ] )
        {
            starts.push_back( i );
        }
    }
    EXPECT_EQ( starts, ( Order{ 0, 3, 14 } ) );
}

TEST_F( TextShaperTest, IllFormedUtf8IsOneReplacementPerMaximalSubpart )
{
    // Overlong, surrogate, truncated, stray continuations, above U+10FFFF and truncated at the end.
    const std::string     text = "a\xC0\xAF"
                                 "b\xED\xA0\x80"
                                 "c\xE2\x82"
                                 "d\x80\xBF"
                                 "e\xF4\x90\x80\x80"
                                 "f\xF0\x9F\x98";
    std::string           valid;
    std::vector<uint32_t> origins;
    ASSERT_TRUE( TextReplaceIllFormed( text.data( ), static_cast<uint32_t>( text.size( ) ), valid, origins ) );
    std::string expected;
    for ( const char c : std::string( "a__b___c_d__e____f_" ) )
    {
        expected += c == '_' ? "\xEF\xBF\xBD" : std::string( 1, c );
    }
    EXPECT_EQ( valid, expected );
    EXPECT_FALSE( TextReplaceIllFormed( "a\xC3\xA9\xF0\x9F\x98\x80", 7, valid, origins ) );

    Shape( text );
    std::vector<uint32_t> clusters;
    for ( const TextGlyph &glyph : m_shaped.Glyphs )
    {
        clusters.push_back( glyph.Cluster );
        const bool replacement = glyph.Cluster < text.size( ) && static_cast<uint8_t>( text[ glyph.Cluster ] ) >= 0x80;
        EXPECT_EQ( glyph.GlyphId, NominalGlyph( replacement ? 0xFFFD : text[ glyph.Cluster ] ) ) << glyph.Cluster;
    }
    EXPECT_EQ( clusters, ( Order{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } ) );
    std::vector<uint32_t> starts;
    for ( uint32_t i = 0; i < m_shaped.GraphemeStarts.size( ); ++i )
    {
        if ( m_shaped.GraphemeStarts[ i ] )
        {
            starts.push_back( i );
        }
    }
    clusters.push_back( static_cast<uint32_t>( text.size( ) ) );
    EXPECT_EQ( starts, clusters );

    const DenOfIz_StringView view{ text.data( ), text.size( ) };
    EXPECT_EQ( DenOfIz_Text_NextGrapheme( view, 8 ), 10u );
    EXPECT_EQ( DenOfIz_Text_PreviousGrapheme( view, 10 ), 8u );
    EXPECT_EQ( DenOfIz_Text_NextGrapheme( view, 19 ), 22u );
    EXPECT_EQ( DenOfIz_Text_PreviousGrapheme( view, 22 ), 19u );
}

TEST_F( TextShaperTest, DefaultIgnorablesNeverDraw )
{
    // Soft hyphen, ZWSP, word joiner, BOM, LRM, CGJ, ZWJ and VS1 outside any sequence, a bidi isolate and an embedding.
    const std::string text = "a\u00ADb\u200Bc\u2060d\uFEFFe\u200Ef\u034Fg\u200Dh\uFE00i\u2066j\u2069k\u202Al\u202Cm";
    Shape( text );
    hb_codepoint_t space = 0;
    hb_font_get_nominal_glyph( m_unicode.Get( )->HbFont( ), ' ', &space );
    uint32_t visible = 0;
    for ( const TextGlyph &glyph : m_shaped.Glyphs )
    {
        // They stay in the font of the text around them, a fallback font would split the item.
        EXPECT_EQ( glyph.Font, 0 ) << glyph.Cluster;
        if ( glyph.GlyphId == NominalGlyph( text[ glyph.Cluster ] ) )
        {
            ++visible;
            EXPECT_GT( glyph.Advance, 0.0f );
            continue;
        }
        EXPECT_EQ( glyph.Advance, 0.0f ) << glyph.Cluster;
        EXPECT_TRUE( glyph.GlyphId == TextHiddenGlyph || glyph.GlyphId == space ) << glyph.Cluster;
    }
    EXPECT_EQ( visible, 13u );
}

TEST_F( TextShaperTest, BidiControlsGiveTheBrowserOrder )
{
    // Visual order of the visible characters only, the controls take no space.
    auto visible = [ this ]( const std::string &text )
    {
        Shape( text );
        Order order;
        for ( const uint32_t c : Visual( text ) )
        {
            uint32_t byte = 0;
            for ( uint32_t i = 0, n = 0; i < text.size( ); ++i )
            {
                if ( ( static_cast<uint8_t>( text[ i ] ) & 0xC0 ) != 0x80 && n++ == c )
                {
                    byte = i;
                    break;
                }
            }
            if ( !TextIsDefaultIgnorable( CodePointAt( text, byte ) ) )
            {
                order.push_back( c );
            }
        }
        return order;
    };
    EXPECT_EQ( visible( "a\u202Ebc\u202Cd" ), ( Order{ 0, 3, 2, 5 } ) );
    EXPECT_EQ( visible( "a\u202Bb c\u202Cd" ), ( Order{ 0, 2, 3, 4, 6 } ) );
    EXPECT_EQ( visible( "a\u202B1 2\u202Cd" ), ( Order{ 0, 4, 3, 2, 6 } ) );
    EXPECT_EQ( visible( "שלום \u2066abc def\u2069 עולם" ), ( Order{ 18, 17, 16, 15, 14, 6, 7, 8, 9, 10, 11, 12, 4, 3, 2, 1, 0 } ) );
    EXPECT_EQ( visible( "x \u2068שלום abc\u2069 y" ), ( Order{ 0, 1, 8, 9, 10, 7, 6, 5, 4, 3, 12, 13 } ) );
    EXPECT_EQ( visible( "\u202Dשלום\u202C" ), ( Order{ 1, 2, 3, 4 } ) );
    // The space between RLM and the digits resolves to R and goes right of them.
    EXPECT_EQ( visible( "abc\u200F 123" ), ( Order{ 0, 1, 2, 5, 6, 7, 4 } ) );
}

namespace
{
    class UITextLayoutTest : public TextShaperTest
    {
    protected:
        FontFile m_mono{ MonoPath };
        UIText   m_text;

        void SetUp( ) override
        {
            TextShaperTest::SetUp( );
            ASSERT_NE( m_mono.Get( ), nullptr );
            m_text.AddFont( 1, m_unicode.Get( ) );
            m_text.AddFont( 2, m_mono.Get( ) );
        }

        const UITextLayout *Layout( const std::string &text, const uint16_t fontId, const float width = -1.0f, const DenOfIz_UITextWrap wrap = DENOFIZ_UI_TEXT_WRAP_WORDS )
        {
            const UIShapedRun *run = m_text.Shape( fontId, text.data( ), static_cast<uint32_t>( text.size( ) ), UIText::Hash( text.data( ), text.size( ) ) );
            UITextParams       params;
            params.FontId   = fontId;
            params.FontSize = 20.0f;
            params.Wrap     = wrap;
            return m_text.Layout( run, params, width );
        }

        static std::vector<uint32_t> Stops( const std::string &text )
        {
            std::vector<uint32_t> stops{ 0 };
            while ( stops.back( ) < text.size( ) )
            {
                stops.push_back( DenOfIz_Text_NextGrapheme( DenOfIz_StringView{ text.data( ), text.size( ) }, stops.back( ) ) );
            }
            return stops;
        }
    };
} // namespace

TEST_F( UITextLayoutTest, CaretMovesRightForLtr )
{
    const std::string   text   = "office hours";
    const UITextLayout *layout = Layout( text, 2 );
    float               prev   = -1.0f;
    for ( const uint32_t stop : Stops( text ) )
    {
        const float x = layout->CaretX( 0, stop );
        EXPECT_GT( x, prev ) << stop;
        EXPECT_EQ( layout->OffsetAt( x + 0.1f, 1.0f, 0.0f, DENOFIZ_UI_TEXT_ALIGN_LEFT ), stop );
        prev = x;
    }
    EXPECT_FLOAT_EQ( prev, layout->Width );
}

TEST_F( UITextLayoutTest, CaretMovesLeftForRtl )
{
    const std::string   text   = "שָׁלוֹם עולם";
    const UITextLayout *layout = Layout( text, 1 );
    ASSERT_TRUE( layout->Lines[ 0 ].Rtl );
    float prev = FLT_MAX;
    for ( const uint32_t stop : Stops( text ) )
    {
        const float x = layout->CaretX( 0, stop );
        EXPECT_LT( x, prev ) << stop;
        EXPECT_EQ( layout->OffsetAt( x - 0.1f, 1.0f, 0.0f, DENOFIZ_UI_TEXT_ALIGN_LEFT ), stop );
        prev = x;
    }
    EXPECT_FLOAT_EQ( layout->CaretX( 0, 0 ), layout->Width );
    EXPECT_NEAR( prev, 0.0f, 1e-4f );
}

TEST_F( UITextLayoutTest, CaretAtDirectionBoundaries )
{
    // Visually "abc גבא def": a caret before an RTL letter sits at its right edge, so the start of the Hebrew run is where " def" begins.
    const std::string   text   = "abc אבג def";
    const UITextLayout *layout = Layout( text, 1 );
    EXPECT_FLOAT_EQ( layout->CaretX( 0, 4 ), layout->CaretX( 0, 10 ) );
    EXPECT_LT( layout->CaretX( 0, 8 ), layout->CaretX( 0, 6 ) );
    EXPECT_LT( layout->CaretX( 0, 6 ), layout->CaretX( 0, 4 ) );
    EXPECT_GT( layout->CaretX( 0, 8 ), layout->CaretX( 0, 3 ) );

    // Selecting "c א" covers "c " on the left and the first Hebrew letter at the right end of the run.
    std::vector<DenOfIz_Float2> spans;
    layout->SelectionSpans( 0, 2, 6, 0.0f, spans );
    ASSERT_EQ( spans.size( ), 2u );
    EXPECT_FLOAT_EQ( spans[ 0 ].X, layout->CaretX( 0, 2 ) );
    EXPECT_GT( spans[ 0 ].Y, layout->CaretX( 0, 3 ) );
    EXPECT_LT( spans[ 0 ].Y, layout->CaretX( 0, 8 ) );
    EXPECT_FLOAT_EQ( spans[ 1 ].X, layout->CaretX( 0, 6 ) );
    EXPECT_FLOAT_EQ( spans[ 1 ].Y, layout->CaretX( 0, 4 ) );
}

TEST_F( UITextLayoutTest, LigatureSplitsCaretEvenly )
{
    // Lam alef is one glyph covering two grapheme clusters.
    const std::string   text   = "لا";
    const UITextLayout *layout = Layout( text, 1 );
    if ( layout->Run->Shaped.Glyphs.size( ) != 1 )
    {
        GTEST_SKIP( ) << "the font has no lam alef ligature";
    }
    EXPECT_FLOAT_EQ( layout->CaretX( 0, 0 ), layout->Width );
    EXPECT_FLOAT_EQ( layout->CaretX( 0, 2 ), layout->Width * 0.5f );
    EXPECT_NEAR( layout->CaretX( 0, 4 ), 0.0f, 1e-4f );
    EXPECT_EQ( layout->OffsetAt( layout->Width * 0.45f, 1.0f, 0.0f, DENOFIZ_UI_TEXT_ALIGN_LEFT ), 2u );
}

TEST_F( UITextLayoutTest, RtlParagraphWraps )
{
    const std::string   text   = "זהו טקסט עברי ארוך שנשבר לכמה שורות בתוך תיבה צרה";
    const UITextLayout *layout = Layout( text, 1, 160.0f );
    ASSERT_GT( layout->Lines.size( ), 2u );
    const auto &glyphs = layout->Glyphs( );
    for ( size_t i = 0; i < layout->Lines.size( ); ++i )
    {
        const UITextLineLayout &line = layout->Lines[ i ];
        EXPECT_TRUE( line.Rtl );
        EXPECT_LE( line.Width, 160.01f );
        // The first logical glyph is at the right end of the visible line, a trailing space hangs left of zero.
        EXPECT_NEAR( layout->CaretX( static_cast<uint32_t>( i ), line.ByteStart ), line.Width, 1e-3f );
        if ( i + 1 < layout->Lines.size( ) )
        {
            ASSERT_TRUE( glyphs[ line.GlyphEnd - 1 ].Flags & TextGlyphFlags::Space );
            EXPECT_LT( layout->GlyphX[ line.GlyphEnd - 1 ], 0.0f );
        }
    }
}

TEST_F( UITextLayoutTest, CjkWrapsBetweenCharacters )
{
    const UITextLayout *layout = Layout( "日本語のテキスト", 1, 50.0f );
    EXPECT_GT( layout->Lines.size( ), 2u );
    for ( const UITextLineLayout &line : layout->Lines )
    {
        EXPECT_LE( line.Width, 50.01f );
    }
}

TEST_F( UITextLayoutTest, RtlEllipsisIsOnTheLeft )
{
    const std::string  text = "זהו טקסט עברי ארוך שלא נכנס בשורה אחת";
    const UIShapedRun *run  = m_text.Shape( 1, text.data( ), static_cast<uint32_t>( text.size( ) ), UIText::Hash( text.data( ), text.size( ) ) );
    UITextParams       params;
    params.FontId                  = 1;
    params.FontSize                = 20.0f;
    params.Overflow                = DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS;
    params.MaxLines                = 1;
    const UITextLayout     *layout = m_text.Layout( run, params, 150.0f );
    const UITextLineLayout &line   = layout->Lines[ 0 ];
    ASSERT_TRUE( line.Ellipsis && line.Rtl );
    EXPECT_LE( line.Width, 150.01f );
    const float ellipsis = layout->Face->EllipsisAdvance * layout->Scale;
    for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
    {
        EXPECT_GE( layout->GlyphX[ i ], ellipsis - 1e-3f );
    }
    EXPECT_NEAR( layout->CaretX( 0, 0 ), line.Width, 1e-3f );
}

TEST_F( UITextLayoutTest, StartAndEndFollowTheParagraphDirection )
{
    const UITextLayout *ltr  = Layout( "office", 2 );
    const UITextLayout *rtl  = Layout( "שלום", 1 );
    const float         box  = 400.0f;
    const float         free = box - rtl->Lines[ 0 ].Width;
    EXPECT_FLOAT_EQ( ltr->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_START ), 0.0f );
    EXPECT_FLOAT_EQ( ltr->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_END ), box - ltr->Lines[ 0 ].Width );
    EXPECT_FLOAT_EQ( rtl->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_START ), free );
    EXPECT_FLOAT_EQ( rtl->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_END ), 0.0f );
    EXPECT_FLOAT_EQ( rtl->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_LEFT ), 0.0f );
    EXPECT_FLOAT_EQ( rtl->AlignOffset( 0, box, DENOFIZ_UI_TEXT_ALIGN_RIGHT ), free );
    EXPECT_EQ( DenOfIz_UITextDesc{ }.Align, DENOFIZ_UI_TEXT_ALIGN_START );
}

TEST_F( UITextLayoutTest, KerningStopsAtAnEmergencyBreak )
{
    FontFile inter{ DZ_TEST_DATA_SRC_DIR "/Fonts/Inter-Regular.ttf" };
    ASSERT_NE( inter.Get( ), nullptr );
    m_text.AddFont( 3, inter.Get( ) );
    const UITextLayout *layout = Layout( "AVAVAVAVAVAVAVAVAVAVAVAV", 3, 70.0f );
    ASSERT_GT( layout->Lines.size( ), 2u );
    ASSERT_FALSE( layout->Reshaped.empty( ) );
    const auto &glyphs = layout->Glyphs( );
    hb_font_t  *font   = inter.Get( )->HbFont( );
    const float upem   = static_cast<float>( inter.Get( )->Upem( ) );
    for ( size_t i = 0; i + 1 < layout->Lines.size( ); ++i )
    {
        // The last glyph of a line has no pair to kern with, the one shaped as a whole word does.
        const UITextLineLayout &line = layout->Lines[ i ];
        const TextGlyph        &last = glyphs[ line.GlyphEnd - 1 ];
        EXPECT_FLOAT_EQ( last.Advance, static_cast<float>( hb_font_get_glyph_h_advance( font, last.GlyphId ) ) / upem );
        EXPECT_LT( layout->Run->Shaped.Glyphs[ last.Cluster ].Advance, last.Advance );
        EXPECT_LE( line.Width, 70.01f );
        EXPECT_EQ( glyphs[ line.GlyphStart ].Cluster, line.ByteStart );
    }

    // In a later paragraph after ill-formed UTF-8 the pieces are shaped from the same replaced text and map back to the bytes given.
    const std::string   text  = "x\xC0\nAVAVAVAVAVAV\xE2\x82VAVAVAVAVAVAVAV";
    const UITextLayout *later = Layout( text, 3, 70.0f );
    ASSERT_GT( later->Lines.size( ), 3u );
    ASSERT_FALSE( later->Reshaped.empty( ) );
    for ( const UITextLineLayout &line : later->Lines )
    {
        for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
        {
            const TextGlyph &glyph = later->Glyphs( )[ i ];
            EXPECT_GE( glyph.Cluster, line.ByteStart );
            EXPECT_LT( glyph.Cluster, line.ByteEnd );
            EXPECT_NE( glyph.Cluster, 16u ) << "inside the replaced sequence";
        }
        EXPECT_LE( line.Width, 70.01f );
    }
    m_text.RemoveFont( 3 );
}

TEST_F( UITextLayoutTest, ArabicKeepsJoiningAcrossAnEmergencyBreak )
{
    // Joining forms stay as if the word were whole (CSS Text 3), only the pieces are shaped apart.
    const std::string   text   = "مستشفياتمستشفياتمستشفيات";
    const UITextLayout *layout = Layout( text, 1, 60.0f );
    ASSERT_GT( layout->Lines.size( ), 1u );
    ASSERT_FALSE( layout->Reshaped.empty( ) );
    const auto &glyphs = layout->Glyphs( );
    const auto &whole  = layout->Run->Shaped.Glyphs;
    for ( const UITextLineLayout &line : layout->Lines )
    {
        for ( uint32_t i = line.GlyphStart; i < line.GlyphEnd; ++i )
        {
            const auto it = std::find_if( whole.begin( ), whole.end( ), [ & ]( const TextGlyph &g ) { return g.Cluster == glyphs[ i ].Cluster; } );
            ASSERT_NE( it, whole.end( ) );
            EXPECT_EQ( glyphs[ i ].GlyphId, it->GlyphId ) << glyphs[ i ].Cluster;
            EXPECT_GE( glyphs[ i ].Cluster, line.ByteStart );
            EXPECT_LT( glyphs[ i ].Cluster, line.ByteEnd );
        }
    }
}

TEST_F( UITextLayoutTest, LineHeightIsTheTallestFontOnTheLine )
{
    // JetBrains Mono has neither emoji nor Myanmar, both come from system fonts with taller metrics.
    const UITextLayout *layout = Layout( "abc\nabc\xF0\x9F\x98\x80\n\xE1\x80\x99\xE1\x80\xBC\xE1\x80\x94\xE1\x80\xBA\xE1\x80\x99\xE1\x80\xAC", 2 );
    ASSERT_EQ( layout->Lines.size( ), 3u );
    const float mono = m_mono.Get( )->Metrics( ).LineHeight * 20.0f;
    EXPECT_FLOAT_EQ( layout->Lines[ 0 ].Height, mono );
    float top = 0.0f;
    for ( const UITextLineLayout &line : layout->Lines )
    {
        EXPECT_FLOAT_EQ( line.Top, top );
        EXPECT_GT( line.Baseline, line.Top );
        EXPECT_LT( line.Baseline, line.Top + line.Height );
        top += line.Height;
    }
    EXPECT_FLOAT_EQ( layout->Height, top );
    for ( const uint32_t i : { 1u, 2u } )
    {
        const UITextLineLayout &line   = layout->Lines[ i ];
        const auto             &glyphs = layout->Glyphs( );
        const Font             *other  = layout->Run->Shaped.Fonts[ glyphs[ line.GlyphEnd - 1 ].Font ];
        ASSERT_NE( other, m_mono.Get( ) ) << i;
        const DenOfIz_FontMetrics &a     = m_mono.Get( )->Metrics( );
        const DenOfIz_FontMetrics &b     = other->Metrics( );
        const float                above = std::max( a.Ascent + a.LineGap * 0.5f, b.Ascent + b.LineGap * 0.5f );
        const float                below = std::max( a.Descent + a.LineGap * 0.5f, b.Descent + b.LineGap * 0.5f );
        EXPECT_NEAR( line.Height, ( above + below ) * 20.0f, 1e-3f ) << i;
        EXPECT_NEAR( line.Baseline - line.Top, above * 20.0f, 1e-3f ) << i;
    }
    // Myanmar MN is taller than JetBrains Mono, Apple Color Emoji is not.
    EXPECT_GT( layout->Lines[ 2 ].Height, mono );

    UITextParams params;
    params.FontId     = 2;
    params.FontSize   = 20.0f;
    params.LineHeight = 30.0f;
    const auto *fixed = m_text.Layout( layout->Run, params, -1.0f );
    for ( const UITextLineLayout &line : fixed->Lines )
    {
        EXPECT_FLOAT_EQ( line.Height, 30.0f );
    }
    EXPECT_FLOAT_EQ( fixed->Height, 90.0f );
    // Hit testing finds the taller line.
    EXPECT_EQ( layout->LineAt( layout->Lines[ 1 ].Top + layout->Lines[ 1 ].Height - 0.5f ), 1u );
}
