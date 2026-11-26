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

    struct ColourBitmap
    {
        int32_t               Left           = 0;
        int32_t               Top            = 0;
        uint32_t              Width          = 0;
        uint32_t              Height         = 0;
        bool                  UsesForeground = false;
        std::vector<uint32_t> Pixels;
    };

    bool RasteriseColourGlyph( const Font &font, uint32_t glyphId, uint32_t ppem, hb_color_t foreground, ColourBitmap &out );

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

        uint32_t           Find( const Font *font, uint32_t glyphId );
        const ColourGlyph &Get( uint32_t source, float pixelsPerEm, uint32_t foreground );

        void     BeginFrame( );
        uint32_t Generation( ) const;

        const std::vector<uint32_t> &GetPixels( ) const;
        size_t                       GetUploadMark( ) const;
        bool                         HasPendingUpload( ) const;
        void                         MarkUploaded( );
    };
} // namespace DenOfIz
