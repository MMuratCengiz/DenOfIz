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
#include "DenOfIzExamples/FileIO.h"
#include "DenOfIzExamples/InteropMathConverter.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"

using namespace DenOfIz;

void TextRenderingExample::Init( )
{
    ImportFont( );

    m_fontLibrary = DenOfIz_FontLibrary_Create( );

    DenOfIz_BinaryReaderDesc binaryReaderDesc{ };
    binaryReaderDesc.NumBytes = 0;
    m_binaryReader            = DenOfIz_BinaryReader_CreateFromFile( DENOFIZ_STRING( m_fontAssetPath.c_str( ) ), &binaryReaderDesc );

    DenOfIz_FontAssetReaderDesc fontAssetReaderDesc{ };
    fontAssetReaderDesc.Reader = m_binaryReader;
    m_fontAssetReader          = DenOfIz_FontAssetReader_Create( &fontAssetReaderDesc );

    m_fontAsset = DenOfIz_FontAssetReader_Read( m_fontAssetReader );

    DenOfIz_FontDesc fontDesc{ };
    fontDesc.FontAsset = m_fontAsset;
    m_font             = DenOfIz_FontLibrary_LoadFontFromDesc( m_fontLibrary, &fontDesc );

    DenOfIz_TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice      = m_logicalDevice;
    textRendererDesc.InitialAtlasWidth  = DenOfIz_FontAsset_AtlasWidth( m_fontAsset );
    textRendererDesc.InitialAtlasHeight = DenOfIz_FontAsset_AtlasHeight( m_fontAsset );
    textRendererDesc.Width              = m_pixelWidth;
    textRendererDesc.Height             = m_pixelHeight;
    textRendererDesc.Font               = m_font;
    m_textRenderer                      = DenOfIz_TextRenderer_Create( &textRendererDesc );

    DenOfIz_FrameDebugRendererDesc debugRendererDesc{ };
    debugRendererDesc.GraphicsApi   = m_graphicsApi;
    debugRendererDesc.LogicalDevice = m_logicalDevice;
    debugRendererDesc.Width         = m_pixelWidth;
    debugRendererDesc.Height        = m_pixelHeight;
    debugRendererDesc.FontAsset     = m_fontAsset;
    debugRendererDesc.TextColor     = { 0.8f, 1.0f, 0.8f, 1.0f };
    debugRendererDesc.Enabled       = m_debugInfoEnabled;

    m_debugRenderer           = DenOfIz_FrameDebugRenderer_Create( &debugRendererDesc );
    DenOfIz_Float4 debugColor = { 0.8f, 0.8f, 0.8f, 1.0f };
    DenOfIz_FrameDebugRenderer_AddDebugLine( m_debugRenderer, DENOFIZ_STRING( "Press F1 to toggle debug info" ), &debugColor );
    m_animTime = 0.0f;
}

void TextRenderingExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
    // defaultApiPreference.OSX = DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE;
}

void TextRenderingExample::Update( )
{
    const auto deltaTime  = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.DeltaTime = deltaTime;
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    m_animTime += m_worldData.DeltaTime;

    if ( DENOFIZ_HANDLE_IS_VALID( m_debugRenderer ) && m_debugInfoEnabled )
    {
        DenOfIz_FrameDebugRenderer_ClearCustomDebugLines( m_debugRenderer );
        DenOfIz_Float4 debugColor = { 0.8f, 0.8f, 0.8f, 1.0f };
        DenOfIz_FrameDebugRenderer_AddDebugLine( m_debugRenderer, DENOFIZ_STRING( "Press F1 to toggle debug info" ), &debugColor );
    }

    RenderAndPresentFrame( );
}

void TextRenderingExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource   = renderTarget;
    attachmentDesc.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    attachmentDesc.ClearColor = DenOfIz_Float4{ 0.2f, 0.2f, 0.2f, 1.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, viewport->Width, viewport->Height );
    DenOfIz_TextRenderer_BeginBatch( m_textRenderer );

    float                  verticalOffset = 120.0f;
    static int             increment      = 0;
    std::string            text           = "Text Rendering Example: " + std::to_string( increment++ );
    DenOfIz_TextRenderDesc titleParams{ };
    titleParams.Text     = DENOFIZ_STRING( text.c_str( ) );
    titleParams.X        = 50.0f;
    titleParams.Y        = 100.0f + verticalOffset;
    titleParams.Color    = { 1.0f, 1.0f, 0.5f, 1.0f };
    titleParams.FontSize = 36;
    titleParams.FontId   = 0;
    DenOfIz_TextRenderer_AddText( m_textRenderer, &titleParams );

    DenOfIz_TextRenderDesc sampleParams{ };
    sampleParams.Text     = DENOFIZ_STRING( "Sample Text - The quick brown fox jumps over the lazy dog" );
    sampleParams.X        = 50.0f;
    sampleParams.Y        = 300.0f + verticalOffset;
    sampleParams.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
    sampleParams.FontSize = 24;
    sampleParams.FontId   = 0;
    DenOfIz_TextRenderer_AddText( m_textRenderer, &sampleParams );

    DenOfIz_TextRenderDesc smallParams{ };
    smallParams.Text     = DENOFIZ_STRING( "Learn Text - (0123456789) The quick brown fox jumps over the lazy dog" );
    smallParams.X        = 50.0f;
    smallParams.Y        = 500.0f + verticalOffset;
    smallParams.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
    smallParams.FontSize = 14;
    smallParams.FontId   = 0;
    DenOfIz_TextRenderer_AddText( m_textRenderer, &smallParams );
    DenOfIz_TextRenderer_EndBatch( m_textRenderer, commandList );

    if ( DENOFIZ_HANDLE_IS_VALID( m_debugRenderer ) && m_debugInfoEnabled )
    {
        DenOfIz_FrameDebugRenderer_Render( m_debugRenderer, commandList );
    }

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void TextRenderingExample::HandleEvent( DenOfIz_Event &event )
{
    if ( event.Type == DENOFIZ_EVENT_TYPE_KEY_DOWN && event.Key.KeyCode == DENOFIZ_KEY_CODE_F1 )
    {
        m_debugInfoEnabled = !m_debugInfoEnabled;
        if ( DENOFIZ_HANDLE_IS_VALID( m_debugRenderer ) )
        {
            DenOfIz_FrameDebugRenderer_SetEnabled( m_debugRenderer, m_debugInfoEnabled );
        }
        spdlog::info( "Debug info {}", ( m_debugInfoEnabled ? "enabled" : "disabled" ) );
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void TextRenderingExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    if ( DENOFIZ_HANDLE_IS_VALID( m_debugRenderer ) )
    {
        DenOfIz_FrameDebugRenderer_Destroy( m_debugRenderer );
        m_debugRenderer = DENOFIZ_NULL_HANDLE;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_textRenderer ) )
    {
        DenOfIz_TextRenderer_Destroy( m_textRenderer );
        m_textRenderer = DENOFIZ_NULL_HANDLE;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_fontAsset ) )
    {
        DenOfIz_FontAsset_Destroy( m_fontAsset );
        m_fontAsset = DENOFIZ_NULL_HANDLE;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_fontAssetReader ) )
    {
        DenOfIz_FontAssetReader_Destroy( m_fontAssetReader );
        m_fontAssetReader = DENOFIZ_NULL_HANDLE;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_binaryReader ) )
    {
        DenOfIz_BinaryReader_Destroy( m_binaryReader );
        m_binaryReader = DENOFIZ_NULL_HANDLE;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_fontLibrary ) )
    {
        DenOfIz_FontLibrary_Destroy( m_fontLibrary );
        m_fontLibrary = DENOFIZ_NULL_HANDLE;
    }

    IExample::Quit( );
}

void TextRenderingExample::ImportFont( ) const
{
    if ( !FileIO::FileExists( DENOFIZ_STRING( m_fontAssetPath.c_str( ) ) ) )
    {
        spdlog::warn( "Font is missing, running import." );

        DenOfIz_FontImporter importer = DenOfIz_FontImporter_Create( );

        DenOfIz_FontImportDesc desc{ };
        desc.SourceFilePath  = DENOFIZ_STRING( "Assets/Fonts/Inconsolata-Regular.ttf" );
        desc.TargetDirectory = DENOFIZ_STRING( "Assets/Fonts/" );
        desc.AssetNamePrefix = { };
        desc.InitialFontSize = 24;
        desc.AtlasWidth      = 512;
        desc.AtlasHeight     = 512;
        desc.CustomRanges    = { };
        desc.TargetContainer = DENOFIZ_NULL_HANDLE;

        DenOfIz_ImporterResult result = DenOfIz_FontImporter_Import( importer, &desc );
        if ( result.ResultCode != DENOFIZ_IMPORTER_RESULT_SUCCESS )
        {
            spdlog::error( "Import failed: {}", std::string( result.ErrorMessage.Chars, result.ErrorMessage.NumChars ) );
        }

        for ( size_t i = 0; i < result.CreatedAssets.NumElements; ++i )
        {
            const DenOfIz_StringView uriView = result.CreatedAssets.Elements[ i ];
            const std::string        uri     = uriView.Chars != nullptr ? std::string( uriView.Chars, uriView.NumChars ) : std::string( );
            spdlog::info( "Created asset: {}", uri );
        }

        DenOfIz_ImporterResult_Free( &result );
        DenOfIz_FontImporter_Destroy( importer );

        if ( !FileIO::FileExists( DENOFIZ_STRING( m_fontAssetPath.c_str( ) ) ) )
        {
            spdlog::critical( "Import completed but some font is still missing." );
        }
    }
}
