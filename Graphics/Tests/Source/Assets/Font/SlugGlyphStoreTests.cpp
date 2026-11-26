/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <harfbuzz/hb.h>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/SlugReference.h"

#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif

using namespace DenOfIz;
using SlugReference::float2;

namespace
{
    const std::string FontDir        = DZ_TEST_DATA_SRC_DIR "/Fonts/";
    const std::string FontAwesomeOtf = DZ_TEST_DATA_SRC_DIR "/../../../Examples/Assets/Fonts/FontAwesome7.otf";
    const std::string OutDir         = DZ_TEST_DATA_DEST_DIR "/Slug/";

    // Same maths over every curve of the glyph with no bands and no early-out; banded results must match it.
    float SlugRenderUnbanded( const std::vector<DenOfIz_Float4> &curveData, const std::vector<uint32_t> &curveLocs, const float2 sampleCoord, const float2 emsPerPixel,
                              const int flags )
    {
        const float2 renderCoord{ sampleCoord.x + SlugReference::SampleBias, sampleCoord.y + SlugReference::SampleBias };
        float        xcov = 0.0f, xwgt = 0.0f, ycov = 0.0f, ywgt = 0.0f;
        for ( const uint32_t curveLoc : curveLocs )
        {
            const SlugReference::float4 p12 = SlugReference::LoadCurve( curveData, curveLoc, renderCoord );
            const float2                p3  = SlugReference::LoadP3( curveData, curveLoc, renderCoord );
            SlugReference::AccumulateHoriz( p12, p3, 1.0f / emsPerPixel.x, xcov, xwgt );
            SlugReference::AccumulateVert( p12, p3, 1.0f / emsPerPixel.y, ycov, ywgt );
        }
        return SlugReference::CalcCoverage( xcov, ycov, xwgt, ywgt, flags );
    }

    const char *GeezaProPath  = "/System/Library/Fonts/GeezaPro.ttc";
    const char *HiraginoPath  = "/System/Library/Fonts/ヒラギノ角ゴシック W3.ttc";
    const char *SampleLatin   = "Ag@&%éß";
    const float RenderSizes[] = { 96.0f, 16.0f };

    struct HbFont
    {
        hb_blob_t *Blob = nullptr;
        hb_face_t *Face = nullptr;
        hb_font_t *Font = nullptr;
        uint32_t   Upem = 0;

        HbFont( const std::string &path, const unsigned faceIndex )
        {
            Blob = hb_blob_create_from_file_or_fail( path.c_str( ) );
            if ( !Blob )
            {
                return;
            }
            Face = hb_face_create( Blob, faceIndex );
            Upem = hb_face_get_upem( Face );
            Font = hb_font_create( Face );
            hb_font_set_scale( Font, static_cast<int>( Upem ), static_cast<int>( Upem ) );
        }

        ~HbFont( )
        {
            hb_font_destroy( Font );
            hb_face_destroy( Face );
            hb_blob_destroy( Blob );
        }

        HbFont( const HbFont & )            = delete;
        HbFont &operator=( const HbFont & ) = delete;
    };

    struct PlacedGlyph
    {
        uint32_t GlyphId;
        float    X;
        float    Y;
    };

    std::vector<PlacedGlyph> Shape( const HbFont &font, const char *text, const hb_direction_t direction = HB_DIRECTION_LTR, const hb_script_t script = HB_SCRIPT_LATIN,
                                    const char *language = "en", const std::vector<const char *> &features = { } )
    {
        hb_buffer_t *buffer = hb_buffer_create( );
        hb_buffer_add_utf8( buffer, text, -1, 0, -1 );
        hb_buffer_set_direction( buffer, direction );
        hb_buffer_set_script( buffer, script );
        hb_buffer_set_language( buffer, hb_language_from_string( language, -1 ) );

        std::vector<hb_feature_t> hbFeatures( features.size( ) );
        for ( size_t i = 0; i < features.size( ); ++i )
        {
            hb_feature_from_string( features[ i ], -1, &hbFeatures[ i ] );
        }
        hb_shape( font.Font, buffer, hbFeatures.data( ), static_cast<unsigned>( hbFeatures.size( ) ) );

        unsigned int               count;
        const hb_glyph_info_t     *infos     = hb_buffer_get_glyph_infos( buffer, &count );
        const hb_glyph_position_t *positions = hb_buffer_get_glyph_positions( buffer, &count );
        const float                upem      = static_cast<float>( font.Upem );
        std::vector<PlacedGlyph>   result;
        int                        pen = 0;
        for ( unsigned int i = 0; i < count; ++i )
        {
            result.push_back( { infos[ i ].codepoint, static_cast<float>( pen + positions[ i ].x_offset ) / upem, static_cast<float>( positions[ i ].y_offset ) / upem } );
            pen += positions[ i ].x_advance;
        }
        hb_buffer_destroy( buffer );
        return result;
    }

    // Independent copy of the outline straight from HarfBuzz: exact segments (incl. cubics) for winding, and the quads the store should contain.
    struct Segment
    {
        int    Degree;
        double P[ 4 ][ 2 ];
    };

    struct Outline
    {
        float                             Upem = 1.0f;
        std::vector<Segment>              Segments;
        std::vector<std::array<float, 6>> Quads;
        bool                              HasCubics = false;
    };

    void OutlineAdd( Outline &o, const int degree, const float *xy )
    {
        Segment s{ degree, {} };
        for ( int i = 0; i <= degree; ++i )
        {
            s.P[ i ][ 0 ] = static_cast<double>( xy[ i * 2 ] ) / o.Upem;
            s.P[ i ][ 1 ] = static_cast<double>( xy[ i * 2 + 1 ] ) / o.Upem;
        }
        o.Segments.push_back( s );
        if ( degree == 3 )
        {
            o.HasCubics = true;
            return;
        }
        std::array<float, 6> q{ };
        for ( int i = 0; i < 3; ++i )
        {
            const int src  = std::min( i, degree );
            q[ i * 2 ]     = xy[ src * 2 ] / o.Upem;
            q[ i * 2 + 1 ] = xy[ src * 2 + 1 ] / o.Upem;
        }
        if ( !( q[ 0 ] == q[ 2 ] && q[ 2 ] == q[ 4 ] && q[ 1 ] == q[ 3 ] && q[ 3 ] == q[ 5 ] ) )
        {
            o.Quads.push_back( q );
        }
    }

    hb_draw_funcs_t *OutlineFuncs( )
    {
        static hb_draw_funcs_t *funcs = []
        {
            hb_draw_funcs_t *f = hb_draw_funcs_create( );
            hb_draw_funcs_set_line_to_func(
                f,
                []( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float x, const float y, void * )
                {
                    const float xy[] = { st->current_x, st->current_y, x, y };
                    OutlineAdd( *static_cast<Outline *>( data ), 1, xy );
                },
                nullptr, nullptr );
            hb_draw_funcs_set_quadratic_to_func(
                f,
                []( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float cx, const float cy, const float x, const float y, void * )
                {
                    const float xy[] = { st->current_x, st->current_y, cx, cy, x, y };
                    OutlineAdd( *static_cast<Outline *>( data ), 2, xy );
                },
                nullptr, nullptr );
            hb_draw_funcs_set_cubic_to_func(
                f,
                []( hb_draw_funcs_t *, void *data, hb_draw_state_t *st, const float c1x, const float c1y, const float c2x, const float c2y, const float x, const float y, void * )
                {
                    const float xy[] = { st->current_x, st->current_y, c1x, c1y, c2x, c2y, x, y };
                    OutlineAdd( *static_cast<Outline *>( data ), 3, xy );
                },
                nullptr, nullptr );
            hb_draw_funcs_make_immutable( f );
            return f;
        }( );
        return funcs;
    }

    Outline DrawOutline( const HbFont &font, const uint32_t glyphId )
    {
        Outline outline;
        outline.Upem = static_cast<float>( font.Upem );
        hb_font_draw_glyph( font.Font, glyphId, OutlineFuncs( ), &outline );
        return outline;
    }

    double Bezier( const Segment &s, const int axis, const double t )
    {
        double p[ 4 ];
        for ( int i = 0; i <= s.Degree; ++i )
        {
            p[ i ] = s.P[ i ][ axis ];
        }
        for ( int k = s.Degree; k > 0; --k )
        {
            for ( int i = 0; i < k; ++i )
            {
                p[ i ] += ( p[ i + 1 ] - p[ i ] ) * t;
            }
        }
        return p[ 0 ];
    }

    // Nonzero winding of the exact outline by casting a +x ray, splitting segments into y-monotonic pieces.
    int Winding( const std::vector<Segment> &segments, const double x, const double y )
    {
        int winding = 0;
        for ( const Segment &s : segments )
        {
            std::vector<double> ts      = { 0.0 };
            auto                addRoot = [ &ts ]( const double t )
            {
                if ( t > 0.0 && t < 1.0 )
                {
                    ts.push_back( t );
                }
            };
            if ( s.Degree == 2 )
            {
                const double d = s.P[ 0 ][ 1 ] - 2.0 * s.P[ 1 ][ 1 ] + s.P[ 2 ][ 1 ];
                if ( std::fabs( d ) > 1e-15 )
                {
                    addRoot( ( s.P[ 0 ][ 1 ] - s.P[ 1 ][ 1 ] ) / d );
                }
            }
            else if ( s.Degree == 3 )
            {
                const double d0 = s.P[ 1 ][ 1 ] - s.P[ 0 ][ 1 ];
                const double d1 = s.P[ 2 ][ 1 ] - s.P[ 1 ][ 1 ];
                const double d2 = s.P[ 3 ][ 1 ] - s.P[ 2 ][ 1 ];
                const double a  = d0 - 2.0 * d1 + d2;
                const double b  = 2.0 * ( d1 - d0 );
                if ( std::fabs( a ) < 1e-15 )
                {
                    if ( std::fabs( b ) > 1e-15 )
                    {
                        addRoot( -d0 / b );
                    }
                }
                else
                {
                    const double disc = b * b - 4.0 * a * d0;
                    if ( disc >= 0.0 )
                    {
                        addRoot( ( -b - std::sqrt( disc ) ) / ( 2.0 * a ) );
                        addRoot( ( -b + std::sqrt( disc ) ) / ( 2.0 * a ) );
                    }
                }
            }
            std::sort( ts.begin( ), ts.end( ) );
            ts.push_back( 1.0 );

            std::vector<double> ys( ts.size( ) );
            for ( size_t i = 0; i < ts.size( ); ++i )
            {
                ys[ i ] = i == 0 ? s.P[ 0 ][ 1 ] : i + 1 == ts.size( ) ? s.P[ s.Degree ][ 1 ] : Bezier( s, 1, ts[ i ] );
            }
            for ( size_t i = 0; i + 1 < ts.size( ); ++i )
            {
                const double ya = ys[ i ], yb = ys[ i + 1 ];
                const bool   up = yb > ya;
                if ( ya == yb || !( up ? ya <= y && y < yb : yb <= y && y < ya ) )
                {
                    continue;
                }
                double lo = ts[ i ], hi = ts[ i + 1 ];
                for ( int it = 0; it < 60; ++it )
                {
                    const double mid = 0.5 * ( lo + hi );
                    const double ym  = Bezier( s, 1, mid );
                    if ( up ? ym <= y : ym > y )
                    {
                        lo = mid;
                    }
                    else
                    {
                        hi = mid;
                    }
                }
                if ( Bezier( s, 0, 0.5 * ( lo + hi ) ) > x )
                {
                    winding += up ? 1 : -1;
                }
            }
        }
        return winding;
    }

    double PointSegmentDistance( const double px, const double py, const double ax, const double ay, const double bx, const double by )
    {
        const double dx = bx - ax, dy = by - ay;
        const double l2 = dx * dx + dy * dy;
        const double t  = l2 > 0.0 ? std::clamp( ( ( px - ax ) * dx + ( py - ay ) * dy ) / l2, 0.0, 1.0 ) : 0.0;
        return std::hypot( px - ( ax + t * dx ), py - ( ay + t * dy ) );
    }

    // True when no point of the exact outline lies within `margin` of (x, y): coverage must then be exactly 0 or 1.
    bool FarFromOutline( const std::vector<Segment> &segments, const double x, const double y, const double margin )
    {
        constexpr int Steps = 16;
        for ( const Segment &s : segments )
        {
            double minX = s.P[ 0 ][ 0 ], maxX = minX, minY = s.P[ 0 ][ 1 ], maxY = minY;
            for ( int i = 1; i <= s.Degree; ++i )
            {
                minX = std::min( minX, s.P[ i ][ 0 ] );
                maxX = std::max( maxX, s.P[ i ][ 0 ] );
                minY = std::min( minY, s.P[ i ][ 1 ] );
                maxY = std::max( maxY, s.P[ i ][ 1 ] );
            }
            if ( x < minX - margin || x > maxX + margin || y < minY - margin || y > maxY + margin )
            {
                continue;
            }
            // Chord error bound of the polyline: |B''| h^2 / 8.
            double secondDiff = 0.0;
            for ( int i = 0; i + 2 <= s.Degree; ++i )
            {
                secondDiff =
                    std::max( secondDiff, std::hypot( s.P[ i ][ 0 ] - 2.0 * s.P[ i + 1 ][ 0 ] + s.P[ i + 2 ][ 0 ], s.P[ i ][ 1 ] - 2.0 * s.P[ i + 1 ][ 1 ] + s.P[ i + 2 ][ 1 ] ) );
            }
            const int    steps     = s.Degree == 1 ? 1 : Steps;
            const double chordSlop = s.Degree * ( s.Degree - 1 ) * secondDiff / ( 8.0 * steps * steps );
            double       px = s.P[ 0 ][ 0 ], py = s.P[ 0 ][ 1 ];
            for ( int i = 1; i <= steps; ++i )
            {
                const double t  = static_cast<double>( i ) / steps;
                const double nx = Bezier( s, 0, t ), ny = Bezier( s, 1, t );
                if ( PointSegmentDistance( x, y, px, py, nx, ny ) - chordSlop <= margin )
                {
                    return false;
                }
                px = nx;
                py = ny;
            }
        }
        return true;
    }

    double OutlineSignedArea( const std::vector<Segment> &segments )
    {
        double area = 0.0;
        for ( const Segment &s : segments )
        {
            const int steps = s.Degree == 1 ? 1 : 64;
            double    px = s.P[ 0 ][ 0 ], py = s.P[ 0 ][ 1 ];
            for ( int i = 1; i <= steps; ++i )
            {
                const double t  = static_cast<double>( i ) / steps;
                const double nx = Bezier( s, 0, t ), ny = Bezier( s, 1, t );
                area += px * ny - nx * py;
                px = nx;
                py = ny;
            }
        }
        return 0.5 * area;
    }

    bool IsEmpty( const SlugGlyph &glyph )
    {
        return glyph.Bounds.X == 0.0f && glyph.Bounds.Y == 0.0f && glyph.Bounds.Z == 0.0f && glyph.Bounds.W == 0.0f;
    }

    struct StoredCurve
    {
        uint32_t Loc;
        float    P[ 6 ];
        float    Min[ 2 ];
        float    Max[ 2 ];
    };

    StoredCurve ReadCurve( const SlugGlyphStore &store, const uint32_t loc )
    {
        const DenOfIz_Float4 &a = store.GetCurves( )[ loc ];
        const DenOfIz_Float4 &b = store.GetCurves( )[ loc + 1 ];
        StoredCurve           c{ loc, { a.X, a.Y, a.Z, a.W, b.X, b.Y }, { }, {} };
        for ( int axis = 0; axis < 2; ++axis )
        {
            c.Min[ axis ] = std::min( { c.P[ axis ], c.P[ 2 + axis ], c.P[ 4 + axis ] } );
            c.Max[ axis ] = std::max( { c.P[ axis ], c.P[ 2 + axis ], c.P[ 4 + axis ] } );
        }
        return c;
    }

    struct GlyphStats
    {
        uint32_t              Curves      = 0;
        uint32_t              HBands      = 0;
        uint32_t              VBands      = 0;
        uint32_t              MaxH        = 0;
        uint32_t              MaxV        = 0;
        uint32_t              Texels      = 0;
        uint32_t              BandEntries = 0;
        std::vector<uint32_t> Locs;

        size_t Bytes( ) const
        {
            return Texels * sizeof( DenOfIz_Float4 ) + BandEntries * sizeof( DenOfIz_UInt2 ) + sizeof( SlugGlyph );
        }
    };

    constexpr float AxisLineEpsilon = 1.0f / 1048576.0f;

    // Structural invariants of the band data; every failure is reported with the glyph id from SCOPED_TRACE.
    GlyphStats ValidateGlyph( const SlugGlyphStore &store, const SlugGlyph &glyph, const Outline &outline )
    {
        GlyphStats stats;
        if ( IsEmpty( glyph ) )
        {
            EXPECT_TRUE( outline.Quads.empty( ) && !outline.HasCubics ) << "glyph with curves compiled as empty";
            return stats;
        }

        const auto    &bands  = store.GetBands( );
        const uint32_t start  = glyph.Data.X;
        const uint32_t hCount = ( glyph.Data.W & 0xFFU ) + 1;
        const uint32_t vCount = glyph.Data.Z + 1;
        EXPECT_EQ( glyph.Data.Y, 0u );
        EXPECT_EQ( glyph.Data.W & ~0xFFU, 0u ) << "unexpected flags";
        EXPECT_LE( hCount, 16u );
        EXPECT_LE( vCount, 16u );
        EXPECT_LE( start + hCount + vCount, bands.size( ) );
        stats.HBands = hCount;
        stats.VBands = vCount;

        std::vector<std::set<uint32_t>> lists[ 2 ];
        std::set<uint32_t>              axisUnion[ 2 ];
        uint32_t                        bandEnd = start + hCount + vCount;
        for ( int axis = 1; axis >= 0; --axis )
        {
            const uint32_t count      = axis == 1 ? hCount : vCount;
            const uint32_t headerBase = start + ( axis == 1 ? 0 : hCount );
            lists[ axis ].resize( count );
            for ( uint32_t b = 0; b < count; ++b )
            {
                const DenOfIz_UInt2 header  = bands[ headerBase + b ];
                const uint32_t      entries = ( header.X + 1 ) / 2;
                EXPECT_GE( header.Y, hCount + vCount );
                if ( start + header.Y + entries > bands.size( ) )
                {
                    ADD_FAILURE( ) << "band list out of range";
                    return stats;
                }
                bandEnd   = std::max( bandEnd, start + header.Y + entries );
                float key = INFINITY;
                if ( header.X % 2 == 1 )
                {
                    EXPECT_EQ( bands[ start + header.Y + entries - 1 ].Y, 0u ) << "unused half of the last entry";
                }
                for ( uint32_t i = 0; i < header.X; ++i )
                {
                    const DenOfIz_UInt2 entry = bands[ start + header.Y + i / 2 ];
                    const uint32_t      loc   = i % 2 == 0 ? entry.X : entry.Y;
                    if ( loc + 1 >= store.GetCurves( ).size( ) )
                    {
                        ADD_FAILURE( ) << "curve location out of range";
                        return stats;
                    }
                    const StoredCurve c = ReadCurve( store, loc );
                    EXPECT_LE( c.Max[ 1 - axis ], key ) << "band " << b << " axis " << axis << " not sorted descending";
                    key = c.Max[ 1 - axis ];
                    lists[ axis ][ b ].insert( loc );
                    axisUnion[ axis ].insert( loc );
                }
                uint32_t &maxInAxis = axis == 1 ? stats.MaxH : stats.MaxV;
                maxInAxis           = std::max( maxInAxis, header.X );
            }
        }

        std::set<uint32_t> all = axisUnion[ 0 ];
        all.insert( axisUnion[ 1 ].begin( ), axisUnion[ 1 ].end( ) );
        EXPECT_FALSE( all.empty( ) );
        const float bounds[ 4 ] = { glyph.Bounds.X, glyph.Bounds.Y, glyph.Bounds.Z, glyph.Bounds.W };
        const float scale[ 2 ]  = { glyph.BandTransform.X, glyph.BandTransform.Y };
        const float offset[ 2 ] = { glyph.BandTransform.Z, glyph.BandTransform.W };

        std::vector<std::array<float, 6>> stored;
        for ( const uint32_t loc : all )
        {
            const StoredCurve c = ReadCurve( store, loc );
            stored.push_back( { c.P[ 0 ], c.P[ 1 ], c.P[ 2 ], c.P[ 3 ], c.P[ 4 ], c.P[ 5 ] } );
            for ( int axis = 0; axis < 2; ++axis )
            {
                EXPECT_GE( c.Min[ axis ], bounds[ axis ] );
                EXPECT_LE( c.Max[ axis ], bounds[ axis + 2 ] );
                const bool axisLine = c.Max[ axis ] - c.Min[ axis ] <= AxisLineEpsilon;
                EXPECT_EQ( axisUnion[ axis ].count( loc ) == 0, axisLine ) << "curve " << loc << " axis " << axis << ": band membership vs axis-aligned line mismatch";
                if ( axisLine )
                {
                    continue;
                }
                // Every band whose em range (as the shader computes it) touches the curve must list it.
                for ( uint32_t b = 0; b < lists[ axis ].size( ); ++b )
                {
                    float lo = -INFINITY, hi = INFINITY;
                    if ( scale[ axis ] != 0.0f )
                    {
                        lo = ( static_cast<float>( b ) - offset[ axis ] ) / scale[ axis ];
                        hi = ( static_cast<float>( b + 1 ) - offset[ axis ] ) / scale[ axis ];
                    }
                    const bool touches  = c.Max[ axis ] >= lo - 1.0f / 2048.0f && c.Min[ axis ] <= hi + 1.0f / 2048.0f;
                    const bool nearBand = c.Max[ axis ] >= lo - 1.0f / 512.0f && c.Min[ axis ] <= hi + 1.0f / 512.0f;
                    const bool listed   = lists[ axis ][ b ].count( loc ) != 0;
                    EXPECT_TRUE( !touches || listed ) << "curve " << loc << " missing from band " << b << " axis " << axis;
                    EXPECT_TRUE( !listed || nearBand ) << "curve " << loc << " listed in unrelated band " << b << " axis " << axis;
                }
            }
        }

        if ( !outline.HasCubics )
        {
            std::vector<std::array<float, 6>> expected = outline.Quads;
            std::sort( expected.begin( ), expected.end( ) );
            std::sort( stored.begin( ), stored.end( ) );
            EXPECT_EQ( expected, stored ) << "stored curves differ from the HarfBuzz outline";
        }

        stats.Curves      = static_cast<uint32_t>( all.size( ) );
        stats.Texels      = *all.rbegin( ) + 2 - *all.begin( );
        stats.BandEntries = bandEnd - start;
        stats.Locs.assign( all.begin( ), all.end( ) );
        return stats;
    }

    struct CoverageResult
    {
        double Area       = 0.0;
        int    FarSamples = 0;
    };

    // Samples the glyph on a pixel-centre grid and checks the banded oracle against the unbanded one and the exact winding.
    CoverageResult CheckCoverage( const SlugGlyphStore &store, const SlugGlyph &glyph, const GlyphStats &stats, const Outline &outline, const float ppem )
    {
        CoverageResult result;
        if ( IsEmpty( glyph ) )
        {
            return result;
        }
        const int    left   = static_cast<int>( std::floor( glyph.Bounds.X * ppem ) ) - 3;
        const int    right  = static_cast<int>( std::ceil( glyph.Bounds.Z * ppem ) ) + 3;
        const int    bottom = static_cast<int>( std::floor( glyph.Bounds.Y * ppem ) ) - 3;
        const int    top    = static_cast<int>( std::ceil( glyph.Bounds.W * ppem ) ) + 3;
        const float2 emsPerPixel{ 1.0f / ppem, 1.0f / ppem };
        const int    flags = static_cast<int>( glyph.Data.W );

        int    bandMismatches = 0, windingMismatches = 0, outsideMismatches = 0;
        double sum = 0.0;
        for ( int py = bottom; py < top; ++py )
        {
            for ( int px = left; px < right; ++px )
            {
                const float2 p{ ( static_cast<float>( px ) + 0.5f ) / ppem, ( static_cast<float>( py ) + 0.5f ) / ppem };
                const float  coverage = SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), p, emsPerPixel, glyph );
                const float  unbanded = SlugRenderUnbanded( store.GetCurves( ), stats.Locs, p, emsPerPixel, flags );
                sum += coverage;
                if ( std::fabs( coverage - unbanded ) > 1e-4f && bandMismatches++ == 0 )
                {
                    ADD_FAILURE( ) << "banded " << coverage << " vs unbanded " << unbanded << " at (" << p.x << ", " << p.y << ") ppem " << ppem;
                }
                const bool outside =
                    p.x < glyph.Bounds.X - 1.0f / ppem || p.x > glyph.Bounds.Z + 1.0f / ppem || p.y < glyph.Bounds.Y - 1.0f / ppem || p.y > glyph.Bounds.W + 1.0f / ppem;
                if ( outside && coverage != 0.0f && outsideMismatches++ == 0 )
                {
                    ADD_FAILURE( ) << "coverage " << coverage << " outside the dilated quad at (" << p.x << ", " << p.y << ")";
                }
                if ( FarFromOutline( outline.Segments, p.x, p.y, 0.75 / ppem ) )
                {
                    ++result.FarSamples;
                    const float expected = Winding( outline.Segments, p.x, p.y ) != 0 ? 1.0f : 0.0f;
                    if ( std::fabs( coverage - expected ) > 1e-4f && windingMismatches++ == 0 )
                    {
                        ADD_FAILURE( ) << "coverage " << coverage << " vs winding " << expected << " at (" << p.x << ", " << p.y << ") ppem " << ppem;
                    }
                }
            }
        }
        EXPECT_EQ( bandMismatches, 0 );
        EXPECT_EQ( windingMismatches, 0 );
        EXPECT_EQ( outsideMismatches, 0 );
        result.Area = sum / ( static_cast<double>( ppem ) * ppem );
        return result;
    }

    struct Image
    {
        int                  Width  = 0;
        int                  Height = 0;
        std::vector<uint8_t> Pixels;
    };

    Image Rasterize( const SlugGlyphStore &store, const std::vector<std::pair<uint32_t, PlacedGlyph>> &glyphs, const float ppem )
    {
        float minX = INFINITY, minY = INFINITY, maxX = -INFINITY, maxY = -INFINITY;
        for ( const auto &[ index, placed ] : glyphs )
        {
            const SlugGlyph &g = store.GetGlyph( index );
            if ( IsEmpty( g ) )
            {
                continue;
            }
            minX = std::min( minX, placed.X + g.Bounds.X );
            minY = std::min( minY, placed.Y + g.Bounds.Y );
            maxX = std::max( maxX, placed.X + g.Bounds.Z );
            maxY = std::max( maxY, placed.Y + g.Bounds.W );
        }
        Image image;
        if ( minX > maxX )
        {
            return image;
        }
        const int    pad  = 4;
        const int    left = static_cast<int>( std::floor( minX * ppem ) ) - pad;
        const int    top  = static_cast<int>( std::ceil( maxY * ppem ) ) + pad;
        const float2 emsPerPixel{ 1.0f / ppem, 1.0f / ppem };
        image.Width  = static_cast<int>( std::ceil( maxX * ppem ) ) + pad - left;
        image.Height = top - ( static_cast<int>( std::floor( minY * ppem ) ) - pad );
        image.Pixels.assign( static_cast<size_t>( image.Width ) * image.Height, 255 );
        for ( int y = 0; y < image.Height; ++y )
        {
            for ( int x = 0; x < image.Width; ++x )
            {
                const float ex       = ( static_cast<float>( left + x ) + 0.5f ) / ppem;
                const float ey       = ( static_cast<float>( top - y ) - 0.5f ) / ppem;
                float       coverage = 0.0f;
                for ( const auto &[ index, placed ] : glyphs )
                {
                    const SlugGlyph &g = store.GetGlyph( index );
                    const float2     p{ ex - placed.X, ey - placed.Y };
                    // Mimics the 1 px dilated quad the renderer will draw.
                    if ( IsEmpty( g ) || p.x < g.Bounds.X - emsPerPixel.x || p.x > g.Bounds.Z + emsPerPixel.x || p.y < g.Bounds.Y - emsPerPixel.y ||
                         p.y > g.Bounds.W + emsPerPixel.y )
                    {
                        continue;
                    }
                    const float c = SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), p, emsPerPixel, g );
                    coverage += c * ( 1.0f - coverage );
                }
                image.Pixels[ static_cast<size_t>( y ) * image.Width + x ] = static_cast<uint8_t>( std::lround( 255.0f * ( 1.0f - coverage ) ) );
            }
        }
        return image;
    }

    void WritePng( const std::string &name, const Image &image, const int upscale = 1 )
    {
        ASSERT_GT( image.Width, 0 );
        std::filesystem::create_directories( OutDir );
        const int            w = image.Width * upscale, h = image.Height * upscale;
        std::vector<uint8_t> pixels( static_cast<size_t>( w ) * h );
        for ( int y = 0; y < h; ++y )
        {
            for ( int x = 0; x < w; ++x )
            {
                pixels[ static_cast<size_t>( y ) * w + x ] = image.Pixels[ static_cast<size_t>( y / upscale ) * image.Width + x / upscale ];
            }
        }
        const std::string path = OutDir + name + ".png";
        EXPECT_NE( stbi_write_png( path.c_str( ), w, h, 1, pixels.data( ), w ), 0 ) << path;
    }

    void PrintStats( const char *label, const uint32_t glyphId, const GlyphStats &s )
    {
        std::printf( "[slug] %-18s gid %5u: curves %3u texels %3u bands h%2u v%2u max h%3u v%3u entries %4u bytes %5zu\n", label, glyphId, s.Curves, s.Texels, s.HBands, s.VBands,
                     s.MaxH, s.MaxV, s.BandEntries, s.Bytes( ) );
    }

    // Compiles, validates and renders a shaped run; returns the store indices for further checks.
    std::vector<std::pair<uint32_t, PlacedGlyph>> ExerciseRun( SlugGlyphStore &store, const uint32_t fontKey, const HbFont &font, const char *label,
                                                               const std::vector<PlacedGlyph> &run )
    {
        std::vector<std::pair<uint32_t, PlacedGlyph>> placed;
        std::set<uint32_t>                            seen;
        for ( const PlacedGlyph &g : run )
        {
            const uint32_t index = store.Get( fontKey, font.Font, font.Upem, g.GlyphId );
            placed.emplace_back( index, g );
            if ( !seen.insert( g.GlyphId ).second )
            {
                continue;
            }
            SCOPED_TRACE( std::string( label ) + " glyph " + std::to_string( g.GlyphId ) );
            const Outline    outline = DrawOutline( font, g.GlyphId );
            const SlugGlyph &glyph   = store.GetGlyph( index );
            const GlyphStats stats   = ValidateGlyph( store, glyph, outline );
            PrintStats( label, g.GlyphId, stats );
            if ( IsEmpty( glyph ) )
            {
                continue;
            }
            const CoverageResult large = CheckCoverage( store, glyph, stats, outline, RenderSizes[ 0 ] );
            const CoverageResult small = CheckCoverage( store, glyph, stats, outline, RenderSizes[ 1 ] );
            const double         exact = std::fabs( OutlineSignedArea( outline.Segments ) );
            std::printf( "[slug] %-18s gid %5u: area 96px %.5f 16px %.5f outline %.5f far samples %d/%d\n", label, g.GlyphId, large.Area, small.Area, exact, large.FarSamples,
                         small.FarSamples );
            EXPECT_GT( large.FarSamples, 0 );
            EXPECT_NEAR( small.Area, large.Area, 0.05 * large.Area + 2e-4 ) << "coverage sum differs between sizes";
        }
        return placed;
    }

    void RenderRun( const SlugGlyphStore &store, const std::string &name, const std::vector<std::pair<uint32_t, PlacedGlyph>> &placed )
    {
        WritePng( name + "_96px", Rasterize( store, placed, 96.0f ) );
        const Image small = Rasterize( store, placed, 16.0f );
        WritePng( name + "_16px", small );
        WritePng( name + "_16px_x4", small, 4 );
    }

    uint32_t NominalGlyph( const HbFont &font, const hb_codepoint_t codepoint )
    {
        hb_codepoint_t glyph = 0;
        hb_font_get_nominal_glyph( font.Font, codepoint, &glyph );
        return glyph;
    }

    void ExpectStemCoverage( const SlugGlyphStore &store, const uint32_t index )
    {
        const SlugGlyph &g = store.GetGlyph( index );
        const float2     centre{ 0.5f * ( g.Bounds.X + g.Bounds.Z ), 0.5f * ( g.Bounds.Y + g.Bounds.W ) };
        for ( const float ppem : RenderSizes )
        {
            const float2 emsPerPixel{ 1.0f / ppem, 1.0f / ppem };
            EXPECT_NEAR( SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), centre, emsPerPixel, g ), 1.0f, 1e-3f ) << "stem centre at " << ppem << " ppem";
            const float d = 3.0f / ppem;
            for ( const float2 p : { float2{ g.Bounds.X - d, centre.y }, float2{ g.Bounds.Z + d, centre.y }, float2{ centre.x, g.Bounds.Y - d }, float2{ centre.x, g.Bounds.W + d },
                                     float2{ g.Bounds.X - d, g.Bounds.Y - d }, float2{ g.Bounds.Z + d, g.Bounds.W + d } } )
            {
                EXPECT_EQ( SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), p, emsPerPixel, g ), 0.0f ) << "outside at " << ppem << " ppem";
            }
        }
    }

    // Samples on a pixel-snapped origin whose coverage changes by more than 0.1 when moved 1e-6 em along x or y. At these sizes rows and columns
    // often land exactly level with the outline's points, where coverage steps, unless SlugRender biases its samples.
    int CountCoverageFlips( const HbFont &font, const std::vector<uint32_t> &glyphIds )
    {
        SlugGlyphStore store;
        int            flips = 0;
        for ( const uint32_t glyphId : glyphIds )
        {
            const SlugGlyph g = store.GetGlyph( store.Get( 1, font.Font, font.Upem, glyphId ) );
            if ( IsEmpty( g ) )
            {
                continue;
            }
            for ( const float ppem : { 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 18.0f, 20.0f, 24.0f, 32.0f } )
            {
                const float2 emsPerPixel{ 1.0f / ppem, 1.0f / ppem };
                for ( int py = static_cast<int>( std::floor( g.Bounds.Y * ppem ) ) - 1; py <= static_cast<int>( std::ceil( g.Bounds.W * ppem ) ); ++py )
                {
                    for ( int px = static_cast<int>( std::floor( g.Bounds.X * ppem ) ) - 1; px <= static_cast<int>( std::ceil( g.Bounds.Z * ppem ) ); ++px )
                    {
                        const float2 p{ ( static_cast<float>( px ) + 0.5f ) / ppem, ( static_cast<float>( py ) + 0.5f ) / ppem };
                        const float  coverage = SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), p, emsPerPixel, g );
                        for ( const float2 d : { float2{ 1e-6f, 0.0f }, float2{ -1e-6f, 0.0f }, float2{ 0.0f, 1e-6f }, float2{ 0.0f, -1e-6f } } )
                        {
                            const float moved = SlugReference::SlugRender( store.GetCurves( ), store.GetBands( ), float2{ p.x + d.x, p.y + d.y }, emsPerPixel, g );
                            flips += std::fabs( moved - coverage ) > 0.1f ? 1 : 0;
                        }
                    }
                }
            }
        }
        return flips;
    }
} // namespace

TEST( SlugGlyphStore, CacheAndUploadMark )
{
    const HbFont font( FontDir + "Inter-Regular.ttf", 0 );
    ASSERT_NE( font.Font, nullptr );
    SlugGlyphStore store;
    EXPECT_FALSE( store.HasPendingUpload( ) );

    const uint32_t a = store.Get( 1, font.Font, font.Upem, NominalGlyph( font, 'A' ) );
    EXPECT_EQ( store.Get( 1, font.Font, font.Upem, NominalGlyph( font, 'A' ) ), a );
    EXPECT_NE( store.Get( 2, font.Font, font.Upem, NominalGlyph( font, 'A' ) ), a ) << "font keys must not alias";
    EXPECT_TRUE( store.HasPendingUpload( ) );

    store.MarkUploaded( );
    EXPECT_FALSE( store.HasPendingUpload( ) );
    const SlugUploadMark mark = store.GetUploadMark( );
    EXPECT_EQ( mark.Curves, store.GetCurves( ).size( ) );
    EXPECT_EQ( mark.Bands, store.GetBands( ).size( ) );
    EXPECT_EQ( mark.Glyphs, store.GetGlyphs( ).size( ) );

    const uint32_t space = store.Get( 1, font.Font, font.Upem, NominalGlyph( font, ' ' ) );
    EXPECT_TRUE( IsEmpty( store.GetGlyph( space ) ) );
    EXPECT_EQ( store.GetCurves( ).size( ), mark.Curves ) << "empty glyphs add no curve data";
    EXPECT_EQ( store.GetBands( ).size( ), mark.Bands ) << "empty glyphs add no band data";
    EXPECT_TRUE( store.HasPendingUpload( ) ) << "the empty glyph record still needs uploading";

    store.MarkUploaded( );
    store.Get( 1, font.Font, font.Upem, NominalGlyph( font, 'g' ) );
    EXPECT_TRUE( store.HasPendingUpload( ) );
    EXPECT_GT( store.GetCurves( ).size( ), store.GetUploadMark( ).Curves );
    EXPECT_EQ( store.GetGlyphs( ).size( ), store.GetUploadMark( ).Glyphs + 1 );
}

TEST( SlugGlyphStore, InterTrueType )
{
    const HbFont font( FontDir + "Inter-Regular.ttf", 0 );
    ASSERT_NE( font.Font, nullptr );
    SlugGlyphStore store;

    RenderRun( store, "inter_ttf", ExerciseRun( store, 1, font, "Inter.ttf", Shape( font, SampleLatin ) ) );

    const auto alternates = Shape( font, "a1ß fi", HB_DIRECTION_LTR, HB_SCRIPT_LATIN, "en", { "cv11", "cv01", "cv07" } );
    const auto nominal    = Shape( font, "a1ß fi" );
    bool       differs    = alternates.size( ) != nominal.size( );
    for ( size_t i = 0; !differs && i < nominal.size( ); ++i )
    {
        differs = alternates[ i ].GlyphId != nominal[ i ].GlyphId;
    }
    EXPECT_TRUE( differs ) << "character variants should select alternate glyph ids";
    RenderRun( store, "inter_ttf_alternates", ExerciseRun( store, 1, font, "Inter.ttf alt", alternates ) );

    for ( const char c : { 'l', 'I' } )
    {
        SCOPED_TRACE( std::string( "stem " ) + c );
        ExpectStemCoverage( store, store.Get( 1, font.Font, font.Upem, NominalGlyph( font, static_cast<hb_codepoint_t>( c ) ) ) );
    }
}

TEST( SlugGlyphStore, JetBrainsMonoTrueType )
{
    const HbFont font( FontDir + "JetBrainsMono-Regular.ttf", 0 );
    ASSERT_NE( font.Font, nullptr );
    SlugGlyphStore store;

    RenderRun( store, "jbmono_ttf", ExerciseRun( store, 1, font, "JetBrainsMono", Shape( font, SampleLatin ) ) );

    const char *code      = "a->b != c <= d === e";
    const auto  ligatures = Shape( font, code );
    const auto  plain     = Shape( font, code, HB_DIRECTION_LTR, HB_SCRIPT_LATIN, "en", { "-calt", "-liga" } );
    bool        alternate = ligatures.size( ) != plain.size( );
    for ( size_t i = 0; !alternate && i < plain.size( ); ++i )
    {
        alternate = ligatures[ i ].GlyphId != plain[ i ].GlyphId;
    }
    EXPECT_TRUE( alternate ) << "calt should replace operator sequences with ligature glyphs";
    RenderRun( store, "jbmono_ligatures", ExerciseRun( store, 1, font, "JetBrainsMono liga", ligatures ) );
}

TEST( SlugGlyphStore, CffCubics )
{
    const HbFont font( FontAwesomeOtf, 0 );
    ASSERT_NE( font.Font, nullptr );
    SlugGlyphStore store;
    RenderRun( store, "awesome_otf", ExerciseRun( store, 1, font, "FontAwesome7.otf", Shape( font, "&@" ) ) );
}

TEST( SlugGlyphStore, ArabicShaping )
{
    const HbFont font( GeezaProPath, 0 );
    if ( !font.Font )
    {
        GTEST_SKIP( ) << "GeezaPro.ttc not available";
    }
    SlugGlyphStore store;
    const auto     run = Shape( font, "سلام", HB_DIRECTION_RTL, HB_SCRIPT_ARABIC, "ar" );
    EXPECT_GE( run.size( ), 3u );
    for ( const PlacedGlyph &g : run )
    {
        EXPECT_NE( g.GlyphId, 0u ) << "missing glyph";
    }
    RenderRun( store, "geeza_arabic", ExerciseRun( store, 1, font, "GeezaPro", run ) );
}

TEST( SlugGlyphStore, CjkGlyphs )
{
    const HbFont font( HiraginoPath, 0 );
    if ( !font.Font )
    {
        GTEST_SKIP( ) << "Hiragino Sans W3 not available";
    }
    SlugGlyphStore store;
    RenderRun( store, "hiragino_cjk", ExerciseRun( store, 1, font, "Hiragino", Shape( font, "漢字", HB_DIRECTION_LTR, HB_SCRIPT_HAN, "ja" ) ) );
}

TEST( SlugGlyphStore, InterAllGlyphs )
{
    const HbFont font( FontDir + "Inter-Regular.ttf", 0 );
    ASSERT_NE( font.Font, nullptr );
    SlugGlyphStore store;

    const auto     begin      = std::chrono::steady_clock::now( );
    const uint32_t glyphCount = hb_face_get_glyph_count( font.Face );
    for ( uint32_t gid = 0; gid < glyphCount; ++gid )
    {
        store.Get( 1, font.Font, font.Upem, gid );
    }
    const double compileMs = std::chrono::duration<double, std::milli>( std::chrono::steady_clock::now( ) - begin ).count( );

    uint32_t empty = 0, totalCurves = 0, maxBand = 0, worstGid = 0;
    uint64_t sumMax                   = 0;
    uint32_t bandCountHistogram[ 17 ] = { };
    int      checked                  = 0;
    for ( uint32_t gid = 0; gid < glyphCount; ++gid )
    {
        SCOPED_TRACE( "Inter glyph " + std::to_string( gid ) );
        const SlugGlyph &glyph   = store.GetGlyph( gid );
        const Outline    outline = DrawOutline( font, gid );
        const GlyphStats stats   = ValidateGlyph( store, glyph, outline );
        if ( IsEmpty( glyph ) )
        {
            ++empty;
            continue;
        }
        totalCurves += stats.Curves;
        const uint32_t worst = std::max( stats.MaxH, stats.MaxV );
        sumMax += worst;
        if ( worst > maxBand )
        {
            maxBand  = worst;
            worstGid = gid;
        }
        ++bandCountHistogram[ stats.HBands ];
        if ( gid % 8 == 0 )
        {
            CheckCoverage( store, glyph, stats, outline, 24.0f );
            ++checked;
        }
    }

    const size_t nonEmpty = glyphCount - empty;
    const size_t bytes =
        store.GetCurves( ).size( ) * sizeof( DenOfIz_Float4 ) + store.GetBands( ).size( ) * sizeof( DenOfIz_UInt2 ) + store.GetGlyphs( ).size( ) * sizeof( SlugGlyph );
    std::printf( "[slug] Inter.ttf all glyphs: %u glyphs (%u empty), %u curves, %zu texels (%zu B), %zu band entries (%zu B), %zu records (%zu B), total %.1f KiB, %.1f B/glyph\n",
                 glyphCount, empty, totalCurves, store.GetCurves( ).size( ), store.GetCurves( ).size( ) * sizeof( DenOfIz_Float4 ), store.GetBands( ).size( ),
                 store.GetBands( ).size( ) * sizeof( DenOfIz_UInt2 ), store.GetGlyphs( ).size( ), store.GetGlyphs( ).size( ) * sizeof( SlugGlyph ), bytes / 1024.0,
                 static_cast<double>( bytes ) / static_cast<double>( nonEmpty ) );
    std::printf( "[slug] Inter.ttf all glyphs: compile %.1f ms, mean max-curves-per-band %.1f, worst %u (gid %u), coverage-checked %d glyphs at 24px\n", compileMs,
                 static_cast<double>( sumMax ) / static_cast<double>( nonEmpty ), maxBand, worstGid, checked );
    std::printf( "[slug] Inter.ttf horizontal band count histogram:" );
    for ( int i = 1; i <= 16; ++i )
    {
        std::printf( " %d:%u", i, bandCountHistogram[ i ] );
    }
    std::printf( "\n" );
}

// Without the sample bias these glyphs flip 52 (Inter) and 845 (Font Awesome) times.
TEST( SlugGlyphStore, StableOnOutlinePoints )
{
    const HbFont inter( FontDir + "Inter-Regular.ttf", 0 );
    const HbFont awesome( FontAwesomeOtf, 0 );
    ASSERT_NE( inter.Font, nullptr );
    ASSERT_NE( awesome.Font, nullptr );
    std::vector<uint32_t> latin{ 0 };
    for ( const char c : std::string( "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz&@%" ) )
    {
        latin.push_back( NominalGlyph( inter, static_cast<hb_codepoint_t>( c ) ) );
    }
    std::vector<uint32_t> icons( 100 );
    std::iota( icons.begin( ), icons.end( ), 1u );
    EXPECT_LE( CountCoverageFlips( inter, latin ), 2 );
    EXPECT_LE( CountCoverageFlips( awesome, icons ), 2 );
}
