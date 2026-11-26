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
    constexpr uint8_t  TextAutoLevel   = 0xFE;

    namespace TextGlyphFlags
    {
        constexpr uint8_t BreakAfter    = 1 << 0;
        constexpr uint8_t MustBreak     = 1 << 1;
        constexpr uint8_t Space         = 1 << 2;
        constexpr uint8_t Newline       = 1 << 3;
        constexpr uint8_t BidiTrailing  = 1 << 4;
        constexpr uint8_t BidiSeparator = 1 << 5;
        constexpr uint8_t UnsafeToBreak = 1 << 6;
    } // namespace TextGlyphFlags

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

    struct TextParagraph
    {
        uint32_t ByteStart;
        uint32_t ByteEnd;
        uint32_t GlyphStart;
        uint32_t GlyphEnd;
        uint8_t  Level;
    };

    struct TextItem
    {
        uint32_t    GlyphStart;
        uint32_t    ByteEnd;
        uint32_t    Paragraph;
        hb_script_t Script;
    };

    struct ShapedText
    {
        std::vector<Font *>        Fonts;
        std::vector<TextGlyph>     Glyphs;
        std::vector<TextParagraph> Paragraphs;
        std::vector<TextItem>      Items;
        std::vector<uint8_t>       GraphemeStarts;
    };

    struct TextSegment
    {
        uint32_t ByteEnd;
        uint32_t ChainStart;
        uint32_t ChainCount;
        bool     Object;
        float    ObjectAdvance;
    };

    class TextShaper
    {
        hb_buffer_t             *m_buffer;
        std::vector<char>        m_breaks;
        std::string              m_valid;
        std::vector<uint32_t>    m_origins;
        std::vector<TextSegment> m_segments;
        std::vector<uint8_t>     m_chain;

    public:
        TextShaper( );
        ~TextShaper( );
        TextShaper( const TextShaper & )            = delete;
        TextShaper &operator=( const TextShaper & ) = delete;

        void Shape( const char *text, uint32_t length, std::span<Font *const> fonts, uint8_t baseLevel, ShapedText &out );
        void Shape( const char *text, uint32_t length, std::span<Font *const> fonts, std::span<const TextSegment> segments, std::span<const uint8_t> chains, uint8_t baseLevel,
                    ShapedText &out );
        void Reshape( const char *text, uint32_t length, const ShapedText &shaped, uint32_t start, uint32_t end, std::span<const uint32_t> cuts, std::vector<TextGlyph> &out );

    private:
        void ShapeWellFormed( const char *text, uint32_t length, std::span<Font *const> fonts, std::span<const TextSegment> segments, std::span<const uint8_t> chains,
                              uint8_t baseLevel, ShapedText &out );
        void ShapeItem( const char *paragraph, uint32_t paragraphLength, uint32_t paragraphStart, uint32_t start, uint32_t end, uint8_t level, hb_script_t script, const Font *font,
                        uint8_t fontIndex, std::vector<TextGlyph> &out );
    };

    void TextVisualOrder( const TextGlyph *glyphs, uint32_t count, uint8_t paragraphLevel, uint8_t *levels, uint32_t *order );

    uint32_t TextNextGrapheme( const char *text, uint32_t length, uint32_t offset );
    uint32_t TextPreviousGrapheme( const char *text, uint32_t length, uint32_t offset );

    bool TextIsDefaultIgnorable( uint32_t codePoint );
    bool TextReplaceIllFormed( const char *text, uint32_t length, std::string &out, std::vector<uint32_t> &origins );
} // namespace DenOfIz
