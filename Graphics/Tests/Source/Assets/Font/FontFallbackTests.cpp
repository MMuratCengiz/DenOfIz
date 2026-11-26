/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "gtest/gtest.h"

#include <harfbuzz/hb-ot.h>
#include <cstring>
#include <filesystem>
#include <string>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/UI/UIText.h"

using namespace DenOfIz;

namespace
{
    const char *ArialUnicodePath = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
    const char *LatinOnlyPath    = "/System/Library/Fonts/Supplemental/Futura.ttc";
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

    uint32_t Nominal( const Font *font, const uint32_t codePoint )
    {
        hb_codepoint_t glyph = 0;
        hb_font_get_nominal_glyph( font->HbFont( ), codePoint, &glyph );
        return glyph;
    }

    // Outlines the glyph store can compile and no colour bitmaps, which a glyf table may only box.
    bool Drawable( const Font *font )
    {
        hb_face_t *face  = hb_font_get_face( font->HbFont( ) );
        auto       table = [ face ]( const hb_tag_t tag )
        {
            hb_blob_t *blob   = hb_face_reference_table( face, tag );
            const bool exists = hb_blob_get_length( blob ) > 0;
            hb_blob_destroy( blob );
            return exists;
        };
        return ( table( HB_TAG( 'g', 'l', 'y', 'f' ) ) || table( HB_TAG( 'C', 'F', 'F', ' ' ) ) || table( HB_TAG( 'C', 'F', 'F', '2' ) ) ) && !hb_ot_color_has_png( face );
    }

    // JetBrains Mono covers Latin, Greek and Cyrillic but no Hebrew, Arial Unicode covers all of them.
    class FontFallbackTest : public testing::Test
    {
    protected:
        FontFile   m_mono{ MonoPath };
        FontFile   m_unicode{ ArialUnicodePath };
        TextShaper m_shaper;
        ShapedText m_shaped;

        void SetUp( ) override
        {
            ASSERT_NE( m_mono.Get( ), nullptr );
            if ( m_unicode.Get( ) == nullptr )
            {
                GTEST_SKIP( ) << ArialUnicodePath << " is missing";
            }
        }

        void Shape( const std::string &text, const std::initializer_list<Font *> fonts )
        {
            const std::vector<Font *> chain( fonts );
            m_shaper.Shape( text.data( ), static_cast<uint32_t>( text.size( ) ), chain, TextAutoLevel, m_shaped );
        }

        const TextGlyph &GlyphAt( const uint32_t byte ) const
        {
            for ( const TextGlyph &glyph : m_shaped.Glyphs )
            {
                if ( glyph.Cluster == byte )
                {
                    return glyph;
                }
            }
            static constexpr TextGlyph none{ };
            ADD_FAILURE( ) << "no glyph at byte " << byte;
            return none;
        }
    };
} // namespace

TEST_F( FontFallbackTest, ChainOrder )
{
    const std::string text = "ab \xD7\x90\xD7\x91";
    Shape( text, { m_mono.Get( ), m_unicode.Get( ) } );
    ASSERT_EQ( m_shaped.Fonts.size( ), 2u );
    EXPECT_EQ( GlyphAt( 0 ).Font, 0 );
    EXPECT_EQ( GlyphAt( 3 ).Font, 1 );
    EXPECT_EQ( GlyphAt( 3 ).GlyphId, Nominal( m_unicode.Get( ), 0x5D0 ) );

    // Every character is covered by the first font, so the second is never used.
    Shape( text, { m_unicode.Get( ), m_mono.Get( ) } );
    for ( const TextGlyph &glyph : m_shaped.Glyphs )
    {
        EXPECT_EQ( glyph.Font, 0 );
    }
}

TEST_F( FontFallbackTest, ClusterStaysInOneFont )
{
    // The mono font has the base letter but not the Hebrew point, so the whole cluster moves to the font covering both.
    const std::string text = "xa\xD6\xB8y";
    Shape( text, { m_mono.Get( ), m_unicode.Get( ) } );
    EXPECT_EQ( GlyphAt( 0 ).Font, 0 );
    EXPECT_EQ( GlyphAt( 4 ).Font, 0 );
    uint32_t inCluster = 0;
    for ( const TextGlyph &glyph : m_shaped.Glyphs )
    {
        if ( glyph.Cluster >= 1 && glyph.Cluster < 4 )
        {
            EXPECT_EQ( glyph.Font, 1 );
            EXPECT_NE( glyph.GlyphId, 0u );
            ++inCluster;
        }
    }
    EXPECT_GE( inCluster, 1u );
}

TEST_F( FontFallbackTest, FontsWithDifferentUpem )
{
    ASSERT_NE( m_mono.Get( )->Upem( ), m_unicode.Get( )->Upem( ) );
    const std::string text = "a \xD7\x90";
    Shape( text, { m_mono.Get( ), m_unicode.Get( ) } );
    for ( const uint32_t byte : { 0u, 2u } )
    {
        const TextGlyph &glyph = GlyphAt( byte );
        const Font      *font  = m_shaped.Fonts[ glyph.Font ];
        EXPECT_EQ( font, byte == 0 ? m_mono.Get( ) : m_unicode.Get( ) );
        const float advance = static_cast<float>( hb_font_get_glyph_h_advance( font->HbFont( ), glyph.GlyphId ) ) / static_cast<float>( font->Upem( ) );
        EXPECT_NEAR( glyph.Advance, advance, 1e-6f );
    }
}

TEST_F( FontFallbackTest, UITextChainAndInvalidation )
{
    UIText text;
    text.AddFont( 1, m_mono.Get( ) );
    // A fallback may be named before it is registered.
    text.SetFallbacks( 1, std::vector<uint16_t>{ 2 } );
    text.AddFont( 2, m_unicode.Get( ) );

    const std::string chars = "a \xD7\x90";
    auto              shape = [ & ] { return text.Shape( 1, chars.data( ), static_cast<uint32_t>( chars.size( ) ), UIText::Hash( chars.data( ), chars.size( ) ) ); };
    auto              alef  = []( const UIShapedRun *run ) { return run->Shaped.Fonts[ run->Shaped.Glyphs[ 2 ].Font ]; };

    const UIShapedRun *run = shape( );
    EXPECT_EQ( alef( run ), m_unicode.Get( ) );
    UITextParams params;
    params.FontId = 1;
    EXPECT_EQ( text.Layout( run, params, -1.0f )->GetQuads( text.Glyphs( ), text.ColourGlyphs( ) ).size( ), 2u );

    const uint64_t epoch = text.Epoch( );
    text.RemoveFont( 2 );
    EXPECT_GT( text.Epoch( ), epoch );
    EXPECT_NE( alef( shape( ) ), m_unicode.Get( ) );

    text.AddFont( 2, m_unicode.Get( ) );
    EXPECT_EQ( alef( shape( ) ), m_unicode.Get( ) );
    text.SetFallbacks( 1, { } );
    EXPECT_NE( alef( shape( ) ), m_unicode.Get( ) );
}

namespace
{
    class SystemFallbackTest : public testing::Test
    {
    protected:
        FontFile   m_latin{ LatinOnlyPath };
        TextShaper m_shaper;
        ShapedText m_shaped;

        void SetUp( ) override
        {
#ifndef __APPLE__
            GTEST_SKIP( ) << "only the CoreText lookup is tested";
#endif
            if ( m_latin.Get( ) == nullptr )
            {
                GTEST_SKIP( ) << LatinOnlyPath << " is missing";
            }
        }

        void Shape( const std::string &text )
        {
            Font *const fonts[] = { m_latin.Get( ) };
            m_shaper.Shape( text.data( ), static_cast<uint32_t>( text.size( ) ), fonts, TextAutoLevel, m_shaped );
        }
    };
} // namespace

TEST_F( SystemFallbackTest, ScriptsMissingFromThePrimaryFont )
{
    const char *samples[] = { "مرحبا بالعالم", "שלום עולם", "नमस्ते दुनिया", "สวัสดีชาวโลก", "日本語のテキスト", "中文字体", "한국어", "Ελληνικά", "Русский язык" };
    for ( const std::string text : samples )
    {
        Shape( text );
        ASSERT_FALSE( m_shaped.Glyphs.empty( ) );
        EXPECT_GT( m_shaped.Fonts.size( ), 1u ) << text;
        for ( const TextGlyph &glyph : m_shaped.Glyphs )
        {
            if ( glyph.GlyphId == TextHiddenGlyph )
            {
                continue;
            }
            EXPECT_GT( glyph.Font, 0 ) << text;
            EXPECT_NE( glyph.GlyphId, 0u ) << text << " byte " << glyph.Cluster;
            EXPECT_TRUE( Drawable( m_shaped.Fonts[ glyph.Font ] ) ) << text;
        }
    }
}

TEST_F( SystemFallbackTest, CacheReturnsTheSameFont )
{
    Font *first = SystemFallbackFont( "\xE6\x97\xA5", 3, HB_SCRIPT_HAN, false );
    ASSERT_NE( first, nullptr );
    EXPECT_EQ( SystemFallbackFont( "\xE6\x97\xA5", 3, HB_SCRIPT_HAN, false ), first );
    // A new character of the same script is found in the font already picked for it.
    EXPECT_EQ( SystemFallbackFont( "\xE6\x9C\xAC", 3, HB_SCRIPT_HAN, false ), first );

    Shape( "\xE6\x97\xA5 \xE6\x97\xA5" );
    ASSERT_EQ( m_shaped.Fonts.size( ), 2u );
    EXPECT_EQ( m_shaped.Fonts[ 1 ], first );
}
