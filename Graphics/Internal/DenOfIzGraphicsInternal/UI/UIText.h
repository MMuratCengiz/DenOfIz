/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <array>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/ColourGlyphStore.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SlugGlyphStore.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"
#include "DenOfIzGraphicsInternal/UI/UITypes.h"

namespace DenOfIz
{
    class Font;

    constexpr uint32_t UIMaxFonts = 64;

    // Metrics are in em, Generation is the font's id.
    struct UIFontFace
    {
        Font      *FontPtr      = nullptr;
        uint32_t   Generation   = 0;
        float      Ascent       = 0.0f;
        float      Descent      = 0.0f;
        float      LineHeight   = 0.0f;
        float      SpaceAdvance = 0.0f;
        ShapedText Ellipsis;
        float      EllipsisAdvance = 0.0f;
    };

    // A resolved DenOfIz_UITextRun: FontId is registered and Size is in points.
    struct UIRunStyle
    {
        uint32_t                      ByteStart = 0;
        uint32_t                      ByteEnd   = 0;
        uint32_t                      Id        = 0;
        uint16_t                      FontId    = 0;
        float                         Size      = 0.0f;
        float                         Shift     = 0.0f;
        DenOfIz_UIColor               Color{ };
        DenOfIz_UIColor               Background{ };
        DenOfIz_UIColor               DecorationColor{ };
        DenOfIz_UITextDecorationFlags Decoration = 0;
        bool                          Object     = false;
        DenOfIz_Float2                ObjectSize{ };
        DenOfIz_Texture               Image = DENOFIZ_NULL_HANDLE;

        bool operator==( const UIRunStyle &other ) const;
    };

    // Styles is empty for plain text, whose glyphs take the layout's size. LongestWord is in em for plain text and in points for rich text.
    struct UIShapedRun
    {
        uint16_t                FontId     = 0;
        uint32_t                Generation = 0;
        uint64_t                Hash       = 0;
        std::string             Text;
        ShapedText              Shaped;
        std::vector<UIRunStyle> Styles;
        float                   LongestWord       = 0.0f;
        uint32_t                LongestWordGlyphs = 0;
        mutable uint64_t        LastUsedFrame     = 0;

        // Index of the style holding a byte, the last one past the end.
        uint32_t StyleAt( uint32_t byte ) const;
    };

    struct UITextParams
    {
        uint16_t               FontId        = 0;
        float                  FontSize      = 14.0f;
        float                  LetterSpacing = 0.0f;
        float                  LineHeight    = 0.0f;
        DenOfIz_UITextWrap     Wrap          = DENOFIZ_UI_TEXT_WRAP_WORDS;
        DenOfIz_UITextOverflow Overflow      = DENOFIZ_UI_TEXT_OVERFLOW_CLIP;
        uint32_t               MaxLines      = 0;

        bool operator==( const UITextParams &other ) const;
        static UITextParams From( const DenOfIz_UITextDesc &desc );
    };

    // A line is a logical range of glyphs. Rtl is the paragraph direction, which decides where trailing spaces hang and the ellipsis goes. Top and
    // Baseline are measured from the top of the text.
    struct UITextLineLayout
    {
        uint32_t GlyphStart;
        uint32_t GlyphEnd;
        uint32_t ByteStart;
        uint32_t ByteEnd;
        float    Width;
        bool     Ellipsis;
        bool     Rtl;
        float    Top      = 0.0f;
        float    Height   = 0.0f;
        float    Baseline = 0.0f;
    };

    // The glyphs [First, Last] of one cluster on a line and the x range they cover.
    struct UITextCluster
    {
        uint32_t First;
        uint32_t Last;
        uint32_t ByteStart;
        uint32_t ByteEnd;
        float    Left;
        float    Right;
        bool     Rtl;
    };

    // U/V are em-space glyph coordinates (y up), Glyph is the SlugGlyphStore record. A colour glyph has its pen position in X0/Y0 and its
    // ColourGlyphStore source in Glyph, the bitmap is picked for the size it is drawn at.
    // Em is the size the glyph is drawn at, Style its run style or UINT32_MAX when it takes the text's colour.
    struct UIGlyphQuad
    {
        float    X0, Y0, X1, Y1;
        float    U0, V0, U1, V1;
        uint32_t Glyph;
        uint32_t Line;
        bool     Colour;
        float    Em;
        uint32_t Style;
    };

    class UITextLayout
    {
    public:
        const UIShapedRun            *Run  = nullptr;
        const UIFontFace             *Face = nullptr;
        UITextParams                  Params;
        float                         MaxWidth = 0.0f;
        float                         Scale    = 1.0f;
        float                         Width    = 0.0f;
        float                         Height   = 0.0f;
        float                         Baseline = 0.0f; // Of the first line.
        std::vector<UITextLineLayout> Lines;
        // The run's glyphs with the ones around cuts HarfBuzz marks unsafe shaped again, empty when no line needed it.
        std::vector<TextGlyph> Reshaped;
        // Per glyph: the visual x of its origin and its bidi level after UAX #9 L1.
        std::vector<float>               GlyphX;
        std::vector<uint8_t>             Levels;
        mutable std::vector<UIGlyphQuad> Quads;
        mutable bool                     QuadsBuilt    = false;
        mutable uint64_t                 LastUsedFrame = 0;

        const std::vector<TextGlyph> &Glyphs( ) const;
        float                         GlyphScale( const TextGlyph &glyph ) const;
        uint32_t                      LineAt( float y ) const;
        float                         AlignOffset( uint32_t line, float boxWidth, DenOfIz_UITextAlign align ) const;
        float                         CaretX( uint32_t line, uint32_t byteOffset ) const;
        uint32_t                      LineOfOffset( uint32_t byteOffset ) const;
        uint32_t                      OffsetAt( float x, float y, float boxWidth, DenOfIz_UITextAlign align ) const;
        // The x ranges of the selected bytes [start, end) on a line, left to right. A selection past the end of the line adds newlineWidth at its end.
        void                            SelectionSpans( uint32_t line, uint32_t start, uint32_t end, float newlineWidth, std::vector<DenOfIz_Float2> &out ) const;
        const std::vector<UIGlyphQuad> &GetQuads( SlugGlyphStore &slugGlyphs, ColourGlyphStore &colourGlyphs ) const;
        // The run style drawn under a point, UINT32_MAX for none.
        uint32_t StyleAtPoint( float x, float y, float boxWidth, DenOfIz_UITextAlign align ) const;

    private:
        float         GlyphWidth( uint32_t glyph ) const;
        UITextCluster ClusterAt( const UITextLineLayout &line, uint32_t glyph ) const;
        float         ClusterCaret( const UITextCluster &cluster, uint32_t byteOffset ) const;
    };

    struct UITextLayoutKey
    {
        const UIShapedRun *Run;
        UITextParams       Params;
        float              MaxWidth;

        bool operator==( const UITextLayoutKey &other ) const;
    };

    struct UITextLayoutKeyHash
    {
        size_t operator( )( const UITextLayoutKey &key ) const;
    };

    struct UIRunKey
    {
        uint64_t Hash;
        uint32_t Length;
        uint16_t FontId;
        uint32_t Generation;

        bool operator==( const UIRunKey &other ) const;
    };

    struct UIRunKeyHash
    {
        size_t operator( )( const UIRunKey &key ) const;
    };

    class UIText
    {
        std::array<UIFontFace, UIMaxFonts>                                                      m_faces{ };
        std::array<std::vector<uint16_t>, UIMaxFonts>                                           m_fallbacks{ };
        std::vector<Font *>                                                                     m_chain;
        SlugGlyphStore                                                                          m_glyphs;
        ColourGlyphStore                                                                        m_colourGlyphs;
        TextShaper                                                                              m_shaper;
        std::unordered_map<UIRunKey, std::unique_ptr<UIShapedRun>, UIRunKeyHash>                m_runs;
        std::unordered_map<UITextLayoutKey, std::unique_ptr<UITextLayout>, UITextLayoutKeyHash> m_layouts;
        std::vector<uint32_t>                                                                   m_order;
        std::vector<TextGlyph>                                                                  m_pieces;
        std::vector<TextSegment>                                                                m_segments;
        std::vector<uint8_t>                                                                    m_segmentChains;
        uint64_t                                                                                m_frame = 1;
        uint64_t                                                                                m_epoch = 1;

    public:
        UIText( );
        ~UIText( );

        void              AddFont( uint16_t fontId, Font *font );
        void              RemoveFont( uint16_t fontId );
        void              SetFallbacks( uint16_t fontId, std::span<const uint16_t> fallbacks );
        const UIFontFace *GetFace( uint16_t fontId ) const;
        SlugGlyphStore   &Glyphs( );
        ColourGlyphStore &ColourGlyphs( );

        static uint64_t     Hash( const char *chars, size_t length );
        const UIShapedRun  *Shape( uint16_t fontId, const char *chars, uint32_t length, uint64_t hash );
        // fontId is the strut font, the styles tile the text. hash covers the text and the styles, see HashStyles.
        const UIShapedRun  *ShapeRich( uint16_t fontId, const char *chars, uint32_t length, std::span<const UIRunStyle> styles, uint64_t hash );
        static uint64_t     HashStyles( uint64_t textHash, std::span<const UIRunStyle> styles );
        const UITextLayout *Layout( const UIShapedRun *run, const UITextParams &params, float maxWidth );
        float               MinContentWidth( const UIShapedRun *run, const UITextParams &params ) const;
        DenOfIz_Float2      Measure( const char *chars, uint32_t length, const DenOfIz_UITextDesc &desc, float maxWidth );

        void     BeginFrame( uint64_t frame );
        void     Evict( uint64_t maxAge );
        uint64_t Epoch( ) const;
        uint64_t Frame( ) const;
        uint16_t ResolveFontId( uint16_t fontId ) const;

    private:
        void LoadFace( UIFontFace &face, Font *font );
        void MeasureWords( UIShapedRun &run ) const;
        bool Uses( const UIShapedRun &run, uint16_t fontId ) const;
        void ReleaseFace( UIFontFace &face );
        // Drops the runs and layouts drawn with the font, as primary or fallback.
        void  Invalidate( uint16_t fontId );
        void  BuildLayout( UITextLayout &layout );
        bool  ReshapeCuts( UITextLayout &layout );
        float ReshapedEnd( const UITextLayout &layout, uint32_t lineStart, uint32_t cut );
    };
} // namespace DenOfIz
