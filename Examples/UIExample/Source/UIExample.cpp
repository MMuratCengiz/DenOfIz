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
    uiDesc.MaxNumElements     = 16384;

    m_clay = std::make_unique<Clay>( uiDesc );

    const auto viewport = m_swapChain->GetViewport( );
    m_clay->SetViewportSize( viewport.Width, viewport.Height );
    m_clay->SetDebugModeEnabled( true );

    m_containerId       = m_clay->HashString( "Container" );
    m_scrollContainerId = m_clay->HashString( "ScrollContainer" );

    m_themeOptions.AddElement( InteropString( "Modern Light" ) );
    m_themeOptions.AddElement( InteropString( "Modern Dark" ) );
    m_themeOptions.AddElement( InteropString( "Classic" ) );
    m_themeOptions.AddElement( InteropString( "High Contrast" ) );

    m_languageOptions.AddElement( InteropString( "English" ) );
    m_languageOptions.AddElement( InteropString( "Spanish" ) );
    m_languageOptions.AddElement( InteropString( "French" ) );
    m_languageOptions.AddElement( InteropString( "German" ) );
    m_languageOptions.AddElement( InteropString( "Japanese" ) );

    // Create widgets using the new widget system
    m_darkModeCheckbox         = m_clay->CreateCheckbox( m_clay->HashString( "DarkModeCheckbox" ), false );
    m_enableAnimationsCheckbox = m_clay->CreateCheckbox( m_clay->HashString( "AnimationsCheckbox" ), true );

    SliderStyle sliderStyle = { };
    sliderStyle.MinValue    = 0.0f;
    sliderStyle.MaxValue    = 1.0f;
    sliderStyle.Step        = 0.01f;
    m_volumeSlider          = m_clay->CreateSlider( m_clay->HashString( "VolumeSlider" ), 0.75f, sliderStyle );
    m_brightnessSlider      = m_clay->CreateSlider( m_clay->HashString( "BrightnessSlider" ), 0.5f, sliderStyle );

    m_themeDropdown    = m_clay->CreateDropdown( m_clay->HashString( "ThemeDropdown" ), m_themeOptions );
    m_languageDropdown = m_clay->CreateDropdown( m_clay->HashString( "LanguageDropdown" ), m_languageOptions );

    m_accentColorPicker = m_clay->CreateColorPicker( m_clay->HashString( "AccentColorPicker" ), Float_3{ 0.2f, 0.6f, 1.0f } );

    m_usernameField = m_clay->CreateTextField( m_clay->HashString( "UsernameField" ) );
    m_passwordField = m_clay->CreateTextField( m_clay->HashString( "PasswordField" ) );
    m_commentsField = m_clay->CreateTextField( m_clay->HashString( "CommentsField" ) );

    m_themeDropdown->SetSelectedIndex( 0 );
    m_languageDropdown->SetSelectedIndex( 0 );

    m_time.OnEachSecond = []( const double fps ) { LOG( INFO ) << "FPS: " << fps; };
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

void UIExample::CreateUI( )
{
    const bool      darkMode           = m_darkModeCheckbox ? m_darkModeCheckbox->IsChecked( ) : false;
    const ClayColor bgColor            = darkMode ? ClayColor( 30, 30, 33, 255 ) : ClayColor( 245, 245, 250, 255 );
    const ClayColor cardColor          = darkMode ? ClayColor( 45, 45, 48, 255 ) : ClayColor( 255, 255, 255, 255 );
    const ClayColor textColor          = darkMode ? ClayColor( 240, 240, 240, 255 ) : ClayColor( 20, 20, 20, 255 );
    const ClayColor secondaryTextColor = darkMode ? ClayColor( 180, 180, 180, 255 ) : ClayColor( 100, 100, 100, 255 );

    ClayElementDeclaration container;
    container.Id                     = m_containerId;
    container.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    container.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    container.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    container.Layout.Padding         = ClayPadding( 24 );
    container.Layout.ChildGap        = 24;
    container.BackgroundColor        = bgColor;

    m_clay->OpenElement( container );

    CreateHeader( textColor );
    ClayElementDeclaration contentContainer;
    contentContainer.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    contentContainer.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    contentContainer.Layout.LayoutDirection = ClayLayoutDirection::LeftToRight;
    contentContainer.Layout.ChildGap        = 24;

    m_clay->OpenElement( contentContainer );
    CreateSettingsPanel( cardColor, textColor, secondaryTextColor );

    CreateFormsPanel( cardColor, textColor, secondaryTextColor );
    m_clay->CloseElement( );

    CreateFooter( cardColor, textColor, secondaryTextColor );
    m_clay->CloseElement( );

    m_mouseJustReleased = false;
}

void UIExample::CreateHeader( const ClayColor &textColor ) const
{
    ClayElementDeclaration headerContainer;
    headerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    headerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 80 );
    headerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    headerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

    m_clay->OpenElement( headerContainer );

    ClayTextDesc headerTextDesc;
    headerTextDesc.FontSize  = 32;
    headerTextDesc.TextColor = textColor;
    m_clay->Text( "🎨 Clay UI Widget Showcase", headerTextDesc );

    m_clay->CloseElement( );
}

void UIExample::CreateSettingsPanel( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const
{
    ClayElementDeclaration leftColumn;
    leftColumn.Layout.Sizing.Width    = ClaySizingAxis::Fixed( 400 );
    leftColumn.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    leftColumn.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    leftColumn.Layout.ChildGap        = 16;

    m_clay->OpenElement( leftColumn );

    CreateCard( cardColor, textColor, "⚙️ Settings" );
    CreateCheckboxRow( "Dark Mode", m_darkModeCheckbox, textColor );
    CreateCheckboxRow( "Enable Animations", m_enableAnimationsCheckbox, textColor );

    m_clay->CloseElement( );

    CreateCard( cardColor, textColor, "🔊 Audio & Display" );
    CreateSliderRow( "Volume", m_volumeSlider, textColor, secondaryTextColor );
    CreateSliderRow( "Brightness", m_brightnessSlider, textColor, secondaryTextColor );

    m_clay->CloseElement( );

    CreateCard( cardColor, textColor, "🎨 Appearance" );
    CreateDropdownRow( "Theme", m_themeDropdown, textColor );
    CreateDropdownRow( "Language", m_languageDropdown, textColor );

    m_clay->CloseElement( );

    m_clay->CloseElement( );
}

void UIExample::CreateFormsPanel( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const
{
    ClayElementDeclaration rightColumn;
    rightColumn.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    rightColumn.Layout.Sizing.Height   = ClaySizingAxis::Grow( );
    rightColumn.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    rightColumn.Layout.ChildGap        = 16;

    m_clay->OpenElement( rightColumn );

    CreateCard( cardColor, textColor, "👤 User Profile" );
    CreateTextFieldRow( "Username", m_usernameField, textColor );
    CreateTextFieldRow( "Password", m_passwordField, textColor );

    m_clay->CloseElement( );

    CreateCard( cardColor, textColor, "💬 Comments" );

    ClayTextDesc labelDesc;
    labelDesc.FontSize  = 14;
    labelDesc.TextColor = textColor;
    m_clay->Text( "Comments:", labelDesc );

    ClayElementDeclaration textFieldContainer;
    textFieldContainer.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    textFieldContainer.Layout.Sizing.Height = ClaySizingAxis::Fixed( 120 );
    textFieldContainer.Layout.Padding       = ClayPadding( 4 );

    m_clay->OpenElement( textFieldContainer );

    if ( m_commentsField )
    {
        m_commentsField->CreateLayoutElement( );
    }

    m_clay->CloseElement( );

    m_clay->CloseElement( );

    CreateCard( cardColor, textColor, "🌈 Accent Color (Surprise!)" );

    ClayTextDesc colorLabelDesc;
    colorLabelDesc.FontSize  = 14;
    colorLabelDesc.TextColor = secondaryTextColor;
    m_clay->Text( "Choose your favorite accent color:", colorLabelDesc );

    ClayElementDeclaration colorPickerContainer;
    colorPickerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    colorPickerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 180 );
    colorPickerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    colorPickerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    colorPickerContainer.Layout.Padding          = ClayPadding( 20 );

    m_clay->OpenElement( colorPickerContainer );

    if ( m_accentColorPicker )
    {
        m_accentColorPicker->CreateLayoutElement( );
    }

    m_clay->CloseElement( );

    m_clay->CloseElement( );

    m_clay->CloseElement( );
}

void UIExample::CreateFooter( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor )
{
    ClayElementDeclaration footerContainer;
    footerContainer.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    footerContainer.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 60 );
    footerContainer.Layout.ChildAlignment.X = ClayAlignmentX::Center;
    footerContainer.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    footerContainer.BackgroundColor         = cardColor;
    footerContainer.CornerRadius            = ClayCornerRadius( 8 );

    m_clay->OpenElement( footerContainer );

    ClayTextDesc statusDesc;
    statusDesc.FontSize  = 14;
    statusDesc.TextColor = secondaryTextColor;

    char statusBuffer[ 256 ];
    snprintf( statusBuffer, sizeof( statusBuffer ), "Status: %s | FPS: %.0f | Volume: %.0f%% | Theme: %s", m_statusText.c_str( ), 1.0 / m_worldData.DeltaTime,
              m_volumeSlider ? m_volumeSlider->GetValue( ) * 100.0f : 0.0f,
              m_themeDropdown && m_themeDropdown->GetSelectedIndex( ) >= 0 ? m_themeOptions.GetElement( m_themeDropdown->GetSelectedIndex( ) ).Get( ) : "None" );

    m_clay->Text( statusBuffer, statusDesc );

    m_clay->CloseElement( );
}

void UIExample::CreateCard( const ClayColor &cardColor, const ClayColor &textColor, const char *title ) const
{
    ClayElementDeclaration card;
    card.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
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

void UIExample::CreateSliderRow( const char *label, SliderWidget *widget, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const
{
    ClayElementDeclaration row;
    row.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    row.Layout.Sizing.Height   = ClaySizingAxis::Fixed( 50 );
    row.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    row.Layout.ChildGap        = 8;

    m_clay->OpenElement( row );

    // Label with value
    ClayElementDeclaration labelRow;
    labelRow.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    labelRow.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 20 );
    labelRow.Layout.LayoutDirection  = ClayLayoutDirection::LeftToRight;
    labelRow.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

    m_clay->OpenElement( labelRow );

    ClayTextDesc labelDesc;
    labelDesc.FontSize  = 14;
    labelDesc.TextColor = textColor;
    m_clay->Text( label, labelDesc );

    ClayElementDeclaration spacer;
    spacer.Layout.Sizing.Width = ClaySizingAxis::Grow( );
    m_clay->OpenElement( spacer );
    m_clay->CloseElement( );

    char valueText[ 32 ];
    snprintf( valueText, sizeof( valueText ), "%.0f%%", widget ? widget->GetValue( ) * 100.0f : 0.0f );
    ClayTextDesc valueDesc;
    valueDesc.FontSize  = 12;
    valueDesc.TextColor = secondaryTextColor;
    m_clay->Text( valueText, valueDesc );

    m_clay->CloseElement( ); // Close label row

    // Slider
    if ( widget )
    {
        widget->CreateLayoutElement( );
    }

    m_clay->CloseElement( ); // Close main row
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

void UIExample::CreateTextFieldRow( const char *label, TextFieldWidget *widget, const ClayColor &textColor ) const
{
    ClayElementDeclaration row;
    row.Layout.Sizing.Width    = ClaySizingAxis::Grow( );
    row.Layout.Sizing.Height   = ClaySizingAxis::Fixed( 70 );
    row.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    row.Layout.ChildGap        = 8;

    m_clay->OpenElement( row );

    ClayTextDesc labelDesc;
    labelDesc.FontSize  = 14;
    labelDesc.TextColor = textColor;
    m_clay->Text( label, labelDesc );

    ClayElementDeclaration textFieldContainer;
    textFieldContainer.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    textFieldContainer.Layout.Sizing.Height = ClaySizingAxis::Fixed( 40 );

    m_clay->OpenElement( textFieldContainer );

    if ( widget )
    {
        widget->CreateLayoutElement( );
    }

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
    m_clay.reset( );
    IExample::Quit( );
}
