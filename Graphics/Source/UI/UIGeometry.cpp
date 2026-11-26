/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/UI/UIGeometry.h"

#include <algorithm>
#include <bit>
#include <cmath>

using namespace DenOfIz;

namespace
{
    constexpr float    HalfPi              = 1.57079632679489661923f;
    constexpr float    MaxExtent           = 1e18f;
    constexpr float    MaxMiterScale       = 2.0f;
    constexpr float    MinEpsilon          = 1e-4f;
    constexpr float    RelativeEpsilon     = 1e-6f;
    constexpr float    SolidMergeTolerance = 0.05f;
    constexpr uint32_t MinCornerSegments   = 2;
    constexpr uint32_t MaxCornerSegments   = 32;
    constexpr uint32_t MaxOutlinePoints    = 4 * ( MaxCornerSegments + 1 );
    constexpr uint32_t MaxBorderPairs      = 4 * ( MaxCornerSegments + 5 );

    constexpr float CornerSelectX[ 4 ] = { 0.0f, 1.0f, 1.0f, 0.0f };
    constexpr float CornerSelectY[ 4 ] = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr float ArcUX[ 4 ]         = { -1.0f, 0.0f, 1.0f, 0.0f };
    constexpr float ArcUY[ 4 ]         = { 0.0f, -1.0f, 0.0f, 1.0f };
    constexpr float ArcVX[ 4 ]         = { 0.0f, 1.0f, 0.0f, -1.0f };
    constexpr float ArcVY[ 4 ]         = { -1.0f, 0.0f, 1.0f, 0.0f };

    struct Vec2
    {
        float X;
        float Y;
    };

    Vec2 operator+( const Vec2 &a, const Vec2 &b )
    {
        return Vec2{ a.X + b.X, a.Y + b.Y };
    }

    Vec2 operator-( const Vec2 &a, const Vec2 &b )
    {
        return Vec2{ a.X - b.X, a.Y - b.Y };
    }

    Vec2 operator*( const Vec2 &a, const float s )
    {
        return Vec2{ a.X * s, a.Y * s };
    }

    float Dot( const Vec2 &a, const Vec2 &b )
    {
        return a.X * b.X + a.Y * b.Y;
    }

    float Length2( const Vec2 &a )
    {
        return Dot( a, a );
    }

    bool IsPositiveFinite( const float value )
    {
        return value > 0.0f && std::isfinite( value );
    }

    float Sanitize( const float value )
    {
        return value > 0.0f ? std::min( value, MaxExtent ) : 0.0f;
    }

    float SquaredEpsilon( const float width, const float height )
    {
        const float epsilon = std::max( MinEpsilon, RelativeEpsilon * std::max( width, height ) );
        return epsilon * epsilon;
    }

    float Coverage( const float thickness, const float feather )
    {
        if ( feather <= 0.0f )
        {
            return thickness > 0.0f ? 1.0f : 0.0f;
        }
        return std::clamp( thickness / feather, 0.0f, 1.0f );
    }

    struct CornerFrame
    {
        Vec2 Point;
        Vec2 Inward;
    };

    CornerFrame GetCornerFrame( const uint32_t corner, const float width, const float height )
    {
        const float sx = CornerSelectX[ corner ];
        const float sy = CornerSelectY[ corner ];
        return CornerFrame{ Vec2{ sx * width, sy * height }, Vec2{ 1.0f - 2.0f * sx, 1.0f - 2.0f * sy } };
    }

    struct ArcTable
    {
        float Cos[ MaxCornerSegments + 1 ];
        float Sin[ MaxCornerSegments + 1 ];
    };

    uint32_t CornerSegments( const float radius, const float feather )
    {
        if ( radius <= 0.0f )
        {
            return 0;
        }
        const float maxError = 0.25f * ( feather > 0.0f ? feather : 1.0f );
        if ( radius <= maxError )
        {
            return MinCornerSegments;
        }
        const float segments = std::ceil( HalfPi / std::acos( 1.0f - maxError / radius ) );
        return static_cast<uint32_t>( std::clamp( segments, static_cast<float>( MinCornerSegments ), static_cast<float>( MaxCornerSegments ) ) );
    }

    void BuildArcTable( const uint32_t segments, ArcTable &table )
    {
        const float step = HalfPi / static_cast<float>( segments );
        for ( uint32_t i = 1; i < segments; ++i )
        {
            const float angle = step * static_cast<float>( i );
            table.Cos[ i ]    = std::cos( angle );
            table.Sin[ i ]    = std::sin( angle );
        }
        table.Cos[ 0 ]        = 1.0f;
        table.Sin[ 0 ]        = 0.0f;
        table.Cos[ segments ] = 0.0f;
        table.Sin[ segments ] = 1.0f;
    }

    Vec2 ArcPoint( const uint32_t corner, const Vec2 &center, const float rx, const float ry, const float cosine, const float sine )
    {
        return Vec2{ center.X + ( ArcUX[ corner ] * cosine + ArcVX[ corner ] * sine ) * rx, center.Y + ( ArcUY[ corner ] * cosine + ArcVY[ corner ] * sine ) * ry };
    }

    void LimitScale( const float extent, const float a, const float b, float &scale )
    {
        const float sum = a + b;
        if ( sum > extent )
        {
            scale = std::min( scale, extent / sum );
        }
    }

    float RadiusScale( const float width, const float height, const float rx[ 4 ], const float ry[ 4 ] )
    {
        float scale = 1.0f;
        LimitScale( width, rx[ 0 ], rx[ 1 ], scale );
        LimitScale( width, rx[ 3 ], rx[ 2 ], scale );
        LimitScale( height, ry[ 0 ], ry[ 3 ], scale );
        LimitScale( height, ry[ 1 ], ry[ 2 ], scale );
        return scale;
    }

    bool ClampRadii( const float width, const float height, const DenOfIz_UICorners &radius, float outRadii[ 4 ] )
    {
        outRadii[ 0 ]     = Sanitize( radius.TopLeft );
        outRadii[ 1 ]     = Sanitize( radius.TopRight );
        outRadii[ 2 ]     = Sanitize( radius.BottomRight );
        outRadii[ 3 ]     = Sanitize( radius.BottomLeft );
        const float scale = RadiusScale( width, height, outRadii, outRadii );
        bool        any   = false;
        for ( uint32_t corner = 0; corner < 4; ++corner )
        {
            outRadii[ corner ] *= scale;
            any = any || outRadii[ corner ] > 0.0f;
        }
        return any;
    }

    void AppendQuad( std::vector<uint32_t> &indices, const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t d )
    {
        indices.insert( indices.end( ), { a, b, c, a, c, d } );
    }

    float Cross( const UIShapeVertex &a, const UIShapeVertex &b, const UIShapeVertex &c )
    {
        return ( b.X - a.X ) * ( c.Y - a.Y ) - ( b.Y - a.Y ) * ( c.X - a.X );
    }

    bool SamePosition( const UIShapeVertex &a, const UIShapeVertex &b )
    {
        return a.X == b.X && a.Y == b.Y;
    }

    void AppendQuad( UIShape &shape, const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t d, const bool keepEdgeAB )
    {
        const auto &v = shape.Vertices;
        bool        diagonalAC;
        if ( SamePosition( v[ b ], v[ c ] ) )
        {
            diagonalAC = !keepEdgeAB;
        }
        else if ( SamePosition( v[ a ], v[ d ] ) )
        {
            diagonalAC = keepEdgeAB;
        }
        else
        {
            diagonalAC = Cross( v[ a ], v[ b ], v[ c ] ) * Cross( v[ a ], v[ c ], v[ d ] ) >= 0.0f;
        }
        if ( diagonalAC )
        {
            AppendQuad( shape.Indices, a, b, c, d );
            return;
        }
        shape.Indices.insert( shape.Indices.end( ), { a, b, d, b, c, d } );
    }

    void EmitRect( const float width, const float height, UIShape &outShape )
    {
        outShape.Vertices.assign(
            { UIShapeVertex{ 0.0f, 0.0f, 1.0f }, UIShapeVertex{ width, 0.0f, 1.0f }, UIShapeVertex{ width, height, 1.0f }, UIShapeVertex{ 0.0f, height, 1.0f } } );
        outShape.Indices.assign( { 0, 1, 2, 0, 2, 3 } );
    }

    void EmitSharpBorder( const float width, const float height, const float sides[ 4 ], UIShape &outShape )
    {
        const float left   = sides[ 3 ];
        const float top    = sides[ 0 ];
        const float right  = sides[ 1 ];
        const float bottom = sides[ 2 ];
        outShape.Vertices.assign( { UIShapeVertex{ 0.0f, 0.0f, 1.0f }, UIShapeVertex{ width, 0.0f, 1.0f }, UIShapeVertex{ width, height, 1.0f },
                                    UIShapeVertex{ 0.0f, height, 1.0f }, UIShapeVertex{ left, top, 1.0f }, UIShapeVertex{ width - right, top, 1.0f },
                                    UIShapeVertex{ width - right, height - bottom, 1.0f }, UIShapeVertex{ left, height - bottom, 1.0f } } );
        outShape.Indices.reserve( 24 );
        for ( uint32_t side = 0; side < 4; ++side )
        {
            if ( sides[ side ] > 0.0f )
            {
                const uint32_t next = ( side + 1 ) & 3;
                AppendQuad( outShape.Indices, side, next, 4 + next, 4 + side );
            }
        }
    }

    struct Outline
    {
        Vec2     Points[ MaxOutlinePoints ];
        uint8_t  Corner[ MaxOutlinePoints ];
        uint32_t Count = 0;

        void Append( const Vec2 &point, const uint32_t corner, const float epsilon2 )
        {
            if ( Count > 0 && Length2( point - Points[ Count - 1 ] ) <= epsilon2 )
            {
                return;
            }
            Points[ Count ] = point;
            Corner[ Count ] = static_cast<uint8_t>( corner );
            ++Count;
        }
    };

    void BuildOutline( const float width, const float height, const float radii[ 4 ], const float feather, const float epsilon2, Outline &outline )
    {
        ArcTable table;
        for ( uint32_t corner = 0; corner < 4; ++corner )
        {
            const CornerFrame frame    = GetCornerFrame( corner, width, height );
            const float       r        = radii[ corner ];
            const uint32_t    segments = CornerSegments( r, feather );
            if ( segments == 0 )
            {
                outline.Append( frame.Point, corner, epsilon2 );
                continue;
            }
            BuildArcTable( segments, table );
            const Vec2 center = frame.Point + frame.Inward * r;
            for ( uint32_t i = 0; i <= segments; ++i )
            {
                outline.Append( ArcPoint( corner, center, r, r, table.Cos[ i ], table.Sin[ i ] ), corner, epsilon2 );
            }
        }
        while ( outline.Count > 1 && Length2( outline.Points[ outline.Count - 1 ] - outline.Points[ 0 ] ) <= epsilon2 )
        {
            --outline.Count;
        }
    }

    Vec2 InsetPoint( const Vec2 &point, const Vec2 &normal, const float distance, const uint32_t corner, const float radius, const float width, const float height )
    {
        if ( radius > 0.0f && radius <= distance )
        {
            const CornerFrame frame = GetCornerFrame( corner, width, height );
            return frame.Point + frame.Inward * distance;
        }
        return point - normal * distance;
    }

    void ComputeNormals( const Vec2 *points, const uint32_t count, const float epsilon2, Vec2 *outNormals )
    {
        Vec2     edgeNormals[ MaxBorderPairs ];
        bool     edgeValid[ MaxBorderPairs ];
        uint32_t incoming[ MaxBorderPairs ];
        uint32_t firstValid = count;
        uint32_t lastValid  = count;
        for ( uint32_t i = 0; i < count; ++i )
        {
            const Vec2  delta   = points[ i + 1 < count ? i + 1 : 0 ] - points[ i ];
            const float length2 = Length2( delta );
            edgeValid[ i ]      = length2 > epsilon2;
            if ( edgeValid[ i ] )
            {
                const float invLength = 1.0f / std::sqrt( length2 );
                edgeNormals[ i ]      = Vec2{ delta.Y * invLength, -delta.X * invLength };
                firstValid            = std::min( firstValid, i );
                lastValid             = i;
            }
        }
        if ( firstValid == count )
        {
            std::fill_n( outNormals, count, Vec2{ 0.0f, 0.0f } );
            return;
        }

        uint32_t previous = lastValid;
        for ( uint32_t i = 0; i < count; ++i )
        {
            incoming[ i ] = previous;
            if ( edgeValid[ i ] )
            {
                previous = i;
            }
        }
        uint32_t next = firstValid;
        for ( uint32_t i = count; i-- > 0; )
        {
            if ( edgeValid[ i ] )
            {
                next = i;
            }
            const Vec2  average = ( edgeNormals[ incoming[ i ] ] + edgeNormals[ next ] ) * 0.5f;
            const float length2 = Length2( average );
            outNormals[ i ]     = average * ( length2 > 1.0f / MaxMiterScale ? 1.0f / length2 : MaxMiterScale );
        }
    }

    enum class PairKind : uint8_t
    {
        Smooth,
        SharpInner,
        Split
    };

    struct BorderPairs
    {
        Vec2     Outer[ MaxBorderPairs ];
        Vec2     Inner[ MaxBorderPairs ];
        float    Thickness[ MaxBorderPairs ];
        uint8_t  Corner[ MaxBorderPairs ];
        PairKind Kind[ MaxBorderPairs ];
        uint32_t Count = 0;

        bool IsDuplicate( const uint32_t a, const Vec2 &outer, const Vec2 &inner, const float epsilon2 ) const
        {
            return Kind[ a ] != PairKind::Split && Length2( outer - Outer[ a ] ) <= epsilon2 && Length2( inner - Inner[ a ] ) <= epsilon2;
        }

        void Push( const Vec2 &outer, const Vec2 &inner, const float thickness, const uint32_t corner, const PairKind kind, const float epsilon2 )
        {
            if ( kind != PairKind::Split && Count > 0 && IsDuplicate( Count - 1, outer, inner, epsilon2 ) )
            {
                return;
            }
            Outer[ Count ]     = outer;
            Inner[ Count ]     = inner;
            Thickness[ Count ] = thickness;
            Corner[ Count ]    = static_cast<uint8_t>( corner );
            Kind[ Count ]      = kind;
            ++Count;
        }
    };

    struct BorderLayout
    {
        float Width;
        float Height;
        float Feather;
        float Epsilon2;
        float Radii[ 4 ];
        float InnerRx[ 4 ];
        float InnerRy[ 4 ];
        float SideX[ 4 ];
        float SideY[ 4 ];
    };

    float DistanceToContour( const Vec2 &point, const Vec2 *contour, const uint32_t count )
    {
        float best2 = Length2( point - contour[ 0 ] );
        for ( uint32_t i = 0; i < count; ++i )
        {
            const Vec2  a       = contour[ i ];
            const Vec2  edge    = contour[ i + 1 < count ? i + 1 : 0 ] - a;
            const float length2 = Length2( edge );
            const float t       = length2 > 0.0f ? std::clamp( Dot( point - a, edge ) / length2, 0.0f, 1.0f ) : 0.0f;
            best2               = std::min( best2, Length2( point - ( a + edge * t ) ) );
        }
        return std::sqrt( best2 );
    }

    float QuadrantDistance( const Vec2 &point, const Vec2 &corner, const Vec2 &inward )
    {
        const float dx = std::max( 0.0f, ( corner.X - point.X ) * inward.X );
        const float dy = std::max( 0.0f, ( corner.Y - point.Y ) * inward.Y );
        return std::sqrt( dx * dx + dy * dy );
    }

    Vec2 EdgePoint( const CornerFrame &frame, const bool vertical, const float distance )
    {
        return vertical ? Vec2{ frame.Point.X, frame.Point.Y + frame.Inward.Y * distance } : Vec2{ frame.Point.X + frame.Inward.X * distance, frame.Point.Y };
    }

    void BuildBorderPairs( const BorderLayout &layout, BorderPairs &pairs )
    {
        const float halfFeather = 0.5f * layout.Feather;
        ArcTable    table;
        for ( uint32_t corner = 0; corner < 4; ++corner )
        {
            const CornerFrame frame            = GetCornerFrame( corner, layout.Width, layout.Height );
            const bool        verticalIncoming = ( corner & 1 ) == 0;
            const float       sideX            = layout.SideX[ corner ];
            const float       sideY            = layout.SideY[ corner ];
            const float       inWidth          = verticalIncoming ? sideX : sideY;
            const float       outWidth         = verticalIncoming ? sideY : sideX;
            const float       inLength         = verticalIncoming ? layout.Height : layout.Width;
            const float       outLength        = verticalIncoming ? layout.Width : layout.Height;
            const float       r                = layout.Radii[ corner ];
            const float       previousRadius   = layout.Radii[ ( corner + 3 ) & 3 ];
            const float       nextRadius       = layout.Radii[ ( corner + 1 ) & 3 ];
            const bool        sharpInner       = layout.InnerRx[ corner ] <= 0.0f;
            const Vec2        innerCorner      = Vec2{ frame.Point.X + frame.Inward.X * sideX, frame.Point.Y + frame.Inward.Y * sideY };
            const bool        split            = layout.Feather > 0.0f && sharpInner && Coverage( inWidth, layout.Feather ) != Coverage( outWidth, layout.Feather );

            if ( split && outWidth > r && outWidth <= inLength - previousRadius )
            {
                const float farDistance  = std::min( outWidth + halfFeather, inLength - previousRadius );
                const float nearDistance = std::max( outWidth - halfFeather, r );
                pairs.Push( EdgePoint( frame, verticalIncoming, farDistance ), innerCorner, inWidth, corner, PairKind::Split, layout.Epsilon2 );
                pairs.Push( EdgePoint( frame, verticalIncoming, nearDistance ), innerCorner, outWidth, corner, PairKind::Split, layout.Epsilon2 );
            }

            const uint32_t segments = CornerSegments( r, layout.Feather );
            if ( segments == 0 )
            {
                pairs.Push( frame.Point, innerCorner, std::max( inWidth, outWidth ), corner, PairKind::SharpInner, layout.Epsilon2 );
            }
            else
            {
                BuildArcTable( segments, table );
                const float rx          = layout.InnerRx[ corner ];
                const float ry          = layout.InnerRy[ corner ];
                const Vec2  outerCenter = frame.Point + frame.Inward * r;
                const Vec2  innerCenter = Vec2{ frame.Point.X + frame.Inward.X * ( sideX + rx ), frame.Point.Y + frame.Inward.Y * ( sideY + ry ) };
                for ( uint32_t i = 0; i <= segments; ++i )
                {
                    const Vec2 outer = ArcPoint( corner, outerCenter, r, r, table.Cos[ i ], table.Sin[ i ] );
                    if ( sharpInner )
                    {
                        pairs.Push( outer, innerCorner, QuadrantDistance( outer, innerCorner, frame.Inward ), corner, PairKind::SharpInner, layout.Epsilon2 );
                    }
                    else
                    {
                        pairs.Push( outer, ArcPoint( corner, innerCenter, rx, ry, table.Cos[ i ], table.Sin[ i ] ), 0.0f, corner, PairKind::Smooth, layout.Epsilon2 );
                    }
                }
            }

            if ( split && inWidth > r && inWidth <= outLength - nextRadius )
            {
                const float nearDistance = std::max( inWidth - halfFeather, r );
                const float farDistance  = std::min( inWidth + halfFeather, outLength - nextRadius );
                pairs.Push( EdgePoint( frame, !verticalIncoming, nearDistance ), innerCorner, inWidth, corner, PairKind::Split, layout.Epsilon2 );
                pairs.Push( EdgePoint( frame, !verticalIncoming, farDistance ), innerCorner, outWidth, corner, PairKind::Split, layout.Epsilon2 );
            }
        }
        while ( pairs.Count > 1 && pairs.Kind[ pairs.Count - 1 ] != PairKind::Split &&
                pairs.IsDuplicate( 0, pairs.Outer[ pairs.Count - 1 ], pairs.Inner[ pairs.Count - 1 ], layout.Epsilon2 ) )
        {
            --pairs.Count;
        }
    }
} // namespace

bool UIShapeKey::operator==( const UIShapeKey &other ) const
{
    const auto bits = []( const float value ) { return std::bit_cast<uint32_t>( value ); };
    return bits( Width ) == bits( other.Width ) && bits( Height ) == bits( other.Height ) && bits( Radius.TopLeft ) == bits( other.Radius.TopLeft ) &&
           bits( Radius.TopRight ) == bits( other.Radius.TopRight ) && bits( Radius.BottomRight ) == bits( other.Radius.BottomRight ) &&
           bits( Radius.BottomLeft ) == bits( other.Radius.BottomLeft ) && bits( Border.Left ) == bits( other.Border.Left ) && bits( Border.Top ) == bits( other.Border.Top ) &&
           bits( Border.Right ) == bits( other.Border.Right ) && bits( Border.Bottom ) == bits( other.Border.Bottom ) && bits( Feather ) == bits( other.Feather ) &&
           IsBorder == other.IsBorder;
}

size_t UIShapeKeyHash::operator( )( const UIShapeKey &key ) const
{
    const auto pack = []( const float a, const float b )
    { return static_cast<uint64_t>( std::bit_cast<uint32_t>( a ) ) | static_cast<uint64_t>( std::bit_cast<uint32_t>( b ) ) << 32; };
    const auto mix = []( uint64_t h )
    {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    };
    uint64_t h = key.IsBorder ? 0x9e3779b97f4a7c15ULL : 0x6a09e667f3bcc909ULL;
    h          = mix( h ^ pack( key.Width, key.Height ) );
    h          = mix( h ^ pack( key.Radius.TopLeft, key.Radius.TopRight ) );
    h          = mix( h ^ pack( key.Radius.BottomRight, key.Radius.BottomLeft ) );
    h          = mix( h ^ pack( key.Border.Left, key.Border.Top ) );
    h          = mix( h ^ pack( key.Border.Right, key.Border.Bottom ) );
    h          = mix( h ^ std::bit_cast<uint32_t>( key.Feather ) );
    return static_cast<size_t>( h );
}

void UIGeometry::Fill( const float width, const float height, const DenOfIz_UICorners &radius, const float feather, UIShape &outShape )
{
    outShape.Vertices.clear( );
    outShape.Indices.clear( );
    if ( !IsPositiveFinite( width ) || !IsPositiveFinite( height ) )
    {
        return;
    }

    float radii[ 4 ];
    if ( !ClampRadii( width, height, radius, radii ) )
    {
        EmitRect( width, height, outShape );
        return;
    }

    const float aa       = IsPositiveFinite( feather ) ? feather : 0.0f;
    const float epsilon2 = SquaredEpsilon( width, height );
    Outline     outline;
    BuildOutline( width, height, radii, aa, epsilon2, outline );
    const uint32_t count = outline.Count;
    if ( count < 3 )
    {
        EmitRect( width, height, outShape );
        return;
    }

    const Vec2 *points = outline.Points;
    Vec2        normals[ MaxOutlinePoints ];
    ComputeNormals( points, count, epsilon2, normals );

    const float halfFeather = 0.5f * aa;
    const float minExtent   = std::min( width, height );
    const float inset       = std::min( halfFeather, minExtent - halfFeather );
    const float coverage    = Coverage( minExtent, aa );
    const bool  fringe      = halfFeather > 0.0f;

    outShape.Vertices.reserve( fringe ? 2 * count : count );
    outShape.Indices.reserve( 3 * ( count - 2 ) + ( fringe ? 6 * count : 0 ) );
    for ( uint32_t i = 0; i < count; ++i )
    {
        const uint32_t corner = outline.Corner[ i ];
        const Vec2     p      = InsetPoint( points[ i ], normals[ i ], inset, corner, radii[ corner ], width, height );
        outShape.Vertices.push_back( UIShapeVertex{ p.X, p.Y, coverage } );
    }
    for ( uint32_t i = 1; i + 1 < count; ++i )
    {
        outShape.Indices.insert( outShape.Indices.end( ), { 0, i, i + 1 } );
    }
    if ( !fringe )
    {
        return;
    }
    for ( uint32_t i = 0; i < count; ++i )
    {
        const Vec2 p = points[ i ] + normals[ i ] * halfFeather;
        outShape.Vertices.push_back( UIShapeVertex{ p.X, p.Y, 0.0f } );
    }
    for ( uint32_t i = 0; i < count; ++i )
    {
        const uint32_t next = i + 1 < count ? i + 1 : 0;
        AppendQuad( outShape, i, next, count + next, count + i, true );
    }
}

void UIGeometry::Border( const float width, const float height, const DenOfIz_UIEdges &border, const DenOfIz_UICorners &radius, const float feather, UIShape &outShape )
{
    outShape.Vertices.clear( );
    outShape.Indices.clear( );
    if ( !IsPositiveFinite( width ) || !IsPositiveFinite( height ) )
    {
        return;
    }

    const float left   = Sanitize( border.Left );
    const float top    = Sanitize( border.Top );
    const float right  = Sanitize( border.Right );
    const float bottom = Sanitize( border.Bottom );
    if ( left <= 0.0f && top <= 0.0f && right <= 0.0f && bottom <= 0.0f )
    {
        return;
    }
    if ( left + right >= width || top + bottom >= height )
    {
        Fill( width, height, radius, feather, outShape );
        return;
    }

    BorderLayout layout{ };
    if ( !ClampRadii( width, height, radius, layout.Radii ) )
    {
        const float sides[ 4 ] = { top, right, bottom, left };
        EmitSharpBorder( width, height, sides, outShape );
        return;
    }

    layout.Width    = width;
    layout.Height   = height;
    layout.Feather  = IsPositiveFinite( feather ) ? feather : 0.0f;
    layout.Epsilon2 = SquaredEpsilon( width, height );
    for ( uint32_t corner = 0; corner < 4; ++corner )
    {
        layout.SideX[ corner ]   = corner == 0 || corner == 3 ? left : right;
        layout.SideY[ corner ]   = corner < 2 ? top : bottom;
        const float rx           = layout.Radii[ corner ] - layout.SideX[ corner ];
        const float ry           = layout.Radii[ corner ] - layout.SideY[ corner ];
        const bool  rounded      = rx > 0.0f && ry > 0.0f;
        layout.InnerRx[ corner ] = rounded ? rx : 0.0f;
        layout.InnerRy[ corner ] = rounded ? ry : 0.0f;
    }
    const float innerScale = RadiusScale( width - left - right, height - top - bottom, layout.InnerRx, layout.InnerRy );
    const bool  concentric = innerScale >= 1.0f;
    for ( uint32_t corner = 0; !concentric && corner < 4; ++corner )
    {
        layout.InnerRx[ corner ] *= innerScale;
        layout.InnerRy[ corner ] *= innerScale;
    }

    BorderPairs pairs;
    BuildBorderPairs( layout, pairs );
    const uint32_t count = pairs.Count;
    if ( count < 3 )
    {
        Fill( width, height, radius, feather, outShape );
        return;
    }

    Vec2 outerNormals[ MaxBorderPairs ];
    Vec2 innerNormals[ MaxBorderPairs ];
    ComputeNormals( pairs.Outer, count, layout.Epsilon2, outerNormals );
    const auto unitOuterNormal = [ & ]( const uint32_t i )
    {
        const float length2 = Length2( outerNormals[ i ] );
        return length2 > 0.0f ? outerNormals[ i ] * ( 1.0f / std::sqrt( length2 ) ) : Vec2{ 0.0f, 0.0f };
    };
    if ( !concentric )
    {
        const float tolerance = std::sqrt( layout.Epsilon2 );
        for ( uint32_t i = 0; i < count; ++i )
        {
            if ( pairs.Kind[ i ] != PairKind::Split && Dot( pairs.Inner[ i ] - pairs.Outer[ i ], unitOuterNormal( i ) ) > tolerance )
            {
                pairs.Inner[ i ] = pairs.Outer[ i ];
                pairs.Kind[ i ]  = PairKind::Smooth;
            }
        }
    }
    ComputeNormals( pairs.Inner, count, layout.Epsilon2, innerNormals );

    const float halfFeather = 0.5f * layout.Feather;
    float       coverages[ MaxBorderPairs ];
    outShape.Vertices.reserve( 4 * count );
    outShape.Indices.reserve( 18 * count );
    for ( uint32_t i = 0; i < count; ++i )
    {
        const Vec2 &outer = pairs.Outer[ i ];
        const Vec2 &inner = pairs.Inner[ i ];
        if ( !concentric && pairs.Kind[ i ] != PairKind::Split )
        {
            pairs.Thickness[ i ] = DistanceToContour( outer, pairs.Inner, count );
        }
        else if ( pairs.Kind[ i ] == PairKind::Smooth )
        {
            pairs.Thickness[ i ] = std::max( 0.0f, Dot( outer - inner, unitOuterNormal( i ) ) );
        }
        const float thickness = pairs.Thickness[ i ];
        const float coverage  = Coverage( thickness, layout.Feather );
        coverages[ i ]        = coverage;

        const PairKind    kind     = pairs.Kind[ i ];
        const uint32_t    corner   = pairs.Corner[ i ];
        const CornerFrame frame    = GetCornerFrame( corner, width, height );
        const float       depth    = std::min( halfFeather, thickness - halfFeather );
        const Vec2        outerOut = outer + outerNormals[ i ] * halfFeather;
        Vec2 outerIn = kind == PairKind::Split ? outer - outerNormals[ i ] * depth : InsetPoint( outer, outerNormals[ i ], depth, corner, layout.Radii[ corner ], width, height );
        const float innerR   = std::min( layout.InnerRx[ corner ], layout.InnerRy[ corner ] );
        Vec2        innerOut = inner - innerNormals[ i ] * halfFeather;
        Vec2        innerIn  = inner + innerNormals[ i ] * depth;
        if ( kind == PairKind::Smooth && innerR > 0.0f && innerR <= halfFeather )
        {
            const Vec2 innerCorner = Vec2{ frame.Point.X + frame.Inward.X * layout.SideX[ corner ], frame.Point.Y + frame.Inward.Y * layout.SideY[ corner ] };
            innerOut               = innerCorner + frame.Inward * halfFeather;
        }
        if ( kind != PairKind::Smooth )
        {
            const float depthX = std::min( depth, layout.SideX[ corner ] - halfFeather );
            const float depthY = std::min( depth, layout.SideY[ corner ] - halfFeather );
            innerIn            = Vec2{ inner.X - frame.Inward.X * depthX, inner.Y - frame.Inward.Y * depthY };
        }
        if ( std::fabs( thickness - layout.Feather ) <= SolidMergeTolerance * layout.Feather || Dot( innerIn - outerIn, inner - outer ) < 0.0f )
        {
            outerIn = ( outerIn + innerIn ) * 0.5f;
            innerIn = outerIn;
        }
        outShape.Vertices.insert( outShape.Vertices.end( ), { UIShapeVertex{ outerOut.X, outerOut.Y, 0.0f }, UIShapeVertex{ outerIn.X, outerIn.Y, coverage },
                                                              UIShapeVertex{ innerIn.X, innerIn.Y, coverage }, UIShapeVertex{ innerOut.X, innerOut.Y, 0.0f } } );
    }

    for ( uint32_t i = 0; i < count; ++i )
    {
        const uint32_t next = i + 1 < count ? i + 1 : 0;
        if ( coverages[ i ] <= 0.0f && coverages[ next ] <= 0.0f )
        {
            continue;
        }
        const uint32_t a = 4 * i;
        const uint32_t b = 4 * next;
        for ( uint32_t ring = 0; ring < 3; ++ring )
        {
            AppendQuad( outShape, a + ring, b + ring, b + ring + 1, a + ring + 1, ring != 0 );
        }
    }
}

const UIShape &UIShapeCache::GetFill( const float width, const float height, const DenOfIz_UICorners &radius, const float feather, const uint64_t frame )
{
    const UIShapeKey key{ width, height, radius, DenOfIz_UIEdges{ }, feather, false };
    auto [ it, inserted ] = m_shapes.try_emplace( key );
    if ( inserted )
    {
        UIGeometry::Fill( width, height, radius, feather, it->second );
    }
    it->second.LastUsedFrame = frame;
    return it->second;
}

const UIShape &UIShapeCache::GetBorder( const float width, const float height, const DenOfIz_UIEdges &border, const DenOfIz_UICorners &radius, const float feather,
                                        const uint64_t frame )
{
    const UIShapeKey key{ width, height, radius, border, feather, true };
    auto [ it, inserted ] = m_shapes.try_emplace( key );
    if ( inserted )
    {
        UIGeometry::Border( width, height, border, radius, feather, it->second );
    }
    it->second.LastUsedFrame = frame;
    return it->second;
}

void UIShapeCache::Evict( const uint64_t frame, const uint64_t maxAge )
{
    std::erase_if( m_shapes, [ & ]( const auto &entry ) { return frame > entry.second.LastUsedFrame && frame - entry.second.LastUsedFrame > maxAge; } );
}

void UIShapeCache::Clear( )
{
    m_shapes.clear( );
}
