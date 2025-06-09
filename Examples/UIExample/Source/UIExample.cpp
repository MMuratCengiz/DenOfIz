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

#include "DenOfIzExamples/UIExample.h"
#include <cstdio>
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

void UIExample::Init( )
{
    m_darkMode  = m_darkModeCheckbox ? m_darkModeCheckbox->IsChecked( ) : false;
    m_bgColor   = m_darkMode ? ClayColor( 30, 30, 33, 255 ) : ClayColor( 245, 245, 250, 255 );
    m_cardColor = m_darkMode ? ClayColor( 45, 45, 48, 255 ) : ClayColor( 255, 255, 255, 255 );
    m_textColor = m_darkMode ? ClayColor( 240, 240, 240, 255 ) : ClayColor( 20, 20, 20, 255 );

    ClayDesc uiDesc{ };
    uiDesc.LogicalDevice      = m_logicalDevice;
    uiDesc.RenderTargetFormat = Format::B8G8R8A8Unorm;
    uiDesc.NumFrames          = 3;
    uiDesc.Width              = m_windowDesc.Width;
    uiDesc.Height             = m_windowDesc.Height;
    uiDesc.MaxNumElements     = 16384;

    m_clay = std::make_unique<Clay>( uiDesc );

    const auto viewport = m_swapChain->GetViewport( );
    m_clay->SetViewportSize( viewport.Width, viewport.Height );
    m_clay->SetDebugModeEnabled( true );

    m_containerId = m_clay->HashString( "Container" );

    m_dpiScaleOptions.AddElement( InteropString( "100%" ) );
    m_dpiScaleOptions.AddElement( InteropString( "125%" ) );
    m_dpiScaleOptions.AddElement( InteropString( "150%" ) );
    m_dpiScaleOptions.AddElement( InteropString( "175%" ) );
    m_dpiScaleOptions.AddElement( InteropString( "200%" ) );

    m_darkModeCheckbox = m_clay->CreateCheckbox( m_clay->HashString( "DarkModeCheckbox" ), false );

    SliderStyle sliderStyle = { };
    sliderStyle.MinValue    = 0.0f;
    sliderStyle.MaxValue    = 2.0f;
    sliderStyle.Step        = 0.01f;
    m_cubeRotationSlider    = m_clay->CreateSlider( m_clay->HashString( "CubeRotationSlider" ), 1.0f, sliderStyle );

    m_dpiScaleDropdown = m_clay->CreateDropdown( m_clay->HashString( "DpiScaleDropdown" ), m_dpiScaleOptions );
    m_dpiScaleDropdown->SetSelectedIndex( 0 );

    auto multilineStyle  = ClayWidgets::CreateTextArea( "Enter your text here..." );
    m_multilineTextField = m_clay->CreateTextField( m_clay->HashString( "MultilineTextField" ), multilineStyle );

    m_dockingManager = std::unique_ptr<DockingManager>( m_clay->CreateDockingManager( ) );

    m_cubeContainer = m_clay->CreateDockableContainer( m_clay->HashString( "CubeContainer" ), m_dockingManager.get( ) );
    m_cubeContainer->SetFloatingPosition( Float_2{ 400.0f, 200.0f } );
    m_cubeContainer->Close( ); // Start hidden

    m_textContainer = m_clay->CreateDockableContainer( m_clay->HashString( "TextContainer" ), m_dockingManager.get( ) );
    m_textContainer->SetFloatingPosition( Float_2{ 100.0f, 300.0f } );
    m_textContainer->Close( ); // Start hidden

    m_spinningCubeWidget = std::make_unique<Spinning3DCubeWidget>( m_clay->GetContext( ), m_clay->HashString( "SpinningCubeWidget" ) );
    m_spinningCubeWidget->SetRotationSpeed( 1.0f );
    m_spinningCubeWidget->SetCubeColor( XMFLOAT4( 0.2f, 0.6f, 1.0f, 1.0f ) );

    m_clay->RegisterPipelineWidget( m_spinningCubeWidget.get( ) );
}

void UIExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void UIExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_clay->UpdateScrollContainers( false, Float_2( 0, 0 ), m_worldData.DeltaTime );

    if ( m_dockingManager )
    {
        m_dockingManager->Update( m_worldData.DeltaTime );
    }

    if ( m_spinningCubeWidget && m_cubeRotationSlider )
    {
        m_spinningCubeWidget->SetRotationSpeed( m_cubeRotationSlider->GetValue( ) );
    }

    if ( m_dpiScaleDropdown )
    {
        const int selectedIndex = m_dpiScaleDropdown->GetSelectedIndex( );
        if ( selectedIndex >= 0 && selectedIndex < 5 )
        {
            constexpr float dpiScales[] = { 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };
            m_clay->SetDpiScale( dpiScales[ selectedIndex ] ); // Todo this works terribly :/
        }
    }

    RenderAndPresentFrame( );
}

void UIExample::CreateUI( )
{
    const bool previousDarkMode = m_darkMode;
    m_darkMode                  = m_darkModeCheckbox ? m_darkModeCheckbox->IsChecked( ) : false;
    m_bgColor                   = m_darkMode ? ClayColor( 30, 30, 33, 255 ) : ClayColor( 245, 245, 250, 255 );
    m_cardColor                 = m_darkMode ? ClayColor( 45, 45, 48, 255 ) : ClayColor( 255, 255, 255, 255 );
    m_textColor                 = m_darkMode ? ClayColor( 240, 240, 240, 255 ) : ClayColor( 20, 20, 20, 255 );

    // Container styles will be updated in OpenElement calls below

    ClayElementDeclaration container;
    container.Id                     = m_containerId;
    container.Layout.Sizing.Width    = ClaySizingAxis::Grow( 400 );
    container.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    container.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    container.Layout.Padding         = ClayPadding( 24 );
    container.Layout.ChildGap        = 24;
    container.BackgroundColor        = m_bgColor;

    m_clay->OpenElement( container );

    CreateHeader( m_textColor );
    CreateMainContent( m_cardColor, m_textColor );

    m_clay->CloseElement( );

    if ( !m_cubeContainer->IsClosed( ) )
    {
        DockableContainerStyle cubeStyle;
        cubeStyle.Title           = InteropString( "3D Cube Control" );
        cubeStyle.MinWidth        = 300.0f;
        cubeStyle.MinHeight       = 250.0f;
        cubeStyle.BackgroundColor = m_bgColor;
        cubeStyle.TitleBarColor   = m_darkMode ? ClayColor( 60, 60, 65, 255 ) : ClayColor( 230, 230, 235, 255 );
        cubeStyle.TitleTextColor  = m_textColor;
        cubeStyle.BorderColor     = m_darkMode ? ClayColor( 80, 80, 85, 255 ) : ClayColor( 200, 200, 205, 255 );
        cubeStyle.BorderWidth     = 1.0f;
        cubeStyle.TitleBarHeight  = 30.0f;
        cubeStyle.FontSize        = 14;
        cubeStyle.ShowCloseButton = true;
        cubeStyle.AllowResize     = true;
        cubeStyle.AllowUndock     = true;
        m_cubeContainer->OpenElement( cubeStyle );

        ClayElementDeclaration cubeContent;
        cubeContent.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
        cubeContent.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
        cubeContent.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
        cubeContent.Layout.Padding         = ClayPadding( 16 );
        cubeContent.Layout.ChildGap        = 32;

        m_clay->OpenElement( cubeContent );

        ClayTextDesc sliderLabel;
        sliderLabel.FontSize  = 14;
        sliderLabel.TextColor = m_textColor;
        m_clay->Text( "Rotation Speed:", sliderLabel );
        m_cubeRotationSlider->CreateLayoutElement( );
        ClayElementDeclaration cubeWidgetContainer;
        cubeWidgetContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
        cubeWidgetContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 150 );
        cubeWidgetContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
        cubeWidgetContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

        m_clay->OpenElement( cubeWidgetContainer );
        m_spinningCubeWidget->CreateLayoutElement( );
        m_clay->CloseElement( );
        m_clay->CloseElement( );

        m_cubeContainer->CloseElement( );
    }

    if ( !m_textContainer->IsClosed( ) )
    {
        DockableContainerStyle textStyle;
        textStyle.Title           = InteropString( "Text Editor" );
        textStyle.MinWidth        = 400.0f;
        textStyle.MinHeight       = 300.0f;
        textStyle.BackgroundColor = m_cardColor;
        textStyle.TitleBarColor   = m_darkMode ? ClayColor( 60, 60, 65, 255 ) : ClayColor( 230, 230, 235, 255 );
        textStyle.TitleTextColor  = m_textColor;
        textStyle.BorderColor     = m_darkMode ? ClayColor( 80, 80, 85, 255 ) : ClayColor( 200, 200, 205, 255 );
        textStyle.BorderWidth     = 1.0f;
        textStyle.TitleBarHeight  = 30.0f;
        textStyle.FontSize        = 14;
        textStyle.ShowCloseButton = true;
        textStyle.AllowResize     = true;
        textStyle.AllowUndock     = true;
        m_textContainer->OpenElement( textStyle );

        ClayElementDeclaration textContent;
        textContent.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
        textContent.Layout.Sizing.Height = ClaySizingAxis::Grow( );
        textContent.Layout.Padding       = ClayPadding( 16 );

        m_clay->OpenElement( textContent );

        ClayTextDesc textDesc{ };
        textDesc.FontSize  = 18;
        textDesc.TextColor = m_textColor;
        m_clay->Text( m_multilineTextField->GetText( ), textDesc );

        m_clay->CloseElement( );

        m_textContainer->CloseElement( );
    }
    if ( m_dockingManager )
    {
        m_dockingManager->Render( );
    }

    m_mouseJustReleased = false;
}

void UIExample::CreateHeader( const ClayColor &textColor ) const
{
    ClayElementDeclaration headerContainer;
    headerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    headerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 60 );
    headerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    headerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

    m_clay->OpenElement( headerContainer );

    ClayTextDesc headerTextDesc;
    headerTextDesc.FontSize  = 28;
    headerTextDesc.TextColor = textColor;
    m_clay->Text( "UI Example", headerTextDesc );

    m_clay->CloseElement( );
}

void UIExample::CreateMainContent( const ClayColor &cardColor, const ClayColor &textColor ) const
{
    ClayElementDeclaration contentContainer;
    contentContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    contentContainer.Layout.Sizing.Height    = ClaySizingAxis::Grow( );
    contentContainer.Layout.LayoutDirection  = ClayLayoutDirection::TopToBottom;
    contentContainer.Layout.ChildGap         = 20;
    contentContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;

    m_clay->OpenElement( contentContainer );

    CreateCard( cardColor, textColor, "⚙️ Settings" );

    CreateCheckboxRow( "Dark Mode", m_darkModeCheckbox, textColor );
    CreateDropdownRow( "DPI Scale", m_dpiScaleDropdown, textColor );

    m_clay->CloseElement( );

    ClayElementDeclaration buttonRow;
    buttonRow.Layout.Sizing.Width     = ClaySizingAxis::Fit( );
    buttonRow.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 50 );
    buttonRow.Layout.LayoutDirection  = ClayLayoutDirection::LeftToRight;
    buttonRow.Layout.ChildGap         = 16;
    buttonRow.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

    m_clay->OpenElement( buttonRow );

    const ClayColor buttonBg   = m_darkModeCheckbox && m_darkModeCheckbox->IsChecked( ) ? ClayColor( 70, 130, 180, 255 ) : ClayColor( 100, 149, 237, 255 );
    const auto      buttonText = ClayColor( 255, 255, 255, 255 );

    const uint32_t showBoxButtonId = m_clay->HashString( "ShowBoxButton" );
    CreateButton( "Show Box", buttonBg, buttonText, showBoxButtonId );

    if ( m_clay->PointerOver( showBoxButtonId ) && m_mouseJustReleased )
    {
        m_cubeContainer->Show( );
    }

    const uint32_t popTextButtonId = m_clay->HashString( "PopTextButton" );
    CreateButton( "Pop Text!", buttonBg, buttonText, popTextButtonId );

    if ( m_clay->PointerOver( popTextButtonId ) && m_mouseJustReleased )
    {
        m_textContainer->Show( );
    }

    m_clay->CloseElement( );
    CreateCard( cardColor, textColor, "Text Area" );

    ClayElementDeclaration textFieldContainer;
    textFieldContainer.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    textFieldContainer.Layout.Sizing.Height = ClaySizingAxis::Fixed( 150 );
    textFieldContainer.Layout.Padding       = ClayPadding( 4 );

    m_clay->OpenElement( textFieldContainer );

    if ( m_multilineTextField )
    {
        m_multilineTextField->CreateLayoutElement( );
    }

    m_clay->CloseElement( );
    m_clay->CloseElement( ); // Close text card
    m_clay->CloseElement( ); // Close content container
}

void UIExample::CreateCard( const ClayColor &cardColor, const ClayColor &textColor, const char *title ) const
{
    ClayElementDeclaration card;
    card.Layout.Sizing.Width    = ClaySizingAxis::Fixed( 400 );
    card.Layout.Sizing.Height   = ClaySizingAxis::Fit( );
    card.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    card.Layout.Padding         = ClayPadding( 20 );
    card.Layout.ChildGap        = 12;
    card.BackgroundColor        = cardColor;
    card.CornerRadius           = ClayCornerRadius( 12 );
    card.Border.Color           = ClayColor( 200, 200, 200, 50 );
    card.Border.Width           = ClayBorderWidth( 1 );

    m_clay->OpenElement( card );

    ClayTextDesc titleDesc;
    titleDesc.FontSize  = 18;
    titleDesc.TextColor = textColor;
    m_clay->Text( title, titleDesc );
}

void UIExample::CreateCheckboxRow( const char *label, CheckboxWidget *widget, const ClayColor &textColor ) const
{
    ClayElementDeclaration row;
    row.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    row.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 32 );
    row.Layout.LayoutDirection  = ClayLayoutDirection::LeftToRight;
    row.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    row.Layout.ChildGap         = 12;

    m_clay->OpenElement( row );

    if ( widget )
    {
        widget->CreateLayoutElement( );
    }

    ClayTextDesc labelDesc;
    labelDesc.FontSize  = 14;
    labelDesc.TextColor = textColor;
    m_clay->Text( label, labelDesc );

    m_clay->CloseElement( );
}

void UIExample::CreateDropdownRow( const char *label, DropdownWidget *widget, const ClayColor &textColor ) const
{
    ClayElementDeclaration row;
    row.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    row.Layout.Sizing.Height   = ClaySizingAxis::Fixed( 60 );
    row.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    row.Layout.ChildGap        = 8;

    m_clay->OpenElement( row );

    ClayTextDesc labelDesc;
    labelDesc.FontSize  = 14;
    labelDesc.TextColor = textColor;
    m_clay->Text( label, labelDesc );

    if ( widget )
    {
        widget->CreateLayoutElement( );
    }

    m_clay->CloseElement( );
}

void UIExample::CreateButton( const char *text, const ClayColor &bgColor, const ClayColor &textColor, const uint32_t buttonId ) const
{
    ClayElementDeclaration button;
    button.Id                      = buttonId;
    button.Layout.Sizing.Width     = ClaySizingAxis::Fixed( 120 );
    button.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 40 );
    button.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    button.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    button.BackgroundColor         = bgColor;
    button.CornerRadius            = ClayCornerRadius( 8 );

    if ( m_clay->PointerOver( buttonId ) )
    {
        button.BackgroundColor = ClayColor( bgColor.R - 20, bgColor.G - 20, bgColor.B - 20, bgColor.A );
    }

    m_clay->OpenElement( button );

    ClayTextDesc buttonTextDesc;
    buttonTextDesc.FontSize  = 14;
    buttonTextDesc.TextColor = textColor;
    m_clay->Text( text, buttonTextDesc );

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
    renderingDesc.RTAttachments.Elements = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_clay->BeginLayout( );
    CreateUI( );
    m_clay->EndLayout( commandList, frameIndex, m_worldData.DeltaTime );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void UIExample::HandleEvent( Event &event )
{
    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        m_mousePressed      = true;
        m_mouseJustReleased = false;
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        m_mousePressed      = false;
        m_mouseJustReleased = true;
    }

    m_clay->HandleEvent( event );
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void UIExample::Quit( )
{
    m_frameSync->WaitIdle( );
    if ( m_spinningCubeWidget )
    {
        m_clay->RemoveWidget( m_spinningCubeWidget->GetId( ) );
    }
    m_spinningCubeWidget.reset( );
    m_clay.reset( );
    IExample::Quit( );
}
