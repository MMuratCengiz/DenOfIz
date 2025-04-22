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
#include <DenOfIzExamples/FontRenderingExample.h>

using namespace DenOfIz;

void FontRenderingExample::Init( )
{
    m_fontRenderer = std::make_unique<FontRenderer>( m_graphicsApi, m_logicalDevice );
    m_fontRenderer->Initialize( );

    m_fontRenderer->LoadFont( "Assets/Fonts/Inconsolata-Regular.ttf", 36 );
    m_fontRenderer->SetFont( "Assets/Fonts/Inconsolata-Regular.ttf", 36 );

    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, static_cast<float>( m_windowDesc.Width ), static_cast<float>( m_windowDesc.Height ), 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_orthoProjection, projection );
    m_fontRenderer->SetProjectionMatrix( m_orthoProjection );

    m_animTime = 0.0f;

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

void FontRenderingExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void FontRenderingExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    m_animTime += m_worldData.DeltaTime;

    const uint64_t frameIndex = m_frameSync->NextFrame( );
    Render( frameIndex, m_frameSync->GetCommandList( frameIndex ) );
    m_frameSync->ExecuteCommandList( frameIndex );
    Present( frameIndex );
}

void FontRenderingExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource        = renderTarget;
    attachmentDesc.LoadOp          = LoadOp::Clear;
    attachmentDesc.ClearColor[ 0 ] = 0.2f;
    attachmentDesc.ClearColor[ 1 ] = 0.2f;
    attachmentDesc.ClearColor[ 2 ] = 0.2f;
    attachmentDesc.ClearColor[ 3 ] = 1.0f;

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( attachmentDesc );

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_fontRenderer->BeginBatch( );

    TextRenderDesc staticTextParams;
    staticTextParams.Text  = "DenOfIz Font Rendering System";
    staticTextParams.X     = 50.0f;
    staticTextParams.Y     = 100.0f;
    staticTextParams.Color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    staticTextParams.Scale = 1.0f;
    m_fontRenderer->AddText( staticTextParams );

    TextRenderDesc animatedTextParams;
    animatedTextParams.Text = "Animated text with color";
    animatedTextParams.X    = static_cast<float>( m_windowDesc.Width ) / 2.0f;
    animatedTextParams.Y    = static_cast<float>( m_windowDesc.Height ) / 2.0f;

    float r                             = ( sin( m_animTime * 2.0f ) + 1.0f ) / 2.0f;
    float g                             = ( sin( m_animTime * 1.5f + 2.0f ) + 1.0f ) / 2.0f;
    float b                             = ( sin( m_animTime * 1.0f + 4.0f ) + 1.0f ) / 2.0f;
    animatedTextParams.Color            = XMFLOAT4( r, g, b, 1.0f );
    animatedTextParams.Scale            = 1.0f + 0.2f * sin( m_animTime * 3.0f );
    animatedTextParams.HorizontalCenter = true;
    animatedTextParams.VerticalCenter   = true;

    m_fontRenderer->AddText( animatedTextParams );

    TextRenderDesc multiLineParams;
    multiLineParams.Text  = "This font rendering system supports:\n"
                            "- Basic text layout\n"
                            "- UTF-8 encoding\n"
                            "- Glyph caching\n"
                            "- Texture atlases\n"
                            "- Future: SDF rendering\n"
                            "- Future: HarfBuzz integration";
    multiLineParams.X     = 50.0f;
    multiLineParams.Y     = 200.0f;
    multiLineParams.Color = XMFLOAT4( 0.8f, 0.9f, 1.0f, 1.0f );
    multiLineParams.Scale = 0.75f;
    m_fontRenderer->AddText( multiLineParams );
    m_fontRenderer->EndBatch( commandList );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void FontRenderingExample::HandleEvent( SDL_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void FontRenderingExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}
