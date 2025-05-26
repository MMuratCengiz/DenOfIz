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
#include <DenOfIzGraphics/Utilities/FrameDebugRenderer.h>

#include "DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h"
#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DenOfIz;

FrameDebugRenderer::FrameDebugRenderer( const FrameDebugRendererDesc &desc ) : m_desc( desc )
{
    if ( m_desc.LogicalDevice == nullptr )
    {
        LOG( WARNING ) << "FrameDebugRendererDesc.LogicalDevice cannot be null.";
        return;
    }
    if ( m_desc.GraphicsApi == nullptr )
    {
        LOG( WARNING ) << "FrameDebugRendererDesc.GraphicsApi is null, debug info will not contain API information.";
    }
    if ( m_desc.ScreenWidth == 0 || m_desc.ScreenHeight == 0 )
    {
        LOG( ERROR ) << "FrameDebugRendererDesc.ScreenWidth and FrameDebugRendererDesc.ScreenHeight must be set.";
        return;
    }

    m_frameTimes.resize( m_maxFrameTimeSamples, 0.0 );
    m_time.OnEachSecond = [ this ]( const double fps ) { m_fps = fps; };

    if ( m_desc.FontAsset == nullptr )
    {
        m_desc.FontAsset = EmbeddedFonts::GetInconsolataRegular( );
    }

    m_fontLibrary = std::make_unique<FontLibrary>( );
    FontDesc fontDesc{ };
    fontDesc.FontAsset = m_desc.FontAsset;
    m_font             = m_fontLibrary->LoadFont( fontDesc );

    if ( m_desc.GraphicsApi != nullptr )
    {
        m_backendName = m_desc.GraphicsApi->ActiveAPI( );
    }
    m_gpuName = m_desc.LogicalDevice->DeviceInfo( ).Name;
    TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice      = m_desc.LogicalDevice;
    textRendererDesc.InitialAtlasWidth  = m_desc.FontAsset->AtlasWidth;
    textRendererDesc.InitialAtlasHeight = m_desc.FontAsset->AtlasHeight;
    textRendererDesc.Width              = m_desc.ScreenWidth;
    textRendererDesc.Height             = m_desc.ScreenHeight;
    textRendererDesc.Font               = m_font;
    m_textRenderer                      = std::make_unique<TextRenderer>( textRendererDesc );
    m_textRenderer->SetAntiAliasingMode( AntiAliasingMode::Grayscale );
    SetViewport( Viewport{ 0.0f, 0.0f, static_cast<float>( m_desc.ScreenWidth ), static_cast<float>( m_desc.ScreenHeight ) } );
}

void FrameDebugRenderer::UpdateStats( const float deltaTime )
{
    if ( !m_desc.Enabled || !m_textRenderer )
    {
        return;
    }

    m_time.Tick( );
    UpdateFrameTimeStats( deltaTime );

    m_statsRefreshTimer += deltaTime;
    if ( m_statsRefreshTimer >= m_desc.RefreshRate )
    {
        UpdatePerformanceStats( );
        m_statsRefreshTimer = 0.0f;
    }
}

void FrameDebugRenderer::UpdateFrameTimeStats( float deltaTime )
{
    m_frameTimes.pop_front( );
    m_frameTimes.push_back( deltaTime * 1000.0 );

    double totalTime = 0.0;
    for ( const double time : m_frameTimes )
    {
        totalTime += time;
    }
    m_frameTimeMs = totalTime / m_frameTimes.size( );
}

void FrameDebugRenderer::UpdatePerformanceStats( )
{
    m_cpuUsagePercent  = 0; // TODO
    m_gpuUsagePercent  = 0; // TODO
    m_gpuMemoryUsageMB = 0; // TODO
}

void FrameDebugRenderer::Render( ICommandList *commandList )
{
    if ( !m_desc.Enabled || !m_textRenderer || !commandList )
    {
        return;
    }

    m_textRenderer->BeginBatch( );

    float averageCharWidth = 4.0f * m_desc.FontSize / m_font->Asset( )->InitialFontSize;
    float maxLineLength    = 155.0f;
    float rightMargin      = static_cast<float>( m_desc.ScreenWidth ) - averageCharWidth * maxLineLength / 2.0f;
    float yPos             = 20.0f;
    float lineHeight       = 42.0f * m_desc.FontSize / m_font->Asset( )->InitialFontSize;

    char buffer[ 128 ];

    snprintf( buffer, sizeof( buffer ), "Frame Time: %.2f ms (%.1f FPS)", m_frameTimeMs, m_fps );
    TextRenderDesc fpsParams;
    fpsParams.Text             = buffer;
    fpsParams.X                = rightMargin;
    fpsParams.Y                = yPos;
    fpsParams.Color            = m_frameTimeMs > 16.7f ? Float_4{ 1.0f, 0.4f, 0.4f, 1.0f } : m_desc.TextColor;
    fpsParams.FontSize         = m_desc.FontSize;
    fpsParams.HorizontalCenter = true;
    fpsParams.Direction        = m_desc.Direction;
    m_textRenderer->AddText( fpsParams );
    yPos += lineHeight;

    snprintf( buffer, sizeof( buffer ), "CPU: %.1f%% | GPU: %.1f%%", m_cpuUsagePercent, m_gpuUsagePercent );
    TextRenderDesc usageParams;
    usageParams.Text             = buffer;
    usageParams.X                = rightMargin;
    usageParams.Y                = yPos;
    usageParams.Color            = m_desc.TextColor;
    usageParams.FontSize         = m_desc.FontSize;
    usageParams.HorizontalCenter = true;
    usageParams.Direction        = m_desc.Direction;
    m_textRenderer->AddText( usageParams );
    yPos += lineHeight;

    snprintf( buffer, sizeof( buffer ), "GPU Mem: %llu MB", m_gpuMemoryUsageMB );
    TextRenderDesc memParams;
    memParams.Text             = buffer;
    memParams.X                = rightMargin;
    memParams.Y                = yPos;
    memParams.Color            = m_desc.TextColor;
    memParams.FontSize         = m_desc.FontSize;
    memParams.HorizontalCenter = true;
    memParams.Direction        = m_desc.Direction;
    m_textRenderer->AddText( memParams );
    yPos += lineHeight;

    if ( m_backendName.NumChars( ) > 0 )
    {
        TextRenderDesc backendParams;
        backendParams.Text             = InteropString( "API: " ).Append( m_backendName.Get( ) );
        backendParams.X                = rightMargin;
        backendParams.Y                = yPos;
        backendParams.Color            = m_desc.TextColor;
        backendParams.FontSize         = m_desc.FontSize;
        backendParams.HorizontalCenter = true;
        backendParams.Direction        = m_desc.Direction;
        m_textRenderer->AddText( backendParams );
        yPos += lineHeight;
    }

    if ( !m_gpuName.IsEmpty( ) )
    {
        TextRenderDesc gpuParams;
        gpuParams.Text             = InteropString( "GPU: " ).Append( m_gpuName.Get( ) );
        gpuParams.X                = rightMargin;
        gpuParams.Y                = yPos;
        gpuParams.Color            = m_desc.TextColor;
        gpuParams.FontSize         = m_desc.FontSize;
        gpuParams.HorizontalCenter = true;
        gpuParams.Direction        = m_desc.Direction;
        m_textRenderer->AddText( gpuParams );
        yPos += lineHeight;
    }

    for ( const auto &line : m_customDebugLines )
    {
        TextRenderDesc customParams;
        customParams.Text             = line.Text;
        customParams.X                = rightMargin;
        customParams.Y                = yPos;
        customParams.Color            = line.Color;
        customParams.FontSize         = m_desc.FontSize;
        customParams.HorizontalCenter = true;
        customParams.Direction        = m_desc.Direction;
        m_textRenderer->AddText( customParams );
        yPos += lineHeight;
    }

    m_textRenderer->EndBatch( commandList );
}

void FrameDebugRenderer::SetViewport( const Viewport &viewport )
{
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( viewport.X, viewport.Width, viewport.Height, viewport.Y, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
    m_textRenderer->SetViewport( viewport );
}

void FrameDebugRenderer::SetProjectionMatrix( const Float_4x4 &projectionMatrix )
{
    m_projectionMatrix = InteropMathConverter::Float_4x4ToXMFLOAT4X4( projectionMatrix );
    if ( m_textRenderer )
    {
        m_textRenderer->SetProjectionMatrix( projectionMatrix );
    }
}

void FrameDebugRenderer::SetScreenSize( const uint32_t width, const uint32_t height )
{
    m_desc.ScreenWidth  = width;
    m_desc.ScreenHeight = height;
    SetViewport( Viewport{ 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) } );
}

void FrameDebugRenderer::AddDebugLine( const InteropString &text, const Float_4 &color )
{
    m_customDebugLines.push_back( { text, color } );
}

void FrameDebugRenderer::ClearCustomDebugLines( )
{
    m_customDebugLines.clear( );
}
void FrameDebugRenderer::SetEnabled( const bool enabled )
{
    m_desc.Enabled = enabled;
}
bool FrameDebugRenderer::IsEnabled( ) const
{
    return m_desc.Enabled;
}
void FrameDebugRenderer::ToggleVisibility( )
{
    m_desc.Enabled = !m_desc.Enabled;
}
