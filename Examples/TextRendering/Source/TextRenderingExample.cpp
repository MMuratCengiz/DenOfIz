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

/**
 * This example demonstrates text rendering using Multi-channel Signed Distance Fields (MSDF).
 *
 * MSDF rendering provides high-quality text at any scale without pixelation or blurring.
 * It works by generating distance fields that represent the shape of each glyph and
 * using a special shader to render them with sharp edges.
 */
#include "DenOfIzExamples/TextRenderingExample.h"
#include "DenOfIzExamples/InteropMathConverter.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"

using namespace DenOfIz;

void TextRenderingExample::Init( )
{
    ImportFont( );

    m_fontLibrary  = std::make_unique<FontLibrary>( );
    m_binaryReader = std::make_unique<BinaryReader>( m_fontAssetPath );

    FontAssetReaderDesc fontAssetReaderDesc{ };
    fontAssetReaderDesc.Reader = m_binaryReader.get( );
    m_fontAssetReader          = std::make_unique<FontAssetReader>( fontAssetReaderDesc );

    m_fontAsset = std::unique_ptr<FontAsset>( m_fontAssetReader->Read( ) );

    FontDesc fontDesc{ };
    fontDesc.FontAsset = m_fontAsset.get( );
    m_font             = m_fontLibrary->LoadFont( fontDesc );

    TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice      = m_logicalDevice;
    textRendererDesc.InitialAtlasWidth  = m_font->Asset( )->AtlasWidth;
    textRendererDesc.InitialAtlasHeight = m_font->Asset( )->AtlasHeight;
    textRendererDesc.Width              = m_windowDesc.Width;
    textRendererDesc.Height             = m_windowDesc.Height;
    textRendererDesc.Font               = m_font;
    m_textRenderer                      = std::make_unique<TextRenderer>( textRendererDesc );
    m_textRenderer->SetAntiAliasingMode( AntiAliasingMode::Grayscale );

    // Initialize the debug renderer with the same font asset
    FrameDebugRendererDesc debugRendererDesc{ };
    debugRendererDesc.GraphicsApi   = m_graphicsApi;
    debugRendererDesc.LogicalDevice = m_logicalDevice;
    debugRendererDesc.ScreenWidth   = m_windowDesc.Width;
    debugRendererDesc.ScreenHeight  = m_windowDesc.Height;
    debugRendererDesc.FontAsset     = m_font->Asset( );
    debugRendererDesc.TextColor     = { 0.8f, 1.0f, 0.8f, 1.0f };
    debugRendererDesc.Enabled       = m_debugInfoEnabled;

    m_debugRenderer = std::make_unique<FrameDebugRenderer>( debugRendererDesc );
    m_debugRenderer->AddDebugLine( "Press F1 to toggle debug info", { 0.8f, 0.8f, 0.8f, 1.0f } );
    m_animTime = 0.0f;
}

void TextRenderingExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void TextRenderingExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    m_animTime += m_worldData.DeltaTime;

    if ( m_debugRenderer && m_debugInfoEnabled )
    {
        m_debugRenderer->UpdateStats( m_worldData.DeltaTime );

        auto currentAAModeName = "Unknown";
        switch ( m_currentAAModeIndex )
        {
        case 0:
            currentAAModeName = "None";
            break;
        case 1:
            currentAAModeName = "Grayscale";
            break;
        case 2:
            currentAAModeName = "Subpixel";
            break;
        }

        m_debugRenderer->ClearCustomDebugLines( );
        m_debugRenderer->AddDebugLine( "Press F1 to toggle debug info", { 0.8f, 0.8f, 0.8f, 1.0f } );
    }

    const uint64_t frameIndex = m_frameSync->NextFrame( );
    Render( frameIndex, m_frameSync->GetCommandList( frameIndex ) );
    m_frameSync->ExecuteCommandList( frameIndex );
    Present( frameIndex );
}

void TextRenderingExample::Render( const uint32_t frameIndex, ICommandList *commandList )
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
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_textRenderer->BeginBatch( );

    // Determine current antialiasing mode name
    const char *currentAAModeName = "Unknown";
    switch ( m_currentAAModeIndex )
    {
    case 0:
        currentAAModeName = "None";
        break;
    case 1:
        currentAAModeName = "Grayscale";
        break;
    case 2:
        currentAAModeName = "Subpixel";
        break;
    }

    float          verticalOffset = 120.0f;
    TextRenderDesc titleParams;
    titleParams.Text     = "Font Rendering Example";
    titleParams.X        = 50.0f;
    titleParams.Y        = 50.0f + verticalOffset;
    titleParams.Color    = { 1.0f, 1.0f, 0.5f, 1.0f };
    titleParams.FontSize = 36;
    m_textRenderer->AddText( titleParams );

    // Current AA mode display
    TextRenderDesc currentModeParams;
    currentModeParams.Text  = InteropString( "Current Mode: " ).Append( currentAAModeName ).Append( " Antialiasing" );
    currentModeParams.X     = 50.0f;
    currentModeParams.Y     = 100.0f + verticalOffset;
    currentModeParams.Color = { 0.5f, 1.0f, 1.0f, 1.0f };
    m_textRenderer->AddText( currentModeParams );

    // Instructions
    TextRenderDesc instructionsParams;
    instructionsParams.Text     = "Press SPACE to cycle through antialiasing modes | Press F1 to toggle debug info";
    instructionsParams.X        = 50.0f;
    instructionsParams.Y        = 150.0f + verticalOffset;
    instructionsParams.Color    = { 0.8f, 0.8f, 0.8f, 1.0f };
    instructionsParams.FontSize = 28;
    m_textRenderer->AddText( instructionsParams );

    // Sample text at different sizes
    TextRenderDesc sampleParams;
    sampleParams.Text  = "Sample Text - The quick brown fox jumps over the lazy dog";
    sampleParams.X     = 50.0f;
    sampleParams.Y     = 200.0f + verticalOffset;
    sampleParams.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_textRenderer->AddText( sampleParams );

    // Smaller text
    TextRenderDesc smallParams;
    smallParams.Text     = "Learn Text - (0123456789) The quick brown fox jumps over the lazy dog";
    smallParams.X        = 50.0f;
    smallParams.Y        = 250.0f + verticalOffset;
    smallParams.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
    smallParams.FontSize = 14;
    m_textRenderer->AddText( smallParams );
    m_textRenderer->EndBatch( commandList );

    // Render debug info on top of everything else
    if ( m_debugRenderer && m_debugInfoEnabled )
    {
        m_debugRenderer->Render( commandList );
    }

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void TextRenderingExample::HandleEvent( Event &event )
{
    if ( event.Type == EventType::KeyDown && event.Key.Keycode == KeyCode::Down )
    {
        m_currentAAModeIndex = ( m_currentAAModeIndex + 1 ) % 3;
        m_textRenderer->SetAntiAliasingMode( static_cast<AntiAliasingMode>( m_currentAAModeIndex ) );

        const char *modeNames[] = { "None", "Grayscale", "Subpixel" };
        spdlog::info( "Switched to antialiasing mode: {}", modeNames[ m_currentAAModeIndex ] );
    }
    else if ( event.Type == EventType::KeyDown && event.Key.Keycode == KeyCode::F1 )
    {
        m_debugInfoEnabled = !m_debugInfoEnabled;
        if ( m_debugRenderer )
        {
            m_debugRenderer->SetEnabled( m_debugInfoEnabled );
        }
        spdlog::info( "Debug info {}", ( m_debugInfoEnabled ? "enabled" : "disabled" ) );
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void TextRenderingExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}

void TextRenderingExample::ImportFont( ) const
{
    if ( !FileIO::FileExists( m_fontAssetPath ) )
    {
        spdlog::warn( "Font is missing, running import." );

        FontImportDesc desc{ };
        desc.SourceFilePath  = "Assets/Fonts/Inconsolata-Regular.ttf";
        desc.TargetDirectory = "Assets/Fonts/";

        FontImporter         importer{ };
        const ImporterResult result = importer.Import( desc );
        if ( result.ResultCode != ImporterResultCode::Success )
        {
            spdlog::error( "Import failed: {}", result.ErrorMessage.Get( ) );
        }

        for ( size_t i = 0; i < result.CreatedAssets.NumElements; ++i )
        {
            AssetUri uri = result.CreatedAssets.Elements[ i ];
            spdlog::info( "Created asset: {}", uri.Path.Get( ) );
        }

        if ( !FileIO::FileExists( m_fontAssetPath ) )
        {
            spdlog::critical( "Import completed but some font is still missing." );
        }
    }
}
