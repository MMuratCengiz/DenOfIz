/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <harfbuzz/hb.h>
#include <unordered_map>
#include <vector>

namespace DenOfIz
{
    class Font;

    // Premultiplied RGBA8 with red in the low byte, rows top to bottom. Left and Top place the top left corner relative to the pen in pixels, y down.
    struct ColourBitmap
    {
        int32_t               Left           = 0;
        int32_t               Top            = 0;
        uint32_t              Width          = 0;
        uint32_t              Height         = 0;
        bool                  UsesForeground = false;
        std::vector<uint32_t> Pixels;
    };

    // A COLR, sbix or CBDT glyph painted at ppem pixels per em, false when the glyph has no colour data. Palette index 0xFFFF paints foreground.
    bool RasteriseColourGlyph( const Font &font, uint32_t glyphId, uint32_t ppem, hb_color_t foreground, ColourBitmap &out );

    // Offset is the width | height << 16 header in the pixel stream, the texels follow row by row. Ppem is the size the bitmap was painted at.
    struct ColourGlyph
    {
        int32_t  Left     = 0;
        int32_t  Top      = 0;
        uint32_t Width    = 0;
        uint32_t Height   = 0;
        uint32_t Offset   = 0;
        uint32_t Ppem     = 0;
        uint64_t LastUsed = 0;
    };

    // Colour glyphs are painted on first use at the smallest size bucket covering the requested size, bitmap glyphs at most at their largest strike.
    // The pixel stream grows until it passes a budget, then BeginFrame drops the bitmaps no frame drew for a while and packs the rest.
    class ColourGlyphStore
    {
        struct Source
        {
            const Font *FontPtr;
            uint32_t    GlyphId;
            uint32_t    LargestStrike;
            bool        UsesForeground;
        };

        std::vector<Source>                       m_sources;
        std::unordered_map<uint64_t, uint32_t>    m_sourceLookup;
        std::unordered_map<uint64_t, ColourGlyph> m_glyphs;
        std::vector<uint32_t>                     m_pixels;
        size_t                                    m_uploaded = 0;
        uint64_t                                  m_frame      = 0;
        size_t                                    m_packAt     = 0;
        uint32_t                                  m_generation = 0;

    public:
        static constexpr uint32_t None = UINT32_MAX;

        // The source of a glyph's colour data, None when it has none and is drawn from its outline.
        uint32_t Find( const Font *font, uint32_t glyphId );
        // foreground is RGBA8 with red in the high byte, it only keys glyphs that paint with it.
        const ColourGlyph &Get( uint32_t source, float pixelsPerEm, uint32_t foreground );

        // Before the frame's Get calls. Packing moves bitmaps and changes Generation, offsets from earlier frames are stale then.
        void     BeginFrame( );
        uint32_t Generation( ) const;

        const std::vector<uint32_t> &GetPixels( ) const;
        size_t                       GetUploadMark( ) const;
        bool                         HasPendingUpload( ) const;
        void                         MarkUploaded( );
    };
} // namespace DenOfIz
