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
#include <DenOfIzExamples/UIExample.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Utilities/InteropUtilities.h>
#include <DenOfIzGraphics/Utilities/Time.h>
#include <cstdio>

using namespace DenOfIz;

void UIExample::Init( )
{
    ClayDesc uiDesc{ };
    uiDesc.LogicalDevice      = m_logicalDevice;
    uiDesc.RenderTargetFormat = Format::B8G8R8A8Unorm;
    uiDesc.NumFrames          = 3;
    uiDesc.Width              = m_windowDesc.Width;
    uiDesc.Height             = m_windowDesc.Height;

    m_clay = std::make_unique<Clay>( uiDesc );

    const auto viewport = m_swapChain->GetViewport( );
    m_clay->SetViewportSize( viewport.Width, viewport.Height );
    m_clay->SetDebugModeEnabled( true );

    m_buttonId    = m_clay->HashString( "Button" );
    m_textId      = m_clay->HashString( "Text" );
    m_containerId = m_clay->HashString( "Container" );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

void UIExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void UIExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_clay->UpdateScrollContainers( false, Float_2( 0, 0 ), m_worldData.DeltaTime );

    RenderAndPresentFrame( );
}

void UIExample::CreateUI( ) const
{
    ClayElementDeclaration container;
    container.Id                     = m_containerId;
    container.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    container.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    container.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    container.Layout.Padding         = ClayPadding( 32 );
    container.Layout.ChildGap        = 16;
    container.BackgroundColor        = ClayColor( 45, 45, 48, 255 );

    m_clay->OpenElement( container );

    ClayElementDeclaration headerContainer;
    headerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    headerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 60 );
    headerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    headerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    headerContainer.BackgroundColor         = ClayColor( 30, 30, 33, 255 );

    m_clay->OpenElement( headerContainer );

    ClayTextDesc headerTextDesc;
    headerTextDesc.FontSize  = 24;
    headerTextDesc.TextColor = ClayColor( 255, 255, 255, 255 );
    m_clay->Text( "Clay UI Example", headerTextDesc );

    m_clay->CloseElement( );

    ClayElementDeclaration contentContainer;
    contentContainer.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    contentContainer.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    contentContainer.Layout.LayoutDirection = ClayLayoutDirection::LeftToRight;
    contentContainer.Layout.ChildGap        = 24;
    contentContainer.Layout.Padding         = ClayPadding( 24 );

    m_clay->OpenElement( contentContainer );

    ClayElementDeclaration card;
    card.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    card.Layout.Sizing.Height   = ClaySizingAxis::Fit( 0, 400 );
    card.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    card.Layout.Padding         = ClayPadding( 20 );
    card.Layout.ChildGap        = 12;
    card.BackgroundColor        = ClayColor( 255, 255, 255, 255 );
    card.CornerRadius           = ClayCornerRadius( 8 );
    card.Border.Color           = ClayColor( 200, 200, 200, 255 );
    card.Border.Width           = ClayBorderWidth( 1 );

    for ( int i = 0; i < 3; ++i )
    {
        m_clay->OpenElement( card );

        ClayTextDesc cardTitleConfig;
        cardTitleConfig.FontSize  = 18;
        cardTitleConfig.TextColor = ClayColor( 30, 30, 33, 255 );

        char titleBuffer[ 64 ];
        snprintf( titleBuffer, sizeof( titleBuffer ), "Card %d", i + 1 );
        m_clay->Text( titleBuffer, cardTitleConfig );

        ClayTextDesc cardTextConfig;
        cardTextConfig.FontSize  = 14;
        cardTextConfig.TextColor = ClayColor( 100, 100, 100, 255 );
        cardTextConfig.WrapMode  = ClayTextWrapMode::Words;

        m_clay->Text( "This is a sample card component with some descriptive text inside. It demonstrates the layout capabilities of Clay UI.", cardTextConfig );

        ClayElementDeclaration spacer;
        spacer.Layout.Sizing.Height = ClaySizingAxis::Grow( );
        m_clay->OpenElement( spacer );
        m_clay->CloseElement( );

        ClayElementDeclaration button;
        button.Id                      = m_clay->HashString( "Button", i );
        button.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
        button.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 36 );
        button.Layout.ChildAlignment.X = ClayAlignmentX::Center;
        button.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
        button.CornerRadius            = ClayCornerRadius( 4 );

        bool isHovered         = m_clay->PointerOver( button.Id );
        button.BackgroundColor = isHovered ? ClayColor( 0, 122, 204, 255 ) : ClayColor( 0, 102, 184, 255 );

        m_clay->OpenElement( button );

        ClayTextDesc buttonTextConfig;
        buttonTextConfig.FontSize  = 14;
        buttonTextConfig.TextColor = ClayColor( 255, 255, 255, 255 );

        m_clay->Text( "Learn More", buttonTextConfig );
        m_clay->CloseElement( );
        m_clay->CloseElement( );
    }

    m_clay->CloseElement( );

    ClayElementDeclaration footerContainer;
    footerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    footerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 40 );
    footerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    footerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    footerContainer.BackgroundColor         = ClayColor( 30, 30, 33, 255 );

    m_clay->OpenElement( footerContainer );

    ClayTextDesc footerTextConfig;
    footerTextConfig.FontSize  = 12;
    footerTextConfig.TextColor = ClayColor( 150, 150, 150, 255 );

    char fpsBuffer[ 64 ];
    snprintf( fpsBuffer, sizeof( fpsBuffer ), "FPS: %.0f", 1.0 / m_worldData.DeltaTime );
    m_clay->Text( fpsBuffer, footerTextConfig );

    m_clay->CloseElement( );
    m_clay->CloseElement( );
}

void UIExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;
    attachmentDesc.SetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( attachmentDesc );

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_clay->BeginLayout( );
    CreateUI( );
    m_clay->EndLayout( commandList, frameIndex );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void UIExample::HandleEvent( Event &event )
{
    m_clay->HandleEvent( event );
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void UIExample::Quit( )
{
    m_frameSync->WaitIdle( );
    m_clay.reset( );
    IExample::Quit( );
}
