/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzExamples/TextRenderingExample.h"

#include <cmath>
#include <string>

using namespace DenOfIz;

namespace
{
    constexpr uint16_t InconsolataFont = 0;
    constexpr uint16_t DefaultFont     = 1;
} // namespace

void TextRenderingExample::Init( )
{
    m_font = DenOfIz_Font_CreateFromPath( DENOFIZ_STRING( "Assets/Fonts/Inconsolata-Regular.ttf" ), 0 );

    DenOfIz_TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice = m_logicalDevice;
    textRendererDesc.Width         = m_pixelWidth;
    textRendererDesc.Height        = m_pixelHeight;
    textRendererDesc.Font          = m_font;
    m_textRenderer                 = DenOfIz_TextRenderer_Create( &textRendererDesc );
    DenOfIz_TextRenderer_AddFont( m_textRenderer, DenOfIz_Font_GetDefault( ), DefaultFont );

    DenOfIz_FrameDebugRendererDesc debugRendererDesc{ };
    debugRendererDesc.GraphicsApi   = m_graphicsApi;
    debugRendererDesc.LogicalDevice = m_logicalDevice;
    debugRendererDesc.Width         = m_pixelWidth;
    debugRendererDesc.Height        = m_pixelHeight;
    debugRendererDesc.TextColor     = { 0.8f, 1.0f, 0.8f, 1.0f };
    debugRendererDesc.Enabled       = m_debugInfoEnabled;
    m_debugRenderer                 = DenOfIz_FrameDebugRenderer_Create( &debugRendererDesc );
    m_animTime                      = 0.0f;
}

void TextRenderingExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
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

    DenOfIz_Texture renderTarget = m_frame.BackBuffer;

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

    static int        increment = 0;
    const std::string title     = "Text Rendering Example: " + std::to_string( increment++ );

    DenOfIz_TextRenderDesc text{ };
    text.X        = 50.0f;
    text.Y        = 60.0f;
    text.Color    = { 1.0f, 1.0f, 0.5f, 1.0f };
    text.FontSize = 36.0f;
    text.Text     = DENOFIZ_STRING( title.c_str( ) );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );

    text.Y        = 140.0f;
    text.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
    text.FontSize = 24.0f;
    text.Text     = DENOFIZ_STRING( "Sample Text - The quick brown fox jumps over the lazy dog" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );

    text.Y        = 190.0f;
    text.FontSize = 14.0f;
    text.Text     = DENOFIZ_STRING( "Learn Text - (0123456789) The quick brown fox jumps over the lazy dog\nA second line, split on the line break" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );

    text.Y        = 260.0f;
    text.FontSize = 28.0f;
    text.FontId   = DefaultFont;
    text.Color    = { 0.6f, 0.9f, 1.0f, 1.0f };
    text.Text     = DENOFIZ_STRING( "JetBrains Mono ligatures: -> => != === <= >= :: ||" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );

    text.Y        = 320.0f;
    text.FontSize = 26.0f;
    text.FontId   = InconsolataFont;
    text.Color    = { 0.8f, 1.0f, 0.7f, 1.0f };
    text.Text     = DENOFIZ_STRING( "Bidi: the word שלום (peace) stays readable\nقرأت 3 كتب في Book World اليوم\nFallback: Ελληνικά · 日本語 · 한국어 · हिन्दी · ไทย" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );

    text.Y        = 440.0f;
    text.FontSize = 32.0f;
    text.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
    text.Text     = DENOFIZ_STRING( "Emoji: 😀 👍🏽 👨‍👩‍👧 🇳🇱 1️⃣ ❤️ 🦄 and ☺ as text" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );
    text.X                = viewport->Width * 0.5f;
    text.Y                = viewport->Height * 0.6f;
    text.FontSize         = 72.0f + 48.0f * std::sin( m_animTime );
    text.FontId           = InconsolataFont;
    text.Color            = { 1.0f, 0.7f, 0.4f, 1.0f };
    text.HorizontalCenter = true;
    text.VerticalCenter   = true;
    text.Text             = DENOFIZ_STRING( "Scales without an atlas 🚀" );
    DenOfIz_TextRenderer_AddText( m_textRenderer, &text );
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

    DenOfIz_Font_Destroy( m_font );
    m_font = DENOFIZ_NULL_HANDLE;

    IExample::Quit( );
}

void TextRenderingExample::OnResize( const uint32_t width, const uint32_t height )
{
    IExample::OnResize( width, height );
    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    if ( viewport != nullptr )
    {
        DenOfIz_TextRenderer_SetViewport( m_textRenderer, viewport );
    }
    DenOfIz_FrameDebugRenderer_SetViewportSize( m_debugRenderer, width, height );
}
