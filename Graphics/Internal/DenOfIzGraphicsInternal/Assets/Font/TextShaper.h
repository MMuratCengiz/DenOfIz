/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <harfbuzz/hb.h>
#include <span>
#include <string>
#include <vector>

namespace DenOfIz
{
    class Font;

    constexpr uint32_t TextHiddenGlyph = UINT32_MAX;
    // First strong character decides, LTR when there is none (UAX #9 P2, P3).
    constexpr uint8_t TextAutoLevel = 0xFE;

    namespace TextGlyphFlags
    {
        constexpr uint8_t BreakAfter = 1 << 0; // Line break opportunity after the cluster, on its last glyph.
        constexpr uint8_t MustBreak  = 1 << 1; // Mandatory break inside a paragraph (LS, VT, FF).
        constexpr uint8_t Space      = 1 << 2; // Hangs at the end of a line.
        constexpr uint8_t Newline    = 1 << 3; // First glyph of a paragraph separator.
        // UAX #9 L1: whitespace and isolates take the paragraph level at the end of a line and before a separator, separators always.
        constexpr uint8_t BidiTrailing  = 1 << 4;
        constexpr uint8_t BidiSeparator = 1 << 5;
        // HarfBuzz: cutting the text before this cluster changes the shaping on both sides of the cut.
        constexpr uint8_t UnsafeToBreak = 1 << 6;
    } // namespace TextGlyphFlags

    // Logical order, clusters ascend whatever the direction. Advance and offsets are in em, Font indexes ShapedText::Fonts.
    struct TextGlyph
    {
        uint32_t GlyphId;
        uint32_t Cluster;
        float    Advance;
        float    OffsetX;
        float    OffsetY;
        uint8_t  Font;
        uint8_t  Level;
        uint8_t  Flags;
    };

    // Ranges exclude the separator, whose glyphs sit between GlyphEnd and the next paragraph's GlyphStart.
    struct TextParagraph
    {
        uint32_t ByteStart;
        uint32_t ByteEnd;
        uint32_t GlyphStart;
        uint32_t GlyphEnd;
        uint8_t  Level;
    };

    // The glyphs shaped in one buffer, from GlyphStart to the next item's: one font, bidi level and script. ByteEnd ends its text.
    struct TextItem
    {
        uint32_t    GlyphStart;
        uint32_t    ByteEnd;
        uint32_t    Paragraph;
        hb_script_t Script;
    };

    // Byte offsets refer to the text given to Shape, ill-formed UTF-8 is shaped as U+FFFD per maximal subpart.
    struct ShapedText
    {
        std::vector<Font *>        Fonts; // The fonts given to Shape, then the system fonts it fell back to.
        std::vector<TextGlyph>     Glyphs;
        std::vector<TextParagraph> Paragraphs;
        std::vector<TextItem>      Items;
        std::vector<uint8_t>       GraphemeStarts; // One per byte plus one for the end, non zero where a grapheme cluster starts.
    };

    class TextShaper
    {
        hb_buffer_t          *m_buffer;
        std::vector<char>     m_breaks;
        std::string           m_valid;
        std::vector<uint32_t> m_origins;

    public:
        TextShaper( );
        ~TextShaper( );
        TextShaper( const TextShaper & )            = delete;
        TextShaper &operator=( const TextShaper & ) = delete;

        // Every grapheme cluster takes the first font that covers it, else the system font covering it. baseLevel is 0, 1 or TextAutoLevel.
        void Shape( const char *text, uint32_t length, std::span<Font *const> fonts, uint8_t baseLevel, ShapedText &out );
        // Shapes the glyphs [start, end) of one item of shaped again as separate pieces cut before the glyphs in cuts, with the paragraph around
        // them as context, and appends them to out. Each new glyph takes the flags of the glyph it replaces.
        void Reshape( const char *text, uint32_t length, const ShapedText &shaped, uint32_t start, uint32_t end, std::span<const uint32_t> cuts, std::vector<TextGlyph> &out );

    private:
        void ShapeWellFormed( const char *text, uint32_t length, std::span<Font *const> fonts, uint8_t baseLevel, ShapedText &out );
        void ShapeItem( const char *paragraph, uint32_t paragraphLength, uint32_t paragraphStart, uint32_t start, uint32_t end, uint8_t level, hb_script_t script, const Font *font,
                        uint8_t fontIndex, std::vector<TextGlyph> &out );
    };

    // UAX #9 L1 and L2 for one line of one paragraph: levels receives the resolved level of each glyph and order the glyph indices left to right.
    void TextVisualOrder( const TextGlyph *glyphs, uint32_t count, uint8_t paragraphLevel, uint8_t *levels, uint32_t *order );

    uint32_t TextNextGrapheme( const char *text, uint32_t length, uint32_t offset );
    uint32_t TextPreviousGrapheme( const char *text, uint32_t length, uint32_t offset );

    // Default_Ignorable_Code_Point as HarfBuzz hides it: never drawn and without advance, though joiners and selectors still shape.
    bool TextIsDefaultIgnorable( uint32_t codePoint );
    // Copies text with every maximal subpart of an ill-formed sequence replaced by U+FFFD (Unicode 3.9, WHATWG), origins maps each byte written
    // and the end to the byte of text it came from. False, writing nothing, when text is well-formed.
    bool TextReplaceIllFormed( const char *text, uint32_t length, std::string &out, std::vector<uint32_t> &origins );
} // namespace DenOfIz
