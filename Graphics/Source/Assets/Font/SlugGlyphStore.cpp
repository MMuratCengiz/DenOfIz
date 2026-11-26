/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/SlugGlyphStore.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <span>

using namespace DenOfIz;

namespace
{
    constexpr uint32_t MaxBands        = 16;
    constexpr float    BandEpsilon     = 1.0f / 1024.0f;
    constexpr float    AxisLineEpsilon = 1.0f / 1048576.0f;
    constexpr double   CubicTolerance  = 1.0 / 4096.0;
    constexpr double   CubicErrorScale = 0.0481125224324688; // sqrt(3) / 36
    constexpr int      MaxCubicDepth   = 6;

    struct Quad
    {
        DenOfIz_Float2 P1;
        DenOfIz_Float2 P2;
        DenOfIz_Float2 P3;
    };

    struct Double2
    {
        double X;
        double Y;
    };

    bool Equal( const DenOfIz_Float2 &a, const DenOfIz_Float2 &b )
    {
        return a.X == b.X && a.Y == b.Y;
    }

    Double2 ToDouble( const DenOfIz_Float2 &p )
    {
        return { p.X, p.Y };
    }

    Double2 Mid( const Double2 &a, const Double2 &b )
    {
        return { ( a.X + b.X ) * 0.5, ( a.Y + b.Y ) * 0.5 };
    }

    struct DrawContext
    {
        SlugTransform     Transform;
        double            Tolerance;
        std::vector<Quad> Quads;

        DenOfIz_Float2 Em( const float x, const float y ) const
        {
            const SlugTransform &t = Transform;
            return { ( t.Xx * x + t.Xy * y + t.Dx ) / t.Divisor, ( t.Yx * x + t.Yy * y + t.Dy ) / t.Divisor };
        }

        void Add( const DenOfIz_Float2 &p1, const DenOfIz_Float2 &p2, const DenOfIz_Float2 &p3 )
        {
            if ( Equal( p1, p2 ) && Equal( p2, p3 ) )
            {
                return;
            }
            Quads.push_back( { p1, p2, p3 } );
        }

        // Endpoints stay the exact input floats so neighbouring curves still share texels.
        void AddCubic( const DenOfIz_Float2 &p0, const Double2 &c1, const Double2 &c2, const DenOfIz_Float2 &p3, const int depth )
        {
            const Double2 a  = ToDouble( p0 );
            const Double2 d  = ToDouble( p3 );
            const double  ex = d.X - 3.0 * c2.X + 3.0 * c1.X - a.X;
            const double  ey = d.Y - 3.0 * c2.Y + 3.0 * c1.Y - a.Y;
            if ( depth >= MaxCubicDepth || CubicErrorScale * std::hypot( ex, ey ) <= Tolerance )
            {
                const DenOfIz_Float2 q = { static_cast<float>( ( 3.0 * ( c1.X + c2.X ) - ( a.X + d.X ) ) * 0.25 ),
                                           static_cast<float>( ( 3.0 * ( c1.Y + c2.Y ) - ( a.Y + d.Y ) ) * 0.25 ) };
                Add( p0, q, p3 );
                return;
            }

            const Double2        ab  = Mid( a, c1 );
            const Double2        bc  = Mid( c1, c2 );
            const Double2        cd  = Mid( c2, d );
            const Double2        abc = Mid( ab, bc );
            const Double2        bcd = Mid( bc, cd );
            const Double2        m   = Mid( abc, bcd );
            const DenOfIz_Float2 mid = { static_cast<float>( m.X ), static_cast<float>( m.Y ) };
            AddCubic( p0, ab, abc, mid, depth + 1 );
            AddCubic( mid, bcd, cd, p3, depth + 1 );
        }
    };

    // HarfBuzz closes every contour with an explicit line_to, so move_to/close_path are not needed.
    void LineTo( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float x, const float y, void * )
    {
        auto                *ctx = static_cast<DrawContext *>( data );
        const DenOfIz_Float2 to  = ctx->Em( x, y );
        ctx->Add( ctx->Em( st->current_x, st->current_y ), to, to );
    }

    void QuadraticTo( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float cx, const float cy, const float x, const float y, void * )
    {
        auto *ctx = static_cast<DrawContext *>( data );
        ctx->Add( ctx->Em( st->current_x, st->current_y ), ctx->Em( cx, cy ), ctx->Em( x, y ) );
    }

    void CubicTo( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float c1x, const float c1y, const float c2x, const float c2y, const float x, const float y, void * )
    {
        auto *ctx = static_cast<DrawContext *>( data );
        ctx->AddCubic( ctx->Em( st->current_x, st->current_y ), ToDouble( ctx->Em( c1x, c1y ) ), ToDouble( ctx->Em( c2x, c2y ) ), ctx->Em( x, y ), 0 );
    }

    struct CurveInfo
    {
        uint32_t Loc;
        float    Min[ 2 ];
        float    Max[ 2 ];
    };

    // Band b lists the curves Curves[ Starts[ b ] .. Starts[ b + 1 ] ).
    struct BandSet
    {
        uint32_t              Count  = 1;
        float                 Scale  = 0.0f;
        float                 Offset = 0.0f;
        std::vector<uint32_t> Curves;
        std::vector<uint32_t> Starts;
    };

    bool Overlaps( const CurveInfo &curve, const int axis, const float lo, const float hi )
    {
        return curve.Max[ axis ] >= lo && curve.Min[ axis ] <= hi;
    }

    // axis 1 builds horizontal bands (partition y, sorted by max x), axis 0 vertical bands (partition x, sorted by max y).
    BandSet BuildBands( const std::vector<CurveInfo> &curves, const int axis, const float lo, const float hi )
    {
        std::vector<uint32_t> candidates;
        for ( uint32_t i = 0; i < curves.size( ); ++i )
        {
            // Straight lines parallel to the band's rays never change the winding number.
            if ( curves[ i ].Max[ axis ] - curves[ i ].Min[ axis ] > AxisLineEpsilon )
            {
                candidates.push_back( i );
            }
        }

        BandSet     set;
        const float size = hi - lo;
        if ( size > 0.0f && !candidates.empty( ) )
        {
            // A curve overlaps [bandLo, bandHi] unless it ends below bandLo or starts above bandHi, and one ending below starts below too.
            std::vector<float> mins, maxs;
            for ( const uint32_t c : candidates )
            {
                mins.push_back( curves[ c ].Min[ axis ] );
                maxs.push_back( curves[ c ].Max[ axis ] );
            }
            std::sort( mins.begin( ), mins.end( ) );
            std::sort( maxs.begin( ), maxs.end( ) );
            size_t bestMax = SIZE_MAX;
            for ( uint32_t count = 1; count <= MaxBands; ++count )
            {
                const float thickness = size / static_cast<float>( count );
                size_t      maxInBand = 0;
                for ( uint32_t b = 0; b < count; ++b )
                {
                    const float  bandLo = lo + static_cast<float>( b ) * thickness - BandEpsilon;
                    const float  bandHi = lo + static_cast<float>( b + 1 ) * thickness + BandEpsilon;
                    const size_t inBand = static_cast<size_t>( std::upper_bound( mins.begin( ), mins.end( ), bandHi ) - mins.begin( ) ) -
                                          static_cast<size_t>( std::lower_bound( maxs.begin( ), maxs.end( ), bandLo ) - maxs.begin( ) );
                    maxInBand = std::max( maxInBand, inBand );
                }
                if ( maxInBand < bestMax )
                {
                    bestMax   = maxInBand;
                    set.Count = count;
                }
            }
            set.Scale  = static_cast<float>( set.Count ) / size;
            set.Offset = -lo * set.Scale;
        }

        const int   sortAxis  = 1 - axis;
        const float thickness = size / static_cast<float>( set.Count );
        set.Starts.reserve( set.Count + 1 );
        for ( uint32_t b = 0; b < set.Count; ++b )
        {
            const float bandLo = lo + static_cast<float>( b ) * thickness - BandEpsilon;
            const float bandHi = lo + static_cast<float>( b + 1 ) * thickness + BandEpsilon;
            const auto  first  = static_cast<uint32_t>( set.Curves.size( ) );
            set.Starts.push_back( first );
            for ( const uint32_t c : candidates )
            {
                if ( Overlaps( curves[ c ], axis, bandLo, bandHi ) )
                {
                    set.Curves.push_back( c );
                }
            }
            std::sort( set.Curves.begin( ) + first, set.Curves.end( ),
                       [ & ]( const uint32_t l, const uint32_t r )
                       {
                           const CurveInfo &a = curves[ l ];
                           const CurveInfo &b = curves[ r ];
                           return a.Max[ sortAxis ] != b.Max[ sortAxis ] ? a.Max[ sortAxis ] > b.Max[ sortAxis ] : a.Loc < b.Loc;
                       } );
        }
        set.Starts.push_back( static_cast<uint32_t>( set.Curves.size( ) ) );
        return set;
    }

    // A band's curve locations are packed two per entry, its header's offset counts entries from the glyph's first header.
    void AppendBands( std::vector<DenOfIz_UInt2> &bands, const BandSet &set, const std::vector<CurveInfo> &curves, const uint32_t glyphStart, const uint32_t firstHeader )
    {
        const auto list = [ &set ]( const uint32_t b ) { return std::span<const uint32_t>( set.Curves.data( ) + set.Starts[ b ], set.Starts[ b + 1 ] - set.Starts[ b ] ); };
        for ( uint32_t b = 0; b < set.Count; ++b )
        {
            const uint32_t                  header  = glyphStart + firstHeader + b;
            const std::span<const uint32_t> current = list( b );
            if ( b > 0 && std::ranges::equal( current, list( b - 1 ) ) )
            {
                bands[ header ] = bands[ header - 1 ];
                continue;
            }
            bands[ header ] = { static_cast<uint32_t>( current.size( ) ), static_cast<uint32_t>( bands.size( ) ) - glyphStart };
            for ( size_t i = 0; i < current.size( ); i += 2 )
            {
                bands.push_back( { curves[ current[ i ] ].Loc, i + 1 < current.size( ) ? curves[ current[ i + 1 ] ].Loc : 0 } );
            }
        }
    }

    hb_draw_funcs_t *DrawFuncs( )
    {
        static const struct Funcs
        {
            hb_draw_funcs_t *Value = hb_draw_funcs_create( );

            Funcs( )
            {
                hb_draw_funcs_set_line_to_func( Value, LineTo, nullptr, nullptr );
                hb_draw_funcs_set_quadratic_to_func( Value, QuadraticTo, nullptr, nullptr );
                hb_draw_funcs_set_cubic_to_func( Value, CubicTo, nullptr, nullptr );
                hb_draw_funcs_make_immutable( Value );
            }
            ~Funcs( )
            {
                hb_draw_funcs_destroy( Value );
            }
        } funcs;
        return funcs.Value;
    }
} // namespace

uint32_t SlugGlyphStore::Get( const uint32_t fontKey, hb_font_t *font, const uint32_t upem, const uint32_t glyphId )
{
    const uint64_t key          = static_cast<uint64_t>( fontKey ) << 32 | glyphId;
    const auto [ it, inserted ] = m_lookup.try_emplace( key, static_cast<uint32_t>( m_glyphs.size( ) ) );
    if ( inserted )
    {
        SlugTransform em;
        em.Divisor = static_cast<float>( upem );
        m_glyphs.push_back( CompileSlugGlyph( font, glyphId, em, m_curves, m_bands ) );
    }
    return it->second;
}

const SlugGlyph &SlugGlyphStore::GetGlyph( const uint32_t index ) const
{
    return m_glyphs[ index ];
}

const std::vector<DenOfIz_Float4> &SlugGlyphStore::GetCurves( ) const
{
    return m_curves;
}

const std::vector<DenOfIz_UInt2> &SlugGlyphStore::GetBands( ) const
{
    return m_bands;
}

const std::vector<SlugGlyph> &SlugGlyphStore::GetGlyphs( ) const
{
    return m_glyphs;
}

const SlugUploadMark &SlugGlyphStore::GetUploadMark( ) const
{
    return m_uploaded;
}

bool SlugGlyphStore::HasPendingUpload( ) const
{
    return m_uploaded.Curves != m_curves.size( ) || m_uploaded.Bands != m_bands.size( ) || m_uploaded.Glyphs != m_glyphs.size( );
}

void SlugGlyphStore::MarkUploaded( )
{
    m_uploaded = { m_curves.size( ), m_bands.size( ), m_glyphs.size( ) };
}

SlugGlyph DenOfIz::CompileSlugGlyph( hb_font_t *font, const uint32_t glyphId, const SlugTransform &transform, std::vector<DenOfIz_Float4> &curveData,
                                     std::vector<DenOfIz_UInt2> &bandData )
{
    // The tolerance is in em, scaled to the units the curves are stored in.
    const double upem  = hb_face_get_upem( hb_font_get_face( font ) );
    const double scale = std::sqrt( std::fabs( static_cast<double>( transform.Xx ) * transform.Yy - static_cast<double>( transform.Xy ) * transform.Yx ) ) / transform.Divisor;
    DrawContext  ctx{ transform, CubicTolerance * upem * scale, {} };
    hb_font_draw_glyph( font, glyphId, DrawFuncs( ), &ctx );

    SlugGlyph glyph{ };
    if ( ctx.Quads.empty( ) )
    {
        return glyph;
    }

    std::vector<CurveInfo> curves( ctx.Quads.size( ) );
    float                  minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for ( size_t i = 0; i < ctx.Quads.size( ); ++i )
    {
        const Quad &q = ctx.Quads[ i ];
        // A curve starting where the previous one ended reuses that texel: its .xy is p1, .zw becomes p2.
        if ( i > 0 && Equal( ctx.Quads[ i - 1 ].P3, q.P1 ) )
        {
            curveData.back( ).Z = q.P2.X;
            curveData.back( ).W = q.P2.Y;
        }
        else
        {
            curveData.push_back( { q.P1.X, q.P1.Y, q.P2.X, q.P2.Y } );
        }

        CurveInfo &info = curves[ i ];
        info.Loc        = static_cast<uint32_t>( curveData.size( ) - 1 );
        info.Min[ 0 ]   = std::min( { q.P1.X, q.P2.X, q.P3.X } );
        info.Min[ 1 ]   = std::min( { q.P1.Y, q.P2.Y, q.P3.Y } );
        info.Max[ 0 ]   = std::max( { q.P1.X, q.P2.X, q.P3.X } );
        info.Max[ 1 ]   = std::max( { q.P1.Y, q.P2.Y, q.P3.Y } );
        curveData.push_back( { q.P3.X, q.P3.Y, 0.0f, 0.0f } );

        minX = std::min( minX, info.Min[ 0 ] );
        minY = std::min( minY, info.Min[ 1 ] );
        maxX = std::max( maxX, info.Max[ 0 ] );
        maxY = std::max( maxY, info.Max[ 1 ] );
    }

    const BandSet  hBands     = BuildBands( curves, 1, minY, maxY );
    const BandSet  vBands     = BuildBands( curves, 0, minX, maxX );
    const uint32_t bandOffset = static_cast<uint32_t>( bandData.size( ) );
    bandData.resize( bandOffset + hBands.Count + vBands.Count );
    AppendBands( bandData, hBands, curves, bandOffset, 0 );
    AppendBands( bandData, vBands, curves, bandOffset, hBands.Count );

    glyph.Bounds        = { minX, minY, maxX, maxY };
    glyph.BandTransform = { vBands.Scale, hBands.Scale, vBands.Offset, hBands.Offset };
    glyph.Data          = { bandOffset, 0, vBands.Count - 1, hBands.Count - 1 };
    return glyph;
}
