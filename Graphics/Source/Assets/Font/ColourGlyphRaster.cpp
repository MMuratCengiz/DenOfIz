/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <algorithm>
#include <cmath>
#include "DenOfIzGraphicsInternal/Assets/Font/ColourGlyphStore.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/SlugReference.h"
#include "stb_image.h"

using namespace DenOfIz;

namespace
{
    constexpr int   MaxCanvas = 1024;
    constexpr float TwoPi     = 6.28318530717958647692f;

    // Premultiplied.
    struct Rgba
    {
        float R = 0.0f;
        float G = 0.0f;
        float B = 0.0f;
        float A = 0.0f;

        Rgba operator*( const float k ) const
        {
            return { R * k, G * k, B * k, A * k };
        }

        Rgba operator+( const Rgba &o ) const
        {
            return { R + o.R, G + o.G, B + o.B, A + o.A };
        }
    };

    Rgba Lerp( const Rgba &a, const Rgba &b, const float t )
    {
        return a * ( 1.0f - t ) + b * t;
    }

    Rgba Premultiply( const hb_color_t color )
    {
        const float a = static_cast<float>( hb_color_get_alpha( color ) ) / 255.0f;
        const float k = a / 255.0f;
        return { static_cast<float>( hb_color_get_red( color ) ) * k, static_cast<float>( hb_color_get_green( color ) ) * k, static_cast<float>( hb_color_get_blue( color ) ) * k,
                 a };
    }

    void Over( Rgba &dst, const Rgba &src )
    {
        dst = src + dst * ( 1.0f - src.A );
    }

    // Maps ( x, y ) to ( Xx * x + Xy * y + Dx, Yx * x + Yy * y + Dy ), the layout of hb_paint_push_transform_func_t.
    struct Affine
    {
        float Xx = 1.0f;
        float Yx = 0.0f;
        float Xy = 0.0f;
        float Yy = 1.0f;
        float Dx = 0.0f;
        float Dy = 0.0f;

        // This transform applied after inner.
        Affine Then( const Affine &inner ) const
        {
            return { Xx * inner.Xx + Xy * inner.Yx, Yx * inner.Xx + Yy * inner.Yx,      Xx * inner.Xy + Xy * inner.Yy,
                     Yx * inner.Xy + Yy * inner.Yy, Xx * inner.Dx + Xy * inner.Dy + Dx, Yx * inner.Dx + Yy * inner.Dy + Dy };
        }

        bool Invert( Affine &out ) const
        {
            const float det = Xx * Yy - Xy * Yx;
            if ( std::fabs( det ) < 1e-12f )
            {
                return false;
            }
            const float inv = 1.0f / det;
            out             = { Yy * inv, -Yx * inv, -Xy * inv, Xx * inv, ( Xy * Dy - Yy * Dx ) * inv, ( Yx * Dx - Xx * Dy ) * inv };
            return true;
        }

        void Apply( const float x, const float y, float &outX, float &outY ) const
        {
            outX = Xx * x + Xy * y + Dx;
            outY = Yx * x + Yy * y + Dy;
        }
    };

    // Half open pixel ranges.
    struct Box
    {
        int X0 = 0;
        int Y0 = 0;
        int X1 = 0;
        int Y1 = 0;

        Box Intersect( const Box &o ) const
        {
            return { std::max( X0, o.X0 ), std::max( Y0, o.Y0 ), std::min( X1, o.X1 ), std::min( Y1, o.Y1 ) };
        }
    };

    struct Clip
    {
        std::vector<float> Coverage;
        Box                Bounds;
    };

    // A colour line resolved to sorted, premultiplied stops over [0, 1] of its offset range, see the COLR spec on extend modes.
    class ColorLine
    {
        std::vector<float> m_offsets;
        std::vector<Rgba>  m_colors;
        float              m_min    = 0.0f;
        float              m_max    = 0.0f;
        hb_paint_extend_t  m_extend = HB_PAINT_EXTEND_PAD;

    public:
        bool Load( hb_color_line_t *line, std::vector<hb_color_stop_t> &stops, bool &usesForeground )
        {
            unsigned int count = hb_color_line_get_color_stops( line, 0, nullptr, nullptr );
            stops.resize( count );
            hb_color_line_get_color_stops( line, 0, &count, stops.data( ) );
            if ( count == 0 )
            {
                return false;
            }
            std::stable_sort( stops.begin( ), stops.end( ), []( const hb_color_stop_t &a, const hb_color_stop_t &b ) { return a.offset < b.offset; } );
            m_min    = stops.front( ).offset;
            m_max    = stops.back( ).offset;
            m_extend = hb_color_line_get_extend( line );
            m_offsets.clear( );
            m_colors.clear( );
            for ( const hb_color_stop_t &stop : stops )
            {
                usesForeground = usesForeground || stop.is_foreground;
                m_offsets.push_back( m_max > m_min ? ( stop.offset - m_min ) / ( m_max - m_min ) : 0.0f );
                m_colors.push_back( Premultiply( stop.color ) );
            }
            return true;
        }

        Rgba At( const float t ) const
        {
            if ( !( m_max > m_min ) )
            {
                return t < m_min ? m_colors.front( ) : m_colors.back( );
            }
            float s = ( t - m_min ) / ( m_max - m_min );
            if ( m_extend == HB_PAINT_EXTEND_REPEAT )
            {
                s -= std::floor( s );
            }
            else if ( m_extend == HB_PAINT_EXTEND_REFLECT )
            {
                s = std::fmod( std::fabs( s ), 2.0f );
                s = s > 1.0f ? 2.0f - s : s;
            }
            if ( s <= m_offsets.front( ) )
            {
                return m_colors.front( );
            }
            for ( size_t i = 1; i < m_offsets.size( ); ++i )
            {
                if ( s < m_offsets[ i ] )
                {
                    return Lerp( m_colors[ i - 1 ], m_colors[ i ], ( s - m_offsets[ i - 1 ] ) / ( m_offsets[ i ] - m_offsets[ i - 1 ] ) );
                }
            }
            return m_colors.back( );
        }
    };

    float Lum( const float r, const float g, const float b )
    {
        return 0.3f * r + 0.59f * g + 0.11f * b;
    }

    void ClipColor( float c[ 3 ] )
    {
        const float l = Lum( c[ 0 ], c[ 1 ], c[ 2 ] );
        const float n = std::min( { c[ 0 ], c[ 1 ], c[ 2 ] } );
        const float x = std::max( { c[ 0 ], c[ 1 ], c[ 2 ] } );
        for ( int i = 0; i < 3; ++i )
        {
            if ( n < 0.0f )
            {
                c[ i ] = l + ( c[ i ] - l ) * l / ( l - n );
            }
            if ( x > 1.0f )
            {
                c[ i ] = l + ( c[ i ] - l ) * ( 1.0f - l ) / ( x - l );
            }
        }
    }

    void SetLum( float c[ 3 ], const float l )
    {
        const float d = l - Lum( c[ 0 ], c[ 1 ], c[ 2 ] );
        for ( int i = 0; i < 3; ++i )
        {
            c[ i ] += d;
        }
        ClipColor( c );
    }

    void SetSat( float c[ 3 ], const float s )
    {
        int order[ 3 ] = { 0, 1, 2 };
        std::sort( order, order + 3, [ c ]( const int a, const int b ) { return c[ a ] < c[ b ]; } );
        float &lo = c[ order[ 0 ] ], &mid = c[ order[ 1 ] ], &hi = c[ order[ 2 ] ];
        if ( hi > lo )
        {
            mid = ( mid - lo ) * s / ( hi - lo );
            hi  = s;
        }
        else
        {
            mid = hi = 0.0f;
        }
        lo = 0.0f;
    }

    float Sat( const float c[ 3 ] )
    {
        return std::max( { c[ 0 ], c[ 1 ], c[ 2 ] } ) - std::min( { c[ 0 ], c[ 1 ], c[ 2 ] } );
    }

    // W3C Compositing and Blending separable blend functions, cb is the backdrop and cs the source, both straight.
    float Blend( const hb_paint_composite_mode_t mode, const float cb, const float cs )
    {
        switch ( mode )
        {
        case HB_PAINT_COMPOSITE_MODE_MULTIPLY:
            return cb * cs;
        case HB_PAINT_COMPOSITE_MODE_SCREEN:
            return cb + cs - cb * cs;
        case HB_PAINT_COMPOSITE_MODE_OVERLAY:
            return Blend( HB_PAINT_COMPOSITE_MODE_HARD_LIGHT, cs, cb );
        case HB_PAINT_COMPOSITE_MODE_DARKEN:
            return std::min( cb, cs );
        case HB_PAINT_COMPOSITE_MODE_LIGHTEN:
            return std::max( cb, cs );
        case HB_PAINT_COMPOSITE_MODE_COLOR_DODGE:
            return cb <= 0.0f ? 0.0f : cs >= 1.0f ? 1.0f : std::min( 1.0f, cb / ( 1.0f - cs ) );
        case HB_PAINT_COMPOSITE_MODE_COLOR_BURN:
            return cb >= 1.0f ? 1.0f : cs <= 0.0f ? 0.0f : 1.0f - std::min( 1.0f, ( 1.0f - cb ) / cs );
        case HB_PAINT_COMPOSITE_MODE_HARD_LIGHT:
            return cs <= 0.5f ? cb * 2.0f * cs : Blend( HB_PAINT_COMPOSITE_MODE_SCREEN, cb, 2.0f * cs - 1.0f );
        case HB_PAINT_COMPOSITE_MODE_SOFT_LIGHT:
            {
                if ( cs <= 0.5f )
                {
                    return cb - ( 1.0f - 2.0f * cs ) * cb * ( 1.0f - cb );
                }
                const float d = cb <= 0.25f ? ( ( 16.0f * cb - 12.0f ) * cb + 4.0f ) * cb : std::sqrt( cb );
                return cb + ( 2.0f * cs - 1.0f ) * ( d - cb );
            }
        case HB_PAINT_COMPOSITE_MODE_DIFFERENCE:
            return std::fabs( cb - cs );
        case HB_PAINT_COMPOSITE_MODE_EXCLUSION:
            return cb + cs - 2.0f * cb * cs;
        default:
            return cs;
        }
    }

    Rgba Composite( const hb_paint_composite_mode_t mode, const Rgba &s, const Rgba &d )
    {
        switch ( mode )
        {
        case HB_PAINT_COMPOSITE_MODE_CLEAR:
            return { };
        case HB_PAINT_COMPOSITE_MODE_SRC:
            return s;
        case HB_PAINT_COMPOSITE_MODE_DEST:
            return d;
        case HB_PAINT_COMPOSITE_MODE_SRC_OVER:
            return s + d * ( 1.0f - s.A );
        case HB_PAINT_COMPOSITE_MODE_DEST_OVER:
            return s * ( 1.0f - d.A ) + d;
        case HB_PAINT_COMPOSITE_MODE_SRC_IN:
            return s * d.A;
        case HB_PAINT_COMPOSITE_MODE_DEST_IN:
            return d * s.A;
        case HB_PAINT_COMPOSITE_MODE_SRC_OUT:
            return s * ( 1.0f - d.A );
        case HB_PAINT_COMPOSITE_MODE_DEST_OUT:
            return d * ( 1.0f - s.A );
        case HB_PAINT_COMPOSITE_MODE_SRC_ATOP:
            return s * d.A + d * ( 1.0f - s.A );
        case HB_PAINT_COMPOSITE_MODE_DEST_ATOP:
            return s * ( 1.0f - d.A ) + d * s.A;
        case HB_PAINT_COMPOSITE_MODE_XOR:
            return s * ( 1.0f - d.A ) + d * ( 1.0f - s.A );
        case HB_PAINT_COMPOSITE_MODE_PLUS:
            return { std::min( 1.0f, s.R + d.R ), std::min( 1.0f, s.G + d.G ), std::min( 1.0f, s.B + d.B ), std::min( 1.0f, s.A + d.A ) };
        default:
            break;
        }

        // Blend modes: the blended colour where both are present, each one alone elsewhere.
        const float cs[ 3 ] = { s.A > 0.0f ? s.R / s.A : 0.0f, s.A > 0.0f ? s.G / s.A : 0.0f, s.A > 0.0f ? s.B / s.A : 0.0f };
        const float cb[ 3 ] = { d.A > 0.0f ? d.R / d.A : 0.0f, d.A > 0.0f ? d.G / d.A : 0.0f, d.A > 0.0f ? d.B / d.A : 0.0f };
        float       b[ 3 ];
        switch ( mode )
        {
        case HB_PAINT_COMPOSITE_MODE_HSL_HUE:
            std::copy_n( cs, 3, b );
            SetSat( b, Sat( cb ) );
            SetLum( b, Lum( cb[ 0 ], cb[ 1 ], cb[ 2 ] ) );
            break;
        case HB_PAINT_COMPOSITE_MODE_HSL_SATURATION:
            std::copy_n( cb, 3, b );
            SetSat( b, Sat( cs ) );
            SetLum( b, Lum( cb[ 0 ], cb[ 1 ], cb[ 2 ] ) );
            break;
        case HB_PAINT_COMPOSITE_MODE_HSL_COLOR:
            std::copy_n( cs, 3, b );
            SetLum( b, Lum( cb[ 0 ], cb[ 1 ], cb[ 2 ] ) );
            break;
        case HB_PAINT_COMPOSITE_MODE_HSL_LUMINOSITY:
            std::copy_n( cb, 3, b );
            SetLum( b, Lum( cs[ 0 ], cs[ 1 ], cs[ 2 ] ) );
            break;
        default:
            for ( int i = 0; i < 3; ++i )
            {
                b[ i ] = Blend( mode, cb[ i ], cs[ i ] );
            }
            break;
        }
        const float both = s.A * d.A;
        return { s.R * ( 1.0f - d.A ) + d.R * ( 1.0f - s.A ) + both * b[ 0 ], s.G * ( 1.0f - d.A ) + d.G * ( 1.0f - s.A ) + both * b[ 1 ],
                 s.B * ( 1.0f - d.A ) + d.B * ( 1.0f - s.A ) + both * b[ 2 ], s.A + d.A - both };
    }

    // Paints into a canvas whose pixel ( x, y ) covers [x, x + 1] x [y, y + 1] with y up. The base transform maps font units onto it.
    class Painter
    {
        int                            m_width;
        int                            m_height;
        float                          m_pixelsPerEm;
        std::vector<Affine>            m_transforms;
        std::vector<Clip>              m_clips;
        uint32_t                       m_clipCount = 0;
        std::vector<std::vector<Rgba>> m_layers;
        uint32_t                       m_layerCount = 1;
        std::vector<DenOfIz_Float4>    m_curves;
        std::vector<DenOfIz_UInt2>     m_bands;
        std::vector<hb_color_stop_t>   m_stops;
        ColorLine                      m_line;

    public:
        bool UsesForeground = false;

        Painter( const int width, const int height, const float pixelsPerEm, const Affine &base ) :
            m_width( width ), m_height( height ), m_pixelsPerEm( pixelsPerEm ), m_transforms( 1, base ), m_layers( 1, std::vector<Rgba>( static_cast<size_t>( width ) * height ) )
        {
        }

        const std::vector<Rgba> &Result( ) const
        {
            return m_layers.front( );
        }

        void PushTransform( const Affine &transform )
        {
            m_transforms.push_back( m_transforms.back( ).Then( transform ) );
        }

        void PopTransform( )
        {
            if ( m_transforms.size( ) > 1 )
            {
                m_transforms.pop_back( );
            }
        }

        void PushClipGlyph( const hb_codepoint_t glyph, hb_font_t *font )
        {
            const Affine &t = m_transforms.back( );
            m_curves.clear( );
            m_bands.clear( );
            // In ems of the canvas rather than pixels, so SlugRender's em-space sample bias keeps samples off the outline's points like on the GPU.
            const float     ppem    = m_pixelsPerEm;
            const SlugGlyph outline = CompileSlugGlyph( font, glyph, SlugTransform{ t.Xx, t.Yx, t.Xy, t.Yy, t.Dx, t.Dy, ppem }, m_curves, m_bands );
            const Box       bounds{ static_cast<int>( std::floor( outline.Bounds.X * ppem ) ) - 1, static_cast<int>( std::floor( outline.Bounds.Y * ppem ) ) - 1,
                              static_cast<int>( std::ceil( outline.Bounds.Z * ppem ) ) + 1, static_cast<int>( std::ceil( outline.Bounds.W * ppem ) ) + 1 };
            const bool      empty = outline.Bounds.Z <= outline.Bounds.X || outline.Bounds.W <= outline.Bounds.Y;
            PushClip( empty ? Box{ } : bounds,
                      [ & ]( const float x, const float y )
                      {
                          return SlugReference::SlugRender( m_curves, m_bands, SlugReference::float2{ x / ppem, y / ppem }, SlugReference::float2{ 1.0f / ppem, 1.0f / ppem },
                                                            outline );
                      } );
        }

        void PushClipRectangle( const float xmin, const float ymin, const float xmax, const float ymax )
        {
            const Affine &t = m_transforms.back( );
            float         cx[ 4 ], cy[ 4 ];
            t.Apply( xmin, ymin, cx[ 0 ], cy[ 0 ] );
            t.Apply( xmax, ymin, cx[ 1 ], cy[ 1 ] );
            t.Apply( xmax, ymax, cx[ 2 ], cy[ 2 ] );
            t.Apply( xmin, ymax, cx[ 3 ], cy[ 3 ] );
            const float x0 = *std::min_element( cx, cx + 4 ), x1 = *std::max_element( cx, cx + 4 );
            const float y0 = *std::min_element( cy, cy + 4 ), y1 = *std::max_element( cy, cy + 4 );
            const Box   bounds{ static_cast<int>( std::floor( x0 ) ), static_cast<int>( std::floor( y0 ) ), static_cast<int>( std::ceil( x1 ) ),
                              static_cast<int>( std::ceil( y1 ) ) };
            if ( t.Xy == 0.0f && t.Yx == 0.0f )
            {
                PushClip( bounds,
                          [ & ]( const float x, const float y )
                          {
                              const float w = std::clamp( std::min( x + 0.5f, x1 ) - std::max( x - 0.5f, x0 ), 0.0f, 1.0f );
                              const float h = std::clamp( std::min( y + 0.5f, y1 ) - std::max( y - 0.5f, y0 ), 0.0f, 1.0f );
                              return w * h;
                          } );
                return;
            }
            // Rotated or skewed: the share of 4x4 samples inside the rectangle.
            Affine inverse;
            if ( !t.Invert( inverse ) )
            {
                PushClip( Box{ }, []( float, float ) { return 0.0f; } );
                return;
            }
            PushClip( bounds,
                      [ & ]( const float x, const float y )
                      {
                          int inside = 0;
                          for ( int i = 0; i < 16; ++i )
                          {
                              float u, v;
                              inverse.Apply( x - 0.375f + static_cast<float>( i % 4 ) * 0.25f, y - 0.375f + static_cast<float>( i / 4 ) * 0.25f, u, v );
                              inside += u >= xmin && u <= xmax && v >= ymin && v <= ymax;
                          }
                          return static_cast<float>( inside ) / 16.0f;
                      } );
        }

        void PopClip( )
        {
            m_clipCount -= m_clipCount > 0 ? 1 : 0;
        }

        void PaintColor( const hb_bool_t isForeground, const hb_color_t color )
        {
            UsesForeground = UsesForeground || isForeground;
            const Rgba c   = Premultiply( color );
            Fill( [ & ]( float, float ) { return c; } );
        }

        void PaintLinear( hb_color_line_t *line, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2 )
        {
            // The gradient runs along p0 p1 projected onto the normal of p0 p2.
            float       dx = x1 - x0, dy = y1 - y0;
            const float qx = x2 - x0, qy = y2 - y0;
            const float q2 = qx * qx + qy * qy;
            if ( q2 > 1e-6f )
            {
                const float k = ( qx * dx + qy * dy ) / q2;
                dx -= k * qx;
                dy -= k * qy;
            }
            const float length2 = dx * dx + dy * dy;
            if ( length2 > 0.0f )
            {
                PaintGradient( line, [ & ]( const float x, const float y ) { return ( ( x - x0 ) * dx + ( y - y0 ) * dy ) / length2; } );
            }
        }

        // Two point conical: the largest t whose circle passes through the point with a radius of at least zero.
        void PaintRadial( hb_color_line_t *line, const float x0, const float y0, const float r0, const float x1, const float y1, const float r1 )
        {
            const float cdx = x1 - x0, cdy = y1 - y0, dr = r1 - r0;
            const float a = cdx * cdx + cdy * cdy - dr * dr;
            PaintGradient( line,
                           [ & ]( const float x, const float y )
                           {
                               const float px = x - x0, py = y - y0;
                               const float b = px * cdx + py * cdy + r0 * dr;
                               const float c = px * px + py * py - r0 * r0;
                               if ( std::fabs( a ) < 1e-6f )
                               {
                                   const float t = b != 0.0f ? c / ( 2.0f * b ) : NAN;
                                   return r0 + t * dr >= 0.0f ? t : NAN;
                               }
                               const float discriminant = b * b - a * c;
                               if ( discriminant < 0.0f )
                               {
                                   return NAN;
                               }
                               const float root = std::sqrt( discriminant );
                               const float t1   = std::max( ( b + root ) / a, ( b - root ) / a );
                               const float t2   = std::min( ( b + root ) / a, ( b - root ) / a );
                               return r0 + t1 * dr >= 0.0f ? t1 : r0 + t2 * dr >= 0.0f ? t2 : NAN;
                           } );
        }

        // Angles run counter clockwise from the x axis in [0, 2 pi), stop 0 at the start angle and 1 at the end angle.
        void PaintSweep( hb_color_line_t *line, const float cx, const float cy, const float start, const float end )
        {
            PaintGradient( line,
                           [ & ]( const float x, const float y )
                           {
                               float angle = std::atan2( y - cy, x - cx );
                               angle += angle < 0.0f ? TwoPi : 0.0f;
                               if ( end == start )
                               {
                                   return angle < start ? -INFINITY : INFINITY;
                               }
                               return ( angle - start ) / ( end - start );
                           } );
        }

        bool PaintImage( hb_blob_t *blob, const hb_tag_t format, const hb_glyph_extents_t *extents )
        {
            if ( format != HB_PAINT_IMAGE_FORMAT_PNG || extents == nullptr || extents->width == 0 || extents->height == 0 )
            {
                return false;
            }
            unsigned int length = 0;
            const char  *data   = hb_blob_get_data( blob, &length );
            int          width = 0, height = 0, channels = 0;
            stbi_uc     *rgba = stbi_load_from_memory( reinterpret_cast<const stbi_uc *>( data ), static_cast<int>( length ), &width, &height, &channels, 4 );
            if ( rgba == nullptr )
            {
                return false;
            }
            std::vector<Rgba> image( static_cast<size_t>( width ) * height );
            for ( size_t i = 0; i < image.size( ); ++i )
            {
                const stbi_uc *p = rgba + i * 4;
                const float    a = static_cast<float>( p[ 3 ] ) / 255.0f;
                image[ i ]       = { static_cast<float>( p[ 0 ] ) / 255.0f * a, static_cast<float>( p[ 1 ] ) / 255.0f * a, static_cast<float>( p[ 2 ] ) / 255.0f * a, a };
            }
            stbi_image_free( rgba );

            Affine inverse;
            if ( !m_transforms.back( ).Invert( inverse ) )
            {
                return true;
            }
            // The image fills the extents, its first row at the top, y_bearing.
            const float left = static_cast<float>( extents->x_bearing ), top = static_cast<float>( extents->y_bearing );
            const float sx    = static_cast<float>( width ) / static_cast<float>( extents->width );
            const float sy    = static_cast<float>( height ) / static_cast<float>( -extents->height );
            auto        texel = [ & ]( const int x, const int y ) { return x < 0 || y < 0 || x >= width || y >= height ? Rgba{ } : image[ static_cast<size_t>( y ) * width + x ]; };
            auto        sample = [ & ]( const float u, const float v )
            {
                const float fx = u - 0.5f, fy = v - 0.5f;
                const int   ix = static_cast<int>( std::floor( fx ) ), iy = static_cast<int>( std::floor( fy ) );
                const float tx = fx - static_cast<float>( ix ), ty = fy - static_cast<float>( iy );
                return Lerp( Lerp( texel( ix, iy ), texel( ix + 1, iy ), tx ), Lerp( texel( ix, iy + 1 ), texel( ix + 1, iy + 1 ), tx ), ty );
            };
            // Downscaling averages n x n bilinear samples per pixel, n covering the image pixels one canvas pixel spans.
            const float span = std::max( std::hypot( inverse.Xx * sx, inverse.Yx * sy ), std::hypot( inverse.Xy * sx, inverse.Yy * sy ) );
            const int   n    = std::clamp( static_cast<int>( std::ceil( span ) ), 1, 8 );
            const float step = 1.0f / static_cast<float>( n );
            Fill(
                [ & ]( const float x, const float y )
                {
                    Rgba sum;
                    for ( int j = 0; j < n; ++j )
                    {
                        for ( int i = 0; i < n; ++i )
                        {
                            float fx, fy;
                            inverse.Apply( x - 0.5f + ( static_cast<float>( i ) + 0.5f ) * step, y - 0.5f + ( static_cast<float>( j ) + 0.5f ) * step, fx, fy );
                            sum = sum + sample( ( fx - left ) * sx, ( top - fy ) * sy );
                        }
                    }
                    return sum * ( step * step );
                } );
            return true;
        }

        void PushGroup( )
        {
            if ( m_layerCount == m_layers.size( ) )
            {
                m_layers.emplace_back( );
            }
            m_layers[ m_layerCount++ ].assign( static_cast<size_t>( m_width ) * m_height, Rgba{ } );
        }

        void PopGroup( const hb_paint_composite_mode_t mode )
        {
            if ( m_layerCount < 2 )
            {
                return;
            }
            const std::vector<Rgba> &source = m_layers[ --m_layerCount ];
            std::vector<Rgba>       &target = m_layers[ m_layerCount - 1 ];
            ForEachPixel( [ & ]( const size_t i, const float coverage, float, float )
                          { target[ i ] = Lerp( target[ i ], Composite( mode, source[ i ], target[ i ] ), coverage ); } );
        }

    private:
        Box Canvas( ) const
        {
            return { 0, 0, m_width, m_height };
        }

        // coverageAt gets pixel centres and is only asked inside bounds, which is clipped to the current clip.
        template <typename CoverageAt>
        void PushClip( const Box &bounds, CoverageAt &&coverageAt )
        {
            const bool nested = m_clipCount > 0;
            const Box  parent = nested ? m_clips[ m_clipCount - 1 ].Bounds : Canvas( );
            if ( m_clipCount == m_clips.size( ) )
            {
                m_clips.emplace_back( );
            }
            Clip &clip = m_clips[ m_clipCount ];
            clip.Coverage.resize( static_cast<size_t>( m_width ) * m_height );
            clip.Bounds = bounds.Intersect( parent );
            for ( int y = clip.Bounds.Y0; y < clip.Bounds.Y1; ++y )
            {
                for ( int x = clip.Bounds.X0; x < clip.Bounds.X1; ++x )
                {
                    const size_t i     = static_cast<size_t>( y ) * m_width + x;
                    const float  outer = nested ? m_clips[ m_clipCount - 1 ].Coverage[ i ] : 1.0f;
                    clip.Coverage[ i ] = outer > 0.0f ? outer * coverageAt( static_cast<float>( x ) + 0.5f, static_cast<float>( y ) + 0.5f ) : 0.0f;
                }
            }
            ++m_clipCount;
        }

        template <typename Visit>
        void ForEachPixel( Visit &&visit ) const
        {
            const Clip *clip   = m_clipCount > 0 ? &m_clips[ m_clipCount - 1 ] : nullptr;
            const Box   bounds = clip != nullptr ? clip->Bounds : Canvas( );
            for ( int y = bounds.Y0; y < bounds.Y1; ++y )
            {
                for ( int x = bounds.X0; x < bounds.X1; ++x )
                {
                    const size_t i        = static_cast<size_t>( y ) * m_width + x;
                    const float  coverage = clip != nullptr ? clip->Coverage[ i ] : 1.0f;
                    if ( coverage > 0.0f )
                    {
                        visit( i, coverage, static_cast<float>( x ) + 0.5f, static_cast<float>( y ) + 0.5f );
                    }
                }
            }
        }

        template <typename ColorAt>
        void Fill( ColorAt &&colorAt )
        {
            std::vector<Rgba> &layer = m_layers[ m_layerCount - 1 ];
            ForEachPixel( [ & ]( const size_t i, const float coverage, const float x, const float y ) { Over( layer[ i ], colorAt( x, y ) * coverage ); } );
        }

        // paramAt gets gradient space points and returns the colour line parameter, NaN where the gradient paints nothing.
        template <typename ParamAt>
        void PaintGradient( hb_color_line_t *line, ParamAt &&paramAt )
        {
            Affine inverse;
            if ( !m_line.Load( line, m_stops, UsesForeground ) || !m_transforms.back( ).Invert( inverse ) )
            {
                return;
            }
            Fill(
                [ & ]( const float x, const float y )
                {
                    float gx, gy;
                    inverse.Apply( x, y, gx, gy );
                    const float t = paramAt( gx, gy );
                    return std::isnan( t ) ? Rgba{ } : m_line.At( t );
                } );
        }
    };

    Painter &Of( void *data )
    {
        return *static_cast<Painter *>( data );
    }

    hb_paint_funcs_t *PaintFuncs( )
    {
        static const struct Funcs
        {
            hb_paint_funcs_t *Value = hb_paint_funcs_create( );

            Funcs( )
            {
                hb_paint_funcs_set_push_transform_func(
                    Value, []( hb_paint_funcs_t *, void *data, const float xx, const float yx, const float xy, const float yy, const float dx, const float dy, void * )
                    { Of( data ).PushTransform( Affine{ xx, yx, xy, yy, dx, dy } ); }, nullptr, nullptr );
                hb_paint_funcs_set_pop_transform_func( Value, []( hb_paint_funcs_t *, void *data, void * ) { Of( data ).PopTransform( ); }, nullptr, nullptr );
                hb_paint_funcs_set_push_clip_glyph_func(
                    Value, []( hb_paint_funcs_t *, void *data, const hb_codepoint_t glyph, hb_font_t *font, void * ) { Of( data ).PushClipGlyph( glyph, font ); }, nullptr,
                    nullptr );
                hb_paint_funcs_set_push_clip_rectangle_func(
                    Value, []( hb_paint_funcs_t *, void *data, const float xmin, const float ymin, const float xmax, const float ymax, void * )
                    { Of( data ).PushClipRectangle( xmin, ymin, xmax, ymax ); }, nullptr, nullptr );
                hb_paint_funcs_set_pop_clip_func( Value, []( hb_paint_funcs_t *, void *data, void * ) { Of( data ).PopClip( ); }, nullptr, nullptr );
                hb_paint_funcs_set_color_func(
                    Value, []( hb_paint_funcs_t *, void *data, const hb_bool_t isForeground, const hb_color_t color, void * ) { Of( data ).PaintColor( isForeground, color ); },
                    nullptr, nullptr );
                hb_paint_funcs_set_image_func(
                    Value, []( hb_paint_funcs_t *, void *data, hb_blob_t *image, unsigned int, unsigned int, const hb_tag_t format, float, hb_glyph_extents_t *extents, void * )
                    { return static_cast<hb_bool_t>( Of( data ).PaintImage( image, format, extents ) ); }, nullptr, nullptr );
                hb_paint_funcs_set_linear_gradient_func(
                    Value,
                    []( hb_paint_funcs_t *, void *data, hb_color_line_t *line, const float x0, const float y0, const float x1, const float y1, const float x2, const float y2,
                        void * ) { Of( data ).PaintLinear( line, x0, y0, x1, y1, x2, y2 ); },
                    nullptr, nullptr );
                hb_paint_funcs_set_radial_gradient_func(
                    Value,
                    []( hb_paint_funcs_t *, void *data, hb_color_line_t *line, const float x0, const float y0, const float r0, const float x1, const float y1, const float r1,
                        void * ) { Of( data ).PaintRadial( line, x0, y0, r0, x1, y1, r1 ); },
                    nullptr, nullptr );
                hb_paint_funcs_set_sweep_gradient_func(
                    Value, []( hb_paint_funcs_t *, void *data, hb_color_line_t *line, const float x, const float y, const float start, const float end, void * )
                    { Of( data ).PaintSweep( line, x, y, start, end ); }, nullptr, nullptr );
                hb_paint_funcs_set_push_group_func( Value, []( hb_paint_funcs_t *, void *data, void * ) { Of( data ).PushGroup( ); }, nullptr, nullptr );
                hb_paint_funcs_set_pop_group_func(
                    Value, []( hb_paint_funcs_t *, void *data, const hb_paint_composite_mode_t mode, void * ) { Of( data ).PopGroup( mode ); }, nullptr, nullptr );
                hb_paint_funcs_make_immutable( Value );
            }
            ~Funcs( )
            {
                hb_paint_funcs_destroy( Value );
            }
        } funcs;
        return funcs.Value;
    }

    uint32_t Pack( const Rgba &c )
    {
        auto channel = []( const float v ) { return static_cast<uint32_t>( std::lround( std::clamp( v, 0.0f, 1.0f ) * 255.0f ) ); };
        return channel( c.R ) | channel( c.G ) << 8 | channel( c.B ) << 16 | channel( c.A ) << 24;
    }

    // ppem picks the bitmap strike, COLR glyphs are painted at any size.
    bool Paint( hb_font_t *font, const uint32_t glyphId, const uint32_t ppem, const float scale, const hb_color_t foreground, ColourBitmap &out )
    {
        hb_font_set_ppem( font, ppem, ppem );
        hb_glyph_extents_t extents{ };
        if ( !hb_font_get_glyph_extents( font, glyphId, &extents ) )
        {
            return false;
        }
        // A pixel of transparent border on every side, the GPU's bilinear filter fades the edges into it.
        const int x0    = static_cast<int>( std::floor( static_cast<float>( extents.x_bearing ) * scale ) ) - 1;
        const int y1    = static_cast<int>( std::ceil( static_cast<float>( extents.y_bearing ) * scale ) ) + 1;
        const int x1    = std::min( static_cast<int>( std::ceil( static_cast<float>( extents.x_bearing + extents.width ) * scale ) ) + 1, x0 + MaxCanvas );
        const int y0    = std::max( static_cast<int>( std::floor( static_cast<float>( extents.y_bearing + extents.height ) * scale ) ) - 1, y1 - MaxCanvas );
        const int width = x1 - x0, height = y1 - y0;

        Painter painter( width, height, scale * static_cast<float>( hb_face_get_upem( hb_font_get_face( font ) ) ),
                         Affine{ scale, 0.0f, 0.0f, scale, static_cast<float>( -x0 ), static_cast<float>( -y0 ) } );
        if ( !hb_font_paint_glyph_or_fail( font, glyphId, PaintFuncs( ), &painter, 0, foreground ) )
        {
            return false;
        }

        // Crop to the painted pixels plus the border.
        const std::vector<Rgba> &pixels = painter.Result( );
        Box                      used{ width, height, 0, 0 };
        for ( int y = 0; y < height; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                if ( pixels[ static_cast<size_t>( y ) * width + x ].A >= 0.5f / 255.0f )
                {
                    used = { std::min( used.X0, x ), std::min( used.Y0, y ), std::max( used.X1, x + 1 ), std::max( used.Y1, y + 1 ) };
                }
            }
        }
        out                = ColourBitmap{ };
        out.UsesForeground = painter.UsesForeground;
        if ( used.X1 <= used.X0 )
        {
            return true;
        }
        used       = Box{ used.X0 - 1, used.Y0 - 1, used.X1 + 1, used.Y1 + 1 }.Intersect( Box{ 0, 0, width, height } );
        out.Left   = x0 + used.X0;
        out.Top    = -( y0 + used.Y1 );
        out.Width  = static_cast<uint32_t>( used.X1 - used.X0 );
        out.Height = static_cast<uint32_t>( used.Y1 - used.Y0 );
        out.Pixels.reserve( static_cast<size_t>( out.Width ) * out.Height );
        for ( int y = used.Y1 - 1; y >= used.Y0; --y )
        {
            for ( int x = used.X0; x < used.X1; ++x )
            {
                out.Pixels.push_back( Pack( pixels[ static_cast<size_t>( y ) * width + x ] ) );
            }
        }
        return true;
    }
} // namespace

bool DenOfIz::RasteriseColourGlyph( const Font &font, const uint32_t glyphId, const uint32_t ppem, const hb_color_t foreground, ColourBitmap &out )
{
    hb_font_t *paintFont = hb_font_create( hb_font_get_face( font.HbFont( ) ) );
    hb_font_set_scale( paintFont, static_cast<int>( font.Upem( ) ), static_cast<int>( font.Upem( ) ) );
    const float scale = static_cast<float>( ppem ) / static_cast<float>( font.Upem( ) );
    // A bitmap strike can lack the glyph, the largest strike is tried then.
    const bool painted = Paint( paintFont, glyphId, ppem, scale, foreground, out ) || Paint( paintFont, glyphId, 0, scale, foreground, out );
    hb_font_destroy( paintFont );
    return painted;
}
