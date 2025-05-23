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

#include <DenOfIzGraphics/Utilities/InteropMathConverter.h>
#include <DenOfIzGraphics/Vector2d/VectorGraphics.h>
#include <DirectXMath.h>
#include <algorithm>
#include <mapbox/earcut.hpp>

#include "DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace DenOfIz;
using namespace DirectX;

VectorGraphics::VectorGraphics( const VectorGraphicsDesc &desc ) :
    m_logicalDevice( desc.LogicalDevice ), m_tessellationTolerance( desc.DefaultTessellationTolerance ), m_textRenderer( desc.TextRenderer )
{
    if ( !m_logicalDevice )
    {
        LOG( FATAL ) << "VectorGraphics::LogicalDevice is null";
        return;
    }

    m_currentStyle.Fill.Enabled    = true;
    m_currentStyle.Fill.Color      = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_currentStyle.Stroke.Enabled  = false;
    m_currentStyle.Stroke.Color    = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_currentStyle.Stroke.Width    = 1.0f;
    m_currentStyle.Composite.Alpha = 1.0f;

    EnsureVertexBufferCapacity( desc.InitialVertexBufferNumBytes );
    EnsureIndexBufferCapacity( desc.InitialIndexBufferNumBytes );

    m_vertexBufferNumBytes = desc.InitialVertexBufferNumBytes;
    m_indexBufferNumBytes  = desc.InitialIndexBufferNumBytes;
}

VectorGraphics::~VectorGraphics( )
{
    if ( m_vertexBuffer )
    {
        m_vertexBuffer->UnmapMemory( );
        m_vertexBufferMappedMemory = nullptr;
    }

    if ( m_indexBuffer )
    {
        m_indexBuffer->UnmapMemory( );
        m_indexBufferMappedMemory = nullptr;
    }
}

void VectorGraphics::BeginBatch( ICommandList *commandList, const uint32_t frameIndex )
{
    m_commandList = commandList;
    m_frameIndex  = frameIndex;
    ClearBatch( );
}

void VectorGraphics::EndBatch( )
{
    DZ_RETURN_IF( !m_commandList );

    Flush( );
    m_commandList = nullptr;
}

void VectorGraphics::Flush( )
{
    DZ_RETURN_IF( !m_commandList || m_renderCommands.empty( ) );
    DZ_RETURN_IF( !m_pipeline );

    UpdateBuffers( );

    const auto pipeline = m_pipeline->GetPipeline( );
    m_commandList->BindPipeline( pipeline );

    m_commandList->BindVertexBuffer( m_vertexBuffer.get( ), 0 );
    m_commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32, 0 );

    if ( m_transform )
    {
        const auto projMatrix = m_transform->GetProjectionMatrix( );
        m_pipeline->UpdateProjection( m_frameIndex, projMatrix );
        if ( const auto bindGroup = m_pipeline->GetBindGroup( m_frameIndex, 0 ) )
        {
            m_commandList->BindResourceGroup( bindGroup );
        }
    }

    for ( size_t i = 0; i < m_renderCommands.size( ); ++i )
    {
        const auto &command = m_renderCommands[ i ];
        if ( command.IndexCount > 0 )
        {
            m_commandList->DrawIndexed( command.IndexCount, 1, command.IndexOffset, 0, 0 );
        }
        else if ( command.VertexCount > 0 )
        {
            m_commandList->Draw( command.VertexCount, 1, command.VertexOffset, 0 );
        }
    }

    ClearBatch( );
}

void VectorGraphics::SetFillColor( const Float_4 &color )
{
    m_currentStyle.Fill.Type    = VGFillType::Color;
    m_currentStyle.Fill.Color   = color;
    m_currentStyle.Fill.Enabled = true;
}

void VectorGraphics::SetFillEnabled( const bool enabled )
{
    m_currentStyle.Fill.Enabled = enabled;
}

void VectorGraphics::SetFillRule( const VGFillRule rule )
{
    m_currentStyle.Fill.FillRule = rule;
}

void VectorGraphics::SetFillLinearGradient( const Float_2 &start, const Float_2 &end, const InteropArray<VGGradientStop> &stops )
{
    m_currentStyle.Fill.Type          = VGFillType::LinearGradient;
    m_currentStyle.Fill.GradientType  = VGGradientType::Linear;
    m_currentStyle.Fill.GradientStart = start;
    m_currentStyle.Fill.GradientEnd   = end;
    m_currentStyle.Fill.GradientStops = stops;
    m_currentStyle.Fill.Enabled       = true;
}

void VectorGraphics::SetFillRadialGradient( const Float_2 &center, const float radius, const InteropArray<VGGradientStop> &stops )
{
    m_currentStyle.Fill.Type           = VGFillType::RadialGradient;
    m_currentStyle.Fill.GradientType   = VGGradientType::Radial;
    m_currentStyle.Fill.GradientCenter = center;
    m_currentStyle.Fill.GradientRadius = radius;
    m_currentStyle.Fill.GradientStops  = stops;
    m_currentStyle.Fill.Enabled        = true;
}

void VectorGraphics::SetFillConicGradient( const Float_2 &center, const float angle, const InteropArray<VGGradientStop> &stops )
{
    m_currentStyle.Fill.Type           = VGFillType::ConicGradient;
    m_currentStyle.Fill.GradientType   = VGGradientType::Conic;
    m_currentStyle.Fill.GradientCenter = center;
    m_currentStyle.Fill.GradientAngle  = angle;
    m_currentStyle.Fill.GradientStops  = stops;
    m_currentStyle.Fill.Enabled        = true;
}

void VectorGraphics::SetFillPattern( ITextureResource *texture, const Float_4x4 &transform )
{
    m_currentStyle.Fill.Type             = VGFillType::Pattern;
    m_currentStyle.Fill.PatternTexture   = texture;
    m_currentStyle.Fill.PatternTransform = transform;
    m_currentStyle.Fill.Enabled          = true;
}

void VectorGraphics::SetStrokeColor( const Float_4 &color )
{
    m_currentStyle.Stroke.Color   = color;
    m_currentStyle.Stroke.Enabled = true;
}

void VectorGraphics::SetStrokeWidth( const float width )
{
    m_currentStyle.Stroke.Width = std::max( 0.0f, width );
}

void VectorGraphics::SetStrokeLineCap( const VGLineCap cap )
{
    m_currentStyle.Stroke.Cap = cap;
}

void VectorGraphics::SetStrokeLineJoin( const VGLineJoin join )
{
    m_currentStyle.Stroke.Join = join;
}

void VectorGraphics::SetStrokeMiterLimit( const float limit )
{
    m_currentStyle.Stroke.MiterLimit = std::max( 1.0f, limit );
}

void VectorGraphics::SetStrokeDashPattern( const InteropArray<float> &pattern, const float offset )
{
    m_currentStyle.Stroke.DashPattern = pattern;
    m_currentStyle.Stroke.DashOffset  = offset;
}

void VectorGraphics::SetStrokeEnabled( const bool enabled )
{
    m_currentStyle.Stroke.Enabled = enabled;
}

void VectorGraphics::SetBlendMode( const VGBlendMode mode )
{
    m_currentStyle.Composite.BlendMode = mode;
}

void VectorGraphics::SetAlpha( const float alpha )
{
    m_currentStyle.Composite.Alpha = std::clamp( alpha, 0.0f, 1.0f );
}

const VGStyle &VectorGraphics::GetCurrentStyle( ) const
{
    return m_currentStyle;
}

void VectorGraphics::SetStyle( const VGStyle &style )
{
    m_currentStyle = style;
}

void VectorGraphics::Save( ) const
{
    if ( m_transform )
    {
        m_transform->PushTransform( );
    }
}

void VectorGraphics::Restore( ) const
{
    if ( m_transform )
    {
        m_transform->PopTransform( );
    }
}

void VectorGraphics::PushTransform( const Float_4x4 &transform ) const
{
    if ( m_transform )
    {
        m_transform->PushTransform( transform );
    }
}

void VectorGraphics::PopTransform( ) const
{
    if ( m_transform )
    {
        m_transform->PopTransform( );
    }
}

void VectorGraphics::ResetTransform( ) const
{
    if ( m_transform )
    {
        m_transform->ResetTransform( );
    }
}

void VectorGraphics::Transform( const Float_4x4 &matrix ) const
{
    if ( m_transform )
    {
        m_transform->Transform( matrix );
    }
}

void VectorGraphics::Translate( const Float_2 &offset ) const
{
    if ( m_transform )
    {
        m_transform->Translate( offset );
    }
}

void VectorGraphics::Scale( const Float_2 &scale ) const
{
    if ( m_transform )
    {
        m_transform->Scale( scale );
    }
}

void VectorGraphics::Scale( const float scale ) const
{
    if ( m_transform )
    {
        m_transform->Scale( scale );
    }
}

void VectorGraphics::Rotate( const float angleRadians ) const
{
    if ( m_transform )
    {
        m_transform->Rotate( angleRadians );
    }
}

void VectorGraphics::Rotate( const float angleRadians, const Float_2 &center ) const
{
    if ( m_transform )
    {
        m_transform->Rotate( angleRadians, center );
    }
}

void VectorGraphics::Skew( const Float_2 &skew ) const
{
    if ( m_transform )
    {
        m_transform->Skew( skew );
    }
}

void VectorGraphics::DrawPath( const VGPath2D &path )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillPath( path );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokePath( path );
    }
}

void VectorGraphics::FillPath( const VGPath2D &path )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellatePath( path, false );
}

void VectorGraphics::StrokePath( const VGPath2D &path )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellatePath( path, true );
}

void VectorGraphics::DrawRect( const VGRect &rect )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillRect( rect );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokeRect( rect );
    }
}

void VectorGraphics::FillRect( const VGRect &rect )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellateRect( rect, false );
}

void VectorGraphics::StrokeRect( const VGRect &rect )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellateRect( rect, true );
}

void VectorGraphics::DrawRoundedRect( const VGRoundedRect &rect )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillRoundedRect( rect );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokeRoundedRect( rect );
    }
}

void VectorGraphics::FillRoundedRect( const VGRoundedRect &rect )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellateRoundedRect( rect, false );
}

void VectorGraphics::StrokeRoundedRect( const VGRoundedRect &rect )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellateRoundedRect( rect, true );
}

void VectorGraphics::DrawCircle( const VGCircle &circle )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillCircle( circle );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokeCircle( circle );
    }
}

void VectorGraphics::FillCircle( const VGCircle &circle )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellateCircle( circle, false );
}

void VectorGraphics::StrokeCircle( const VGCircle &circle )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellateCircle( circle, true );
}

void VectorGraphics::DrawEllipse( const VGEllipse &ellipse )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillEllipse( ellipse );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokeEllipse( ellipse );
    }
}

void VectorGraphics::FillEllipse( const VGEllipse &ellipse )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellateEllipse( ellipse, false );
}

void VectorGraphics::StrokeEllipse( const VGEllipse &ellipse )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellateEllipse( ellipse, true );
}

void VectorGraphics::DrawLine( const VGLine &line )
{
    TessellateLine( line );
}

void VectorGraphics::DrawLines( const InteropArray<Float_2> &points, const bool connected )
{
    DZ_RETURN_IF( points.NumElements( ) < 2 );

    for ( uint32_t i = 0; i < points.NumElements( ) - 1; ++i )
    {
        VGLine line;
        line.StartPoint = points.GetElement( i );
        line.EndPoint   = points.GetElement( i + 1 );
        line.Thickness  = m_currentStyle.Stroke.Width;
        TessellateLine( line );

        if ( !connected )
        {
            ++i; // Skip next point for disconnected lines
        }
    }
}

void VectorGraphics::DrawPolygon( const VGPolygon &polygon )
{
    if ( m_currentStyle.Fill.Enabled )
    {
        FillPolygon( polygon );
    }
    if ( m_currentStyle.Stroke.Enabled )
    {
        StrokePolygon( polygon );
    }
}

void VectorGraphics::FillPolygon( const VGPolygon &polygon )
{
    DZ_RETURN_IF( !m_currentStyle.Fill.Enabled );
    TessellatePolygon( polygon, false );
}

void VectorGraphics::StrokePolygon( const VGPolygon &polygon )
{
    DZ_RETURN_IF( !m_currentStyle.Stroke.Enabled );
    TessellatePolygon( polygon, true );
}

void VectorGraphics::DrawRect( const Float_2 &topLeft, const Float_2 &bottomRight )
{
    VGRect rect;
    rect.TopLeft     = topLeft;
    rect.BottomRight = bottomRight;
    DrawRect( rect );
}

void VectorGraphics::FillRect( const Float_2 &topLeft, const Float_2 &bottomRight )
{
    VGRect rect;
    rect.TopLeft     = topLeft;
    rect.BottomRight = bottomRight;
    FillRect( rect );
}

void VectorGraphics::StrokeRect( const Float_2 &topLeft, const Float_2 &bottomRight )
{
    VGRect rect;
    rect.TopLeft     = topLeft;
    rect.BottomRight = bottomRight;
    StrokeRect( rect );
}

void VectorGraphics::DrawCircle( const Float_2 &center, const float radius )
{
    VGCircle circle;
    circle.Center = center;
    circle.Radius = radius;
    DrawCircle( circle );
}

void VectorGraphics::FillCircle( const Float_2 &center, const float radius )
{
    VGCircle circle;
    circle.Center = center;
    circle.Radius = radius;
    FillCircle( circle );
}

void VectorGraphics::StrokeCircle( const Float_2 &center, const float radius )
{
    VGCircle circle;
    circle.Center = center;
    circle.Radius = radius;
    StrokeCircle( circle );
}

void VectorGraphics::DrawLine( const Float_2 &start, const Float_2 &end, const float thickness )
{
    VGLine line;
    line.StartPoint = start;
    line.EndPoint   = end;
    line.Thickness  = thickness;
    TessellateLine( line );
}

void VectorGraphics::ClipRect( const VGRect &rect )
{
    VGRect transformedRect;
    transformedRect.TopLeft     = TransformPoint( rect.TopLeft );
    transformedRect.BottomRight = TransformPoint( rect.BottomRight );

    if ( m_clipStack.empty( ) )
    {
        m_clipStack.push_back( transformedRect );
    }
    else
    {
        // Intersect with current clip rect
        const VGRect &currentClip  = m_clipStack.back( );
        const VGRect  intersection = IntersectRects( currentClip, transformedRect );
        m_clipStack.push_back( intersection );
    }

    m_clippingEnabled = true;
}

void VectorGraphics::ClipPath( const VGPath2D &path )
{
    // For now, approximate path clipping with bounding box
    // TODO: Implement proper path clipping using polygon clipping algorithms
    DZ_RETURN_IF( path.GetCommands( ).NumElements( ) == 0 );

    // Calculate bounding box of path
    Float_2 minPoint     = { FLT_MAX, FLT_MAX };
    Float_2 maxPoint     = { -FLT_MAX, -FLT_MAX };
    Float_2 currentPoint = { 0.0f, 0.0f };

    for ( uint32_t i = 0; i < path.GetCommands( ).NumElements( ); ++i )
    {
        const auto &command = path.GetCommands( ).GetElement( i );

        switch ( command.Type )
        {
        case VGPathCommandType::MoveTo:
            currentPoint = command.MoveTo.IsRelative ? Float_2{ currentPoint.X + command.MoveTo.Point.X, currentPoint.Y + command.MoveTo.Point.Y } : command.MoveTo.Point;
            break;

        case VGPathCommandType::LineTo:
            currentPoint = command.LineTo.IsRelative ? Float_2{ currentPoint.X + command.LineTo.Point.X, currentPoint.Y + command.LineTo.Point.Y } : command.LineTo.Point;
            break;

        case VGPathCommandType::QuadraticCurveTo:
            currentPoint = command.QuadraticCurveTo.IsRelative
                               ? Float_2{ currentPoint.X + command.QuadraticCurveTo.EndPoint.X, currentPoint.Y + command.QuadraticCurveTo.EndPoint.Y }
                               : command.QuadraticCurveTo.EndPoint;
            break;

        case VGPathCommandType::CubicCurveTo:
            currentPoint = command.CubicCurveTo.IsRelative ? Float_2{ currentPoint.X + command.CubicCurveTo.EndPoint.X, currentPoint.Y + command.CubicCurveTo.EndPoint.Y }
                                                           : command.CubicCurveTo.EndPoint;
            break;

        default:
            break;
        }

        minPoint.X = std::min( minPoint.X, currentPoint.X );
        minPoint.Y = std::min( minPoint.Y, currentPoint.Y );
        maxPoint.X = std::max( maxPoint.X, currentPoint.X );
        maxPoint.Y = std::max( maxPoint.Y, currentPoint.Y );
    }

    VGRect pathBounds;
    pathBounds.TopLeft     = minPoint;
    pathBounds.BottomRight = maxPoint;

    ClipRect( pathBounds );
}

void VectorGraphics::ResetClip( )
{
    m_clipStack.clear( );
    m_clippingEnabled = false;
}

bool VectorGraphics::IsClippingEnabled( ) const
{
    return m_clippingEnabled && !m_clipStack.empty( );
}

VGRect VectorGraphics::GetCurrentClipRect( ) const
{
    if ( m_clipStack.empty( ) )
    {
        return { { -FLT_MAX, -FLT_MAX }, { FLT_MAX, FLT_MAX } };
    }
    return m_clipStack.back( );
}

bool VectorGraphics::IsPointInClipRect( const Float_2 &point ) const
{
    if ( !IsClippingEnabled( ) )
    {
        return true;
    }

    const VGRect &clipRect = GetCurrentClipRect( );
    return point.X >= clipRect.TopLeft.X && point.X <= clipRect.BottomRight.X && point.Y >= clipRect.TopLeft.Y && point.Y <= clipRect.BottomRight.Y;
}

VGRect VectorGraphics::IntersectRects( const VGRect &a, const VGRect &b ) const
{
    VGRect result;
    result.TopLeft.X     = std::max( a.TopLeft.X, b.TopLeft.X );
    result.TopLeft.Y     = std::max( a.TopLeft.Y, b.TopLeft.Y );
    result.BottomRight.X = std::min( a.BottomRight.X, b.BottomRight.X );
    result.BottomRight.Y = std::min( a.BottomRight.Y, b.BottomRight.Y );

    if ( result.TopLeft.X > result.BottomRight.X || result.TopLeft.Y > result.BottomRight.Y )
    {
        result = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
    }

    return result;
}

void VectorGraphics::SetTessellationTolerance( const float tolerance )
{
    m_tessellationTolerance = tolerance;
}

float VectorGraphics::GetTessellationTolerance( ) const
{
    return m_tessellationTolerance;
}

void VectorGraphics::SetPipeline( VGPipeline *pipeline )
{
    m_pipeline = pipeline;
}

void VectorGraphics::SetTransform( VGTransform *transform )
{
    m_transform = transform;
}

VGPipeline *VectorGraphics::GetPipeline( ) const
{
    return m_pipeline;
}

VGTransform *VectorGraphics::GetTransform( ) const
{
    return m_transform;
}

void VectorGraphics::TessellatePath( const VGPath2D &path, bool forStroke )
{
    const auto &commands = path.GetCommands( );
    DZ_RETURN_IF( commands.NumElements( ) == 0 );

    std::vector<Float_2> pathPoints;
    Float_2              currentPoint        = { 0.0f, 0.0f };
    Float_2              startPoint          = { 0.0f, 0.0f };
    Float_2              lastControlPoint    = { 0.0f, 0.0f };
    bool                 hasLastControlPoint = false;

    for ( uint32_t i = 0; i < commands.NumElements( ); ++i )
    {
        auto command = commands.GetElement( i );

        switch ( command.Type )
        {
        case VGPathCommandType::MoveTo:
            {
                currentPoint = command.MoveTo.IsRelative ? Float_2{ currentPoint.X + command.MoveTo.Point.X, currentPoint.Y + command.MoveTo.Point.Y } : command.MoveTo.Point;
                startPoint   = currentPoint;
                pathPoints.clear( );
                pathPoints.push_back( currentPoint );
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::LineTo:
            {
                Float_2 endPoint = command.LineTo.IsRelative ? Float_2{ currentPoint.X + command.LineTo.Point.X, currentPoint.Y + command.LineTo.Point.Y } : command.LineTo.Point;
                pathPoints.push_back( endPoint );
                currentPoint        = endPoint;
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::QuadraticCurveTo:
            {
                Float_2 controlPoint = command.QuadraticCurveTo.IsRelative
                                           ? Float_2{ currentPoint.X + command.QuadraticCurveTo.ControlPoint.X, currentPoint.Y + command.QuadraticCurveTo.ControlPoint.Y }
                                           : command.QuadraticCurveTo.ControlPoint;
                Float_2 endPoint     = command.QuadraticCurveTo.IsRelative
                                           ? Float_2{ currentPoint.X + command.QuadraticCurveTo.EndPoint.X, currentPoint.Y + command.QuadraticCurveTo.EndPoint.Y }
                                           : command.QuadraticCurveTo.EndPoint;

                // Tessellate quadratic Bézier curve
                TessellateQuadraticBezier( currentPoint, controlPoint, endPoint, pathPoints );
                currentPoint        = endPoint;
                lastControlPoint    = controlPoint;
                hasLastControlPoint = true;
            }
            break;

        case VGPathCommandType::CubicCurveTo:
            {
                Float_2 control1 = command.CubicCurveTo.IsRelative
                                       ? Float_2{ currentPoint.X + command.CubicCurveTo.ControlPoint1.X, currentPoint.Y + command.CubicCurveTo.ControlPoint1.Y }
                                       : command.CubicCurveTo.ControlPoint1;
                Float_2 control2 = command.CubicCurveTo.IsRelative
                                       ? Float_2{ currentPoint.X + command.CubicCurveTo.ControlPoint2.X, currentPoint.Y + command.CubicCurveTo.ControlPoint2.Y }
                                       : command.CubicCurveTo.ControlPoint2;
                Float_2 endPoint = command.CubicCurveTo.IsRelative ? Float_2{ currentPoint.X + command.CubicCurveTo.EndPoint.X, currentPoint.Y + command.CubicCurveTo.EndPoint.Y }
                                                                   : command.CubicCurveTo.EndPoint;

                // Tessellate cubic Bézier curve
                TessellateCubicBezier( currentPoint, control1, control2, endPoint, pathPoints );
                currentPoint        = endPoint;
                lastControlPoint    = control2;
                hasLastControlPoint = true;
            }
            break;

        case VGPathCommandType::Close:
            {
                if ( !pathPoints.empty( ) && pathPoints.size( ) > 2 )
                {
                    // Close the path
                    pathPoints.push_back( startPoint );

                    // Tessellate the closed path
                    if ( forStroke )
                    {
                        GenerateStroke( pathPoints, true );
                    }
                    else
                    {
                        TessellateClosedPath( pathPoints );
                    }
                }
                pathPoints.clear( );
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::HorizontalLineTo:
            {
                Float_2 endPoint = command.HorizontalLineTo.IsRelative ? Float_2{ currentPoint.X + command.HorizontalLineTo.X, currentPoint.Y }
                                                                       : Float_2{ command.HorizontalLineTo.X, currentPoint.Y };
                pathPoints.push_back( endPoint );
                currentPoint        = endPoint;
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::VerticalLineTo:
            {
                Float_2 endPoint =
                    command.VerticalLineTo.IsRelative ? Float_2{ currentPoint.X, currentPoint.Y + command.VerticalLineTo.Y } : Float_2{ currentPoint.X, command.VerticalLineTo.Y };
                pathPoints.push_back( endPoint );
                currentPoint        = endPoint;
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::SmoothQuadraticCurveTo:
            {
                // Calculate reflected control point from last control point
                Float_2 controlPoint = hasLastControlPoint ? Float_2{ 2.0f * currentPoint.X - lastControlPoint.X, 2.0f * currentPoint.Y - lastControlPoint.Y } : currentPoint;

                Float_2 endPoint = command.SmoothQuadraticCurveTo.IsRelative
                                       ? Float_2{ currentPoint.X + command.SmoothQuadraticCurveTo.EndPoint.X, currentPoint.Y + command.SmoothQuadraticCurveTo.EndPoint.Y }
                                       : command.SmoothQuadraticCurveTo.EndPoint;

                // Tessellate smooth quadratic Bézier curve
                TessellateQuadraticBezier( currentPoint, controlPoint, endPoint, pathPoints );
                currentPoint        = endPoint;
                lastControlPoint    = controlPoint;
                hasLastControlPoint = true;
            }
            break;

        case VGPathCommandType::SmoothCubicCurveTo:
            {
                // Calculate reflected control point from last control point
                Float_2 control1 = hasLastControlPoint ? Float_2{ 2.0f * currentPoint.X - lastControlPoint.X, 2.0f * currentPoint.Y - lastControlPoint.Y } : currentPoint;

                Float_2 control2 = command.SmoothCubicCurveTo.IsRelative
                                       ? Float_2{ currentPoint.X + command.SmoothCubicCurveTo.ControlPoint2.X, currentPoint.Y + command.SmoothCubicCurveTo.ControlPoint2.Y }
                                       : command.SmoothCubicCurveTo.ControlPoint2;

                Float_2 endPoint = command.SmoothCubicCurveTo.IsRelative
                                       ? Float_2{ currentPoint.X + command.SmoothCubicCurveTo.EndPoint.X, currentPoint.Y + command.SmoothCubicCurveTo.EndPoint.Y }
                                       : command.SmoothCubicCurveTo.EndPoint;

                // Tessellate smooth cubic Bézier curve
                TessellateCubicBezier( currentPoint, control1, control2, endPoint, pathPoints );
                currentPoint        = endPoint;
                lastControlPoint    = control2;
                hasLastControlPoint = true;
            }
            break;

        case VGPathCommandType::EllipticalArc:
            {
                Float_2 endPoint = command.EllipticalArc.IsRelative
                                       ? Float_2{ currentPoint.X + command.EllipticalArc.EndPoint.X, currentPoint.Y + command.EllipticalArc.EndPoint.Y }
                                       : command.EllipticalArc.EndPoint;

                // Tessellate elliptical arc
                TessellateEllipticalArc( currentPoint, command.EllipticalArc.Radii, command.EllipticalArc.XAxisRotation, command.EllipticalArc.LargeArcFlag,
                                         command.EllipticalArc.SweepFlag, endPoint, pathPoints );
                currentPoint        = endPoint;
                hasLastControlPoint = false;
            }
            break;

        case VGPathCommandType::CircularArc:
            {
                // Tessellate circular arc
                TessellateCircularArc( command.CircularArc.Center, command.CircularArc.Radius, command.CircularArc.StartAngle, command.CircularArc.EndAngle,
                                       command.CircularArc.Clockwise, pathPoints );
                // Update current point to end of arc
                const float endAngle = command.CircularArc.EndAngle;
                currentPoint         = Float_2{ command.CircularArc.Center.X + command.CircularArc.Radius * cosf( endAngle ),
                                        command.CircularArc.Center.Y + command.CircularArc.Radius * sinf( endAngle ) };
                hasLastControlPoint  = false;
            }
            break;

        default:
            break;
        }
    }

    // If we have an unclosed path, tessellate it
    if ( !pathPoints.empty( ) && pathPoints.size( ) > 1 )
    {
        if ( forStroke )
        {
            GenerateStroke( pathPoints, false );
        }
        else
        {
            // For open paths, we don't fill
        }
    }
}

void VectorGraphics::TessellateRect( const VGRect &rect, const bool forStroke )
{
    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    if ( forStroke )
    {
        // Generate stroke outline for rectangle
        const float halfWidth = m_currentStyle.Stroke.Width * 0.5f;

        // Outer rectangle
        const auto outerTL = Float_2{ rect.TopLeft.X - halfWidth, rect.TopLeft.Y - halfWidth };
        const auto outerBR = Float_2{ rect.BottomRight.X + halfWidth, rect.BottomRight.Y + halfWidth };

        // Inner rectangle (original)
        const auto innerTL = rect.TopLeft;
        const auto innerBR = rect.BottomRight;

        // Create stroke by subtracting inner from outer rectangle
        // This creates 4 rectangles (top, right, bottom, left borders)

        // Top border
        AddVertex( TransformPoint( outerTL ), color );
        AddVertex( TransformPoint( { outerBR.X, outerTL.Y } ), color );
        AddVertex( TransformPoint( { innerBR.X, innerTL.Y } ), color );
        AddVertex( TransformPoint( innerTL ), color );

        uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

        // Right border
        AddVertex( TransformPoint( { outerBR.X, outerTL.Y } ), color );
        AddVertex( TransformPoint( outerBR ), color );
        AddVertex( TransformPoint( innerBR ), color );
        AddVertex( TransformPoint( { innerBR.X, innerTL.Y } ), color );

        baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

        // Bottom border
        AddVertex( TransformPoint( { innerTL.X, innerBR.Y } ), color );
        AddVertex( TransformPoint( innerBR ), color );
        AddVertex( TransformPoint( outerBR ), color );
        AddVertex( TransformPoint( { outerTL.X, outerBR.Y } ), color );

        baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

        // Left border
        AddVertex( TransformPoint( outerTL ), color );
        AddVertex( TransformPoint( innerTL ), color );
        AddVertex( TransformPoint( { innerTL.X, innerBR.Y } ), color );
        AddVertex( TransformPoint( { outerTL.X, outerBR.Y } ), color );

        baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );
    }
    else
    {
        if ( m_antialiasingMode == VGAntialiasingMode::Geometric )
        {
            // For antialiased rectangles, expand geometry and calculate edge distances
            const float aaWidth = m_antialiasingWidth;

            // Expanded rectangle for antialiasing
            const auto expandedTL = Float_2{ rect.TopLeft.X - aaWidth, rect.TopLeft.Y - aaWidth };
            const auto expandedBR = Float_2{ rect.BottomRight.X + aaWidth, rect.BottomRight.Y + aaWidth };

            // Calculate edge distances (negative values are outside, positive inside)
            // Distance is calculated from the original rectangle boundaries
            auto calcEdgeDistance = []( const Float_2 &point, const VGRect &rect, float aaWidth ) -> float
            {
                const float dx          = std::min( point.X - rect.TopLeft.X, rect.BottomRight.X - point.X );
                const float dy          = std::min( point.Y - rect.TopLeft.Y, rect.BottomRight.Y - point.Y );
                const float rawDistance = std::min( dx, dy ) / aaWidth;
                // Clamp to reasonable antialiasing range [-1, 1]
                return std::max( -1.0f, std::min( 1.0f, rawDistance ) );
            };

            // Create antialiased quad with proper edge distances
            AddVertex( TransformPoint( expandedTL ), color, { 0, 0 }, calcEdgeDistance( expandedTL, rect, aaWidth ) );
            AddVertex( TransformPoint( { expandedBR.X, expandedTL.Y } ), color, { 0, 0 }, calcEdgeDistance( { expandedBR.X, expandedTL.Y }, rect, aaWidth ) );
            AddVertex( TransformPoint( expandedBR ), color, { 0, 0 }, calcEdgeDistance( expandedBR, rect, aaWidth ) );
            AddVertex( TransformPoint( { expandedTL.X, expandedBR.Y } ), color, { 0, 0 }, calcEdgeDistance( { expandedTL.X, expandedBR.Y }, rect, aaWidth ) );
        }
        else
        {
            // Standard non-antialiased rectangle
            AddVertex( TransformPoint( rect.TopLeft ), color );                           // Top Left
            AddVertex( TransformPoint( { rect.BottomRight.X, rect.TopLeft.Y } ), color ); // Top Right
            AddVertex( TransformPoint( rect.BottomRight ), color );                       // Bottom Right
            AddVertex( TransformPoint( { rect.TopLeft.X, rect.BottomRight.Y } ), color ); // Bottom Left
        }

        const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;

        // Triangle 1: top-left, bottom-left, top-right (clockwise)
        AddTriangle( baseIndex, baseIndex + 3, baseIndex + 1 );
        // Triangle 2: top-right, bottom-left, bottom-right (clockwise)
        AddTriangle( baseIndex + 1, baseIndex + 3, baseIndex + 2 );
    }

    const auto primitiveType = forStroke ? VGPrimitiveType::Stroke : VGPrimitiveType::Fill;
    AddRenderCommand( primitiveType, 4, 6 );
}

void VectorGraphics::TessellateRoundedRect( const VGRoundedRect &rect, const bool forStroke )
{
    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    // Clamp corner radii to valid ranges
    const float maxRadius         = std::min( ( rect.BottomRight.X - rect.TopLeft.X ) * 0.5f, ( rect.BottomRight.Y - rect.TopLeft.Y ) * 0.5f );
    const float topLeftRadius     = std::min( rect.CornerRadii.X, maxRadius ); // TopLeft
    const float topRightRadius    = std::min( rect.CornerRadii.Y, maxRadius ); // TopRight
    const float bottomRightRadius = std::min( rect.CornerRadii.Z, maxRadius ); // BottomRight
    const float bottomLeftRadius  = std::min( rect.CornerRadii.W, maxRadius ); // BottomLeft

    if ( forStroke )
    {
        // Generate stroke for rounded rectangle
        const float halfWidth = m_currentStyle.Stroke.Width * 0.5f;

        // For stroke, we need to create an outline path
        std::vector<Float_2> outerPath, innerPath;

        // Generate outer path
        GenerateRoundedRectPath( rect.TopLeft.X - halfWidth, rect.TopLeft.Y - halfWidth, rect.BottomRight.X + halfWidth, rect.BottomRight.Y + halfWidth, topLeftRadius + halfWidth,
                                 topRightRadius + halfWidth, bottomLeftRadius + halfWidth, bottomRightRadius + halfWidth, outerPath );

        // Generate inner path
        const float innerX1 = rect.TopLeft.X + halfWidth;
        const float innerY1 = rect.TopLeft.Y + halfWidth;
        const float innerX2 = rect.BottomRight.X - halfWidth;
        const float innerY2 = rect.BottomRight.Y - halfWidth;

        if ( innerX2 > innerX1 && innerY2 > innerY1 )
        {
            GenerateRoundedRectPath( innerX1, innerY1, innerX2, innerY2, std::max( 0.0f, topLeftRadius - halfWidth ), std::max( 0.0f, topRightRadius - halfWidth ),
                                     std::max( 0.0f, bottomLeftRadius - halfWidth ), std::max( 0.0f, bottomRightRadius - halfWidth ), innerPath );
        }

        // Tessellate stroke using the path difference
        TessellateStrokeFromPaths( outerPath, innerPath );
    }
    else
    {
        // Generate filled rounded rectangle using triangle fan from center
        std::vector<Float_2> path;
        GenerateRoundedRectPath( rect.TopLeft.X, rect.TopLeft.Y, rect.BottomRight.X, rect.BottomRight.Y, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius, path );

        if ( !path.empty( ) )
        {
            // Add center point for fan triangulation
            const Float_2 center = { ( rect.TopLeft.X + rect.BottomRight.X ) * 0.5f, ( rect.TopLeft.Y + rect.BottomRight.Y ) * 0.5f };
            AddVertex( TransformPoint( center ), color );
            const uint32_t centerIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 1;

            // Add all path vertices
            for ( const auto &point : path )
            {
                AddVertex( TransformPoint( point ), color );
            }

            // Create triangles from center to each edge
            const uint32_t pathStartIndex = centerIndex + 1;
            for ( size_t i = 0; i < path.size( ); ++i )
            {
                const size_t next = ( i + 1 ) % path.size( );
                AddTriangle( centerIndex, pathStartIndex + static_cast<uint32_t>( i ), pathStartIndex + static_cast<uint32_t>( next ) );
            }
        }
    }

    const auto primitiveType = forStroke ? VGPrimitiveType::Stroke : VGPrimitiveType::Fill;
    AddRenderCommand( primitiveType, static_cast<uint32_t>( m_vertices.size( ) ), static_cast<uint32_t>( m_indices.size( ) ) );
}

void VectorGraphics::TessellateCircle( const VGCircle &circle, const bool forStroke )
{
    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    // Calculate number of segments based on radius and tessellation tolerance
    int segments = std::max( 8, static_cast<int>( 2.0f * XM_PI * circle.Radius / m_tessellationTolerance ) );
    segments     = std::min( segments, 128 ); // Cap at reasonable maximum

    const float angleStep = 2.0f * XM_PI / segments;

    if ( forStroke )
    {
        // Generate stroke outline
        const float innerRadius = std::max( 0.0f, circle.Radius - m_currentStyle.Stroke.Width * 0.5f );
        const float outerRadius = circle.Radius + m_currentStyle.Stroke.Width * 0.5f;

        for ( int i = 0; i < segments; ++i )
        {
            const float angle1 = i * angleStep;
            const float angle2 = ( i + 1 ) * angleStep;

            // Inner and outer points
            Float_2 inner1 = { circle.Center.X + innerRadius * cosf( angle1 ), circle.Center.Y + innerRadius * sinf( angle1 ) };
            Float_2 outer1 = { circle.Center.X + outerRadius * cosf( angle1 ), circle.Center.Y + outerRadius * sinf( angle1 ) };
            Float_2 inner2 = { circle.Center.X + innerRadius * cosf( angle2 ), circle.Center.Y + innerRadius * sinf( angle2 ) };
            Float_2 outer2 = { circle.Center.X + outerRadius * cosf( angle2 ), circle.Center.Y + outerRadius * sinf( angle2 ) };

            // Add quad for this segment
            AddVertex( TransformPoint( inner1 ), color );
            AddVertex( TransformPoint( outer1 ), color );
            AddVertex( TransformPoint( outer2 ), color );
            AddVertex( TransformPoint( inner2 ), color );

            const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
            AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );
        }
    }
    else
    {
        if ( m_antialiasingMode == VGAntialiasingMode::Geometric )
        {
            // For antialiased circles, extend the radius and calculate edge distances
            const float aaWidth        = m_antialiasingWidth;
            const float extendedRadius = circle.Radius + aaWidth;

            // Calculate edge distance for a point relative to circle
            // Edge distance should be in range [-1, 1] for proper antialiasing
            auto calcCircleEdgeDistance = []( const Float_2 &point, const Float_2 &center, float radius, float aaWidth ) -> float
            {
                const float dx             = point.X - center.X;
                const float dy             = point.Y - center.Y;
                const float distFromCenter = sqrtf( dx * dx + dy * dy );
                const float rawDistance    = ( radius - distFromCenter ) / aaWidth;
                // Clamp to reasonable antialiasing range [-1, 1]
                return std::max( -1.0f, std::min( 1.0f, rawDistance ) );
            };

            // Center vertex with maximum edge distance
            AddVertex( TransformPoint( circle.Center ), color, { 0, 0 }, calcCircleEdgeDistance( circle.Center, circle.Center, circle.Radius, aaWidth ) );

            // Add extended circle perimeter vertices
            for ( int i = 0; i < segments; ++i )
            {
                const float angle = i * angleStep;
                Float_2     point = { circle.Center.X + extendedRadius * cosf( angle ), circle.Center.Y + extendedRadius * sinf( angle ) };
                AddVertex( TransformPoint( point ), color, { 0, 0 }, calcCircleEdgeDistance( point, circle.Center, circle.Radius, aaWidth ) );
            }
        }
        else
        {
            // Standard filled circle using triangle fan
            AddVertex( TransformPoint( circle.Center ), color ); // Center vertex
            // Add circle perimeter vertices (only segments, not segments + 1)
            for ( int i = 0; i < segments; ++i )
            {
                const float angle = i * angleStep;
                Float_2     point = { circle.Center.X + circle.Radius * cosf( angle ), circle.Center.Y + circle.Radius * sinf( angle ) };
                AddVertex( TransformPoint( point ), color );
            }
        }

        // Create triangles with correct winding order (clockwise)
        const uint32_t centerIndex = static_cast<uint32_t>( m_vertices.size( ) ) - segments - 1;
        for ( int i = 0; i < segments; ++i )
        {
            const uint32_t next = ( i + 1 ) % segments;
            // Clockwise winding: center -> next -> current
            AddTriangle( centerIndex, centerIndex + 1 + next, centerIndex + 1 + i );
        }
    }

    const auto primitiveType = forStroke ? VGPrimitiveType::Stroke : VGPrimitiveType::Fill;
    AddRenderCommand( primitiveType, static_cast<uint32_t>( m_vertices.size( ) ), static_cast<uint32_t>( m_indices.size( ) ) );
}

void VectorGraphics::TessellateEllipse( const VGEllipse &ellipse, const bool forStroke )
{
    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    // Calculate number of segments based on circumference approximation and tessellation tolerance
    const float circumference =
        XM_PI * ( 3.0f * ( ellipse.Radii.X + ellipse.Radii.Y ) - sqrtf( ( 3.0f * ellipse.Radii.X + ellipse.Radii.Y ) * ( ellipse.Radii.X + 3.0f * ellipse.Radii.Y ) ) );
    int segments = std::max( 8, static_cast<int>( circumference / m_tessellationTolerance ) );
    segments     = std::min( segments, 128 ); // Cap at reasonable maximum

    const float angleStep   = 2.0f * XM_PI / segments;
    const float cosRotation = cosf( ellipse.Rotation );
    const float sinRotation = sinf( ellipse.Rotation );

    if ( forStroke )
    {
        // Generate stroke outline for ellipse
        const float strokeWidth = m_currentStyle.Stroke.Width;

        // For ellipse stroke, we need to offset along the normal at each point
        // This is complex for ellipses, so we'll use a simplified approach

        for ( int i = 0; i < segments; ++i )
        {
            const float angle1 = i * angleStep;
            const float angle2 = ( i + 1 ) * angleStep;

            // Calculate points on ellipse
            auto calculateEllipsePoint = [ & ]( const float angle, const float radiusScale = 1.0f ) -> Float_2
            {
                const float localX = ellipse.Radii.X * radiusScale * cosf( angle );
                const float localY = ellipse.Radii.Y * radiusScale * sinf( angle );

                // Apply rotation
                const float rotatedX = localX * cosRotation - localY * sinRotation;
                const float rotatedY = localX * sinRotation + localY * cosRotation;

                return { ellipse.Center.X + rotatedX, ellipse.Center.Y + rotatedY };
            };

            // Calculate normal for stroke offset
            auto calculateNormal = [ & ]( const float angle ) -> Float_2
            {
                // Derivative of ellipse parametric equations
                const float dx = -ellipse.Radii.X * sinf( angle );
                const float dy = ellipse.Radii.Y * cosf( angle );

                // Apply rotation to derivative
                const float rotatedDx = dx * cosRotation - dy * sinRotation;
                const float rotatedDy = dx * sinRotation + dy * cosRotation;

                // Perpendicular (normal) vector
                const float normalX = -rotatedDy;
                const float normalY = rotatedDx;

                // Normalize
                const float length = sqrtf( normalX * normalX + normalY * normalY );
                if ( length > 1e-6f )
                {
                    return { normalX / length, normalY / length };
                }
                return { 0.0f, 1.0f };
            };

            const Float_2 point1  = calculateEllipsePoint( angle1 );
            const Float_2 point2  = calculateEllipsePoint( angle2 );
            const Float_2 normal1 = calculateNormal( angle1 );
            const Float_2 normal2 = calculateNormal( angle2 );

            const float halfWidth = strokeWidth * 0.5f;

            // Inner and outer points
            const Float_2 inner1 = { point1.X - normal1.X * halfWidth, point1.Y - normal1.Y * halfWidth };
            const Float_2 outer1 = { point1.X + normal1.X * halfWidth, point1.Y + normal1.Y * halfWidth };
            const Float_2 inner2 = { point2.X - normal2.X * halfWidth, point2.Y - normal2.Y * halfWidth };
            const Float_2 outer2 = { point2.X + normal2.X * halfWidth, point2.Y + normal2.Y * halfWidth };

            // Add quad for this segment
            AddVertex( TransformPoint( inner1 ), color );
            AddVertex( TransformPoint( outer1 ), color );
            AddVertex( TransformPoint( outer2 ), color );
            AddVertex( TransformPoint( inner2 ), color );

            const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
            AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );
        }
    }
    else
    {
        // Generate filled ellipse using triangle fan
        AddVertex( TransformPoint( ellipse.Center ), color ); // Center vertex
        // Add ellipse perimeter vertices
        for ( int i = 0; i < segments; ++i )
        {
            const float angle = i * angleStep;
            // Calculate point on ellipse (before rotation)
            const float localX = ellipse.Radii.X * cosf( angle );
            const float localY = ellipse.Radii.Y * sinf( angle );
            // Apply rotation
            const float rotatedX = localX * cosRotation - localY * sinRotation;
            const float rotatedY = localX * sinRotation + localY * cosRotation;

            const Float_2 point = { ellipse.Center.X + rotatedX, ellipse.Center.Y + rotatedY };
            AddVertex( TransformPoint( point ), color );
        }

        // Create triangles with correct winding order (clockwise)
        const uint32_t centerIndex = static_cast<uint32_t>( m_vertices.size( ) ) - segments - 1;
        for ( int i = 0; i < segments; ++i )
        {
            const uint32_t next = ( i + 1 ) % segments;
            // Clockwise winding: center -> next -> current
            AddTriangle( centerIndex, centerIndex + 1 + next, centerIndex + 1 + i );
        }
    }

    const auto primitiveType = forStroke ? VGPrimitiveType::Stroke : VGPrimitiveType::Fill;
    AddRenderCommand( primitiveType, static_cast<uint32_t>( m_vertices.size( ) ), static_cast<uint32_t>( m_indices.size( ) ) );
}

void VectorGraphics::TessellatePolygon( const VGPolygon &polygon, const bool forStroke )
{
    DZ_RETURN_IF( polygon.Points.NumElements( ) < 3 );

    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    if ( forStroke )
    {
        for ( uint32_t i = 0; i < polygon.Points.NumElements( ); ++i )
        {
            const uint32_t nextIndex = ( i + 1 ) % polygon.Points.NumElements( );
            if ( !polygon.IsClosed && nextIndex == 0 )
            {
                break;
            }

            VGLine line;
            line.StartPoint = polygon.Points.GetElement( i );
            line.EndPoint   = polygon.Points.GetElement( nextIndex );
            line.Thickness  = m_currentStyle.Stroke.Width;
            TessellateLine( line );
        }
    }
    else
    {
        const uint32_t baseVertexIndex = static_cast<uint32_t>( m_vertices.size( ) );
        for ( uint32_t i = 0; i < polygon.Points.NumElements( ); ++i )
        {
            AddVertex( TransformPoint( polygon.Points.GetElement( i ) ), color );
        }
        for ( uint32_t i = 1; i < polygon.Points.NumElements( ) - 1; ++i )
        {
            AddTriangle( baseVertexIndex, baseVertexIndex + i + 1, baseVertexIndex + i );
        }
    }

    const auto primitiveType = forStroke ? VGPrimitiveType::Stroke : VGPrimitiveType::Fill;
    AddRenderCommand( primitiveType, static_cast<uint32_t>( m_vertices.size( ) ), static_cast<uint32_t>( m_indices.size( ) ) );
}

void VectorGraphics::TessellateLine( const VGLine &line )
{
    const auto color = ApplyAlpha( m_currentStyle.Stroke.Color );

    // Calculate line direction and perpendicular
    Float_2     direction = { line.EndPoint.X - line.StartPoint.X, line.EndPoint.Y - line.StartPoint.Y };
    const float length    = sqrtf( direction.X * direction.X + direction.Y * direction.Y );

    DZ_RETURN_IF( length < 1e-6f ); // Skip degenerate lines

    direction.X /= length;
    direction.Y /= length;

    const Float_2 perpendicular = { -direction.Y, direction.X };
    const float   halfWidth     = line.Thickness * 0.5f;

    const Float_2 p1 = { line.StartPoint.X + perpendicular.X * halfWidth, line.StartPoint.Y + perpendicular.Y * halfWidth };
    const Float_2 p2 = { line.StartPoint.X - perpendicular.X * halfWidth, line.StartPoint.Y - perpendicular.Y * halfWidth };
    const Float_2 p3 = { line.EndPoint.X - perpendicular.X * halfWidth, line.EndPoint.Y - perpendicular.Y * halfWidth };
    const Float_2 p4 = { line.EndPoint.X + perpendicular.X * halfWidth, line.EndPoint.Y + perpendicular.Y * halfWidth };

    AddVertex( TransformPoint( p1 ), color );
    AddVertex( TransformPoint( p2 ), color );
    AddVertex( TransformPoint( p3 ), color );
    AddVertex( TransformPoint( p4 ), color );

    const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
    AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

    AddRenderCommand( VGPrimitiveType::Stroke, 4, 6 );
}

void VectorGraphics::EnsureVertexBufferCapacity( const uint32_t vertexCount )
{
    const uint32_t requiredSize = vertexCount * sizeof( VGVertex );
    DZ_RETURN_IF( requiredSize <= m_vertexBufferNumBytes );

    if ( m_vertexBuffer )
    {
        m_vertexBuffer->UnmapMemory( );
        m_vertexBufferMappedMemory = nullptr;
    }

    const uint32_t newSize = std::max( requiredSize, m_vertexBufferNumBytes + m_vertexBufferNumBytes / 2 );
    BufferDesc     bufferDesc;
    bufferDesc.HeapType                  = HeapType::CPU_GPU;
    bufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    bufferDesc.NumBytes                  = newSize;
    bufferDesc.Descriptor                = BitSet( ResourceDescriptor::Buffer ) | ResourceDescriptor::StructuredBuffer;
    bufferDesc.StructureDesc.NumElements = vertexCount;
    bufferDesc.StructureDesc.Stride      = sizeof( VGVertex );

    m_vertexBuffer             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );
    m_vertexBufferNumBytes     = newSize;
    m_vertexBufferMappedMemory = static_cast<Byte *>( m_vertexBuffer->MapMemory( ) );
}

void VectorGraphics::EnsureIndexBufferCapacity( const uint32_t indexCount )
{
    const uint32_t requiredSize = indexCount * sizeof( uint32_t );
    DZ_RETURN_IF( requiredSize <= m_indexBufferNumBytes );

    if ( m_indexBuffer )
    {
        m_indexBuffer->UnmapMemory( );
        m_indexBufferMappedMemory = nullptr;
    }

    const uint32_t newSize = std::max( requiredSize, m_indexBufferNumBytes + m_indexBufferNumBytes / 2 );
    BufferDesc     bufferDesc;
    bufferDesc.HeapType   = HeapType::CPU_GPU;
    bufferDesc.Usages     = ResourceUsage::IndexBuffer;
    bufferDesc.NumBytes   = newSize;
    bufferDesc.Descriptor = ResourceDescriptor::Buffer;

    m_indexBuffer             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );
    m_indexBufferNumBytes     = newSize;
    m_indexBufferMappedMemory = static_cast<Byte *>( m_indexBuffer->MapMemory( ) );
}

void VectorGraphics::UpdateBuffers( )
{
    if ( !m_vertices.empty( ) )
    {
        EnsureVertexBufferCapacity( static_cast<uint32_t>( m_vertices.size( ) ) );
        memcpy( m_vertexBufferMappedMemory, m_vertices.data( ), m_vertices.size( ) * sizeof( VGVertex ) );
    }

    if ( !m_indices.empty( ) )
    {
        EnsureIndexBufferCapacity( static_cast<uint32_t>( m_indices.size( ) ) );
        memcpy( m_indexBufferMappedMemory, m_indices.data( ), m_indices.size( ) * sizeof( uint32_t ) );
    }
}

void VectorGraphics::AddRenderCommand( const VGPrimitiveType type, const uint32_t vertexCount, const uint32_t indexCount )
{
    VGRenderCommand command;
    command.Type         = type;
    command.Style        = m_currentStyle;
    command.VertexOffset = static_cast<uint32_t>( m_vertices.size( ) ) - vertexCount;
    command.VertexCount  = vertexCount;
    command.IndexOffset  = static_cast<uint32_t>( m_indices.size( ) ) - indexCount;
    command.IndexCount   = indexCount;

    m_renderCommands.push_back( command );
}

void VectorGraphics::AddVertex( const Float_2 &position, const Float_4 &color, const Float_2 &texCoord )
{
    VGVertex vertex;
    vertex.Position = position;
    vertex.Color    = color;
    vertex.TexCoord = texCoord;

    SetupGradientVertexData( vertex, position );
    m_vertices.push_back( vertex );
}

void VectorGraphics::AddVertex( const Float_2 &position, const Float_4 &color, const Float_2 &texCoord, const float edgeDistance )
{
    VGVertex vertex;
    vertex.Position = position;
    vertex.Color    = color;
    vertex.TexCoord = texCoord;

    SetupGradientVertexData( vertex, position );

    // Override the edge distance for antialiasing
    if ( m_antialiasingMode == VGAntialiasingMode::Geometric )
    {
        vertex.GradientData.Z = edgeDistance;
    }

    m_vertices.push_back( vertex );
}

void VectorGraphics::AddTriangle( const uint32_t v0, const uint32_t v1, const uint32_t v2 )
{
    m_indices.push_back( v0 );
    m_indices.push_back( v1 );
    m_indices.push_back( v2 );
}

void VectorGraphics::AddQuad( const uint32_t v0, const uint32_t v1, const uint32_t v2, const uint32_t v3 )
{
    AddTriangle( v0, v3, v1 );
    AddTriangle( v1, v3, v2 );
}

Float_2 VectorGraphics::TransformPoint( const Float_2 &point ) const
{
    if ( m_transform )
    {
        const auto transformMatrix = m_transform->GetMatrix( );
        const auto transform       = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &transformMatrix ) );
        const auto vector          = XMVectorSet( point.X, point.Y, 0.0f, 1.0f );
        const auto transformed     = XMVector4Transform( vector, transform );

        Float_2 result;
        XMStoreFloat2( reinterpret_cast<XMFLOAT2 *>( &result ), transformed );
        return result;
    }
    return point;
}

Float_4x4 VectorGraphics::GetCurrentTransform( ) const
{
    if ( m_transform )
    {
        return m_transform->GetMatrix( );
    }
    const auto identity = XMMatrixIdentity( );
    Float_4x4  result;
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &result ), identity );
    return result;
}

Float_4 VectorGraphics::ApplyAlpha( const Float_4 &color ) const
{
    return { color.X, color.Y, color.Z, color.W * m_currentStyle.Composite.Alpha };
}

void VectorGraphics::SetupGradientVertexData( VGVertex &vertex, const Float_2 &position ) const
{
    switch ( m_currentStyle.Fill.Type )
    {
    case VGFillType::LinearGradient:
        {
            // Calculate gradient coordinate for linear gradient
            const Float_2 &start = m_currentStyle.Fill.GradientStart;
            const Float_2 &end   = m_currentStyle.Fill.GradientEnd;

            // Vector from start to end
            const Float_2 gradient         = { end.X - start.X, end.Y - start.Y };
            const float   gradientLengthSq = gradient.X * gradient.X + gradient.Y * gradient.Y;

            if ( gradientLengthSq > 1e-6f )
            {
                // Vector from start to current position
                const Float_2 toPos = { position.X - start.X, position.Y - start.Y };

                // Project position onto gradient line (dot product)
                const float t = ( toPos.X * gradient.X + toPos.Y * gradient.Y ) / gradientLengthSq;

                // Store gradient coordinate (t value) in the x component
                vertex.GradientData.X = t;
                vertex.GradientData.Y = 0.0f; // Unused for linear gradients
            }
            else
            {
                vertex.GradientData.X = 0.0f;
                vertex.GradientData.Y = 0.0f;
            }
        }
        break;

    case VGFillType::RadialGradient:
        {
            // Calculate distance from gradient center for radial gradient
            const Float_2 &center = m_currentStyle.Fill.GradientCenter;
            const float    radius = m_currentStyle.Fill.GradientRadius;

            // Distance from center to position
            const float dx       = position.X - center.X;
            const float dy       = position.Y - center.Y;
            const float distance = sqrtf( dx * dx + dy * dy );

            // Normalize by radius to get gradient coordinate
            const float t = radius > 1e-6f ? distance / radius : 0.0f;

            vertex.GradientData.X = t;
            vertex.GradientData.Y = 0.0f; // Unused for radial gradients
        }
        break;

    case VGFillType::ConicGradient:
        {
            // Calculate angle from gradient center for conic gradient
            const Float_2 &center    = m_currentStyle.Fill.GradientCenter;
            const float    baseAngle = m_currentStyle.Fill.GradientAngle;

            // Calculate angle from center to position
            const float dx    = position.X - center.X;
            const float dy    = position.Y - center.Y;
            float       angle = atan2f( dy, dx );

            // Normalize angle relative to base angle
            angle -= baseAngle;
            while ( angle < 0.0f )
            {
                angle += 2.0f * XM_PI;
            }
            while ( angle >= 2.0f * XM_PI )
            {
                angle -= 2.0f * XM_PI;
            }

            // Normalize to [0, 1]
            const float t = angle / ( 2.0f * XM_PI );

            vertex.GradientData.X = t;
            vertex.GradientData.Y = 0.0f; // Unused for conic gradients
        }
        break;

    case VGFillType::Pattern:
        {
            // For patterns, we could store texture coordinates
            // This depends on how the pattern transform is applied
            // For now, just pass through the position
            vertex.GradientData.X = position.X;
            vertex.GradientData.Y = position.Y;
        }
        break;

    default:
        // For solid colors, no gradient data needed
        vertex.GradientData.X = 0.0f;
        vertex.GradientData.Y = 0.0f;
        break;
    }

    // Store edge distance for antialiasing in Z component
    if ( m_antialiasingMode == VGAntialiasingMode::Geometric )
    {
        // For geometric antialiasing, Z component stores distance to edge
        // Positive values are interior, negative values are near edges
        // This will be properly calculated in tessellation methods
        vertex.GradientData.Z = 0.0f; // Will be set by tessellation methods
    }
    else
    {
        vertex.GradientData.Z = 0.0f; // No antialiasing
    }
    vertex.GradientData.W = 0.0f; // Could store additional parameters
}

void VectorGraphics::ClearBatch( )
{
    m_vertices.clear( );
    m_indices.clear( );
    m_renderCommands.clear( );
}

void VectorGraphics::TessellateQuadraticBezier( const Float_2 &p0, const Float_2 &p1, const Float_2 &p2, std::vector<Float_2> &points )
{
    // Improved adaptive tessellation with better flatness criteria
    const float flatnessTolerance = m_tessellationTolerance * 0.5f; // Stricter tolerance for curves

    // Check if the curve is flat enough using distance from control point to chord
    const float dist = DistancePointToLine( p1, p0, p2 );

    // Also check curve length to avoid over-tessellation of small curves
    const float chordLength = sqrtf( ( p2.X - p0.X ) * ( p2.X - p0.X ) + ( p2.Y - p0.Y ) * ( p2.Y - p0.Y ) );

    // If curve is flat enough OR very small, stop recursion
    if ( dist < flatnessTolerance || chordLength < flatnessTolerance * 2.0f )
    {
        points.push_back( p2 );
        return;
    }

    // Subdivide the curve using De Casteljau's algorithm
    const Float_2 p01  = { ( p0.X + p1.X ) * 0.5f, ( p0.Y + p1.Y ) * 0.5f };
    const Float_2 p12  = { ( p1.X + p2.X ) * 0.5f, ( p1.Y + p2.Y ) * 0.5f };
    const Float_2 p012 = { ( p01.X + p12.X ) * 0.5f, ( p01.Y + p12.Y ) * 0.5f };

    // Recursively tessellate both halves
    TessellateQuadraticBezier( p0, p01, p012, points );
    TessellateQuadraticBezier( p012, p12, p2, points );
}

void VectorGraphics::TessellateCubicBezier( const Float_2 &p0, const Float_2 &p1, const Float_2 &p2, const Float_2 &p3, std::vector<Float_2> &points )
{
    // Improved adaptive tessellation with stricter criteria for cubic curves
    const float flatnessTolerance = m_tessellationTolerance * 0.4f; // Even stricter for cubic curves

    // Check if the curve is flat enough by testing both control points
    const float dist1   = DistancePointToLine( p1, p0, p3 );
    const float dist2   = DistancePointToLine( p2, p0, p3 );
    const float maxDist = std::max( dist1, dist2 );

    // Also check curve length to avoid over-tessellation of small curves
    const float chordLength = sqrtf( ( p3.X - p0.X ) * ( p3.X - p0.X ) + ( p3.Y - p0.Y ) * ( p3.Y - p0.Y ) );

    // If curve is flat enough OR very small, stop recursion
    if ( maxDist < flatnessTolerance || chordLength < flatnessTolerance * 2.0f )
    {
        points.push_back( p3 );
        return;
    }

    // Subdivide the curve using De Casteljau's algorithm
    const Float_2 p01   = { ( p0.X + p1.X ) * 0.5f, ( p0.Y + p1.Y ) * 0.5f };
    const Float_2 p12   = { ( p1.X + p2.X ) * 0.5f, ( p1.Y + p2.Y ) * 0.5f };
    const Float_2 p23   = { ( p2.X + p3.X ) * 0.5f, ( p2.Y + p3.Y ) * 0.5f };
    const Float_2 p012  = { ( p01.X + p12.X ) * 0.5f, ( p01.Y + p12.Y ) * 0.5f };
    const Float_2 p123  = { ( p12.X + p23.X ) * 0.5f, ( p12.Y + p23.Y ) * 0.5f };
    const Float_2 p0123 = { ( p012.X + p123.X ) * 0.5f, ( p012.Y + p123.Y ) * 0.5f };

    // Recursively tessellate both halves
    TessellateCubicBezier( p0, p01, p012, p0123, points );
    TessellateCubicBezier( p0123, p123, p23, p3, points );
}

void VectorGraphics::TessellateClosedPath( const std::vector<Float_2> &points )
{
    DZ_RETURN_IF( points.size( ) < 3 );

    const auto     color           = ApplyAlpha( m_currentStyle.Fill.Color );
    const uint32_t baseVertexIndex = static_cast<uint32_t>( m_vertices.size( ) );

    for ( const auto &point : points )
    {
        AddVertex( TransformPoint( point ), color );
    }

    // Triangulate
    std::vector<uint32_t> triangleIndices;
    TriangulatePolygon( points, triangleIndices );

    for ( size_t i = 0; i < triangleIndices.size( ); i += 3 )
    {
        AddTriangle( baseVertexIndex + triangleIndices[ i ], baseVertexIndex + triangleIndices[ i + 1 ], baseVertexIndex + triangleIndices[ i + 2 ] );
    }

    AddRenderCommand( VGPrimitiveType::Fill, static_cast<uint32_t>( points.size( ) ), static_cast<uint32_t>( triangleIndices.size( ) ) );
}

void VectorGraphics::TriangulatePolygon( const std::vector<Float_2> &points, std::vector<uint32_t> &indices ) const
{
    indices.clear( );
    DZ_RETURN_IF( points.size( ) < 3 );

    // Convert Float_2 points to array format that earcut expects
    std::vector<std::array<float, 2>> vertices;
    vertices.reserve( points.size( ) );
    for ( const auto &point : points )
    {
        vertices.push_back( { point.X, point.Y } );
    }

    const std::vector<std::vector<std::array<float, 2>>> polygon   = { vertices };
    const auto                                           triangles = mapbox::earcut<uint32_t>( polygon );

    // earcut returns indices in counter-clockwise order, reverse order to convert to clockwise
    for ( size_t i = 0; i < triangles.size( ); i += 3 )
    {
        indices.push_back( triangles[ i ] );
        indices.push_back( triangles[ i + 2 ] );
        indices.push_back( triangles[ i + 1 ] );
    }
}

float VectorGraphics::DistancePointToLine( const Float_2 &point, const Float_2 &lineStart, const Float_2 &lineEnd )
{
    const XMVECTOR vPoint = XMVectorSet( point.X, point.Y, 0.0f, 0.0f );
    const XMVECTOR vStart = XMVectorSet( lineStart.X, lineStart.Y, 0.0f, 0.0f );
    const XMVECTOR vEnd   = XMVectorSet( lineEnd.X, lineEnd.Y, 0.0f, 0.0f );

    const XMVECTOR vLine    = XMVectorSubtract( vEnd, vStart );
    const float    lengthSq = XMVectorGetX( XMVector2LengthSq( vLine ) );

    if ( lengthSq < 1e-6f ) // Degenerate line
    {
        return XMVectorGetX( XMVector2Length( XMVectorSubtract( vPoint, vStart ) ) );
    }

    const XMVECTOR vToPoint = XMVectorSubtract( vPoint, vStart );
    float          t        = XMVectorGetX( XMVector2Dot( vToPoint, vLine ) ) / lengthSq;
    t                       = std::clamp( t, 0.0f, 1.0f );

    const XMVECTOR vProj = XMVectorAdd( vStart, XMVectorScale( vLine, t ) );
    return XMVectorGetX( XMVector2Length( XMVectorSubtract( vPoint, vProj ) ) );
}

bool VectorGraphics::IsPointInTriangle( const Float_2 &point, const Float_2 &a, const Float_2 &b, const Float_2 &c )
{
    // Use barycentric coordinates
    const float denom = ( b.Y - c.Y ) * ( a.X - c.X ) + ( c.X - b.X ) * ( a.Y - c.Y );
    if ( fabsf( denom ) < 1e-6f )
    {
        return false; // Degenerate triangle
    }

    const float alpha = ( ( b.Y - c.Y ) * ( point.X - c.X ) + ( c.X - b.X ) * ( point.Y - c.Y ) ) / denom;
    const float beta  = ( ( c.Y - a.Y ) * ( point.X - c.X ) + ( a.X - c.X ) * ( point.Y - c.Y ) ) / denom;
    const float gamma = 1.0f - alpha - beta;

    return alpha >= 0 && beta >= 0 && gamma >= 0;
}

float VectorGraphics::Cross2D( const Float_2 &a, const Float_2 &b )
{
    const XMVECTOR va    = XMVectorSet( a.X, a.Y, 0.0f, 0.0f );
    const XMVECTOR vb    = XMVectorSet( b.X, b.Y, 0.0f, 0.0f );
    const XMVECTOR cross = XMVector3Cross( va, vb );
    return XMVectorGetZ( cross );
}

void VectorGraphics::GenerateStroke( const std::vector<Float_2> &points, const bool closed )
{
    DZ_RETURN_IF( points.size( ) < 2 );

    const auto  color     = ApplyAlpha( m_currentStyle.Stroke.Color );
    const float halfWidth = m_currentStyle.Stroke.Width * 0.5f;

    for ( size_t i = 0; i < points.size( ) - 1; ++i )
    {
        Float_2 current = points[ i ];
        Float_2 next    = points[ i + 1 ];

        const XMVECTOR vCurrent   = XMVectorSet( current.X, current.Y, 0.0f, 0.0f );
        const XMVECTOR vNext      = XMVectorSet( next.X, next.Y, 0.0f, 0.0f );
        XMVECTOR       vDirection = XMVectorSubtract( vNext, vCurrent );

        const float length = XMVectorGetX( XMVector2Length( vDirection ) );
        if ( length < 1e-6f )
        {
            continue;
        }

        vDirection = XMVector2Normalize( vDirection );

        Float_2 direction;
        XMStoreFloat2( reinterpret_cast<XMFLOAT2 *>( &direction ), vDirection );

        const Float_2 perpendicular = { -direction.Y, direction.X };
        Float_2       p1            = { current.X + perpendicular.X * halfWidth, current.Y + perpendicular.Y * halfWidth };
        Float_2       p2            = { current.X - perpendicular.X * halfWidth, current.Y - perpendicular.Y * halfWidth };
        Float_2       p3            = { next.X - perpendicular.X * halfWidth, next.Y - perpendicular.Y * halfWidth };
        Float_2       p4            = { next.X + perpendicular.X * halfWidth, next.Y + perpendicular.Y * halfWidth };

        AddVertex( TransformPoint( p1 ), color );
        AddVertex( TransformPoint( p2 ), color );
        AddVertex( TransformPoint( p3 ), color );
        AddVertex( TransformPoint( p4 ), color );

        const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

        if ( i > 0 )
        {
            const Float_2 prev       = points[ i - 1 ];
            Float_2       prevDir    = { current.X - prev.X, current.Y - prev.Y };
            const float   prevLength = sqrtf( prevDir.X * prevDir.X + prevDir.Y * prevDir.Y );
            if ( prevLength > 1e-6f )
            {
                prevDir.X /= prevLength;
                prevDir.Y /= prevLength;
                GenerateLineJoin( current, prevDir, direction );
            }
        }

        // Add line caps at ends
        if ( i == 0 && !closed )
        {
            GenerateLineCap( current, direction, true );
        }
        if ( i == points.size( ) - 2 && !closed )
        {
            GenerateLineCap( next, direction, false );
        }
    }

    AddRenderCommand( VGPrimitiveType::Stroke, static_cast<uint32_t>( m_vertices.size( ) ), static_cast<uint32_t>( m_indices.size( ) ) );
}

void VectorGraphics::GenerateLineCap( const Float_2 &point, const Float_2 &direction, const bool isStart )
{
    const auto  color     = ApplyAlpha( m_currentStyle.Stroke.Color );
    const float halfWidth = m_currentStyle.Stroke.Width * 0.5f;

    const Float_2 effectiveDir  = isStart ? Float_2{ -direction.X, -direction.Y } : direction;
    const Float_2 perpendicular = { -effectiveDir.Y, effectiveDir.X };

    switch ( m_currentStyle.Stroke.Cap )
    {
    case VGLineCap::Butt:
        // No additional geometry needed
        break;

    case VGLineCap::Round:
        {
            // Generate a half-circle
            constexpr int segments = 8;
            const Float_2 center   = point;

            for ( int i = 0; i <= segments; ++i )
            {
                const float angle = ( isStart ? static_cast<float>( M_PI ) : 0.0f ) + static_cast<float>( M_PI ) * i / segments;
                const float cos_a = cosf( angle );
                const float sin_a = sinf( angle );

                Float_2 capPoint = { center.X + ( effectiveDir.X * cos_a + perpendicular.X * sin_a ) * halfWidth,
                                     center.Y + ( effectiveDir.Y * cos_a + perpendicular.Y * sin_a ) * halfWidth };

                AddVertex( TransformPoint( capPoint ), color );
            }

            // Create triangles for the cap
            const uint32_t centerIndex = static_cast<uint32_t>( m_vertices.size( ) ) - segments - 1;
            for ( int i = 0; i < segments; ++i )
            {
                m_indices.push_back( centerIndex + i );
                m_indices.push_back( centerIndex + i + 1 );
                m_indices.push_back( centerIndex + segments + 1 );
            }
        }
        break;

    case VGLineCap::Square:
        {
            // Extend the line by half the stroke width
            const Float_2 extension     = { effectiveDir.X * halfWidth, effectiveDir.Y * halfWidth };
            const Float_2 extendedPoint = { point.X + extension.X, point.Y + extension.Y };

            const Float_2 p1 = { extendedPoint.X + perpendicular.X * halfWidth, extendedPoint.Y + perpendicular.Y * halfWidth };
            const Float_2 p2 = { extendedPoint.X - perpendicular.X * halfWidth, extendedPoint.Y - perpendicular.Y * halfWidth };
            const Float_2 p3 = { point.X - perpendicular.X * halfWidth, point.Y - perpendicular.Y * halfWidth };
            const Float_2 p4 = { point.X + perpendicular.X * halfWidth, point.Y + perpendicular.Y * halfWidth };

            AddVertex( TransformPoint( p1 ), color );
            AddVertex( TransformPoint( p2 ), color );
            AddVertex( TransformPoint( p3 ), color );
            AddVertex( TransformPoint( p4 ), color );

            const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
            AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );
        }
        break;
    }
}

void VectorGraphics::GenerateLineJoin( const Float_2 &point, const Float_2 &dir1, const Float_2 &dir2 )
{
    const auto  color     = ApplyAlpha( m_currentStyle.Stroke.Color );
    const float halfWidth = m_currentStyle.Stroke.Width * 0.5f;

    const Float_2 perp1 = { -dir1.Y, dir1.X };
    const Float_2 perp2 = { -dir2.Y, dir2.X };

    switch ( m_currentStyle.Stroke.Join )
    {
    case VGLineJoin::Miter:
        {
            // Calculate miter point
            const float cross = Cross2D( dir1, dir2 );
            if ( fabsf( cross ) < 1e-6f )
                break; // Parallel lines

            Float_2     bisector       = { dir1.X + dir2.X, dir1.Y + dir2.Y };
            const float bisectorLength = sqrtf( bisector.X * bisector.X + bisector.Y * bisector.Y );
            if ( bisectorLength > 1e-6f )
            {
                bisector.X /= bisectorLength;
                bisector.Y /= bisectorLength;
            }

            const float miterLength = halfWidth / Cross2D( perp1, bisector );
            if ( fabsf( miterLength ) < m_currentStyle.Stroke.MiterLimit * halfWidth )
            {
                const Float_2 miterPoint = { point.X + bisector.X * miterLength, point.Y + bisector.Y * miterLength };

                AddVertex( TransformPoint( point ), color );
                AddVertex( TransformPoint( miterPoint ), color );
                AddVertex( TransformPoint( { point.X + perp1.X * halfWidth, point.Y + perp1.Y * halfWidth } ), color );
                AddVertex( TransformPoint( { point.X + perp2.X * halfWidth, point.Y + perp2.Y * halfWidth } ), color );

                const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
                m_indices.push_back( baseIndex );
                m_indices.push_back( baseIndex + 1 );
                m_indices.push_back( baseIndex + 2 );
                m_indices.push_back( baseIndex );
                m_indices.push_back( baseIndex + 1 );
                m_indices.push_back( baseIndex + 3 );
            }
            else
            {
                // Fallback to a bevel join when miter limit is exceeded
                AddVertex( TransformPoint( point ), color );
                AddVertex( TransformPoint( { point.X + perp1.X * halfWidth, point.Y + perp1.Y * halfWidth } ), color );
                AddVertex( TransformPoint( { point.X + perp2.X * halfWidth, point.Y + perp2.Y * halfWidth } ), color );

                const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 3;
                m_indices.push_back( baseIndex );
                m_indices.push_back( baseIndex + 1 );
                m_indices.push_back( baseIndex + 2 );
            }
        }
        break;

    case VGLineJoin::Round:
        {
            // Generate arc between the two directions
            constexpr int segments = 6;
            AddVertex( TransformPoint( point ), color );

            for ( int i = 0; i <= segments; ++i )
            {
                const float t = static_cast<float>( i ) / segments;

                // Use DirectXMath for vector interpolation and normalization
                const XMVECTOR vPerp1      = XMVectorSet( perp1.X, perp1.Y, 0.0f, 0.0f );
                const XMVECTOR vPerp2      = XMVectorSet( perp2.X, perp2.Y, 0.0f, 0.0f );
                XMVECTOR       vLerpedPerp = XMVectorLerp( vPerp1, vPerp2, t );

                vLerpedPerp = XMVector2Normalize( vLerpedPerp );
                vLerpedPerp = XMVectorScale( vLerpedPerp, halfWidth );

                const XMVECTOR vPoint    = XMVectorSet( point.X, point.Y, 0.0f, 0.0f );
                const XMVECTOR vArcPoint = XMVectorAdd( vPoint, vLerpedPerp );

                Float_2 arcPoint;
                XMStoreFloat2( reinterpret_cast<XMFLOAT2 *>( &arcPoint ), vArcPoint );
                AddVertex( TransformPoint( arcPoint ), color );
            }

            const uint32_t centerIndex = static_cast<uint32_t>( m_vertices.size( ) ) - segments - 2;
            for ( int i = 0; i < segments; ++i )
            {
                m_indices.push_back( centerIndex );
                m_indices.push_back( centerIndex + i + 1 );
                m_indices.push_back( centerIndex + i + 2 );
            }
        }
        break;

    case VGLineJoin::Bevel:
        {
            // Simple triangle connecting the endpoints
            AddVertex( TransformPoint( point ), color );
            AddVertex( TransformPoint( { point.X + perp1.X * halfWidth, point.Y + perp1.Y * halfWidth } ), color );
            AddVertex( TransformPoint( { point.X + perp2.X * halfWidth, point.Y + perp2.Y * halfWidth } ), color );

            const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 3;
            m_indices.push_back( baseIndex );
            m_indices.push_back( baseIndex + 1 );
            m_indices.push_back( baseIndex + 2 );
        }
        break;
    }
}

void VectorGraphics::DrawText( const InteropString &text, const Float_2 &position, const float scale ) const
{
    DZ_RETURN_IF( !m_textRenderer );

    const auto color = ApplyAlpha( m_currentStyle.Fill.Color );
    if ( m_transform )
    {
        const auto combinedMatrix = m_transform->GetCombinedMatrix( );
        m_textRenderer->SetProjectionMatrix( combinedMatrix );
    }

    TextRenderDesc textDesc;
    textDesc.Text  = text;
    textDesc.X     = position.X;
    textDesc.Y     = position.Y;
    textDesc.Color = color;
    textDesc.Scale = scale;

    m_textRenderer->AddText( textDesc );
}

VGRect VectorGraphics::MeasureText( const InteropString &text, const float scale ) const
{
    VGRect bounds = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };

    if ( !m_textRenderer || text.NumChars( ) == 0 )
    {
        return bounds;
    }

    // Use a reasonable default DPI for text measurement (96 DPI is standard for screen)
    const Float_2 size = m_textRenderer->MeasureText( text, scale, 96.0f );

    bounds.TopLeft     = { 0.0f, 0.0f };
    bounds.BottomRight = { size.X, size.Y };
    return bounds;
}

void VectorGraphics::GenerateRoundedRectPath( const float x1, const float y1, const float x2, const float y2, const float tlRadius, const float trRadius, const float blRadius,
                                              const float brRadius, std::vector<Float_2> &path ) const
{
    path.clear( );
    // Calculate segments for corners based on tessellation tolerance and radius
    const auto calculateSegments = [ this ]( const float radius ) -> int
    {
        if ( radius <= 0.0f )
        {
            return 0;
        }
        // Use tessellation tolerance for more adaptive quality
        // More segments for larger radii, fewer for smaller ones
        const float circumference = XM_PI * radius * 0.5f; // Quarter circle
        const int   segments      = std::max( 6, static_cast<int>( circumference / m_tessellationTolerance ) );
        return std::min( segments, 32 ); // Cap at reasonable maximum
    };

    const int tlSegments = calculateSegments( tlRadius );
    const int trSegments = calculateSegments( trRadius );
    const int blSegments = calculateSegments( blRadius );
    const int brSegments = calculateSegments( brRadius );

    // Start from top-left corner
    // Top-left corner
    if ( tlRadius > 0.0f && tlSegments > 0 )
    {
        for ( int i = 0; i <= tlSegments; ++i )
        {
            const float angle = XM_PI + static_cast<float>( i ) * ( XM_PI * 0.5f ) / tlSegments;
            const float x     = x1 + tlRadius + tlRadius * cosf( angle );
            const float y     = y1 + tlRadius + tlRadius * sinf( angle );
            path.push_back( { x, y } );
        }
    }
    else
    {
        path.push_back( { x1, y1 } );
    }

    // Top-right corner
    if ( trRadius > 0.0f && trSegments > 0 )
    {
        for ( int i = 0; i <= trSegments; ++i )
        {
            const float angle = 1.5f * XM_PI + static_cast<float>( i ) * ( XM_PI * 0.5f ) / trSegments;
            const float x     = x2 - trRadius + trRadius * cosf( angle );
            const float y     = y1 + trRadius + trRadius * sinf( angle );
            path.push_back( { x, y } );
        }
    }
    else
    {
        path.push_back( { x2, y1 } );
    }

    // Bottom-right corner
    if ( brRadius > 0.0f && brSegments > 0 )
    {
        for ( int i = 0; i <= brSegments; ++i )
        {
            const float angle = static_cast<float>( i ) * ( XM_PI * 0.5f ) / brSegments;
            const float x     = x2 - brRadius + brRadius * cosf( angle );
            const float y     = y2 - brRadius + brRadius * sinf( angle );
            path.push_back( { x, y } );
        }
    }
    else
    {
        path.push_back( { x2, y2 } );
    }

    // Bottom-left corner
    if ( blRadius > 0.0f && blSegments > 0 )
    {
        for ( int i = 0; i <= blSegments; ++i )
        {
            const float angle = 0.5f * XM_PI + static_cast<float>( i ) * ( XM_PI * 0.5f ) / blSegments;
            const float x     = x1 + blRadius + blRadius * cosf( angle );
            const float y     = y2 - blRadius + blRadius * sinf( angle );
            path.push_back( { x, y } );
        }
    }
    else
    {
        path.push_back( { x1, y2 } );
    }
}

void VectorGraphics::TessellateStrokeFromPaths( const std::vector<Float_2> &outerPath, const std::vector<Float_2> &innerPath )
{
    const auto color = ApplyAlpha( m_currentStyle.Stroke.Color );
    DZ_RETURN_IF( outerPath.empty( ) );

    if ( innerPath.empty( ) )
    {
        TessellateClosedPath( outerPath );
        return;
    }

    // Create stroke by connecting outer and inner paths
    const size_t outerSize = outerPath.size( );
    const size_t innerSize = innerPath.size( );

    // Add all outer path vertices
    for ( const auto &point : outerPath )
    {
        AddVertex( TransformPoint( point ), color );
    }

    // Add all inner path vertices (in reverse order for proper winding)
    for ( int i = static_cast<int>( innerSize ) - 1; i >= 0; --i )
    {
        AddVertex( TransformPoint( innerPath[ i ] ), color );
    }

    // Create triangles connecting outer and inner paths
    const uint32_t outerStartIndex = static_cast<uint32_t>( m_vertices.size( ) ) - static_cast<uint32_t>( outerSize + innerSize );
    const uint32_t innerStartIndex = outerStartIndex + static_cast<uint32_t>( outerSize );

    // Connect outer path to inner path
    for ( size_t i = 0; i < outerSize; ++i )
    {
        const size_t nextOuter = ( i + 1 ) % outerSize;
        const size_t currInner = ( outerSize - 1 - i ) % innerSize;
        const size_t nextInner = ( outerSize - 2 - i + innerSize ) % innerSize;

        // Create quad between outer and inner edges
        const uint32_t o1 = outerStartIndex + static_cast<uint32_t>( i );
        const uint32_t o2 = outerStartIndex + static_cast<uint32_t>( nextOuter );
        const uint32_t i1 = innerStartIndex + static_cast<uint32_t>( currInner );
        const uint32_t i2 = innerStartIndex + static_cast<uint32_t>( nextInner );

        // Two triangles for the quad
        AddTriangle( o1, i1, o2 );
        AddTriangle( o2, i1, i2 );
    }
}

void VectorGraphics::TessellateEllipticalArc( const Float_2 &start, const Float_2 &radii, const float xAxisRotation, const bool largeArcFlag, const bool sweepFlag,
                                              const Float_2 &end, std::vector<Float_2> &points ) const
{
    // Implementation of SVG elliptical arc conversion to center parameterization
    // Based on SVG specification: https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter

    if ( start.X == end.X && start.Y == end.Y )
    {
        return; // Degenerate case
    }

    // Ensure radii are positive
    const float rx = fabsf( radii.X );
    const float ry = fabsf( radii.Y );

    if ( rx == 0.0f || ry == 0.0f )
    {
        // Degenerate to line
        points.push_back( end );
        return;
    }

    const float cosRot = cosf( xAxisRotation );
    const float sinRot = sinf( xAxisRotation );

    // Step 1: Compute (x1', y1')
    const float dx      = ( start.X - end.X ) * 0.5f;
    const float dy      = ( start.Y - end.Y ) * 0.5f;
    const float x1Prime = cosRot * dx + sinRot * dy;
    const float y1Prime = -sinRot * dx + cosRot * dy;

    // Step 2: Compute (cx', cy')
    float       rxSq      = rx * rx;
    float       rySq      = ry * ry;
    const float x1PrimeSq = x1Prime * x1Prime;
    const float y1PrimeSq = y1Prime * y1Prime;

    // Correct radii if necessary
    const float lambda = x1PrimeSq / rxSq + y1PrimeSq / rySq;
    if ( lambda > 1.0f )
    {
        const float sqrtLambda = sqrtf( lambda );
        rxSq *= lambda;
        rySq *= lambda;
    }

    const float coeff = sqrtf( std::max( 0.0f, ( rxSq * rySq - rxSq * y1PrimeSq - rySq * x1PrimeSq ) / ( rxSq * y1PrimeSq + rySq * x1PrimeSq ) ) );
    const float sign  = largeArcFlag == sweepFlag ? -1.0f : 1.0f;

    const float cxPrime = sign * coeff * ( rx * y1Prime / ry );
    const float cyPrime = sign * coeff * ( -ry * x1Prime / rx );

    // Step 3: Compute (cx, cy) from (cx', cy')
    const float cx = cosRot * cxPrime - sinRot * cyPrime + ( start.X + end.X ) * 0.5f;
    const float cy = sinRot * cxPrime + cosRot * cyPrime + ( start.Y + end.Y ) * 0.5f;

    // Step 4: Compute angles
    auto vectorAngle = []( const float ux, const float uy, const float vx, const float vy ) -> float
    {
        const float dot = ux * vx + uy * vy;
        const float det = ux * vy - uy * vx;
        return atan2f( det, dot );
    };

    const float theta1 = vectorAngle( 1.0f, 0.0f, ( x1Prime - cxPrime ) / rx, ( y1Prime - cyPrime ) / ry );
    const float dTheta = vectorAngle( ( x1Prime - cxPrime ) / rx, ( y1Prime - cyPrime ) / ry, ( -x1Prime - cxPrime ) / rx, ( -y1Prime - cyPrime ) / ry );

    float deltaTheta = dTheta;
    if ( sweepFlag && deltaTheta < 0.0f )
    {
        deltaTheta += 2.0f * XM_PI;
    }
    else if ( !sweepFlag && deltaTheta > 0.0f )
    {
        deltaTheta -= 2.0f * XM_PI;
    }

    // Tessellate the arc with improved quality
    // Calculate segments based on arc length and tessellation tolerance
    const float arcLength = fabsf( deltaTheta ) * std::max( rx, ry );
    const int   segments  = std::max( 6, static_cast<int>( arcLength / m_tessellationTolerance ) );
    const float angleStep = deltaTheta / std::min( segments, 64 ); // Cap maximum segments

    for ( int i = 1; i <= segments; ++i )
    {
        const float angle    = theta1 + i * angleStep;
        const float cosAngle = cosf( angle );
        const float sinAngle = sinf( angle );

        // Point on ellipse in local coordinates
        const float localX = rx * cosAngle;
        const float localY = ry * sinAngle;

        // Transform to global coordinates
        const float x = cx + cosRot * localX - sinRot * localY;
        const float y = cy + sinRot * localX + cosRot * localY;

        points.push_back( { x, y } );
    }
}

void VectorGraphics::TessellateCircularArc( const Float_2 &center, const float radius, const float startAngle, const float endAngle, const bool clockwise,
                                            std::vector<Float_2> &points ) const
{
    DZ_RETURN_IF( radius <= 0.0f );

    float actualEndAngle = endAngle;

    float sweep = actualEndAngle - startAngle;
    if ( clockwise && sweep > 0.0f )
    {
        actualEndAngle -= 2.0f * XM_PI;
        sweep = actualEndAngle - startAngle;
    }
    else if ( !clockwise && sweep < 0.0f )
    {
        actualEndAngle += 2.0f * XM_PI;
        sweep = actualEndAngle - startAngle;
    }

    // Calculate segments based on arc length for better quality
    const float arcLength = fabsf( sweep ) * radius;
    const int   segments  = std::max( 6, static_cast<int>( arcLength / m_tessellationTolerance ) );
    const float angleStep = sweep / std::min( segments, 64 ); // Cap maximum segments

    for ( int i = 1; i <= segments; ++i )
    {
        const float angle = startAngle + i * angleStep;
        const float x     = center.X + radius * cosf( angle );
        const float y     = center.Y + radius * sinf( angle );
        points.push_back( { x, y } );
    }
}

void VectorGraphics::SetAntialiasingMode( const VGAntialiasingMode mode )
{
    m_antialiasingMode = mode;
}

VGAntialiasingMode VectorGraphics::GetAntialiasingMode( ) const
{
    return m_antialiasingMode;
}

void VectorGraphics::SetAntialiasingWidth( const float width )
{
    m_antialiasingWidth = std::max( 0.0f, width );
}

float VectorGraphics::GetAntialiasingWidth( ) const
{
    return m_antialiasingWidth;
}
