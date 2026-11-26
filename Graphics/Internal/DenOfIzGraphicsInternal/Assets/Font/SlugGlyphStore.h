/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstddef>
#include <harfbuzz/hb.h>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct SlugGlyph
    {
        DenOfIz_Float4 Bounds;
        DenOfIz_Float4 BandTransform;
        DenOfIz_UInt4  Data;
    };
    static_assert( sizeof( SlugGlyph ) == 48 && offsetof( SlugGlyph, BandTransform ) == 16 && offsetof( SlugGlyph, Data ) == 32 );
    static_assert( sizeof( DenOfIz_Float4 ) == 16 && sizeof( DenOfIz_UInt2 ) == 8 );

    struct SlugUploadMark
    {
        size_t Curves = 0;
        size_t Bands  = 0;
        size_t Glyphs = 0;
    };

    struct SlugTransform
    {
        float Xx      = 1.0f;
        float Yx      = 0.0f;
        float Xy      = 0.0f;
        float Yy      = 1.0f;
        float Dx      = 0.0f;
        float Dy      = 0.0f;
        float Divisor = 1.0f;
    };

    SlugGlyph CompileSlugGlyph( hb_font_t *font, uint32_t glyphId, const SlugTransform &transform, std::vector<DenOfIz_Float4> &curves, std::vector<DenOfIz_UInt2> &bands );

    class SlugGlyphStore
    {
        std::vector<DenOfIz_Float4>            m_curves;
        std::vector<DenOfIz_UInt2>             m_bands;
        std::vector<SlugGlyph>                 m_glyphs;
        std::unordered_map<uint64_t, uint32_t> m_lookup;
        SlugUploadMark                         m_uploaded;

    public:
        SlugGlyphStore( )                                   = default;
        SlugGlyphStore( const SlugGlyphStore & )            = delete;
        SlugGlyphStore &operator=( const SlugGlyphStore & ) = delete;

        uint32_t         Get( uint32_t fontKey, hb_font_t *font, uint32_t upem, uint32_t glyphId );
        const SlugGlyph &GetGlyph( uint32_t index ) const;

        const std::vector<DenOfIz_Float4> &GetCurves( ) const;
        const std::vector<DenOfIz_UInt2>  &GetBands( ) const;
        const std::vector<SlugGlyph>      &GetGlyphs( ) const;

        const SlugUploadMark &GetUploadMark( ) const;
        bool                  HasPendingUpload( ) const;
        void                  MarkUploaded( );
    };
} // namespace DenOfIz
