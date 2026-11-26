/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "gtest/gtest.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/ColourGlyphStore.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Emoji.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/UI/UIText.h"
#include "stb_image_write.h"

using namespace DenOfIz;

namespace
{
    const char       *AppleEmojiPath = "/System/Library/Fonts/Apple Color Emoji.ttc";
    const char       *MonoPath       = DZ_TEST_DATA_SRC_DIR "/Fonts/JetBrainsMono-Regular.ttf";
    const char       *ColrTestPath   = DZ_TEST_DATA_SRC_DIR "/Fonts/test_glyphs-glyf_colr_1.ttf";
    const std::string OutDir         = DZ_TEST_DATA_DEST_DIR "/Colour/";
    // Painted with the foreground for palette index 0xFFFF: #204080.
    const hb_color_t Foreground = HB_COLOR( 0x80, 0x40, 0x20, 0xFF );

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

    struct Rgba
    {
        int R, G, B, A;
    };

    // Straight colour of the pixel covering a point given in font units.
    Rgba ColourAt( const ColourBitmap &bitmap, const Font &font, const uint32_t ppem, const float x, const float y )
    {
        const float scale = static_cast<float>( ppem ) / static_cast<float>( font.Upem( ) );
        const int   col   = static_cast<int>( std::floor( x * scale ) ) - bitmap.Left;
        const int   row   = static_cast<int>( std::floor( -y * scale ) ) - bitmap.Top;
        if ( col < 0 || row < 0 || col >= static_cast<int>( bitmap.Width ) || row >= static_cast<int>( bitmap.Height ) )
        {
            return { 0, 0, 0, 0 };
        }
        const uint32_t p = bitmap.Pixels[ static_cast<size_t>( row ) * bitmap.Width + col ];
        const int      a = static_cast<int>( p >> 24 );
        auto straight    = [ a ]( const uint32_t c ) { return a == 0 ? 0 : static_cast<int>( std::lround( static_cast<float>( c & 0xFF ) * 255.0f / static_cast<float>( a ) ) ); };
        return { straight( p ), straight( p >> 8 ), straight( p >> 16 ), a };
    }

    void ExpectColour( const Rgba &actual, const Rgba &expected, const int tolerance, const char *what )
    {
        EXPECT_NEAR( actual.R, expected.R, tolerance ) << what;
        EXPECT_NEAR( actual.G, expected.G, tolerance ) << what;
        EXPECT_NEAR( actual.B, expected.B, tolerance ) << what;
        EXPECT_NEAR( actual.A, expected.A, tolerance ) << what;
    }

    void Save( const ColourBitmap &bitmap, const std::string &name )
    {
        std::filesystem::create_directories( OutDir );
        std::vector<uint8_t> rgba( bitmap.Pixels.size( ) * 4 );
        for ( size_t i = 0; i < bitmap.Pixels.size( ); ++i )
        {
            const uint32_t p = bitmap.Pixels[ i ];
            const uint32_t a = p >> 24;
            for ( int c = 0; c < 3; ++c )
            {
                rgba[ i * 4 + c ] = static_cast<uint8_t>( a == 0 ? 0 : std::min( 255u, ( ( p >> ( c * 8 ) & 0xFF ) * 255 + a / 2 ) / a ) );
            }
            rgba[ i * 4 + 3 ] = static_cast<uint8_t>( a );
        }
        stbi_write_png( ( OutDir + name + ".png" ).c_str( ), static_cast<int>( bitmap.Width ), static_cast<int>( bitmap.Height ), 4, rgba.data( ),
                        static_cast<int>( bitmap.Width ) * 4 );
    }

    uint32_t Nominal( const Font &font, const uint32_t codePoint )
    {
        hb_codepoint_t glyph = 0;
        hb_font_get_nominal_glyph( font.HbFont( ), codePoint, &glyph );
        return glyph;
    }

    // The first glyph HarfBuzz shapes the text to, ligatures included.
    uint32_t Shaped( const Font &font, const char *text )
    {
        hb_buffer_t *buffer = hb_buffer_create( );
        hb_buffer_add_utf8( buffer, text, -1, 0, -1 );
        hb_buffer_guess_segment_properties( buffer );
        hb_shape( font.HbFont( ), buffer, nullptr, 0 );
        unsigned int count = 0;
        const auto  *infos = hb_buffer_get_glyph_infos( buffer, &count );
        const auto   glyph = count > 0 ? infos[ 0 ].codepoint : 0;
        hb_buffer_destroy( buffer );
        return glyph;
    }

    double Rasterise( const Font &font, const uint32_t glyph, const uint32_t ppem, ColourBitmap &bitmap )
    {
        const auto start = std::chrono::steady_clock::now( );
        EXPECT_TRUE( RasteriseColourGlyph( font, glyph, ppem, Foreground, bitmap ) ) << "glyph " << glyph << " at " << ppem;
        return std::chrono::duration<double, std::milli>( std::chrono::steady_clock::now( ) - start ).count( );
    }

    class ColrTest : public testing::Test
    {
    protected:
        FontFile m_font{ ColrTestPath };

        void SetUp( ) override
        {
            ASSERT_NE( m_font.Get( ), nullptr );
            ASSERT_TRUE( m_font.Get( )->IsColour( ) );
        }

        ColourBitmap Paint( const uint32_t codePoint, const uint32_t ppem, const char *name )
        {
            ColourBitmap bitmap;
            Rasterise( *m_font.Get( ), Nominal( *m_font.Get( ), codePoint ), ppem, bitmap );
            Save( bitmap, name );
            return bitmap;
        }

        Rgba At( const ColourBitmap &bitmap, const uint32_t ppem, const float x, const float y ) const
        {
            return ColourAt( bitmap, *m_font.Get( ), ppem, x, y );
        }
    };
} // namespace

TEST( EmojiPresentation, Clusters )
{
    auto presentation = []( const char *cluster ) { return ClusterPresentation( cluster, static_cast<uint32_t>( std::strlen( cluster ) ) ); };
    EXPECT_EQ( presentation( "a" ), TextPresentation::Default );
    EXPECT_EQ( presentation( "\xF0\x9F\x98\x80" ), TextPresentation::Emoji );                                   // 😀
    EXPECT_EQ( presentation( "\xE2\x98\xBA" ), TextPresentation::Default );                                     // ☺
    EXPECT_EQ( presentation( "\xE2\x98\xBA\xEF\xB8\x8F" ), TextPresentation::Emoji );                           // ☺ VS16
    EXPECT_EQ( presentation( "\xE2\x9D\xA4\xEF\xB8\x8E" ), TextPresentation::Text );                            // ❤ VS15
    EXPECT_EQ( presentation( "\xF0\x9F\x98\x80\xEF\xB8\x8E" ), TextPresentation::Text );                        // 😀 VS15
    EXPECT_EQ( presentation( "1\xEF\xB8\x8F\xE2\x83\xA3" ), TextPresentation::Emoji );                          // 1️⃣
    EXPECT_EQ( presentation( "\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD" ), TextPresentation::Emoji );                   // 👍🏽
    EXPECT_EQ( presentation( "\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1" ), TextPresentation::Emoji );                   // 🇳🇱
    EXPECT_EQ( presentation( "\xE2\x9C\x8C\xF0\x9F\x8F\xBB" ), TextPresentation::Emoji );                       // ✌🏻, a text default base with a modifier
    EXPECT_EQ( presentation( "\xE0\xA4\x95\xE0\xA5\x8D\xE2\x80\x8D\xE0\xA4\xB7" ), TextPresentation::Default ); // क्‍ष joins without emoji
}

TEST_F( ColrTest, LayersOfColrV0 )
{
    // colored_circles_v0: concentric rings 50 units apart around ( 500, 600 ), red outside to violet inside.
    const uint32_t     ppem   = 200;
    const ColourBitmap bitmap = Paint( 0xF0E00, ppem, "colr0_circles" );
    ASSERT_GT( bitmap.Width, 0u );
    EXPECT_FALSE( bitmap.UsesForeground );
    const std::array<Rgba, 7> rings = {
        { { 255, 0, 0, 255 }, { 255, 165, 0, 255 }, { 255, 255, 0, 255 }, { 0, 128, 0, 255 }, { 0, 0, 255, 255 }, { 75, 0, 130, 255 }, { 238, 130, 238, 255 } }
    };
    for ( size_t i = 0; i < rings.size( ); ++i )
    {
        ExpectColour( At( bitmap, ppem, 500.0f, 600.0f + 325.0f - 50.0f * static_cast<float>( i ) ), rings[ i ], 2, "ring" );
    }
    ExpectColour( At( bitmap, ppem, 500.0f, 990.0f ), { 0, 0, 0, 0 }, 0, "outside" );
}

TEST_F( ColrTest, GradientsOfColrV1 )
{
    const uint32_t ppem = 100;
    // linear_repeat_0_1: red to blue from x 100 to 900, repeated.
    const ColourBitmap linear = Paint( 0xF0100, ppem, "colr1_linear_repeat" );
    ExpectColour( At( linear, ppem, 500.0f, 600.0f ), { 128, 0, 128, 255 }, 4, "linear middle" );
    ExpectColour( At( linear, ppem, 300.0f, 600.0f ), { 191, 0, 64, 255 }, 4, "linear quarter" );

    // radial_contained_gradient_extend_mode_pad: green at ( 166, 768 ), white 128 units out, red from 256 on.
    const ColourBitmap radial = Paint( 0xF0503, ppem, "colr1_radial_pad" );
    ExpectColour( At( radial, ppem, 166.0f, 768.0f ), { 0, 128, 0, 255 }, 12, "radial centre" );
    ExpectColour( At( radial, ppem, 166.0f + 128.0f, 768.0f ), { 255, 255, 255, 255 }, 12, "radial half" );
    ExpectColour( At( radial, ppem, 900.0f, 100.0f ), { 255, 0, 0, 255 }, 2, "radial padded" );

    // sweep_0_360_pad_narrow around ( 500, 600 ): linen up to 90 degrees, blue to red, dark slate gray from 270.
    const ColourBitmap sweep = Paint( 0xF0200, ppem, "colr1_sweep_pad" );
    ExpectColour( At( sweep, ppem, 800.0f, 610.0f ), { 250, 240, 230, 255 }, 2, "sweep 0" );
    ExpectColour( At( sweep, ppem, 500.0f, 900.0f ), { 250, 240, 230, 255 }, 12, "sweep 90" );
    ExpectColour( At( sweep, ppem, 200.0f, 600.0f ), { 128, 0, 128, 255 }, 12, "sweep 180" );
    ExpectColour( At( sweep, ppem, 500.0f, 300.0f ), { 47, 79, 79, 255 }, 12, "sweep 270" );
}

TEST_F( ColrTest, CompositesOfColrV1 )
{
    // A yellow square over a black cross with a blue square composited onto it, the squares overlap around ( 500, 500 ).
    const uint32_t     ppem   = 100;
    const ColourBitmap over   = Paint( 0xF0A03, ppem, "colr1_composite_src_over" );
    const ColourBitmap screen = Paint( 0xF0A0D, ppem, "colr1_composite_screen" );
    const ColourBitmap mult   = Paint( 0xF0A17, ppem, "colr1_composite_multiply" );
    const Rgba         yellow{ 255, 220, 1, 255 }, blue{ 104, 199, 232, 255 };
    for ( const ColourBitmap *bitmap : { &over, &screen, &mult } )
    {
        ExpectColour( At( *bitmap, ppem, 250.0f, 750.0f ), yellow, 2, "backdrop only" );
        ExpectColour( At( *bitmap, ppem, 750.0f, 250.0f ), blue, 2, "source only" );
    }
    ExpectColour( At( over, ppem, 500.0f, 500.0f ), blue, 2, "src over" );
    ExpectColour( At( screen, ppem, 500.0f, 500.0f ), { 255, 247, 232, 255 }, 2, "screen" );
    ExpectColour( At( mult, ppem, 500.0f, 500.0f ), { 104, 172, 1, 255 }, 2, "multiply" );
}

TEST_F( ColrTest, ForegroundPalette )
{
    // foreground_color_solid_alpha_0.3: palette index 0xFFFF at 30 % alpha.
    const uint32_t     ppem   = 64;
    const ColourBitmap bitmap = Paint( 0xF0B07, ppem, "colr1_foreground" );
    EXPECT_TRUE( bitmap.UsesForeground );
    ExpectColour( At( bitmap, ppem, 500.0f, 600.0f ), { 0x20, 0x40, 0x80, 77 }, 3, "foreground" );

    // Only glyphs painting with the foreground get a bitmap per text colour.
    ColourGlyphStore store;
    const uint32_t   fg    = store.Find( m_font.Get( ), Nominal( *m_font.Get( ), 0xF0B07 ) );
    const uint32_t   fixed = store.Find( m_font.Get( ), Nominal( *m_font.Get( ), 0xF0E00 ) );
    ASSERT_NE( fg, ColourGlyphStore::None );
    ASSERT_NE( fixed, ColourGlyphStore::None );
    EXPECT_NE( store.Get( fg, 30.0f, 0xFF0000FF ).Offset, store.Get( fg, 30.0f, 0x00FF00FF ).Offset );
    EXPECT_EQ( store.Get( fg, 30.0f, 0xFF0000FF ).Offset, store.Get( fg, 31.0f, 0xFF000080 ).Offset );
    EXPECT_EQ( store.Get( fixed, 30.0f, 0xFF0000FF ).Offset, store.Get( fixed, 30.0f, 0x00FF00FF ).Offset );
    EXPECT_EQ( store.Get( fixed, 30.0f, 0 ).Ppem, 32u );
    EXPECT_EQ( store.Get( fixed, 300.0f, 0 ).Ppem, 384u );
    EXPECT_EQ( store.Get( fixed, 700.0f, 0 ).Ppem, 512u );
    // The test font's outline glyphs have no colour data and stay on the Slug path.
    EXPECT_EQ( store.Find( m_font.Get( ), Nominal( *m_font.Get( ), 0xFE003 ) ), ColourGlyphStore::None );
}

TEST( ColourGlyph, AppleColorEmojiBitmaps )
{
    const FontFile emoji{ AppleEmojiPath };
    if ( emoji.Get( ) == nullptr )
    {
        GTEST_SKIP( ) << AppleEmojiPath << " is missing";
    }
    const Font &font = *emoji.Get( );
    ASSERT_TRUE( font.IsColour( ) );
    const uint32_t grinning = Nominal( font, 0x1F600 );
    // 👨‍👩‍👧 has no image in the 40, 48 and 52 ppem strikes, the largest strike stands in.
    const uint32_t family = Shaped( font, "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7" );
    ASSERT_NE( grinning, 0u );
    ASSERT_NE( family, grinning );

    for ( const uint32_t ppem : { 16u, 32u, 40u, 128u } )
    {
        for ( const uint32_t glyph : { grinning, family } )
        {
            ColourBitmap bitmap;
            const double ms = Rasterise( font, glyph, ppem, bitmap );
            Save( bitmap, ( glyph == grinning ? "apple_grinning_" : "apple_family_" ) + std::to_string( ppem ) );
            std::printf( "Apple Color Emoji glyph %u at %u ppem: %ux%u, %zu bytes, %.2f ms\n", glyph, ppem, bitmap.Width, bitmap.Height, bitmap.Pixels.size( ) * 4, ms );
            // Inside the em square standing on the baseline, with a pixel of border.
            EXPECT_LE( bitmap.Width, ppem + 2 );
            EXPECT_GE( bitmap.Width, ppem * 3 / 4 );
            EXPECT_GE( bitmap.Top, -static_cast<int32_t>( ppem ) - 1 );
            EXPECT_LE( bitmap.Top + static_cast<int32_t>( bitmap.Height ), 1 );
            uint32_t opaque = 0, coloured = 0;
            for ( const uint32_t p : bitmap.Pixels )
            {
                const uint32_t r = p & 0xFF, g = p >> 8 & 0xFF, b = p >> 16 & 0xFF, a = p >> 24;
                opaque += a > 200;
                coloured += a > 200 && std::max( { r, g, b } ) - std::min( { r, g, b } ) > 60;
            }
            EXPECT_GT( opaque, bitmap.Pixels.size( ) / 4 ) << ppem;
            // The face is yellow, the family a grey and white sign.
            if ( glyph == grinning )
            {
                EXPECT_GT( coloured, bitmap.Pixels.size( ) / 4 ) << ppem;
            }
        }
    }
    // Above its largest strike, 160 ppem, a bitmap glyph is kept at the strike's size and scaled up when drawn.
    ColourGlyphStore store;
    const uint32_t   source = store.Find( &font, grinning );
    EXPECT_EQ( store.Get( source, 100.0f, 0 ).Ppem, 128u );
    EXPECT_EQ( store.Get( source, 150.0f, 0 ).Ppem, 160u );
    EXPECT_EQ( store.Get( source, 400.0f, 0 ).Offset, store.Get( source, 150.0f, 0 ).Offset );

    // The grinning face is yellow.
    ColourBitmap bitmap;
    Rasterise( font, grinning, 64, bitmap );
    const Rgba cheek = ColourAt( bitmap, font, 64, 160.0f, 400.0f );
    EXPECT_GT( cheek.R, 200 );
    EXPECT_GT( cheek.G, 150 );
    EXPECT_LT( cheek.B, 100 );
}

namespace
{
    // JetBrains Mono has no emoji, so emoji come from the system font the OS picks.
    class EmojiFallbackTest : public testing::Test
    {
    protected:
        FontFile   m_mono{ MonoPath };
        FontFile   m_emoji{ AppleEmojiPath };
        TextShaper m_shaper;
        ShapedText m_shaped;

        void SetUp( ) override
        {
#ifndef __APPLE__
            GTEST_SKIP( ) << "only the CoreText lookup is tested";
#endif
            ASSERT_NE( m_mono.Get( ), nullptr );
            if ( m_emoji.Get( ) == nullptr )
            {
                GTEST_SKIP( ) << AppleEmojiPath << " is missing";
            }
        }

        void Shape( const std::string &text )
        {
            Font *const fonts[] = { m_mono.Get( ) };
            m_shaper.Shape( text.data( ), static_cast<uint32_t>( text.size( ) ), fonts, TextAutoLevel, m_shaped );
        }

        // Every glyph of the text is one cluster drawn with one visible glyph of a colour font.
        void ExpectOneColourCluster( const std::string &text )
        {
            Shape( text );
            ASSERT_FALSE( m_shaped.Glyphs.empty( ) ) << text;
            uint32_t visible = 0;
            for ( const TextGlyph &glyph : m_shaped.Glyphs )
            {
                EXPECT_EQ( glyph.Cluster, 0u ) << text;
                EXPECT_TRUE( m_shaped.Fonts[ glyph.Font ]->IsColour( ) ) << text;
                visible += glyph.Advance > 0.0f;
            }
            EXPECT_EQ( visible, 1u ) << text;
        }

        bool IsColourAt( const uint32_t byte ) const
        {
            for ( const TextGlyph &glyph : m_shaped.Glyphs )
            {
                if ( glyph.Cluster == byte )
                {
                    return m_shaped.Fonts[ glyph.Font ]->IsColour( );
                }
            }
            ADD_FAILURE( ) << "no glyph at byte " << byte;
            return false;
        }
    };
} // namespace

TEST_F( EmojiFallbackTest, PresentationPicksTheFont )
{
    Shape( "a\xF0\x9F\x98\x80"
           "b" );
    EXPECT_FALSE( IsColourAt( 0 ) );
    EXPECT_TRUE( IsColourAt( 1 ) );
    EXPECT_FALSE( IsColourAt( 5 ) );

    Shape( "\xE2\x98\xBA" );
    EXPECT_FALSE( IsColourAt( 0 ) ) << "☺ defaults to text";
    Shape( "\xE2\x98\xBA\xEF\xB8\x8F" );
    EXPECT_TRUE( IsColourAt( 0 ) ) << "☺ with VS16";
    Shape( "\xE2\x9D\xA4\xEF\xB8\x8E" );
    EXPECT_FALSE( IsColourAt( 0 ) ) << "❤ with VS15";
    Shape( "\xE2\x9D\xA4\xEF\xB8\x8F" );
    EXPECT_TRUE( IsColourAt( 0 ) ) << "❤ with VS16";
}

TEST_F( EmojiFallbackTest, ChainColourFontsComeFirst )
{
    // An emoji takes the chain's colour font before the system's, a VS15 heart a system text font before the chain's colour font.
    Font *const       emojiFirst[] = { m_mono.Get( ), m_emoji.Get( ) };
    const std::string grinning     = "\xF0\x9F\x98\x80";
    m_shaper.Shape( grinning.data( ), static_cast<uint32_t>( grinning.size( ) ), emojiFirst, TextAutoLevel, m_shaped );
    EXPECT_EQ( m_shaped.Fonts[ m_shaped.Glyphs[ 0 ].Font ], m_emoji.Get( ) );

    Font *const       colourOnly[] = { m_emoji.Get( ) };
    const std::string heart        = "\xE2\x9D\xA4\xEF\xB8\x8E";
    m_shaper.Shape( heart.data( ), static_cast<uint32_t>( heart.size( ) ), colourOnly, TextAutoLevel, m_shaped );
    EXPECT_FALSE( m_shaped.Fonts[ m_shaped.Glyphs[ 0 ].Font ]->IsColour( ) );
    // Without a selector the chain's order decides, like CSS font-family.
    m_shaper.Shape( heart.data( ), 3, colourOnly, TextAutoLevel, m_shaped );
    EXPECT_EQ( m_shaped.Fonts[ m_shaped.Glyphs[ 0 ].Font ], m_emoji.Get( ) );
}

TEST_F( EmojiFallbackTest, SequencesAreOneCluster )
{
    ExpectOneColourCluster( "\xF0\x9F\x98\x80" );                                                                                                 // 😀
    ExpectOneColourCluster( "\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD" );                                                                                 // 👍🏽
    ExpectOneColourCluster( "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7" );                                         // 👨‍👩‍👧
    ExpectOneColourCluster( "\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1" );                                                                                 // 🇳🇱
    ExpectOneColourCluster( "1\xEF\xB8\x8F\xE2\x83\xA3" );                                                                                        // 1️⃣
    ExpectOneColourCluster( "\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xA5\xF3\xA0\x81\xAE\xF3\xA0\x81\xA7\xF3\xA0\x81\xBF" ); // 🏴 England
}

TEST_F( EmojiFallbackTest, UITextDrawsEmojiAsColourQuads )
{
    UIText text;
    text.AddFont( 1, m_mono.Get( ) );
    const std::string  chars = "a\xF0\x9F\x98\x80";
    const UIShapedRun *run   = text.Shape( 1, chars.data( ), static_cast<uint32_t>( chars.size( ) ), UIText::Hash( chars.data( ), chars.size( ) ) );
    UITextParams       params;
    params.FontId     = 1;
    params.FontSize   = 24.0f;
    const auto &quads = text.Layout( run, params, -1.0f )->GetQuads( text.Glyphs( ), text.ColourGlyphs( ) );
    ASSERT_EQ( quads.size( ), 2u );
    EXPECT_FALSE( quads[ 0 ].Colour );
    ASSERT_TRUE( quads[ 1 ].Colour );
    EXPECT_GT( quads[ 1 ].X0, quads[ 0 ].X0 );

    ColourGlyphStore  &store  = text.ColourGlyphs( );
    const ColourGlyph &bitmap = store.Get( quads[ 1 ].Glyph, 24.0f, 0x000000FF );
    EXPECT_EQ( bitmap.Ppem, 24u );
    EXPECT_GT( bitmap.Width, 20u );
    EXPECT_TRUE( store.HasPendingUpload( ) );
    EXPECT_EQ( store.GetPixels( )[ bitmap.Offset ], bitmap.Width | bitmap.Height << 16 );
}
