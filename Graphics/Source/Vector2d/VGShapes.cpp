/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <DenOfIzGraphics/Vector2d/VGShapes.h>
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace DenOfIz;
using namespace DirectX;

namespace
{
    constexpr float PI      = 3.14159265359f;
    constexpr float EPSILON = 1e-6f;

    Float_2 TransformPoint( const Float_2 &point, const Float_4x4 &matrix )
    {
        const XMVECTOR vPoint       = XMVectorSet( point.X, point.Y, 0.0f, 1.0f );
        const XMMATRIX mTransform   = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &matrix ) );
        const XMVECTOR vTransformed = XMVector4Transform( vPoint, mTransform );

        Float_2 result;
        XMStoreFloat2( reinterpret_cast<XMFLOAT2 *>( &result ), vTransformed );
        return result;
    }

    Float_2 ReflectControlPoint( const Float_2 &current, const Float_2 &lastControl )
    {
        return { 2.0f * current.X - lastControl.X, 2.0f * current.Y - lastControl.Y };
    }

    float Distance( const Float_2 &a, const Float_2 &b )
    {
        const XMVECTOR va = XMVectorSet( a.X, a.Y, 0.0f, 0.0f );
        const XMVECTOR vb = XMVectorSet( b.X, b.Y, 0.0f, 0.0f );
        return XMVectorGetX( XMVector2Length( XMVectorSubtract( vb, va ) ) );
    }

    bool IsPointInPolygon( const Float_2 &point, const InteropArray<Float_2> &vertices, const VGFillRule fillRule )
    {
        if ( vertices.NumElements( ) < 3 )
            return false;

        int windingNumber = 0;
        int crossings     = 0;

        for ( size_t i = 0; i < vertices.NumElements( ); ++i )
        {
            const size_t  j  = ( i + 1 ) % vertices.NumElements( );
            const Float_2 v1 = vertices.GetElement( i );
            const Float_2 v2 = vertices.GetElement( j );

            if ( v1.Y <= point.Y )
            {
                if ( v2.Y > point.Y && ( v2.X - v1.X ) * ( point.Y - v1.Y ) - ( v2.Y - v1.Y ) * ( point.X - v1.X ) > 0 )
                {
                    windingNumber++;
                }
            }
            else
            {
                if ( v2.Y <= point.Y && ( v2.X - v1.X ) * ( point.Y - v1.Y ) - ( v2.Y - v1.Y ) * ( point.X - v1.X ) < 0 )
                {
                    windingNumber--;
                }
            }

            if ( v1.Y > point.Y != v2.Y > point.Y )
            {
                const float intersectionX = v1.X + ( point.Y - v1.Y ) / ( v2.Y - v1.Y ) * ( v2.X - v1.X );
                if ( point.X < intersectionX )
                {
                    crossings++;
                }
            }
        }

        return fillRule == VGFillRule::NonZero ? windingNumber != 0 : crossings % 2 == 1;
    }
} // namespace

VGPathCommand::VGPathCommand( const VGPathCommand &other ) : Type( other.Type )
{
    switch ( Type )
    {
    case VGPathCommandType::MoveTo:
        MoveTo = other.MoveTo;
        break;
    case VGPathCommandType::LineTo:
        LineTo = other.LineTo;
        break;
    case VGPathCommandType::HorizontalLineTo:
        HorizontalLineTo = other.HorizontalLineTo;
        break;
    case VGPathCommandType::VerticalLineTo:
        VerticalLineTo = other.VerticalLineTo;
        break;
    case VGPathCommandType::QuadraticCurveTo:
        QuadraticCurveTo = other.QuadraticCurveTo;
        break;
    case VGPathCommandType::SmoothQuadraticCurveTo:
        SmoothQuadraticCurveTo = other.SmoothQuadraticCurveTo;
        break;
    case VGPathCommandType::CubicCurveTo:
        CubicCurveTo = other.CubicCurveTo;
        break;
    case VGPathCommandType::SmoothCubicCurveTo:
        SmoothCubicCurveTo = other.SmoothCubicCurveTo;
        break;
    case VGPathCommandType::EllipticalArc:
        EllipticalArc = other.EllipticalArc;
        break;
    case VGPathCommandType::CircularArc:
        CircularArc = other.CircularArc;
        break;
    case VGPathCommandType::Close:
        Close = other.Close;
        break;
    }
}

VGPathCommand &VGPathCommand::operator=( const VGPathCommand &other )
{
    if ( this != &other )
    {
        Type = other.Type;
        switch ( Type )
        {
        case VGPathCommandType::MoveTo:
            MoveTo = other.MoveTo;
            break;
        case VGPathCommandType::LineTo:
            LineTo = other.LineTo;
            break;
        case VGPathCommandType::HorizontalLineTo:
            HorizontalLineTo = other.HorizontalLineTo;
            break;
        case VGPathCommandType::VerticalLineTo:
            VerticalLineTo = other.VerticalLineTo;
            break;
        case VGPathCommandType::QuadraticCurveTo:
            QuadraticCurveTo = other.QuadraticCurveTo;
            break;
        case VGPathCommandType::SmoothQuadraticCurveTo:
            SmoothQuadraticCurveTo = other.SmoothQuadraticCurveTo;
            break;
        case VGPathCommandType::CubicCurveTo:
            CubicCurveTo = other.CubicCurveTo;
            break;
        case VGPathCommandType::SmoothCubicCurveTo:
            SmoothCubicCurveTo = other.SmoothCubicCurveTo;
            break;
        case VGPathCommandType::EllipticalArc:
            EllipticalArc = other.EllipticalArc;
            break;
        case VGPathCommandType::CircularArc:
            CircularArc = other.CircularArc;
            break;
        case VGPathCommandType::Close:
            Close = other.Close;
            break;
        }
    }
    return *this;
}

class VGPath2D::Impl
{
public:
    InteropArray<VGPathCommand> m_commands;
    Float_2                     m_currentPoint{ 0.0f, 0.0f };
    Float_2                     m_startPoint{ 0.0f, 0.0f };
    Float_2                     m_lastControlPoint{ 0.0f, 0.0f };
    bool                        m_hasLastControlPoint = false;
    bool                        m_isClosed            = false;

    mutable VGBounds m_bounds{ { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    mutable VGBounds m_tightBounds{ { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    mutable bool     m_boundsValid      = false;
    mutable bool     m_tightBoundsValid = false;

    float               m_tessellationTolerance = 0.25f;
    VGFillRule          m_fillRule              = VGFillRule::NonZero;
    float               m_strokeWidth           = 1.0f;
    VGLineCap           m_lineCap               = VGLineCap::Butt;
    VGLineJoin          m_lineJoin              = VGLineJoin::Miter;
    float               m_miterLimit            = 4.0f;
    InteropArray<float> m_dashPattern;
    float               m_dashOffset = 0.0f;
};

VGPath2D::VGPath2D( ) : m_impl( new Impl( ) )
{
}

VGPath2D::~VGPath2D( )
{
    delete m_impl;
}

VGPath2D::VGPath2D( const VGPath2D &other ) : m_impl( new Impl( *other.m_impl ) )
{
}

VGPath2D &VGPath2D::operator=( const VGPath2D &other )
{
    if ( this != &other )
    {
        *m_impl = *other.m_impl;
    }
    return *this;
}

VGPath2D::VGPath2D( VGPath2D &&other ) noexcept : m_impl( other.m_impl )
{
    other.m_impl = nullptr;
}

VGPath2D &VGPath2D::operator=( VGPath2D &&other ) noexcept
{
    if ( this != &other )
    {
        delete m_impl;
        m_impl       = other.m_impl;
        other.m_impl = nullptr;
    }
    return *this;
}

void VGPath2D::Clear( ) const
{
    m_impl->m_commands.Clear( );
    m_impl->m_currentPoint        = { 0.0f, 0.0f };
    m_impl->m_startPoint          = { 0.0f, 0.0f };
    m_impl->m_lastControlPoint    = { 0.0f, 0.0f };
    m_impl->m_hasLastControlPoint = false;
    m_impl->m_isClosed            = false;
    InvalidateBounds( );
}

void VGPath2D::MoveTo( const Float_2 &point ) const
{
    VGPathCommand command;
    command.Type              = VGPathCommandType::MoveTo;
    command.MoveTo.Point      = point;
    command.MoveTo.IsRelative = false;
    AddCommand( command );
    UpdateCurrentPoint( point );
    m_impl->m_startPoint = point;
    m_impl->m_isClosed   = false;
    ClearLastControlPoint( );
}

void VGPath2D::LineTo( const Float_2 &point ) const
{
    VGPathCommand command;
    command.Type              = VGPathCommandType::LineTo;
    command.LineTo.Point      = point;
    command.LineTo.IsRelative = false;
    AddCommand( command );
    UpdateCurrentPoint( point );
    ClearLastControlPoint( );
}

void VGPath2D::HorizontalLineTo( float x ) const
{
    VGPathCommand command;
    command.Type                        = VGPathCommandType::HorizontalLineTo;
    command.HorizontalLineTo.X          = x;
    command.HorizontalLineTo.IsRelative = false;
    AddCommand( command );
    UpdateCurrentPoint( { x, m_impl->m_currentPoint.Y } );
    ClearLastControlPoint( );
}

void VGPath2D::VerticalLineTo( float y ) const
{
    VGPathCommand command;
    command.Type                      = VGPathCommandType::VerticalLineTo;
    command.VerticalLineTo.Y          = y;
    command.VerticalLineTo.IsRelative = false;
    AddCommand( command );
    UpdateCurrentPoint( { m_impl->m_currentPoint.X, y } );
    ClearLastControlPoint( );
}

void VGPath2D::Close( ) const
{
    VGPathCommand command;
    command.Type = VGPathCommandType::Close;
    AddCommand( command );
    m_impl->m_currentPoint = m_impl->m_startPoint;
    m_impl->m_isClosed     = true;
    ClearLastControlPoint( );
}

void VGPath2D::RelativeMoveTo( const Float_2 &offset ) const
{
    VGPathCommand command;
    command.Type              = VGPathCommandType::MoveTo;
    command.MoveTo.Point      = offset;
    command.MoveTo.IsRelative = true;
    AddCommand( command );
    const Float_2 newPoint = { m_impl->m_currentPoint.X + offset.X, m_impl->m_currentPoint.Y + offset.Y };
    UpdateCurrentPoint( newPoint );
    m_impl->m_startPoint = newPoint;
    m_impl->m_isClosed   = false;
    ClearLastControlPoint( );
}

void VGPath2D::RelativeLineTo( const Float_2 &offset ) const
{
    VGPathCommand command;
    command.Type              = VGPathCommandType::LineTo;
    command.LineTo.Point      = offset;
    command.LineTo.IsRelative = true;
    AddCommand( command );
    const Float_2 newPoint = { m_impl->m_currentPoint.X + offset.X, m_impl->m_currentPoint.Y + offset.Y };
    UpdateCurrentPoint( newPoint );
    ClearLastControlPoint( );
}

void VGPath2D::RelativeHorizontalLineTo( float dx ) const
{
    VGPathCommand command;
    command.Type                        = VGPathCommandType::HorizontalLineTo;
    command.HorizontalLineTo.X          = dx;
    command.HorizontalLineTo.IsRelative = true;
    AddCommand( command );
    UpdateCurrentPoint( { m_impl->m_currentPoint.X + dx, m_impl->m_currentPoint.Y } );
    ClearLastControlPoint( );
}

void VGPath2D::RelativeVerticalLineTo( float dy ) const
{
    VGPathCommand command;
    command.Type                      = VGPathCommandType::VerticalLineTo;
    command.VerticalLineTo.Y          = dy;
    command.VerticalLineTo.IsRelative = true;
    AddCommand( command );
    UpdateCurrentPoint( { m_impl->m_currentPoint.X, m_impl->m_currentPoint.Y + dy } );
    ClearLastControlPoint( );
}

void VGPath2D::QuadraticCurveTo( const Float_2 &controlPoint, const Float_2 &endPoint ) const
{
    VGPathCommand command;
    command.Type                          = VGPathCommandType::QuadraticCurveTo;
    command.QuadraticCurveTo.ControlPoint = controlPoint;
    command.QuadraticCurveTo.EndPoint     = endPoint;
    command.QuadraticCurveTo.IsRelative   = false;
    AddCommand( command );
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint );
}

void VGPath2D::SmoothQuadraticCurveTo( const Float_2 &endPoint ) const
{
    VGPathCommand command;
    command.Type                              = VGPathCommandType::SmoothQuadraticCurveTo;
    command.SmoothQuadraticCurveTo.EndPoint   = endPoint;
    command.SmoothQuadraticCurveTo.IsRelative = false;
    AddCommand( command );
    UpdateCurrentPoint( endPoint );
    if ( m_impl->m_hasLastControlPoint )
    {
        UpdateLastControlPoint( ReflectControlPoint( m_impl->m_currentPoint, m_impl->m_lastControlPoint ) );
    }
    else
    {
        UpdateLastControlPoint( m_impl->m_currentPoint );
    }
}

void VGPath2D::CubicCurveTo( const Float_2 &controlPoint1, const Float_2 &controlPoint2, const Float_2 &endPoint ) const
{
    VGPathCommand command;
    command.Type                       = VGPathCommandType::CubicCurveTo;
    command.CubicCurveTo.ControlPoint1 = controlPoint1;
    command.CubicCurveTo.ControlPoint2 = controlPoint2;
    command.CubicCurveTo.EndPoint      = endPoint;
    command.CubicCurveTo.IsRelative    = false;
    AddCommand( command );
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint2 );
}

void VGPath2D::SmoothCubicCurveTo( const Float_2 &controlPoint2, const Float_2 &endPoint ) const
{
    VGPathCommand command;
    command.Type                             = VGPathCommandType::SmoothCubicCurveTo;
    command.SmoothCubicCurveTo.ControlPoint2 = controlPoint2;
    command.SmoothCubicCurveTo.EndPoint      = endPoint;
    command.SmoothCubicCurveTo.IsRelative    = false;
    AddCommand( command );
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint2 );
}

void VGPath2D::RelativeQuadraticCurveTo( const Float_2 &controlOffset, const Float_2 &endOffset ) const
{
    VGPathCommand command;
    command.Type                          = VGPathCommandType::QuadraticCurveTo;
    command.QuadraticCurveTo.ControlPoint = controlOffset;
    command.QuadraticCurveTo.EndPoint     = endOffset;
    command.QuadraticCurveTo.IsRelative   = true;
    AddCommand( command );
    const Float_2 controlPoint = { m_impl->m_currentPoint.X + controlOffset.X, m_impl->m_currentPoint.Y + controlOffset.Y };
    const Float_2 endPoint     = { m_impl->m_currentPoint.X + endOffset.X, m_impl->m_currentPoint.Y + endOffset.Y };
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint );
}

void VGPath2D::RelativeSmoothQuadraticCurveTo( const Float_2 &endOffset ) const
{
    VGPathCommand command;
    command.Type                              = VGPathCommandType::SmoothQuadraticCurveTo;
    command.SmoothQuadraticCurveTo.EndPoint   = endOffset;
    command.SmoothQuadraticCurveTo.IsRelative = true;
    AddCommand( command );
    const Float_2 endPoint = { m_impl->m_currentPoint.X + endOffset.X, m_impl->m_currentPoint.Y + endOffset.Y };
    UpdateCurrentPoint( endPoint );
    if ( m_impl->m_hasLastControlPoint )
    {
        UpdateLastControlPoint( ReflectControlPoint( m_impl->m_currentPoint, m_impl->m_lastControlPoint ) );
    }
    else
    {
        UpdateLastControlPoint( m_impl->m_currentPoint );
    }
}

void VGPath2D::RelativeCubicCurveTo( const Float_2 &control1Offset, const Float_2 &control2Offset, const Float_2 &endOffset ) const
{
    VGPathCommand command;
    command.Type                       = VGPathCommandType::CubicCurveTo;
    command.CubicCurveTo.ControlPoint1 = control1Offset;
    command.CubicCurveTo.ControlPoint2 = control2Offset;
    command.CubicCurveTo.EndPoint      = endOffset;
    command.CubicCurveTo.IsRelative    = true;
    AddCommand( command );
    const Float_2 controlPoint2 = { m_impl->m_currentPoint.X + control2Offset.X, m_impl->m_currentPoint.Y + control2Offset.Y };
    const Float_2 endPoint      = { m_impl->m_currentPoint.X + endOffset.X, m_impl->m_currentPoint.Y + endOffset.Y };
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint2 );
}

void VGPath2D::RelativeSmoothCubicCurveTo( const Float_2 &control2Offset, const Float_2 &endOffset ) const
{
    VGPathCommand command;
    command.Type                             = VGPathCommandType::SmoothCubicCurveTo;
    command.SmoothCubicCurveTo.ControlPoint2 = control2Offset;
    command.SmoothCubicCurveTo.EndPoint      = endOffset;
    command.SmoothCubicCurveTo.IsRelative    = true;
    AddCommand( command );
    const Float_2 controlPoint2 = { m_impl->m_currentPoint.X + control2Offset.X, m_impl->m_currentPoint.Y + control2Offset.Y };
    const Float_2 endPoint      = { m_impl->m_currentPoint.X + endOffset.X, m_impl->m_currentPoint.Y + endOffset.Y };
    UpdateCurrentPoint( endPoint );
    UpdateLastControlPoint( controlPoint2 );
}

void VGPath2D::EllipticalArcTo( const Float_2 &radii, float xAxisRotation, bool largeArcFlag, bool sweepFlag, const Float_2 &endPoint ) const
{
    VGPathCommand command;
    command.Type                        = VGPathCommandType::EllipticalArc;
    command.EllipticalArc.Radii         = radii;
    command.EllipticalArc.XAxisRotation = xAxisRotation;
    command.EllipticalArc.LargeArcFlag  = largeArcFlag;
    command.EllipticalArc.SweepFlag     = sweepFlag;
    command.EllipticalArc.EndPoint      = endPoint;
    command.EllipticalArc.IsRelative    = false;
    AddCommand( command );
    UpdateCurrentPoint( endPoint );
    ClearLastControlPoint( );
}

void VGPath2D::CircularArcTo( const Float_2 &center, float radius, float startAngle, float endAngle, bool clockwise ) const
{
    VGPathCommand command;
    command.Type                   = VGPathCommandType::CircularArc;
    command.CircularArc.Center     = center;
    command.CircularArc.Radius     = radius;
    command.CircularArc.StartAngle = startAngle;
    command.CircularArc.EndAngle   = endAngle;
    command.CircularArc.Clockwise  = clockwise;
    AddCommand( command );
    const Float_2 endPoint = { center.X + radius * std::cos( endAngle ), center.Y + radius * std::sin( endAngle ) };
    UpdateCurrentPoint( endPoint );
    ClearLastControlPoint( );
}

void VGPath2D::RelativeEllipticalArcTo( const Float_2 &radii, float xAxisRotation, bool largeArcFlag, bool sweepFlag, const Float_2 &endOffset ) const
{
    VGPathCommand command;
    command.Type                        = VGPathCommandType::EllipticalArc;
    command.EllipticalArc.Radii         = radii;
    command.EllipticalArc.XAxisRotation = xAxisRotation;
    command.EllipticalArc.LargeArcFlag  = largeArcFlag;
    command.EllipticalArc.SweepFlag     = sweepFlag;
    command.EllipticalArc.EndPoint      = endOffset;
    command.EllipticalArc.IsRelative    = true;
    AddCommand( command );
    const Float_2 endPoint = { m_impl->m_currentPoint.X + endOffset.X, m_impl->m_currentPoint.Y + endOffset.Y };
    UpdateCurrentPoint( endPoint );
    ClearLastControlPoint( );
}

void VGPath2D::ArcTo( const Float_2 &center, const float radius, const float startAngle, const float endAngle, const bool clockwise ) const
{
    CircularArcTo( center, radius, startAngle, endAngle, clockwise );
}

void VGPath2D::ArcByCenter( const Float_2 &center, const Float_2 &radii, const float startAngle, const float endAngle, const bool clockwise ) const
{
    const float cosStart = std::cos( startAngle );
    const float sinStart = std::sin( startAngle );
    const float cosEnd   = std::cos( endAngle );
    const float sinEnd   = std::sin( endAngle );

    MoveTo( { center.X + radii.X * cosStart, center.Y + radii.Y * sinStart } );
    EllipticalArcTo( radii, 0.0f, std::abs( endAngle - startAngle ) > PI, clockwise, { center.X + radii.X * cosEnd, center.Y + radii.Y * sinEnd } );
}

void VGPath2D::AddRect( const VGRect &rect ) const
{
    MoveTo( rect.TopLeft );
    LineTo( { rect.BottomRight.X, rect.TopLeft.Y } );
    LineTo( rect.BottomRight );
    LineTo( { rect.TopLeft.X, rect.BottomRight.Y } );
    Close( );
}

void VGPath2D::AddRoundedRect( const VGRoundedRect &roundedRect ) const
{
    const float width  = roundedRect.BottomRight.X - roundedRect.TopLeft.X;
    const float height = roundedRect.BottomRight.Y - roundedRect.TopLeft.Y;

    const float topLeftRadius     = std::min( roundedRect.CornerRadii.X, std::min( width * 0.5f, height * 0.5f ) );
    const float topRightRadius    = std::min( roundedRect.CornerRadii.Y, std::min( width * 0.5f, height * 0.5f ) );
    const float bottomRightRadius = std::min( roundedRect.CornerRadii.Z, std::min( width * 0.5f, height * 0.5f ) );
    const float bottomLeftRadius  = std::min( roundedRect.CornerRadii.W, std::min( width * 0.5f, height * 0.5f ) );

    MoveTo( { roundedRect.TopLeft.X + topLeftRadius, roundedRect.TopLeft.Y } );
    LineTo( { roundedRect.BottomRight.X - topRightRadius, roundedRect.TopLeft.Y } );

    if ( topRightRadius > 0.0f )
    {
        EllipticalArcTo( { topRightRadius, topRightRadius }, 0.0f, false, true, { roundedRect.BottomRight.X, roundedRect.TopLeft.Y + topRightRadius } );
    }

    LineTo( { roundedRect.BottomRight.X, roundedRect.BottomRight.Y - bottomRightRadius } );

    if ( bottomRightRadius > 0.0f )
    {
        EllipticalArcTo( { bottomRightRadius, bottomRightRadius }, 0.0f, false, true, { roundedRect.BottomRight.X - bottomRightRadius, roundedRect.BottomRight.Y } );
    }

    LineTo( { roundedRect.TopLeft.X + bottomLeftRadius, roundedRect.BottomRight.Y } );

    if ( bottomLeftRadius > 0.0f )
    {
        EllipticalArcTo( { bottomLeftRadius, bottomLeftRadius }, 0.0f, false, true, { roundedRect.TopLeft.X, roundedRect.BottomRight.Y - bottomLeftRadius } );
    }

    LineTo( { roundedRect.TopLeft.X, roundedRect.TopLeft.Y + topLeftRadius } );

    if ( topLeftRadius > 0.0f )
    {
        EllipticalArcTo( { topLeftRadius, topLeftRadius }, 0.0f, false, true, { roundedRect.TopLeft.X + topLeftRadius, roundedRect.TopLeft.Y } );
    }

    Close( );
}

void VGPath2D::AddCircle( const VGCircle &circle ) const
{
    const float radius = circle.Radius;
    MoveTo( { circle.Center.X + radius, circle.Center.Y } );
    EllipticalArcTo( { radius, radius }, 0.0f, false, true, { circle.Center.X, circle.Center.Y + radius } );
    EllipticalArcTo( { radius, radius }, 0.0f, false, true, { circle.Center.X - radius, circle.Center.Y } );
    EllipticalArcTo( { radius, radius }, 0.0f, false, true, { circle.Center.X, circle.Center.Y - radius } );
    EllipticalArcTo( { radius, radius }, 0.0f, false, true, { circle.Center.X + radius, circle.Center.Y } );
    Close( );
}

void VGPath2D::AddEllipse( const VGEllipse &ellipse ) const
{
    const float cosRot = std::cos( ellipse.Rotation );
    const float sinRot = std::sin( ellipse.Rotation );

    const Float_2 right  = { ellipse.Center.X + ellipse.Radii.X * cosRot, ellipse.Center.Y + ellipse.Radii.X * sinRot };
    const Float_2 top    = { ellipse.Center.X - ellipse.Radii.Y * sinRot, ellipse.Center.Y + ellipse.Radii.Y * cosRot };
    const Float_2 left   = { ellipse.Center.X - ellipse.Radii.X * cosRot, ellipse.Center.Y - ellipse.Radii.X * sinRot };
    const Float_2 bottom = { ellipse.Center.X + ellipse.Radii.Y * sinRot, ellipse.Center.Y - ellipse.Radii.Y * cosRot };

    MoveTo( right );
    EllipticalArcTo( ellipse.Radii, ellipse.Rotation, false, true, top );
    EllipticalArcTo( ellipse.Radii, ellipse.Rotation, false, true, left );
    EllipticalArcTo( ellipse.Radii, ellipse.Rotation, false, true, bottom );
    EllipticalArcTo( ellipse.Radii, ellipse.Rotation, false, true, right );
    Close( );
}

void VGPath2D::AddPolygon( const VGPolygon &polygon ) const
{
    if ( polygon.Points.NumElements( ) == 0 )
        return;

    MoveTo( polygon.Points.GetElement( 0 ) );
    for ( size_t i = 1; i < polygon.Points.NumElements( ); ++i )
    {
        LineTo( polygon.Points.GetElement( i ) );
    }

    if ( polygon.IsClosed )
    {
        Close( );
    }
}

void VGPath2D::AddRectWithCorners( const Float_2 &topLeft, const Float_2 &bottomRight, const float cornerRadius ) const
{
    VGRoundedRect rect;
    rect.TopLeft     = topLeft;
    rect.BottomRight = bottomRight;
    rect.CornerRadii = { cornerRadius, cornerRadius, cornerRadius, cornerRadius };
    AddRoundedRect( rect );
}

void VGPath2D::AddRectWithIndividualCorners( const Float_2 &topLeft, const Float_2 &bottomRight, const float topLeftRadius, const float topRightRadius,
                                             const float bottomRightRadius, const float bottomLeftRadius ) const
{
    VGRoundedRect rect;
    rect.TopLeft     = topLeft;
    rect.BottomRight = bottomRight;
    rect.CornerRadii = { topLeftRadius, topRightRadius, bottomRightRadius, bottomLeftRadius };
    AddRoundedRect( rect );
}

bool VGPath2D::IsEmpty( ) const
{
    return m_impl->m_commands.NumElements( ) == 0;
}

bool VGPath2D::IsClosed( ) const
{
    return m_impl->m_isClosed;
}

Float_2 VGPath2D::GetCurrentPoint( ) const
{
    return m_impl->m_currentPoint;
}

Float_2 VGPath2D::GetStartPoint( ) const
{
    return m_impl->m_startPoint;
}

Float_2 VGPath2D::GetLastControlPoint( ) const
{
    return m_impl->m_lastControlPoint;
}

bool VGPath2D::HasLastControlPoint( ) const
{
    return m_impl->m_hasLastControlPoint;
}

uint32_t VGPath2D::GetCommandCount( ) const
{
    return static_cast<uint32_t>( m_impl->m_commands.NumElements( ) );
}

VGBounds VGPath2D::GetBounds( ) const
{
    if ( !m_impl->m_boundsValid )
    {
        CalculateBounds( );
    }
    return m_impl->m_bounds;
}

VGBounds VGPath2D::GetTightBounds( ) const
{
    if ( !m_impl->m_tightBoundsValid )
    {
        CalculateTightBounds( );
    }
    return m_impl->m_tightBounds;
}

const InteropArray<VGPathCommand> &VGPath2D::GetCommands( ) const
{
    return m_impl->m_commands;
}

VGPathCommand VGPath2D::GetCommand( const uint32_t index ) const
{
    if ( index >= m_impl->m_commands.NumElements( ) )
    {
        VGPathCommand empty;
        return empty;
    }
    return m_impl->m_commands.GetElement( index );
}

void VGPath2D::Reverse( ) const
{
    InteropArray<VGPathCommand> reversedCommands;

    for ( int i = static_cast<int>( m_impl->m_commands.NumElements( ) ) - 1; i >= 0; --i )
    {
        reversedCommands.AddElement( m_impl->m_commands.GetElement( i ) );
    }

    m_impl->m_commands = std::move( reversedCommands );
    InvalidateBounds( );
}

void VGPath2D::Transform( const Float_4x4 &matrix ) const
{
    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::MoveTo:
            if ( !cmd.MoveTo.IsRelative )
            {
                cmd.MoveTo.Point = TransformPoint( cmd.MoveTo.Point, matrix );
            }
            break;
        case VGPathCommandType::LineTo:
            if ( !cmd.LineTo.IsRelative )
            {
                cmd.LineTo.Point = TransformPoint( cmd.LineTo.Point, matrix );
            }
            break;
        case VGPathCommandType::QuadraticCurveTo:
            if ( !cmd.QuadraticCurveTo.IsRelative )
            {
                cmd.QuadraticCurveTo.ControlPoint = TransformPoint( cmd.QuadraticCurveTo.ControlPoint, matrix );
                cmd.QuadraticCurveTo.EndPoint     = TransformPoint( cmd.QuadraticCurveTo.EndPoint, matrix );
            }
            break;
        case VGPathCommandType::CubicCurveTo:
            if ( !cmd.CubicCurveTo.IsRelative )
            {
                cmd.CubicCurveTo.ControlPoint1 = TransformPoint( cmd.CubicCurveTo.ControlPoint1, matrix );
                cmd.CubicCurveTo.ControlPoint2 = TransformPoint( cmd.CubicCurveTo.ControlPoint2, matrix );
                cmd.CubicCurveTo.EndPoint      = TransformPoint( cmd.CubicCurveTo.EndPoint, matrix );
            }
            break;
        case VGPathCommandType::EllipticalArc:
            if ( !cmd.EllipticalArc.IsRelative )
            {
                cmd.EllipticalArc.EndPoint = TransformPoint( cmd.EllipticalArc.EndPoint, matrix );
            }
            break;
        case VGPathCommandType::CircularArc:
            cmd.CircularArc.Center = TransformPoint( cmd.CircularArc.Center, matrix );
            break;
        default:
            break;
        }
    }
    InvalidateBounds( );
}

void VGPath2D::Translate( const Float_2 &offset ) const
{
    Float_4x4 matrix;
    matrix._41 = offset.X;
    matrix._42 = offset.Y;
    Transform( matrix );
}

void VGPath2D::Scale( const Float_2 &scale ) const
{
    Float_4x4 matrix;
    matrix._11 = scale.X;
    matrix._22 = scale.Y;
    Transform( matrix );
}

void VGPath2D::Scale( const float scale ) const
{
    Scale( { scale, scale } );
}

void VGPath2D::Rotate( const float angleRadians, const Float_2 &center ) const
{
    const float cosA = std::cos( angleRadians );
    const float sinA = std::sin( angleRadians );

    Float_4x4 matrix;
    matrix._11 = cosA;
    matrix._12 = sinA;
    matrix._21 = -sinA;
    matrix._22 = cosA;
    matrix._41 = center.X - center.X * cosA + center.Y * sinA;
    matrix._42 = center.Y - center.X * sinA - center.Y * cosA;

    Transform( matrix );
}

void VGPath2D::AppendPath( const VGPath2D &other ) const
{
    for ( size_t i = 0; i < other.m_impl->m_commands.NumElements( ); ++i )
    {
        m_impl->m_commands.AddElement( other.m_impl->m_commands.GetElement( i ) );
    }
    InvalidateBounds( );
}

void VGPath2D::AppendPath( const VGPath2D &other, const Float_4x4 &transform ) const
{
    const VGPath2D transformedPath = other;
    transformedPath.Transform( transform );
    AppendPath( transformedPath );
}

void VGPath2D::SetTessellationTolerance( const float tolerance ) const
{
    m_impl->m_tessellationTolerance = std::max( tolerance, 0.01f );
}

float VGPath2D::GetTessellationTolerance( ) const
{
    return m_impl->m_tessellationTolerance;
}

void VGPath2D::SetFillRule( const VGFillRule fillRule ) const
{
    m_impl->m_fillRule = fillRule;
}

VGFillRule VGPath2D::GetFillRule( ) const
{
    return m_impl->m_fillRule;
}

void VGPath2D::SetStrokeWidth( const float width ) const
{
    m_impl->m_strokeWidth = std::max( width, 0.0f );
}

float VGPath2D::GetStrokeWidth( ) const
{
    return m_impl->m_strokeWidth;
}

void VGPath2D::SetLineCap( const VGLineCap cap ) const
{
    m_impl->m_lineCap = cap;
}

VGLineCap VGPath2D::GetLineCap( ) const
{
    return m_impl->m_lineCap;
}

void VGPath2D::SetLineJoin( const VGLineJoin join ) const
{
    m_impl->m_lineJoin = join;
}

VGLineJoin VGPath2D::GetLineJoin( ) const
{
    return m_impl->m_lineJoin;
}

void VGPath2D::SetMiterLimit( const float limit ) const
{
    m_impl->m_miterLimit = std::max( limit, 1.0f );
}

float VGPath2D::GetMiterLimit( ) const
{
    return m_impl->m_miterLimit;
}

void VGPath2D::SetDashPattern( const InteropArray<float> &pattern, const float offset ) const
{
    m_impl->m_dashPattern = pattern;
    m_impl->m_dashOffset  = offset;
}

InteropArray<float> VGPath2D::GetDashPattern( ) const
{
    return m_impl->m_dashPattern;
}

float VGPath2D::GetDashOffset( ) const
{
    return m_impl->m_dashOffset;
}

void VGPath2D::ClearDashPattern( ) const
{
    m_impl->m_dashPattern.Clear( );
    m_impl->m_dashOffset = 0.0f;
}

bool VGPath2D::HasDashPattern( ) const
{
    return m_impl->m_dashPattern.NumElements( ) > 0;
}

float VGPath2D::GetLength( ) const
{
    float   totalLength = 0.0f;
    Float_2 currentPos  = { 0.0f, 0.0f };

    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::MoveTo:
            {
                const Float_2 target = cmd.MoveTo.IsRelative ? Float_2{ currentPos.X + cmd.MoveTo.Point.X, currentPos.Y + cmd.MoveTo.Point.Y } : cmd.MoveTo.Point;
                currentPos           = target;
                break;
            }
        case VGPathCommandType::LineTo:
            {
                const Float_2 target = cmd.LineTo.IsRelative ? Float_2{ currentPos.X + cmd.LineTo.Point.X, currentPos.Y + cmd.LineTo.Point.Y } : cmd.LineTo.Point;
                totalLength += Distance( currentPos, target );
                currentPos = target;
                break;
            }
        case VGPathCommandType::HorizontalLineTo:
            {
                const float   targetX = cmd.HorizontalLineTo.IsRelative ? currentPos.X + cmd.HorizontalLineTo.X : cmd.HorizontalLineTo.X;
                const Float_2 target  = { targetX, currentPos.Y };
                totalLength += Distance( currentPos, target );
                currentPos = target;
                break;
            }
        case VGPathCommandType::VerticalLineTo:
            {
                const float   targetY = cmd.VerticalLineTo.IsRelative ? currentPos.Y + cmd.VerticalLineTo.Y : cmd.VerticalLineTo.Y;
                const Float_2 target  = { currentPos.X, targetY };
                totalLength += Distance( currentPos, target );
                currentPos = target;
                break;
            }
        default:
            break;
        }
    }

    return totalLength;
}

Float_2 VGPath2D::GetPointAtLength( const float distance ) const
{
    float   currentLength = 0.0f;
    Float_2 currentPos    = { 0.0f, 0.0f };

    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::MoveTo:
            {
                const Float_2 target = cmd.MoveTo.IsRelative ? Float_2{ currentPos.X + cmd.MoveTo.Point.X, currentPos.Y + cmd.MoveTo.Point.Y } : cmd.MoveTo.Point;
                currentPos           = target;
                break;
            }
        case VGPathCommandType::LineTo:
            {
                const Float_2 target        = cmd.LineTo.IsRelative ? Float_2{ currentPos.X + cmd.LineTo.Point.X, currentPos.Y + cmd.LineTo.Point.Y } : cmd.LineTo.Point;
                const float   segmentLength = Distance( currentPos, target );
                if ( currentLength + segmentLength >= distance )
                {
                    const float t = ( distance - currentLength ) / segmentLength;
                    return { currentPos.X + t * ( target.X - currentPos.X ), currentPos.Y + t * ( target.Y - currentPos.Y ) };
                }
                currentLength += segmentLength;
                currentPos = target;
                break;
            }
        default:
            break;
        }
    }

    return currentPos;
}

Float_2 VGPath2D::GetTangentAtLength( const float distance ) const
{
    float   currentLength = 0.0f;
    Float_2 currentPos    = { 0.0f, 0.0f };

    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::LineTo:
            {
                const Float_2 target        = cmd.LineTo.IsRelative ? Float_2{ currentPos.X + cmd.LineTo.Point.X, currentPos.Y + cmd.LineTo.Point.Y } : cmd.LineTo.Point;
                const float   segmentLength = Distance( currentPos, target );
                if ( currentLength + segmentLength >= distance )
                {
                    const Float_2 direction = { target.X - currentPos.X, target.Y - currentPos.Y };
                    const float   length    = std::sqrt( direction.X * direction.X + direction.Y * direction.Y );
                    return length > EPSILON ? Float_2{ direction.X / length, direction.Y / length } : Float_2{ 1.0f, 0.0f };
                }
                currentLength += segmentLength;
                currentPos = target;
                break;
            }
        default:
            break;
        }
    }

    return { 1.0f, 0.0f };
}

bool VGPath2D::ContainsPoint( const Float_2 &point ) const
{
    return ContainsPoint( point, m_impl->m_fillRule );
}

bool VGPath2D::ContainsPoint( const Float_2 &point, const VGFillRule fillRule ) const
{
    InteropArray<Float_2> vertices;
    Float_2               currentPos = { 0.0f, 0.0f };

    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::MoveTo:
            {
                const Float_2 target = cmd.MoveTo.IsRelative ? Float_2{ currentPos.X + cmd.MoveTo.Point.X, currentPos.Y + cmd.MoveTo.Point.Y } : cmd.MoveTo.Point;
                currentPos           = target;
                vertices.AddElement( currentPos );
                break;
            }
        case VGPathCommandType::LineTo:
            {
                const Float_2 target = cmd.LineTo.IsRelative ? Float_2{ currentPos.X + cmd.LineTo.Point.X, currentPos.Y + cmd.LineTo.Point.Y } : cmd.LineTo.Point;
                currentPos           = target;
                vertices.AddElement( currentPos );
                break;
            }
        default:
            break;
        }
    }

    return IsPointInPolygon( point, vertices, fillRule );
}

bool VGPath2D::IsValid( ) const
{
    return m_impl->m_commands.NumElements( ) > 0;
}

InteropArray<InteropString> VGPath2D::GetValidationErrors( ) const
{
    InteropArray<InteropString> errors;

    if ( m_impl->m_commands.NumElements( ) == 0 )
    {
        errors.AddElement( InteropString( "Path is empty" ) );
    }

    bool hasMoveTo = false;
    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        if ( cmd.Type == VGPathCommandType::MoveTo )
        {
            hasMoveTo = true;
            break;
        }
    }

    if ( !hasMoveTo )
    {
        errors.AddElement( InteropString( "Path must start with MoveTo command" ) );
    }

    return errors;
}

void VGPath2D::AddCommand( const VGPathCommand &command ) const
{
    m_impl->m_commands.AddElement( command );
    InvalidateBounds( );
}

void VGPath2D::UpdateCurrentPoint( const Float_2 &point ) const
{
    m_impl->m_currentPoint = point;
}

void VGPath2D::UpdateLastControlPoint( const Float_2 &point ) const
{
    m_impl->m_lastControlPoint    = point;
    m_impl->m_hasLastControlPoint = true;
}

void VGPath2D::ClearLastControlPoint( ) const
{
    m_impl->m_hasLastControlPoint = false;
}

void VGPath2D::InvalidateBounds( ) const
{
    m_impl->m_boundsValid      = false;
    m_impl->m_tightBoundsValid = false;
}

void VGPath2D::CalculateBounds( ) const
{
    if ( m_impl->m_commands.NumElements( ) == 0 )
    {
        m_impl->m_bounds      = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
        m_impl->m_boundsValid = true;
        return;
    }

    float minX = std::numeric_limits<float>::max( );
    float minY = std::numeric_limits<float>::max( );
    float maxX = std::numeric_limits<float>::lowest( );
    float maxY = std::numeric_limits<float>::lowest( );

    Float_2 currentPos = { 0.0f, 0.0f };

    for ( size_t i = 0; i < m_impl->m_commands.NumElements( ); ++i )
    {
        const VGPathCommand &cmd = m_impl->m_commands.GetElement( i );
        switch ( cmd.Type )
        {
        case VGPathCommandType::MoveTo:
            {
                const Float_2 target = cmd.MoveTo.IsRelative ? Float_2{ currentPos.X + cmd.MoveTo.Point.X, currentPos.Y + cmd.MoveTo.Point.Y } : cmd.MoveTo.Point;
                currentPos           = target;
                minX                 = std::min( minX, currentPos.X );
                minY                 = std::min( minY, currentPos.Y );
                maxX                 = std::max( maxX, currentPos.X );
                maxY                 = std::max( maxY, currentPos.Y );
                break;
            }
        case VGPathCommandType::LineTo:
            {
                const Float_2 target = cmd.LineTo.IsRelative ? Float_2{ currentPos.X + cmd.LineTo.Point.X, currentPos.Y + cmd.LineTo.Point.Y } : cmd.LineTo.Point;
                currentPos           = target;
                minX                 = std::min( minX, currentPos.X );
                minY                 = std::min( minY, currentPos.Y );
                maxX                 = std::max( maxX, currentPos.X );
                maxY                 = std::max( maxY, currentPos.Y );
                break;
            }
        default:
            break;
        }
    }

    m_impl->m_bounds      = { { minX, minY }, { maxX, maxY } };
    m_impl->m_boundsValid = true;
}

void VGPath2D::CalculateTightBounds( ) const
{
    CalculateBounds( );
    m_impl->m_tightBounds      = m_impl->m_bounds;
    m_impl->m_tightBoundsValid = true;
}
