/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "gtest/gtest.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <harfbuzz/hb-ot.h>
#include <iterator>
#include <string>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/UI/UIText.h"

using namespace DenOfIz;

namespace
{
    struct Sample
    {
        const char *Name;
        std::string Text;
    };

    // Strings browsers draw without tofu on macOS. Invisible code points are escaped.
    const Sample Corpus[] = {
        { "emoji zwj family", "👨‍👩‍👧‍👦 👩🏽‍💻 🏳️‍🌈" },
        { "emoji modifiers", "👍🏽👋🏿🤝🏻" },
        { "emoji flags", "🇯🇵🇺🇳🇪🇺" },
        { "emoji keycaps", "1️⃣#️⃣*️⃣" },
        { "emoji tag flags", "🏴󠁧󠁢󠁳󠁣󠁴󠁿🏴󠁧󠁢󠁷󠁬󠁳󠁿" },
        { "emoji presentation", "☺︎☺️❤️❤︎⚠️" },
        { "combining stack", "Z̴̡̛̩͓̈́a̸̧͚͗l̵̰̈g̶̱̓o̷͎͒" },
        { "devanagari zwj zwnj", "क्‍ष क्‌ष क्ष हिन्दी" },
        { "malayalam chillu", "ന്‍ കേരളം" },
        { "bengali ya-phala", "র‍্যাব বাংলা" },
        { "arabic lam-alef tatweel", "لا لآ عـــربي" },
        { "arabic zwj zwnj", "ع‍ ‍ع می‌خواهم" },
        { "hebrew points", "בְּרֵאשִׁית שָׁלוֹם" },
        { "thai", "ภาษาไทยน้ำ" },
        { "lao", "ພາສາລາວ" },
        { "khmer", "ភាសាខ្មែរ" },
        { "myanmar", "မြန်မာဘာသာ" },
        { "cjk", "漢字かなカナ한국어" },
        { "cjk extension b", "𠀋𪚲𠮷" },
        { "hangul jamo", "각 ᄒᆞᆫ" },
        { "math alphanumerics", "𝔸𝕓𝒸𝓭𝖊𝟘" },
        { "symbols arrows box", "→⇒↔ ┌─┐│└┘ ∑∫√∞≠" },
        { "braille", "⠓⠑⠇⠇⠕" },
        { "ethiopic", "ሰላም ኢትዮጵያ" },
        { "cherokee", "ᏣᎳᎩ" },
        { "canadian syllabics", "ᐃᓄᒃᑎᑐᑦ" },
        { "tifinagh", "ⵜⵉⴼⵉⵏⴰⵖ" },
        { "adlam", "𞤀𞤣𞤤𞤢𞤥" },
        { "nko", "ߒߞߏ" },
        { "syriac", "ܠܫܢܐ ܣܘܪܝܝܐ" },
        { "mongolian", "ᠮᠣᠩᠭᠣᠯ" },
        { "tibetan", "བོད་སྐད" },
        { "georgian", "ქართული" },
        { "armenian", "Հայերեն" },
        { "runic ogham", "ᚠᚢᚦᚨᚱᚲ ᚛ᚑᚌᚐᚋ᚜" },
        { "gothic", "𐌲𐌿𐍄𐌹𐍃𐌺" },
        { "egyptian hieroglyphs", "𓀀𓁐𓂀" },
        { "cuneiform", "𒀀𒀭" },
        { "javanese", "ꦧꦱꦗꦮ" },
        { "ol chiki", "ᱚᱞ ᱪᱤᱠᱤ" },
        { "vai", "ꕙꔤ" },
        { "yi", "ꆈꌠꁱꂷ" },
        { "bopomofo", "ㄅㄆㄇ" },
        { "recent emoji", "🫩🫆🪾" },
        { "unassigned and private use", "\u0378\uE000" },
        { "bidi controls", "a‮bc‬d ⁧abc⁩ ‎‏؜ x‪y‬z ⁦q⁩ ‫שלום‬" },
        { "variation selectors", "葛\U000E0100 a︀ ⚠︎" },
        { "default ignorables", "a­b​c⁠d﻿e͏f" },
        { "invalid utf-8", "a\xC0\xAF"
                           "b\xED\xA0\x80"
                           "c\xE2\x82"
                           "d\x80\xBF"
                           "e\xF4\x90\x80\x80"
                           "f\xF0\x9F\x98" },
    };

    // Outlines or colour glyphs the glyph stores can draw.
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

    // An installed font that covers the cluster and can be drawn, empty when there is none.
    std::string CoveringFont( const std::string &cluster )
    {
        std::vector<SystemFontFile> files;
        FindSystemFonts( cluster.data( ), static_cast<uint32_t>( cluster.size( ) ), files );
        for ( const SystemFontFile &file : files )
        {
            hb_blob_t *blob = hb_blob_create_from_file_or_fail( file.Path.c_str( ) );
            if ( blob == nullptr )
            {
                continue;
            }
            uint32_t           face  = file.FaceIndex;
            const unsigned int faces = hb_face_count( blob );
            for ( unsigned int i = 0; i < faces && !file.PostScriptName.empty( ); ++i )
            {
                hb_face_t   *candidate = hb_face_create( blob, i );
                char         name[ 256 ];
                unsigned int size = sizeof( name );
                hb_ot_name_get_utf8( candidate, HB_OT_NAME_ID_POSTSCRIPT_NAME, HB_LANGUAGE_INVALID, &size, name );
                hb_face_destroy( candidate );
                face = file.PostScriptName == name ? i : face;
            }
            const Font font( blob, face );
            if ( font.IsValid( ) && Drawable( font ) && font.Covers( cluster.data( ), static_cast<uint32_t>( cluster.size( ) ) ) )
            {
                return file.PostScriptName;
            }
        }
        return { };
    }

    std::string CodePoints( const std::string &text )
    {
        std::string result;
        for ( size_t i = 0; i < text.size( ); )
        {
            const auto lead  = static_cast<uint8_t>( text[ i ] );
            const int  extra = lead >= 0xF0 ? 3 : lead >= 0xE0 ? 2 : lead >= 0xC0 ? 1 : 0;
            uint32_t   value = extra == 0 ? lead : lead & ( 0x3F >> extra );
            for ( int k = 1; k <= extra && i + k < text.size( ); ++k )
            {
                value = value << 6 | ( static_cast<uint8_t>( text[ i + k ] ) & 0x3F );
            }
            char buffer[ 16 ];
            std::snprintf( buffer, sizeof( buffer ), "%sU+%04X", result.empty( ) ? "" : " ", value );
            result += buffer;
            i += extra + 1;
        }
        return result;
    }
} // namespace

// Every sample shapes to ascending clusters with sane advances, lays out at a narrow width and draws. A .notdef glyph is only accepted where no
// installed font covers the cluster, so a browser shows tofu too.
TEST( TextCorpus, DrawsLikeABrowser )
{
#ifndef __APPLE__
    GTEST_SKIP( ) << "coverage is checked against the macOS fonts";
#endif
    const char  *interPath = DZ_TEST_DATA_SRC_DIR "/Fonts/Inter-Regular.ttf";
    DenOfIz_Font inter     = DenOfIz_Font_CreateFromPath( DenOfIz_StringView{ interPath, std::strlen( interPath ) }, 0 );
    ASSERT_TRUE( DENOFIZ_HANDLE_IS_VALID( inter ) );
    {
        Font *const fonts[] = { FONT_IMPL( inter ) };
        TextShaper  shaper;
        ShapedText  shaped;
        UIText      text;
        text.AddFont( 1, FONT_IMPL( inter ) );

        uint32_t tofu = 0;
        for ( const Sample &sample : Corpus )
        {
            const std::string &s      = sample.Text;
            const auto         length = static_cast<uint32_t>( s.size( ) );
            shaper.Shape( s.data( ), length, fonts, TextAutoLevel, shaped );
            ASSERT_FALSE( shaped.Glyphs.empty( ) ) << sample.Name;
            uint32_t previous = 0;
            uint32_t reported = UINT32_MAX;
            for ( size_t i = 0; i < shaped.Glyphs.size( ); ++i )
            {
                const TextGlyph &glyph = shaped.Glyphs[ i ];
                EXPECT_GE( glyph.Cluster, previous ) << sample.Name;
                EXPECT_LT( glyph.Cluster, length ) << sample.Name;
                EXPECT_TRUE( std::isfinite( glyph.Advance ) && glyph.Advance > -0.5f && glyph.Advance < 3.0f ) << sample.Name << " advance " << glyph.Advance;
                previous = glyph.Cluster;
                if ( glyph.GlyphId != 0 || glyph.Cluster == reported )
                {
                    continue;
                }
                reported     = glyph.Cluster;
                uint32_t end = static_cast<uint32_t>( i + 1 );
                while ( end < shaped.Glyphs.size( ) && shaped.Glyphs[ end ].Cluster == glyph.Cluster )
                {
                    ++end;
                }
                const std::string cluster  = s.substr( glyph.Cluster, ( end < shaped.Glyphs.size( ) ? shaped.Glyphs[ end ].Cluster : length ) - glyph.Cluster );
                const std::string covering = CoveringFont( cluster );
                SystemFontFile    picked;
                const bool        found = FindSystemFont( cluster.data( ), static_cast<uint32_t>( cluster.size( ) ), picked );
                std::printf( "[ corpus ] %s: .notdef for %s in %s, CoreText picks %s\n", sample.Name, CodePoints( cluster ).c_str( ),
                             shaped.Fonts[ glyph.Font ] == fonts[ 0 ] ? "Inter" : "a system font", found ? picked.PostScriptName.c_str( ) : "LastResort" );
                EXPECT_TRUE( covering.empty( ) ) << sample.Name << ": " << CodePoints( cluster ) << " is covered by " << covering;
                ++tofu;
            }

            const UIShapedRun *run = text.Shape( 1, s.data( ), length, UIText::Hash( s.data( ), s.size( ) ) );
            UITextParams       params;
            params.FontId   = 1;
            params.FontSize = 24.0f;
            for ( const float width : { -1.0f, 40.0f } )
            {
                const UITextLayout *layout = text.Layout( run, params, width );
                ASSERT_FALSE( layout->Lines.empty( ) ) << sample.Name;
                float top = 0.0f;
                for ( const UITextLineLayout &line : layout->Lines )
                {
                    EXPECT_FLOAT_EQ( line.Top, top ) << sample.Name;
                    EXPECT_GE( line.Height, FONT_IMPL( inter )->Metrics( ).LineHeight * params.FontSize - 1e-3f ) << sample.Name;
                    top += line.Height;
                }
                const auto &quads = layout->GetQuads( text.Glyphs( ), text.ColourGlyphs( ) );
                EXPECT_FALSE( quads.empty( ) ) << sample.Name;
                EXPECT_LE( quads.size( ), layout->Glyphs( ).size( ) ) << sample.Name;
            }
        }
        std::printf( "[ corpus ] %zu samples, %u .notdef clusters\n", std::size( Corpus ), tofu );
    }
    DenOfIz_Font_Destroy( inter );
}
