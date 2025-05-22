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

#include <DenOfIzExamples/VGExample.h>

#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

//
void VGExample::Init( )
{
    UpdateProjectionMatrix( );

    // Initialize debug renderer
    FrameDebugRendererDesc debugRendererDesc{ };
    debugRendererDesc.GraphicsApi   = m_graphicsApi;
    debugRendererDesc.LogicalDevice = m_logicalDevice;
    debugRendererDesc.ScreenWidth   = m_windowDesc.Width;
    debugRendererDesc.ScreenHeight  = m_windowDesc.Height;
    debugRendererDesc.TextColor     = { 0.8f, 1.0f, 0.8f, 1.0f };
    debugRendererDesc.Enabled       = true;
    m_debugRenderer                 = std::make_unique<FrameDebugRenderer>( debugRendererDesc );

    // Initialize vector graphics system
    InitializeVectorGraphics( );

    // Initialize animation state
    m_animationTime = 0.0f;
    m_rotationAngle = 0.0f;
    m_scaleAnimTime = 0.0f;
    m_colorAnimTime = 0.0f;
}

void VGExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void VGExample::HandleEvent( Event &event )
{
    IExample::HandleEvent( event );
}

void VGExample::Update( )
{
    m_time.Tick( );
    m_debugRenderer->UpdateStats( m_time.GetDeltaTime( ) );

    // Update animation time
    const float deltaTime = m_time.GetDeltaTime( );
    m_animationTime += deltaTime;
    m_rotationAngle += deltaTime * 0.5f; // 0.5 rad/sec
    m_scaleAnimTime += deltaTime * 2.0f; // Faster scale animation
    m_colorAnimTime += deltaTime * 1.5f; // Color cycling

    RenderAndPresentFrame( );
}

void VGExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingDesc            renderingDesc{ };
    RenderingAttachmentDesc &renderingAttachmentDesc = renderingDesc.RTAttachments.EmplaceElement( );
    renderingAttachmentDesc.Resource                 = renderTarget;
    renderingAttachmentDesc.SetClearColor( 0.31, 0.3, 0.33, 1.0 );
    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    // === Vector Graphics Rendering ===
    // Begin vector graphics batch
    m_vectorGraphics->BeginBatch( commandList );

    // Render different demo sections
    RenderBasicShapes( );
    RenderAnimatedShapes( );
    RenderGradientShapes( );
    RenderCurveDemo( );
    RenderTransformDemo( );
    RenderComplexDemo( );

    // End vector graphics batch (this will flush all geometry)
    m_vectorGraphics->EndBatch( );

    // Render debug info
    m_debugRenderer->Render( commandList );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void VGExample::UpdateProjectionMatrix( )
{
    // Create 2D orthographic projection matrix
    constexpr float left   = 0.0f;
    const float     right  = static_cast<float>( m_windowDesc.Width );
    const float     bottom = static_cast<float>( m_windowDesc.Height );
    constexpr float top    = 0.0f;
    constexpr float nearZ  = -1.0f;
    constexpr float farZ   = 1.0f;

    const auto projection = XMMatrixOrthographicOffCenterLH( left, right, bottom, top, nearZ, farZ );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
}

void VGExample::InitializeVectorGraphics( )
{
    // Create VG Transform with screen dimensions
    m_vgTransform = std::make_unique<VGTransform>( m_windowDesc.Width, m_windowDesc.Height );

    // Create VG Pipeline
    VGPipelineDesc pipelineDesc{ };
    pipelineDesc.LogicalDevice = m_logicalDevice;
    pipelineDesc.NumFrames     = 3;
    pipelineDesc.SetupData     = true; // Use default projection data setup
    m_vgPipeline               = std::make_unique<VGPipeline>( pipelineDesc );

    // Create Vector Graphics renderer
    VectorGraphicsDesc vgDesc{ };
    vgDesc.LogicalDevice                = m_logicalDevice;
    vgDesc.InitialVertexBufferSize      = 256 * 1024;
    vgDesc.InitialIndexBufferSize       = 128 * 1024;
    vgDesc.DefaultTessellationTolerance = 2.0f;

    m_vectorGraphics = std::make_unique<VectorGraphics>( vgDesc );
    m_vectorGraphics->SetPipeline( m_vgPipeline.get( ) );
    m_vectorGraphics->SetTransform( m_vgTransform.get( ) );

    // Set default tessellation tolerance for smooth curves
    m_vectorGraphics->SetTessellationTolerance( 2.0f );
}

void VGExample::RenderBasicShapes( ) const
{
    // === Basic Shapes Demo (Top-Left Quadrant) ===
    // Save transform state
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 50.0f, 50.0f } );

    // 1. Filled Rectangle
    m_vectorGraphics->SetFillColor( { 1.0f, 0.3f, 0.3f, 1.0f } ); // Red
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillRect( { 0.0f, 0.0f }, { 80.0f, 60.0f } );

    // 2. Stroked Rectangle
    m_vectorGraphics->Translate( { 100.0f, 0.0f } );
    m_vectorGraphics->SetFillEnabled( false );
    m_vectorGraphics->SetStrokeColor( { 0.3f, 1.0f, 0.3f, 1.0f } ); // Green
    m_vectorGraphics->SetStrokeWidth( 3.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->StrokeRect( { 0.0f, 0.0f }, { 80.0f, 60.0f } );

    // 3. Filled Circle
    m_vectorGraphics->Translate( { 100.0f, 0.0f } );
    m_vectorGraphics->SetFillColor( { 0.3f, 0.3f, 1.0f, 1.0f } ); // Blue
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillCircle( { 40.0f, 30.0f }, 25.0f );

    // 4. Stroked Circle
    m_vectorGraphics->Translate( { 100.0f, 0.0f } );
    m_vectorGraphics->SetFillEnabled( false );
    m_vectorGraphics->SetStrokeColor( { 1.0f, 1.0f, 0.3f, 1.0f } ); // Yellow
    m_vectorGraphics->SetStrokeWidth( 2.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->StrokeCircle( { 40.0f, 30.0f }, 25.0f );

    // 5. Lines with different thickness
    m_vectorGraphics->Translate( { -300.0f, 80.0f } );
    m_vectorGraphics->SetStrokeColor( { 1.0f, 0.5f, 0.0f, 1.0f } ); // Orange

    for ( int i = 0; i < 5; ++i )
    {
        const float thickness = 1.0f + i * 2.0f;
        m_vectorGraphics->SetStrokeWidth( thickness );
        m_vectorGraphics->DrawLine( { 0.0f, i * 15.0f }, { 100.0f, i * 15.0f }, thickness );
    }

    // 6. Polygon (Triangle)
    m_vectorGraphics->Translate( { 150.0f, -10.0f } );
    VGPolygon triangle;
    triangle.Points.AddElement( { 40.0f, 0.0f } );  // Top
    triangle.Points.AddElement( { 0.0f, 60.0f } );  // Bottom-left
    triangle.Points.AddElement( { 80.0f, 60.0f } ); // Bottom-right
    triangle.IsClosed = true;

    m_vectorGraphics->SetFillColor( { 0.8f, 0.3f, 0.8f, 1.0f } ); // Magenta
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillPolygon( triangle );

    // Restore transform state
    m_vectorGraphics->Restore( );
}

void VGExample::RenderAnimatedShapes( ) const
{
    // === Animated Shapes Demo (Top-Right Quadrant) ===
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 640.0f, 50.0f } );

    // 1. Rotating Square
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 50.0f, 50.0f } );
    m_vectorGraphics->Rotate( m_rotationAngle, { 25.0f, 25.0f } );
    m_vectorGraphics->SetFillColor( GetAnimatedColor( m_colorAnimTime, 0.0f ) );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillRect( { -25.0f, -25.0f }, { 25.0f, 25.0f } );
    m_vectorGraphics->Restore( );

    // 2. Pulsating Circle
    const float scale = 1.0f + 0.3f * sinf( m_scaleAnimTime );
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 150.0f, 50.0f } );
    m_vectorGraphics->Scale( scale );
    m_vectorGraphics->SetFillColor( GetAnimatedColor( m_colorAnimTime, 1.0f ) );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillCircle( { 0.0f, 0.0f }, 25.0f );
    m_vectorGraphics->Restore( );

    // 3. Orbiting Circles
    constexpr int numOrbiters = 6;

    for ( int i = 0; i < numOrbiters; ++i )
    {
        constexpr float   orbitRadius = 50.0f;
        constexpr Float_2 center      = { 300.0f, 80.0f };
        const float       angle       = m_animationTime + i * 2.0f * XM_PI / numOrbiters;
        Float_2           pos         = GetCircularPosition( orbitRadius, angle, center );

        m_vectorGraphics->SetFillColor( GetAnimatedColor( m_colorAnimTime, i * 0.5f ) );
        m_vectorGraphics->SetFillEnabled( true );
        m_vectorGraphics->SetStrokeEnabled( false );
        m_vectorGraphics->FillCircle( pos, 8.0f );
    }

    // 4. Breathing Rectangle
    const float breathScale = 1.0f + 0.2f * sinf( m_animationTime * 3.0f );
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 450.0f, 50.0f } );
    m_vectorGraphics->Scale( { breathScale, breathScale } );
    m_vectorGraphics->SetFillColor( { 0.5f, 0.8f, 1.0f, 0.8f } );
    m_vectorGraphics->SetStrokeColor( { 0.2f, 0.4f, 0.8f, 1.0f } );
    m_vectorGraphics->SetStrokeWidth( 2.0f );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->DrawRect( { -30.0f, -20.0f }, { 30.0f, 20.0f } );
    m_vectorGraphics->Restore( );

    m_vectorGraphics->Restore( );
}

void VGExample::RenderGradientShapes( ) const
{
    // === Gradient Demo (Bottom-Left Quadrant) ===
    // Note: Gradients are set up but full shader support would be needed for rendering

    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 50.0f, 400.0f } );

    // For now, we'll simulate gradients with multiple colored shapes
    // This demonstrates the API even though full gradient rendering isn't implemented yet

    // 1. "Linear Gradient" Rectangle (simulated with color bands)
    for ( int i = 0; i < 10; ++i )
    {
        const float t     = i / 9.0f;
        Float_4     color = { 1.0f * ( 1.0f - t ) + 0.0f * t, // Red to Blue
                              0.0f, 0.0f * ( 1.0f - t ) + 1.0f * t, 1.0f };

        m_vectorGraphics->SetFillColor( color );
        m_vectorGraphics->SetFillEnabled( true );
        m_vectorGraphics->SetStrokeEnabled( false );
        m_vectorGraphics->FillRect( { i * 8.0f, 0.0f }, { ( i + 1 ) * 8.0f, 60.0f } );
    }

    // 2. "Radial Gradient" Circle (simulated with concentric circles)
    m_vectorGraphics->Translate( { 150.0f, 30.0f } );
    for ( int i = 10; i >= 1; --i )
    {
        const float t     = ( 10 - i ) / 9.0f;
        Float_4     color = { 1.0f * ( 1.0f - t ) + 1.0f * t, // White to Yellow
                              1.0f * ( 1.0f - t ) + 1.0f * t, 1.0f * ( 1.0f - t ) + 0.0f * t, 1.0f };

        m_vectorGraphics->SetFillColor( color );
        m_vectorGraphics->SetFillEnabled( true );
        m_vectorGraphics->SetStrokeEnabled( false );
        m_vectorGraphics->FillCircle( { 0.0f, 0.0f }, i * 3.0f );
    }

    // 3. Multiple Alpha Levels
    m_vectorGraphics->Translate( { 100.0f, -30.0f } );
    for ( int i = 0; i < 5; ++i )
    {
        const float alpha = ( i + 1 ) * 0.2f;
        m_vectorGraphics->SetAlpha( alpha );
        m_vectorGraphics->SetFillColor( { 0.2f, 1.0f, 0.4f, 1.0f } );
        m_vectorGraphics->SetFillEnabled( true );
        m_vectorGraphics->SetStrokeEnabled( false );
        m_vectorGraphics->FillRect( { i * 25.0f, 0.0f }, { i * 25.0f + 20.0f, 60.0f } );
    }
    m_vectorGraphics->SetAlpha( 1.0f ); // Reset alpha

    m_vectorGraphics->Restore( );
}

void VGExample::RenderCurveDemo( ) const
{
    // === Curve Rendering Demo (Middle-Left Section) ===

    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 50.0f, 250.0f } );

    // 1. Quadratic Bézier Curve - Animated
    m_vectorGraphics->Save( );
    m_vectorGraphics->SetStrokeColor( { 1.0f, 0.4f, 0.2f, 1.0f } ); // Orange
    m_vectorGraphics->SetStrokeWidth( 3.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->SetFillEnabled( false );

    // Animated control point
    const Float_2 animatedControlPoint = { 100.0f + 30.0f * sinf( m_animationTime * 2.0f ), -20.0f + 40.0f * cosf( m_animationTime * 1.5f ) };

    const VGPath2D quadraticPath;
    quadraticPath.MoveTo( { 0.0f, 0.0f } );
    quadraticPath.QuadraticCurveTo( animatedControlPoint, { 150.0f, 0.0f } );
    m_vectorGraphics->StrokePath( quadraticPath );

    // Draw control point and handles
    m_vectorGraphics->SetStrokeColor( { 0.6f, 0.6f, 0.6f, 0.7f } );
    m_vectorGraphics->SetStrokeWidth( 1.0f );
    m_vectorGraphics->DrawLine( { 0.0f, 0.0f }, animatedControlPoint, 1.0f );
    m_vectorGraphics->DrawLine( animatedControlPoint, { 150.0f, 0.0f }, 1.0f );

    // Draw control point
    m_vectorGraphics->SetFillColor( { 1.0f, 0.2f, 0.2f, 1.0f } );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillCircle( animatedControlPoint, 4.0f );

    m_vectorGraphics->Restore( );

    // 2. Cubic Bézier Curve - Smooth S-curve
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 0.0f, 80.0f } );
    m_vectorGraphics->SetStrokeColor( { 0.2f, 0.8f, 0.4f, 1.0f } ); // Green
    m_vectorGraphics->SetStrokeWidth( 4.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->SetFillEnabled( false );

    const VGPath2D cubicPath;
    cubicPath.MoveTo( { 0.0f, 0.0f } );
    cubicPath.CubicCurveTo( { 50.0f, -40.0f }, { 100.0f, 40.0f }, { 150.0f, 0.0f } );
    m_vectorGraphics->StrokePath( cubicPath );

    // Show control points
    m_vectorGraphics->SetStrokeColor( { 0.6f, 0.6f, 0.6f, 0.5f } );
    m_vectorGraphics->SetStrokeWidth( 1.0f );
    m_vectorGraphics->DrawLine( { 0.0f, 0.0f }, { 50.0f, -40.0f }, 1.0f );
    m_vectorGraphics->DrawLine( { 100.0f, 40.0f }, { 150.0f, 0.0f }, 1.0f );

    m_vectorGraphics->SetFillColor( { 0.2f, 0.8f, 0.4f, 1.0f } );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillCircle( { 50.0f, -40.0f }, 3.0f );
    m_vectorGraphics->FillCircle( { 100.0f, 40.0f }, 3.0f );

    m_vectorGraphics->Restore( );

    // 3. Wave Pattern - Multiple connected curves
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 200.0f, 40.0f } );
    m_vectorGraphics->SetStrokeColor( { 0.4f, 0.2f, 1.0f, 1.0f } ); // Purple
    m_vectorGraphics->SetStrokeWidth( 3.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->SetFillEnabled( false );

    const VGPath2D wavePath;
    wavePath.MoveTo( { 0.0f, 0.0f } );

    // Create a smooth wave using multiple cubic curves
    constexpr int numWaves = 4;

    for ( int i = 0; i < numWaves; ++i )
    {
        constexpr float waveLength = 40.0f;
        constexpr float amplitude  = 25.0f;
        const float     x1         = i * waveLength;
        const float     x2         = ( i + 0.5f ) * waveLength;
        const float     x3         = ( i + 1 ) * waveLength;

        const float y1 = i % 2 == 0 ? -amplitude : amplitude;
        const float y2 = i % 2 == 0 ? amplitude : -amplitude;

        // Add time-based animation to the wave
        const float animOffset = sinf( m_animationTime + i * 0.5f ) * 10.0f;

        wavePath.CubicCurveTo( { x1 + waveLength * 0.3f, y1 + animOffset }, { x2 - waveLength * 0.3f, y2 + animOffset },
                               { x3, i + 1 < numWaves ? ( ( i + 1 ) % 2 == 0 ? -amplitude : amplitude ) : 0.0f } );
    }

    m_vectorGraphics->StrokePath( wavePath );
    m_vectorGraphics->Restore( );

    // 4. Heart Shape - Complex closed curve
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 420.0f, 60.0f } );

    // Animate the heart size
    const float heartScale = 1.0f + 0.2f * sinf( m_animationTime * 3.0f );
    m_vectorGraphics->Scale( heartScale );

    m_vectorGraphics->SetFillColor( { 1.0f, 0.2f, 0.3f, 1.0f } ); // Red
    m_vectorGraphics->SetStrokeColor( { 0.8f, 0.1f, 0.2f, 1.0f } );
    m_vectorGraphics->SetStrokeWidth( 2.0f );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( true );

    const VGPath2D heartPath;
    // Heart shape using Bézier curves
    heartPath.MoveTo( { 0.0f, 15.0f } );
    heartPath.CubicCurveTo( { -25.0f, -10.0f }, { -25.0f, -25.0f }, { 0.0f, -5.0f } );
    heartPath.CubicCurveTo( { 25.0f, -25.0f }, { 25.0f, -10.0f }, { 0.0f, 15.0f } );
    heartPath.Close( );

    m_vectorGraphics->FillPath( heartPath );
    m_vectorGraphics->StrokePath( heartPath );
    m_vectorGraphics->Restore( );

    // 5. Spiral - Parametric curve
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 550.0f, 60.0f } );
    m_vectorGraphics->SetStrokeColor( GetAnimatedColor( m_colorAnimTime, 2.0f ) );
    m_vectorGraphics->SetStrokeWidth( 3.0f );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->SetFillEnabled( false );

    const VGPath2D spiralPath;
    bool           firstPoint = true;

    // Create spiral using line segments (could be improved with curves)
    const float   spiralTurns    = 3.0f + sinf( m_animationTime ) * 0.5f;
    constexpr int spiralSegments = 100;

    for ( int i = 0; i <= spiralSegments; ++i )
    {
        const float t      = static_cast<float>( i ) / spiralSegments;
        const float angle  = t * spiralTurns * 2.0f * XM_PI;
        const float radius = t * 30.0f;

        Float_2 point = { radius * cosf( angle ), radius * sinf( angle ) };

        if ( firstPoint )
        {
            spiralPath.MoveTo( point );
            firstPoint = false;
        }
        else
        {
            spiralPath.LineTo( point );
        }
    }

    m_vectorGraphics->StrokePath( spiralPath );
    m_vectorGraphics->Restore( );

    m_vectorGraphics->Restore( );
}

void VGExample::RenderTransformDemo( ) const
{
    // === Transform Demo (Bottom-Right Quadrant) ===

    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 640.0f, 400.0f } );

    // 1. Nested Transforms
    m_vectorGraphics->SetFillColor( { 1.0f, 0.8f, 0.2f, 1.0f } );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );

    for ( int i = 0; i < 5; ++i )
    {
        m_vectorGraphics->Save( );
        m_vectorGraphics->Translate( { 30.0f, 30.0f } );
        m_vectorGraphics->Rotate( m_rotationAngle * ( i + 1 ) * 0.3f );
        m_vectorGraphics->Scale( 0.8f );

        m_vectorGraphics->FillRect( { -15.0f, -15.0f }, { 15.0f, 15.0f } );
        m_vectorGraphics->Restore( );
    }

    // 2. Skew Transform
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 150.0f, 50.0f } );
    m_vectorGraphics->Skew( { sinf( m_animationTime ) * 0.3f, cosf( m_animationTime * 0.7f ) * 0.2f } );
    m_vectorGraphics->SetFillColor( { 0.8f, 0.2f, 1.0f, 1.0f } );
    m_vectorGraphics->FillRect( { -25.0f, -25.0f }, { 25.0f, 25.0f } );
    m_vectorGraphics->Restore( );

    // 3. Combined Transforms
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 300.0f, 50.0f } );
    m_vectorGraphics->Rotate( m_rotationAngle * 0.5f );
    m_vectorGraphics->Scale( { 1.0f + 0.3f * sinf( m_scaleAnimTime ), 1.0f + 0.3f * cosf( m_scaleAnimTime ) } );
    m_vectorGraphics->SetFillColor( { 0.2f, 0.8f, 1.0f, 1.0f } );
    m_vectorGraphics->FillCircle( { 0.0f, 0.0f }, 20.0f );
    m_vectorGraphics->Restore( );

    m_vectorGraphics->Restore( );
}

void VGExample::RenderComplexDemo( ) const
{
    // === Complex Scene Demo (Center) ===
    m_vectorGraphics->Save( );
    m_vectorGraphics->Translate( { 400.0f, 250.0f } );

    // Create a complex animated scene
    constexpr Float_2 center = { 0.0f, 0.0f };

    // Background circle
    m_vectorGraphics->SetFillColor( { 0.1f, 0.1f, 0.2f, 0.3f } );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( false );
    m_vectorGraphics->FillCircle( center, 80.0f );

    // Spinning spokes
    constexpr int numSpokes = 8;
    for ( int i = 0; i < numSpokes; ++i )
    {
        const float angle = m_rotationAngle * 2.0f + i * 2.0f * XM_PI / numSpokes;
        Float_2     start = GetCircularPosition( 20.0f, angle, center );
        Float_2     end   = GetCircularPosition( 70.0f, angle, center );

        m_vectorGraphics->SetStrokeColor( GetAnimatedColor( m_colorAnimTime, i * 0.3f ) );
        m_vectorGraphics->SetStrokeWidth( 3.0f );
        m_vectorGraphics->DrawLine( start, end, 3.0f );
    }

    // Central rotating square
    m_vectorGraphics->Save( );
    m_vectorGraphics->Rotate( -m_rotationAngle * 3.0f );
    m_vectorGraphics->SetFillColor( { 1.0f, 1.0f, 1.0f, 0.9f } );
    m_vectorGraphics->SetStrokeColor( { 0.0f, 0.0f, 0.0f, 1.0f } );
    m_vectorGraphics->SetStrokeWidth( 2.0f );
    m_vectorGraphics->SetFillEnabled( true );
    m_vectorGraphics->SetStrokeEnabled( true );
    m_vectorGraphics->DrawRect( { -10.0f, -10.0f }, { 10.0f, 10.0f } );
    m_vectorGraphics->Restore( );

    // Orbiting smaller elements
    constexpr int numOrbiters = 4;
    for ( int i = 0; i < numOrbiters; ++i )
    {
        const float orbitAngle = m_animationTime * 0.8f + i * 2.0f * XM_PI / numOrbiters;
        Float_2     orbitPos   = GetCircularPosition( 60.0f, orbitAngle, center );

        m_vectorGraphics->Save( );
        m_vectorGraphics->Translate( orbitPos );
        m_vectorGraphics->Rotate( m_rotationAngle * ( i + 1 ) );

        // Create different shapes for each orbiter
        switch ( i )
        {
        case 0: // Triangle
            {
                VGPolygon triangle;
                triangle.Points.AddElement( { 0.0f, -8.0f } );
                triangle.Points.AddElement( { -7.0f, 6.0f } );
                triangle.Points.AddElement( { 7.0f, 6.0f } );
                triangle.IsClosed = true;
                m_vectorGraphics->SetFillColor( { 1.0f, 0.3f, 0.3f, 1.0f } );
                m_vectorGraphics->FillPolygon( triangle );
            }
            break;
        case 1: // Circle
            m_vectorGraphics->SetFillColor( { 0.3f, 1.0f, 0.3f, 1.0f } );
            m_vectorGraphics->FillCircle( { 0.0f, 0.0f }, 8.0f );
            break;
        case 2: // Square
            m_vectorGraphics->SetFillColor( { 0.3f, 0.3f, 1.0f, 1.0f } );
            m_vectorGraphics->FillRect( { -6.0f, -6.0f }, { 6.0f, 6.0f } );
            break;
        case 3: // Diamond
            {
                VGPolygon diamond;
                diamond.Points.AddElement( { 0.0f, -8.0f } );
                diamond.Points.AddElement( { 8.0f, 0.0f } );
                diamond.Points.AddElement( { 0.0f, 8.0f } );
                diamond.Points.AddElement( { -8.0f, 0.0f } );
                diamond.IsClosed = true;
                m_vectorGraphics->SetFillColor( { 1.0f, 1.0f, 0.3f, 1.0f } );
                m_vectorGraphics->FillPolygon( diamond );
            }
            break;
        default:
            break;
        }

        m_vectorGraphics->Restore( );
    }

    m_vectorGraphics->Restore( );
}

Float_4 VGExample::GetAnimatedColor( const float time, const float offset )
{
    // Create smooth color cycling using sine waves
    const float r = 0.5f + 0.5f * sinf( time + offset );
    const float g = 0.5f + 0.5f * sinf( time + offset + 2.0f * XM_PI / 3.0f );
    const float b = 0.5f + 0.5f * sinf( time + offset + 4.0f * XM_PI / 3.0f );
    return { r, g, b, 1.0f };
}

Float_2 VGExample::GetCircularPosition( const float radius, const float angle, const Float_2 &center )
{
    return { center.X + radius * cosf( angle ), center.Y + radius * sinf( angle ) };
}
