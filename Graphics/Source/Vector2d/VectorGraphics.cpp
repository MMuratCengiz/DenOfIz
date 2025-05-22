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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace DenOfIz;
using namespace DirectX;

VectorGraphics::VectorGraphics( const VectorGraphicsDesc &desc ) :
    m_vertexBufferSize( desc.InitialVertexBufferSize ), m_indexBufferSize( desc.InitialIndexBufferSize ), m_logicalDevice( desc.LogicalDevice ),
    m_tessellationTolerance( desc.DefaultTessellationTolerance )
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

    const auto identity = XMMatrixIdentity( );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), identity );

    BufferDesc vbDesc;
    vbDesc.HeapType   = HeapType::CPU_GPU;
    vbDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    vbDesc.NumBytes   = m_vertexBufferSize;
    vbDesc.Descriptor = ResourceDescriptor::Buffer;
    m_vertexBuffer    = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc;
    ibDesc.HeapType   = HeapType::CPU_GPU;
    ibDesc.Usages     = ResourceUsage::IndexBuffer;
    ibDesc.NumBytes   = m_indexBufferSize;
    ibDesc.Descriptor = ResourceDescriptor::Buffer;
    m_indexBuffer     = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );
}

VectorGraphics::~VectorGraphics( ) = default;

void VectorGraphics::BeginBatch( ICommandList *commandList )
{
    m_commandList = commandList;
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
        m_pipeline->UpdateProjection( 0, projMatrix ); // TODO: frame index management
        if ( const auto bindGroup = m_pipeline->GetBindGroup( 0, 0 ) )
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

void VectorGraphics::Save( )
{
    m_transformStack.push_back( m_currentStyle.Transform );
}

void VectorGraphics::Restore( )
{
    if ( !m_transformStack.empty( ) )
    {
        m_currentStyle.Transform = m_transformStack.back( );
        m_transformStack.pop_back( );
    }
}

void VectorGraphics::PushTransform( const Float_4x4 &transform )
{
    Save( );
    Transform( transform );
}

void VectorGraphics::PopTransform( )
{
    Restore( );
}

void VectorGraphics::ResetTransform( )
{
    const auto identity = XMMatrixIdentity( );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), identity );
}

void VectorGraphics::Transform( const Float_4x4 &matrix )
{
    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto newTransform     = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &matrix ) );
    const auto result           = XMMatrixMultiply( currentTransform, newTransform );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
}

void VectorGraphics::Translate( const Float_2 &offset )
{
    const auto translation      = XMMatrixTranslation( offset.X, offset.Y, 0.0f );
    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto result           = XMMatrixMultiply( currentTransform, translation );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
}

void VectorGraphics::Scale( const Float_2 &scale )
{
    const auto scaling          = XMMatrixScaling( scale.X, scale.Y, 1.0f );
    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto result           = XMMatrixMultiply( currentTransform, scaling );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
}

void VectorGraphics::Scale( const float scale )
{
    Scale( { scale, scale } );
}

void VectorGraphics::Rotate( const float angleRadians )
{
    const auto rotation         = XMMatrixRotationZ( angleRadians );
    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto result           = XMMatrixMultiply( currentTransform, rotation );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
}

void VectorGraphics::Rotate( const float angleRadians, const Float_2 &center )
{
    const auto translate1 = XMMatrixTranslation( -center.X, -center.Y, 0.0f );
    const auto rotation   = XMMatrixRotationZ( angleRadians );
    const auto translate2 = XMMatrixTranslation( center.X, center.Y, 0.0f );
    const auto transform  = XMMatrixMultiply( XMMatrixMultiply( translate1, rotation ), translate2 );

    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto result           = XMMatrixMultiply( currentTransform, transform );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
}

void VectorGraphics::Skew( const Float_2 &skew )
{
    // Create skew matrix manually
    const XMFLOAT4X4 skewMatrix = { 1.0f, skew.Y, 0.0f, 0.0f, skew.X, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    const auto transform        = XMLoadFloat4x4( &skewMatrix );
    const auto currentTransform = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto result           = XMMatrixMultiply( currentTransform, transform );
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &m_currentStyle.Transform ), result );
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
    // TODO: Implement clipping
}

void VectorGraphics::ClipPath( const VGPath2D &path )
{
    // TODO: Implement clipping
}

void VectorGraphics::ResetClip( )
{
    // TODO: Implement clipping
}

void VectorGraphics::SetTessellationTolerance( float tolerance )
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

    // Build a list of points for the path
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
                pathPoints.clear( ); // Start new sub-path
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

        // TODO: Implement other path commands (arcs, smooth curves, etc.)
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
        AddVertex( TransformPoint( rect.TopLeft ), color );                           // Top Left
        AddVertex( TransformPoint( { rect.BottomRight.X, rect.TopLeft.Y } ), color ); // Top Right
        AddVertex( TransformPoint( rect.BottomRight ), color );                       // Bottom Right
        AddVertex( TransformPoint( { rect.TopLeft.X, rect.BottomRight.Y } ), color ); // Bottom Left

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
    // TODO: Implement rounded rectangle tessellation
    // For now, fall back to regular rectangle
    VGRect simpleRect;
    simpleRect.TopLeft     = rect.TopLeft;
    simpleRect.BottomRight = rect.BottomRight;
    TessellateRect( simpleRect, forStroke );
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
        // Generate filled circle using triangle fan
        AddVertex( TransformPoint( circle.Center ), color ); // Center vertex

        // Add circle perimeter vertices (only segments, not segments + 1)
        for ( int i = 0; i < segments; ++i )
        {
            const float angle = i * angleStep;
            Float_2     point = { circle.Center.X + circle.Radius * cosf( angle ), circle.Center.Y + circle.Radius * sinf( angle ) };
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

void VectorGraphics::TessellateEllipse( const VGEllipse &ellipse, const bool forStroke )
{
    // TODO: Implement ellipse tessellation with proper scaling and rotation
    // For now, use circle approximation
    VGCircle approxCircle;
    approxCircle.Center = ellipse.Center;
    approxCircle.Radius = ( ellipse.Radii.X + ellipse.Radii.Y ) * 0.5f; // Average radius
    TessellateCircle( approxCircle, forStroke );
}

void VectorGraphics::TessellatePolygon( const VGPolygon &polygon, const bool forStroke )
{
    DZ_RETURN_IF( polygon.Points.NumElements( ) < 3 );

    const auto color = forStroke ? ApplyAlpha( m_currentStyle.Stroke.Color ) : ApplyAlpha( m_currentStyle.Fill.Color );

    if ( forStroke )
    {
        // Generate stroke for polygon edges
        for ( uint32_t i = 0; i < polygon.Points.NumElements( ); ++i )
        {
            const uint32_t nextIndex = ( i + 1 ) % polygon.Points.NumElements( );
            if ( !polygon.IsClosed && nextIndex == 0 )
                break;

            VGLine line;
            line.StartPoint = polygon.Points.GetElement( i );
            line.EndPoint   = polygon.Points.GetElement( nextIndex );
            line.Thickness  = m_currentStyle.Stroke.Width;
            TessellateLine( line );
        }
    }
    else
    {
        // Simple triangulation using fan method (works for convex polygons)
        const uint32_t baseVertexIndex = static_cast<uint32_t>( m_vertices.size( ) );

        // Add all polygon vertices
        for ( uint32_t i = 0; i < polygon.Points.NumElements( ); ++i )
        {
            AddVertex( TransformPoint( polygon.Points.GetElement( i ) ), color );
        }

        // Create triangles using fan triangulation (clockwise winding)
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
    DZ_RETURN_IF( requiredSize <= m_vertexBufferSize );

    const uint32_t newSize = std::max( requiredSize, m_vertexBufferSize + m_vertexBufferSize / 2 );
    BufferDesc     bufferDesc;
    bufferDesc.HeapType   = HeapType::CPU_GPU;
    bufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    bufferDesc.NumBytes   = newSize;
    bufferDesc.Descriptor = ResourceDescriptor::Buffer;

    m_vertexBuffer     = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );
    m_vertexBufferSize = newSize;
}

void VectorGraphics::EnsureIndexBufferCapacity( const uint32_t indexCount )
{
    const uint32_t requiredSize = indexCount * sizeof( uint32_t );
    DZ_RETURN_IF( requiredSize <= m_indexBufferSize );

    const uint32_t newSize = std::max( requiredSize, m_indexBufferSize + m_indexBufferSize / 2 );
    BufferDesc     bufferDesc;
    bufferDesc.HeapType   = HeapType::CPU_GPU;
    bufferDesc.Usages     = ResourceUsage::IndexBuffer;
    bufferDesc.NumBytes   = newSize;
    bufferDesc.Descriptor = ResourceDescriptor::Buffer;

    m_indexBuffer     = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );
    m_indexBufferSize = newSize;
}

void VectorGraphics::UpdateBuffers( )
{
    if ( !m_vertices.empty( ) )
    {
        EnsureVertexBufferCapacity( static_cast<uint32_t>( m_vertices.size( ) ) );
        if ( m_vertexBuffer )
        {
            if ( void *mappedMemory = m_vertexBuffer->MapMemory( ) )
            {
                memcpy( mappedMemory, m_vertices.data( ), m_vertices.size( ) * sizeof( VGVertex ) );
                m_vertexBuffer->UnmapMemory( );
            }
        }
    }

    if ( !m_indices.empty( ) )
    {
        EnsureIndexBufferCapacity( static_cast<uint32_t>( m_indices.size( ) ) );
        if ( m_indexBuffer )
        {
            if ( void *mappedMemory = m_indexBuffer->MapMemory( ) )
            {
                memcpy( mappedMemory, m_indices.data( ), m_indices.size( ) * sizeof( uint32_t ) );
                m_indexBuffer->UnmapMemory( );
            }
        }
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
    vertex.Position     = position;
    vertex.Color        = color;
    vertex.TexCoord     = texCoord;
    vertex.GradientData = { 0.0f, 0.0f, 0.0f, 0.0f }; // TODO: Setup gradient data
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
    const auto transform   = XMLoadFloat4x4( reinterpret_cast<const XMFLOAT4X4 *>( &m_currentStyle.Transform ) );
    const auto vector      = XMVectorSet( point.X, point.Y, 0.0f, 1.0f );
    const auto transformed = XMVector4Transform( vector, transform );

    Float_2 result;
    XMStoreFloat2( reinterpret_cast<XMFLOAT2 *>( &result ), transformed );
    return result;
}

Float_4x4 VectorGraphics::GetCurrentTransform( ) const
{
    return m_currentStyle.Transform;
}

Float_4 VectorGraphics::ApplyAlpha( const Float_4 &color ) const
{
    return { color.X, color.Y, color.Z, color.W * m_currentStyle.Composite.Alpha };
}

void VectorGraphics::SetupGradientVertexData( VGVertex &vertex, const Float_2 &position )
{
    // TODO: Implement gradient vertex data setup
    // This would calculate gradient coordinates based on fill style
}

void VectorGraphics::ClearBatch( )
{
    m_vertices.clear( );
    m_indices.clear( );
    m_renderCommands.clear( );
}

void VectorGraphics::TessellateQuadraticBezier( const Float_2 &p0, const Float_2 &p1, const Float_2 &p2, std::vector<Float_2> &points )
{
    // Adaptive tessellation based on curvature
    const float flatnessTolerance = m_tessellationTolerance;

    // Check if the curve is flat enough
    const float dist = DistancePointToLine( p1, p0, p2 );
    if ( dist < flatnessTolerance )
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
    // Adaptive tessellation based on curvature
    const float flatnessTolerance = m_tessellationTolerance;

    // Check if the curve is flat enough by testing control points
    const float dist1   = DistancePointToLine( p1, p0, p3 );
    const float dist2   = DistancePointToLine( p2, p0, p3 );
    const float maxDist = std::max( dist1, dist2 );

    if ( maxDist < flatnessTolerance )
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

void VectorGraphics::TriangulatePolygon( const std::vector<Float_2> &points, std::vector<uint32_t> &indices )
{
    indices.clear( );
    DZ_RETURN_IF( points.size( ) < 3 );

    // Simple ear clipping algorithm for polygon triangulation
    std::vector<uint32_t> remaining;
    for ( uint32_t i = 0; i < points.size( ); ++i )
    {
        remaining.push_back( i );
    }

    while ( remaining.size( ) > 3 )
    {
        bool earFound = false;

        for ( size_t i = 0; i < remaining.size( ); ++i )
        {
            uint32_t prev = remaining[ ( i + remaining.size( ) - 1 ) % remaining.size( ) ];
            uint32_t curr = remaining[ i ];
            uint32_t next = remaining[ ( i + 1 ) % remaining.size( ) ];

            // Check if this is a valid ear (convex vertex)
            Float_2 a = points[ prev ];
            Float_2 b = points[ curr ];
            Float_2 c = points[ next ];

            // Check if triangle is oriented correctly (clockwise)
            const float cross = Cross2D( { c.X - a.X, c.Y - a.Y }, { b.X - a.X, b.Y - a.Y } );
            if ( cross > 0 )
            {
                continue; // Reflex vertex, not an ear
            }

            // Check if any other vertex is inside this triangle
            bool isEar = true;
            for ( size_t j = 0; j < remaining.size( ); ++j )
            {
                if ( j == i || j == ( i + remaining.size( ) - 1 ) % remaining.size( ) || j == ( i + 1 ) % remaining.size( ) )
                {
                    continue;
                }

                if ( IsPointInTriangle( points[ remaining[ j ] ], a, b, c ) )
                {
                    isEar = false;
                    break;
                }
            }

            if ( isEar )
            {
                // Add triangle with clockwise winding
                indices.push_back( prev );
                indices.push_back( next );
                indices.push_back( curr );

                // Remove current vertex
                remaining.erase( remaining.begin( ) + i );
                earFound = true;
                break;
            }
        }

        if ( !earFound )
        {
            // Fallback: just use fan triangulation (clockwise)
            indices.clear( );
            for ( size_t i = 1; i < points.size( ) - 1; ++i )
            {
                indices.push_back( 0 );
                indices.push_back( static_cast<uint32_t>( i + 1 ) );
                indices.push_back( static_cast<uint32_t>( i ) );
            }
            break;
        }
    }

    // Add the final triangle (clockwise)
    if ( remaining.size( ) == 3 )
    {
        indices.push_back( remaining[ 0 ] );
        indices.push_back( remaining[ 2 ] );
        indices.push_back( remaining[ 1 ] );
    }
}

float VectorGraphics::DistancePointToLine( const Float_2 &point, const Float_2 &lineStart, const Float_2 &lineEnd )
{
    const float dx       = lineEnd.X - lineStart.X;
    const float dy       = lineEnd.Y - lineStart.Y;
    const float lengthSq = dx * dx + dy * dy;

    if ( lengthSq < 1e-6f ) // Degenerate line
    {
        return sqrtf( ( point.X - lineStart.X ) * ( point.X - lineStart.X ) + ( point.Y - lineStart.Y ) * ( point.Y - lineStart.Y ) );
    }

    float t = ( ( point.X - lineStart.X ) * dx + ( point.Y - lineStart.Y ) * dy ) / lengthSq;
    t       = std::clamp( t, 0.0f, 1.0f );

    const float projX = lineStart.X + t * dx;
    const float projY = lineStart.Y + t * dy;

    const float distX = point.X - projX;
    const float distY = point.Y - projY;

    return sqrtf( distX * distX + distY * distY );
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
    return a.X * b.Y - a.Y * b.X;
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

        // Calculate direction and perpendicular
        Float_2     direction = { next.X - current.X, next.Y - current.Y };
        const float length    = sqrtf( direction.X * direction.X + direction.Y * direction.Y );
        if ( length < 1e-6f )
        {
            continue;
        }

        direction.X /= length;
        direction.Y /= length;

        const Float_2 perpendicular = { -direction.Y, direction.X };

        // Create quad for line segment
        Float_2 p1 = { current.X + perpendicular.X * halfWidth, current.Y + perpendicular.Y * halfWidth };
        Float_2 p2 = { current.X - perpendicular.X * halfWidth, current.Y - perpendicular.Y * halfWidth };
        Float_2 p3 = { next.X - perpendicular.X * halfWidth, next.Y - perpendicular.Y * halfWidth };
        Float_2 p4 = { next.X + perpendicular.X * halfWidth, next.Y + perpendicular.Y * halfWidth };

        AddVertex( TransformPoint( p1 ), color );
        AddVertex( TransformPoint( p2 ), color );
        AddVertex( TransformPoint( p3 ), color );
        AddVertex( TransformPoint( p4 ), color );

        const uint32_t baseIndex = static_cast<uint32_t>( m_vertices.size( ) ) - 4;
        AddQuad( baseIndex, baseIndex + 1, baseIndex + 2, baseIndex + 3 );

        // Add line joins
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
                const float angle = ( isStart ? static_cast<float>( M_PI ) : 0.0f ) + ( static_cast<float>( M_PI ) * i / segments );
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
        }
        break;

    case VGLineJoin::Round:
        {
            // Generate arc between the two directions
            constexpr int segments = 6;
            AddVertex( TransformPoint( point ), color );

            for ( int i = 0; i <= segments; ++i )
            {
                const float t          = static_cast<float>( i ) / segments;
                Float_2     lerpedPerp = { perp1.X * ( 1.0f - t ) + perp2.X * t, perp1.Y * ( 1.0f - t ) + perp2.Y * t };

                const float length = sqrtf( lerpedPerp.X * lerpedPerp.X + lerpedPerp.Y * lerpedPerp.Y );
                if ( length > 1e-6f )
                {
                    lerpedPerp.X = ( lerpedPerp.X / length ) * halfWidth;
                    lerpedPerp.Y = ( lerpedPerp.Y / length ) * halfWidth;
                }

                Float_2 arcPoint = { point.X + lerpedPerp.X, point.Y + lerpedPerp.Y };
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
