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
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <utility>
#include "DenOfIzGraphics/Input/InputSystem.h"

using namespace DenOfIz;

namespace
{
    constexpr const char *PageNames[] = { "Controls", "Text", "Media" };
    constexpr const char *Qualities[] = { "Low", "Medium", "High", "Ultra" };
    constexpr uint32_t    NumQualities = 4;
    constexpr uint32_t    NumRows      = 40;
    constexpr float       KnobSize     = 16.0f;
    constexpr const char *Paragraph    = "Text is measured by the font and wrapped by the layout, resize the window to see it reflow. "
                                         "A word longer than the line, like Supercalifragilisticexpialidocious, overflows instead of breaking.";

    constexpr DenOfIz_UIColor Hex( const uint32_t rgba )
    {
        return DenOfIz_UIColor{ static_cast<uint8_t>( rgba >> 24 ), static_cast<uint8_t>( rgba >> 16 ), static_cast<uint8_t>( rgba >> 8 ), static_cast<uint8_t>( rgba ) };
    }

    namespace Theme
    {
        constexpr DenOfIz_UIColor Background   = Hex( 0xF3F4F6FF );
        constexpr DenOfIz_UIColor Surface      = Hex( 0xFFFFFFFF );
        constexpr DenOfIz_UIColor Sidebar      = Hex( 0xEEF1F5FF );
        constexpr DenOfIz_UIColor RowAlt       = Hex( 0xEEF2F7FF );
        constexpr DenOfIz_UIColor Border       = Hex( 0xD9DEE5FF );
        constexpr DenOfIz_UIColor BorderStrong = Hex( 0xAEB6C2FF );
        constexpr DenOfIz_UIColor Text         = Hex( 0x1F2937FF );
        constexpr DenOfIz_UIColor Muted        = Hex( 0x6B7280FF );
        constexpr DenOfIz_UIColor OnAccent     = Hex( 0xFFFFFFFF );
        constexpr DenOfIz_UIColor Accent       = Hex( 0x2563EBFF );
        constexpr DenOfIz_UIColor AccentHover  = Hex( 0x1D4ED8FF );
        constexpr DenOfIz_UIColor AccentDown   = Hex( 0x1E40AFFF );
        constexpr DenOfIz_UIColor AccentSoft   = Hex( 0xDBEAFEFF );
        constexpr DenOfIz_UIColor Hover        = Hex( 0xE5E9F0FF );
        constexpr DenOfIz_UIColor Pressed      = Hex( 0xD5DBE4FF );
        constexpr DenOfIz_UIColor Disabled     = Hex( 0xE5E7EBFF );
        constexpr DenOfIz_UIColor DisabledText = Hex( 0x9CA3AFFF );
        constexpr DenOfIz_UIColor Dark         = Hex( 0x111827FF );
        constexpr DenOfIz_UIColor Selection    = Hex( 0x2563EB59 );
        constexpr DenOfIz_UIColor Backdrop     = Hex( 0x0F172A73 );
        constexpr DenOfIz_UIColor Clear        = Hex( 0x00000000 );
    } // namespace Theme

    DenOfIz_StringView View( const char *text )
    {
        return DenOfIz_StringView{ text, std::strlen( text ) };
    }

    bool Has( const uint32_t flags, const uint32_t bits )
    {
        return ( flags & bits ) != 0;
    }

    bool Shift( const uint32_t mod )
    {
        return Has( mod, DENOFIZ_KEY_MOD_SHIFT | DENOFIZ_KEY_MOD_LSHIFT | DENOFIZ_KEY_MOD_RSHIFT );
    }

    bool Shortcut( const uint32_t mod )
    {
        return Has( mod, DENOFIZ_KEY_MOD_GUI | DENOFIZ_KEY_MOD_LGUI | DENOFIZ_KEY_MOD_RGUI | DENOFIZ_KEY_MOD_CTRL | DENOFIZ_KEY_MOD_LCTRL | DENOFIZ_KEY_MOD_RCTRL );
    }

    bool IsContinuation( const char c )
    {
        return ( static_cast<uint8_t>( c ) & 0xC0 ) == 0x80;
    }

    uint32_t PrevCodepoint( const std::string &text, uint32_t offset )
    {
        if ( offset == 0 )
        {
            return 0;
        }
        --offset;
        while ( offset > 0 && IsContinuation( text[ offset ] ) )
        {
            --offset;
        }
        return offset;
    }

    uint32_t NextCodepoint( const std::string &text, uint32_t offset )
    {
        const auto size = static_cast<uint32_t>( text.size( ) );
        if ( offset >= size )
        {
            return size;
        }
        ++offset;
        while ( offset < size && IsContinuation( text[ offset ] ) )
        {
            ++offset;
        }
        return offset;
    }

    DenOfIz_UIElementDesc Row( const float gap = 0.0f )
    {
        DenOfIz_UIElementDesc desc{ };
        desc.Layout.AlignItems = DENOFIZ_UI_ALIGN_CENTER;
        desc.Layout.Gap        = DenOfIz_UILength_Points( gap );
        return desc;
    }

    DenOfIz_UIElementDesc Column( const float gap = 0.0f )
    {
        DenOfIz_UIElementDesc desc{ };
        desc.Layout.Direction = DENOFIZ_UI_DIRECTION_COLUMN;
        desc.Layout.Gap       = DenOfIz_UILength_Points( gap );
        return desc;
    }

    void Fill( DenOfIz_UILayoutDesc &layout )
    {
        layout.Grow   = 1.0f;
        layout.Shrink = 1.0f;
        layout.Basis  = DenOfIz_UIValue_Points( 0.0f );
    }

    DenOfIz_UITransitionDesc ColorTransition( const DenOfIz_UITransitionPropertyFlags properties = DENOFIZ_UI_TRANSITION_PROPERTY_COLORS_BIT )
    {
        DenOfIz_UITransitionDesc transition{ };
        transition.Duration   = 0.12f;
        transition.Properties = properties;
        return transition;
    }

    DenOfIz_UITransitionDesc FadeIn( const float duration, const DenOfIz_Float2 offset )
    {
        DenOfIz_UITransitionDesc transition{ };
        transition.Duration      = duration;
        transition.Enter.Flags   = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT | DENOFIZ_UI_TRANSITION_STATE_OFFSET_BIT;
        transition.Enter.Opacity = 0.0f;
        transition.Enter.Offset  = offset;
        transition.Exit.Flags    = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT;
        transition.Exit.Opacity  = 0.0f;
        return transition;
    }
} // namespace

void UIExample::Init( )
{
    DenOfIz_UIDesc uiDesc{ };
    uiDesc.LogicalDevice      = m_logicalDevice;
    uiDesc.ResourceTracking   = m_resourceTracking;
    uiDesc.Window             = m_window;
    uiDesc.RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    uiDesc.NumFrames          = NumSwapChainBuffers;
    uiDesc.Width              = m_pixelWidth;
    uiDesc.Height             = m_pixelHeight;
    m_ui                      = DenOfIz_UI_Create( &uiDesc );

    m_tabsId        = Id( "Tabs" );
    m_pagesId       = Id( "Pages" );
    m_clickId       = Id( "ClickMe" );
    m_qualityId     = Id( "Quality" );
    m_fieldId       = Id( "TextField" );
    m_fieldScrollId = DenOfIz_UI_CombineId( m_fieldId, 1 );
    m_fieldTextId   = DenOfIz_UI_CombineId( m_fieldId, 2 );
    m_listId        = Id( "List" );
    m_dialogId      = Id( "Dialog" );
    m_caret = m_anchor = static_cast<uint32_t>( m_text.size( ) );

    m_cube = std::make_unique<Spinning3DCubeWidget>( Id( "Cube" ) );
    m_cube->SetCubeColor( DirectX::XMFLOAT4( 0.2f, 0.6f, 1.0f, 1.0f ) );
    m_cube->InitializeRenderResources( m_logicalDevice, m_graphicsQueue, 512, 512 );
}

void UIExample::Update( )
{
    m_deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_blinkTimer += m_deltaTime;
    m_cube->Update( m_deltaTime );

    uint32_t frameIndex = 0;
    if ( !BeginFrame( frameIndex ) )
    {
        return;
    }
    m_cube->RenderToTexture( frameIndex );
    BuildUI( frameIndex );

    const DenOfIz_SystemCursor cursor = DenOfIz_UI_GetCursor( m_ui );
    if ( cursor != m_cursor )
    {
        DenOfIz_InputSystem_SetCursor( cursor );
        m_cursor = cursor;
    }

    Render( frameIndex, m_frame.CommandList );
    DenOfIz_Semaphore      cubeSemaphore = m_cube->GetRenderSemaphore( );
    DenOfIz_SemaphoreArray waitSemaphores{ };
    waitSemaphores.Elements    = &cubeSemaphore;
    waitSemaphores.NumElements = 1;
    EndFrame( &waitSemaphores );
}

void UIExample::BuildUI( const uint32_t frameIndex )
{
    DenOfIz_UI_BeginFrame( m_ui, m_deltaTime );
    m_focusId            = DenOfIz_UI_GetFocus( m_ui );
    const bool wantsText = m_focusId == m_fieldId;
    if ( wantsText && !m_textInput )
    {
        DenOfIz_InputSystem_StartTextInput( );
        m_blinkTimer = 0.0f;
    }
    else if ( !wantsText && m_textInput )
    {
        DenOfIz_InputSystem_StopTextInput( );
    }
    m_textInput = wantsText;

    DenOfIz_UIElementDesc root = Column( );
    root.Layout.Grow           = 1.0f;
    root.Style.BackgroundColor = Theme::Background;
    Open( root );
    {
        TopBar( );

        DenOfIz_UIElementDesc body{ };
        Fill( body.Layout );
        Open( body );
        {
            Sidebar( );
            ControlsPage( );
            TextPage( );
            MediaPage( frameIndex );
        }
        Close( );
    }
    Close( );

    QualityPopup( );
    Tooltip( );
    Dialog( );
    DenOfIz_UI_EndFrame( m_ui );
    m_keyActivate = false;
}

void UIExample::TopBar( )
{
    DenOfIz_UIElementDesc bar    = Row( 10 );
    bar.Layout.Height            = DenOfIz_UIValue_Points( 52 );
    bar.Layout.Padding           = DenOfIz_UILengthEdges_Symmetric( 16, 0 );
    bar.Style.BackgroundColor    = Theme::Surface;
    bar.Style.BorderColor        = Theme::Border;
    bar.Style.BorderWidth.Bottom = 1.0f;
    Open( bar );
    {
        DenOfIz_UIElementDesc logo = Row( );
        logo.Layout.Width          = DenOfIz_UIValue_Points( 26 );
        logo.Layout.Height         = DenOfIz_UIValue_Points( 26 );
        logo.Layout.Justify        = DENOFIZ_UI_JUSTIFY_CENTER;
        logo.Style.BackgroundColor = Theme::Accent;
        logo.Style.CornerRadius    = DenOfIz_UICorners_All( 7 );
        Open( logo );
        Label( "Dz", 12, Theme::OnAccent );
        Close( );
        Label( "DenOfIz UI", 17, Theme::Text );

        DenOfIz_UIElementDesc pushRight{ };
        pushRight.Layout.Margin.Left = DenOfIz_UILength_Auto( );
        Open( pushRight );
        if ( Button( m_debug ? "Outlines: on" : "Outlines: off", Id( "Outlines" ), false ) )
        {
            ToggleDebug( );
        }
        Close( );
    }
    Close( );
}

void UIExample::Sidebar( )
{
    DenOfIz_UIElementDesc side   = Column( 4 );
    side.Layout.Width            = DenOfIz_UIValue_Points( 200 );
    side.Layout.Padding          = DenOfIz_UILengthEdges_All( 12 );
    side.Style.BackgroundColor   = Theme::Sidebar;
    side.Style.BorderColor       = Theme::Border;
    side.Style.BorderWidth.Right = 1.0f;
    Open( side );
    for ( uint32_t i = 0; i < static_cast<uint32_t>( Page::Count ); ++i )
    {
        const uint32_t              id          = DenOfIz_UI_CombineId( m_tabsId, i );
        const DenOfIz_UIInteraction interaction = Interaction( id );
        if ( Activated( id, interaction ) && m_page != static_cast<Page>( i ) )
        {
            m_page        = static_cast<Page>( i );
            m_qualityOpen = false;
        }
        const bool selected = m_page == static_cast<Page>( i );
        const bool hovered  = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

        DenOfIz_UIElementDesc tab = Row( );
        tab.Id                    = id;
        tab.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
        tab.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
        tab.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 12, 8 );
        tab.Style.BackgroundColor = selected ? Theme::AccentSoft : hovered ? Theme::Hover : Theme::Clear;
        tab.Style.BorderColor     = Theme::Clear;
        tab.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
        tab.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
        tab.Transition            = ColorTransition( );
        FocusRing( tab );
        Open( tab );
        Label( PageNames[ i ], 14, selected ? Theme::Accent : Theme::Text );
        Close( );
    }

    DenOfIz_UIElementDesc hint{ };
    hint.Layout.Margin.Top = DenOfIz_UILength_Auto( );
    hint.Layout.Padding    = DenOfIz_UILengthEdges_Symmetric( 4, 0 );
    Open( hint );
    Label( "Tab focus, F11 outlines, Esc quits", 12, Theme::Muted );
    Close( );
    Close( );
}

void UIExample::ToggleDebug( )
{
    m_debug = !m_debug;
    DenOfIz_UI_SetDebugFlags( m_ui, m_debug ? DENOFIZ_UI_DEBUG_OUTLINES_BIT | DENOFIZ_UI_DEBUG_HOVERED_BIT : DENOFIZ_UI_DEBUG_NONE_BIT );
}

void UIExample::BeginCard( const char *title, const char *description, const float basis ) const
{
    DenOfIz_UIElementDesc card = Column( 12 );
    if ( basis > 0.0f )
    {
        card.Layout.Grow   = 1.0f;
        card.Layout.Shrink = 1.0f;
        card.Layout.Basis  = DenOfIz_UIValue_Points( basis );
    }
    card.Layout.Padding        = DenOfIz_UILengthEdges_All( 16 );
    card.Style.BackgroundColor = Theme::Surface;
    card.Style.BorderColor     = Theme::Border;
    card.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    card.Style.CornerRadius    = DenOfIz_UICorners_All( 10 );
    Open( card );
    Label( title, 16, Theme::Text );
    Label( description, 13, Theme::Muted );
}

void UIExample::ControlsPage( )
{
    DenOfIz_UIElementDesc page = Column( 16 );
    page.Id                    = DenOfIz_UI_CombineId( m_pagesId, static_cast<uint32_t>( Page::Controls ) );
    page.Layout.Display        = m_page == Page::Controls ? DENOFIZ_UI_DISPLAY_FLEX : DENOFIZ_UI_DISPLAY_NONE;
    page.Layout.Padding        = DenOfIz_UILengthEdges_All( 24 );
    page.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    Fill( page.Layout );
    Open( page );

    DenOfIz_UIElementDesc grid = { };
    grid.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    grid.Layout.Gap            = DenOfIz_UILength_Points( 16 );
    grid.Layout.LineGap        = DenOfIz_UILength_Points( 16 );
    Open( grid );

    BeginCard( "Buttons", "Hover, press and focus feed back through color transitions.", 320 );
    {
        Open( Row( 8 ) );
        if ( Button( "Click me", m_clickId ) )
        {
            ++m_clicks;
        }
        if ( Button( "Reset", Id( "Reset" ), false, m_clicks == 0 ) )
        {
            m_clicks = 0;
        }
        Close( );
        m_showTooltip = Has( Interaction( m_clickId ).Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );
        Label( Format( "Clicked %u times", m_clicks ), 14, Theme::Text );
    }
    Close( );

    BeginCard( "Settings", "A toggle, a slider and a dropdown popup.", 320 );
    {
        Toggle( Id( "Notifications" ), m_notifications, "Notifications" );

        Open( Row( 12 ) );
        Label( "Volume", 14, Theme::Text );
        Slider( Id( "Volume" ), m_volume );
        Label( Format( "%.0f%%", m_volume * 100.0f ), 14, Theme::Muted );
        Close( );

        Open( Row( 12 ) );
        Label( "Quality", 14, Theme::Text );
        Dropdown( );
        Close( );
    }
    Close( );

    BeginCard( "Dialog", "A modal with enter and exit transitions. Click the backdrop or press Esc to dismiss it.", 320 );
    {
        Open( Row( 12 ) );
        if ( Button( "Open dialog", Id( "OpenDialog" ) ) )
        {
            m_dialogOpen = true;
        }
        Label( Format( "Last result: %s", m_dialogResult ), 14, Theme::Muted );
        Close( );
    }
    Close( );

    Close( );
    Close( );
}

void UIExample::TextPage( )
{
    DenOfIz_UIElementDesc page = Column( 16 );
    page.Id                    = DenOfIz_UI_CombineId( m_pagesId, static_cast<uint32_t>( Page::Text ) );
    page.Layout.Display        = m_page == Page::Text ? DENOFIZ_UI_DISPLAY_FLEX : DENOFIZ_UI_DISPLAY_NONE;
    page.Layout.Padding        = DenOfIz_UILengthEdges_All( 24 );
    page.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    Fill( page.Layout );
    Open( page );

    BeginCard( "Text field", "Click to place the caret, drag or Shift+arrows to select, double click selects everything." );
    TextField( );
    Label( Format( "Caret %u, selection %u..%u", m_caret, std::min( m_caret, m_anchor ), std::max( m_caret, m_anchor ) ), 12, Theme::Muted );
    Close( );

    BeginCard( "Wrapping", "Each column gets an equal share of the row and wraps its text to that width." );
    {
        DenOfIz_UIElementDesc columns = Row( 16 );
        columns.Layout.AlignItems     = DENOFIZ_UI_ALIGN_START;
        Open( columns );

        DenOfIz_UIElementDesc column = Column( 6 );
        Fill( column.Layout );

        DenOfIz_UITextDesc text{ };
        text.Color    = Theme::Text;
        text.FontSize = 14;

        Open( column );
        Label( "Words", 12, Theme::Muted );
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &text );
        Close( );

        Open( column );
        Label( "Two lines, ellipsis", 12, Theme::Muted );
        DenOfIz_UITextDesc clamped = text;
        clamped.MaxLines           = 2;
        clamped.Overflow           = DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS;
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &clamped );
        Close( );

        Open( column );
        Label( "Centered", 12, Theme::Muted );
        DenOfIz_UITextDesc centered = text;
        centered.Align              = DENOFIZ_UI_TEXT_ALIGN_CENTER;
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &centered );
        Close( );

        Close( );
    }
    Close( );
    Close( );
}

void UIExample::MediaPage( const uint32_t frameIndex )
{
    DenOfIz_UIElementDesc page = Column( 16 );
    page.Id                    = DenOfIz_UI_CombineId( m_pagesId, static_cast<uint32_t>( Page::Media ) );
    page.Layout.Display        = m_page == Page::Media ? DENOFIZ_UI_DISPLAY_FLEX : DENOFIZ_UI_DISPLAY_NONE;
    page.Layout.Padding        = DenOfIz_UILengthEdges_All( 24 );
    page.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    Fill( page.Layout );
    Open( page );

    BeginCard( "Render target as an image", "The cube is rendered into a texture every frame and drawn with each fit mode." );
    {
        Open( Row( 12 ) );
        constexpr std::pair<DenOfIz_UIImageFit, const char *> fits[] = {
            { DENOFIZ_UI_IMAGE_FIT_FILL, "Fill" }, { DENOFIZ_UI_IMAGE_FIT_CONTAIN, "Contain" }, { DENOFIZ_UI_IMAGE_FIT_COVER, "Cover" } };
        for ( const auto &[ fit, name ] : fits )
        {
            DenOfIz_UIElementDesc column = Column( 6 );
            Fill( column.Layout );
            Open( column );

            DenOfIz_UIElementDesc image = { };
            image.Layout.Height         = DenOfIz_UIValue_Points( 150 );
            image.Style.BackgroundColor = Theme::Dark;
            image.Style.CornerRadius    = DenOfIz_UICorners_All( 8 );
            image.Image.Texture         = m_cube->GetRenderTarget( frameIndex );
            image.Image.Fit             = fit;
            Open( image );
            Close( );
            Label( name, 12, Theme::Muted );
            Close( );
        }
        Close( );
    }
    Close( );

    BeginCard( "Rounded clipping", "The rows are clipped by the list's rounded corners while it scrolls." );
    {
        DenOfIz_UIElementDesc list = Column( );
        list.Id                    = m_listId;
        list.Layout.Height         = DenOfIz_UIValue_Points( 220 );
        list.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
        list.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
        list.Style.BorderColor     = Theme::Border;
        list.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
        list.Style.CornerRadius    = DenOfIz_UICorners_All( 14 );
        Open( list );
        for ( uint32_t i = 0; i < NumRows; ++i )
        {
            const uint32_t              id          = DenOfIz_UI_CombineId( m_listId, i );
            const DenOfIz_UIInteraction interaction = Interaction( id );
            if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) )
            {
                m_selectedRow = static_cast<int32_t>( i );
            }
            const bool selected = m_selectedRow == static_cast<int32_t>( i );
            const bool hovered  = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

            DenOfIz_UIElementDesc row = Row( );
            row.Id                    = id;
            row.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
            row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 16, 10 );
            row.Style.BackgroundColor = selected ? Theme::Accent : hovered ? Theme::Hover : i % 2 == 0 ? Theme::Surface : Theme::RowAlt;
            row.Transition            = ColorTransition( );
            Open( row );
            Label( Format( "Row %u", i + 1 ), 14, selected ? Theme::OnAccent : Theme::Text );
            Close( );
        }
        Close( );
    }
    Close( );

    Close( );
}

bool UIExample::Button( const char *label, const uint32_t id, const bool primary, const bool disabled )
{
    const DenOfIz_UIInteraction interaction = Interaction( id );
    const bool                  hovered     = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );
    const bool                  pressed     = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT );

    DenOfIz_UIColor background = primary ? ( pressed ? Theme::AccentDown : hovered ? Theme::AccentHover : Theme::Accent ) : ( pressed ? Theme::Pressed : hovered ? Theme::Hover : Theme::Surface );
    DenOfIz_UIColor border     = primary ? Theme::Accent : Theme::Border;
    DenOfIz_UIColor text       = primary ? Theme::OnAccent : Theme::Text;
    if ( disabled )
    {
        background = border = Theme::Disabled;
        text                = Theme::DisabledText;
    }

    DenOfIz_UIElementDesc desc = Row( );
    desc.Id                    = id;
    desc.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT | ( disabled ? DENOFIZ_UI_ELEMENT_DISABLED_BIT : DENOFIZ_UI_ELEMENT_NONE_BIT );
    desc.Cursor                = disabled ? DENOFIZ_SYSTEM_CURSOR_NOT_ALLOWED : DENOFIZ_SYSTEM_CURSOR_POINTER;
    desc.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 14, 7 );
    desc.Style.BackgroundColor = background;
    desc.Style.BorderColor     = border;
    desc.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    desc.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    desc.Transition            = ColorTransition( );
    FocusRing( desc );
    Open( desc );
    Label( label, 14, text );
    Close( );
    return !disabled && Activated( id, interaction );
}

bool UIExample::Toggle( const uint32_t id, bool &value, const char *label )
{
    const bool changed = Activated( id, Interaction( id ) );
    if ( changed )
    {
        value = !value;
    }

    DenOfIz_UIElementDesc row = Row( 10 );
    row.Id                    = id;
    row.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    row.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    row.Layout.AlignSelf      = DENOFIZ_UI_ALIGN_SELF_START;
    row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 4, 2 );
    row.Style.BorderColor     = Theme::Clear;
    row.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    row.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    FocusRing( row );
    Open( row );
    {
        DenOfIz_UIElementDesc track = Row( );
        track.Layout.Width          = DenOfIz_UIValue_Points( 40 );
        track.Layout.Height         = DenOfIz_UIValue_Points( 22 );
        track.Layout.Padding        = DenOfIz_UILengthEdges_All( 2 );
        track.Layout.Justify        = value ? DENOFIZ_UI_JUSTIFY_END : DENOFIZ_UI_JUSTIFY_START;
        track.Style.BackgroundColor = value ? Theme::Accent : Theme::BorderStrong;
        track.Style.CornerRadius    = DenOfIz_UICorners_All( 11 );
        track.Transition            = ColorTransition( );
        Open( track );
        {
            DenOfIz_UIElementDesc knob = { };
            knob.Layout.Width          = DenOfIz_UIValue_Points( 18 );
            knob.Layout.Height         = DenOfIz_UIValue_Points( 18 );
            knob.Style.BackgroundColor = Theme::Surface;
            knob.Style.CornerRadius    = DenOfIz_UICorners_All( 9 );
            knob.Transition.Duration   = 0.18f;
            knob.Transition.Easing     = DENOFIZ_UI_EASING_EASE_IN_OUT;
            knob.Transition.Properties = DENOFIZ_UI_TRANSITION_PROPERTY_POSITION_BIT;
            Open( knob );
            Close( );
        }
        Close( );
        Label( label, 14, Theme::Text );
    }
    Close( );
    return changed;
}

void UIExample::Slider( const uint32_t id, float &value )
{
    const DenOfIz_UIInteraction interaction = Interaction( id );
    const float                 track       = interaction.Rect.Width - KnobSize;
    if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT ) && track > 0.0f )
    {
        value = std::clamp( ( interaction.PointerPosition.X - KnobSize * 0.5f ) / track, 0.0f, 1.0f );
    }
    const bool active = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT | DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc slider = Row( );
    slider.Id                    = id;
    slider.Pointer               = DENOFIZ_UI_POINTER_MODE_BLOCK;
    slider.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    slider.Layout.Height         = DenOfIz_UIValue_Points( 20 );
    slider.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( KnobSize * 0.5f, 0 );
    Fill( slider.Layout );
    Open( slider );
    {
        DenOfIz_UIElementDesc rail = { };
        rail.Layout.Height         = DenOfIz_UIValue_Points( 6 );
        rail.Style.BackgroundColor = Theme::Border;
        rail.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
        Fill( rail.Layout );
        Open( rail );
        {
            DenOfIz_UIElementDesc fill = { };
            fill.Layout.Width          = DenOfIz_UIValue_Percent( value );
            fill.Style.BackgroundColor = Theme::Accent;
            fill.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
            Open( fill );
            Close( );

            DenOfIz_UIElementDesc knob = { };
            knob.Layout.Position       = DENOFIZ_UI_POSITION_ABSOLUTE;
            knob.Layout.Insets.Left    = DenOfIz_UIValue_Percent( value );
            knob.Layout.Insets.Top     = DenOfIz_UIValue_Points( ( 6 - KnobSize ) * 0.5f );
            knob.Layout.Margin.Left    = DenOfIz_UILength_Points( -KnobSize * 0.5f );
            knob.Layout.Width          = DenOfIz_UIValue_Points( KnobSize );
            knob.Layout.Height         = DenOfIz_UIValue_Points( KnobSize );
            knob.Style.BackgroundColor = Theme::Surface;
            knob.Style.BorderColor     = active ? Theme::AccentDown : Theme::Accent;
            knob.Style.BorderWidth     = DenOfIz_UIEdges_All( 2 );
            knob.Style.CornerRadius    = DenOfIz_UICorners_All( KnobSize * 0.5f );
            knob.Transition            = ColorTransition( );
            Open( knob );
            Close( );
        }
        Close( );
    }
    Close( );
}

void UIExample::Dropdown( )
{
    const DenOfIz_UIInteraction interaction = Interaction( m_qualityId );
    const uint32_t              popupId     = DenOfIz_UI_CombineId( m_qualityId, 1 );
    if ( m_qualityOpen )
    {
        for ( uint32_t i = 0; i < NumQualities; ++i )
        {
            if ( Has( Interaction( DenOfIz_UI_CombineId( popupId, i ) ).Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) )
            {
                m_quality     = i;
                m_qualityOpen = false;
            }
        }
        if ( Has( Interaction( popupId ).Flags, DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT ) )
        {
            m_qualityOpen = false;
        }
    }
    if ( Activated( m_qualityId, interaction ) )
    {
        m_qualityOpen = !m_qualityOpen;
    }
    const bool hovered = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc button = Row( 8 );
    button.Id                    = m_qualityId;
    button.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    button.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    button.Layout.Width          = DenOfIz_UIValue_Points( 140 );
    button.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 10, 5 );
    button.Layout.Justify        = DENOFIZ_UI_JUSTIFY_SPACE_BETWEEN;
    button.Style.BackgroundColor = hovered || m_qualityOpen ? Theme::Background : Theme::Surface;
    button.Style.BorderColor     = m_qualityOpen ? Theme::Accent : Theme::Border;
    button.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    button.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    button.Transition            = ColorTransition( );
    FocusRing( button );
    Open( button );
    Label( Qualities[ m_quality ], 14, Theme::Text );
    Label( m_qualityOpen ? "^" : "v", 12, Theme::Muted );
    Close( );
}

void UIExample::QualityPopup( )
{
    if ( !m_qualityOpen )
    {
        return;
    }
    DenOfIz_UIElementDesc popup = Column( 2 );
    popup.Id                    = DenOfIz_UI_CombineId( m_qualityId, 1 );
    popup.Layout.Width          = DenOfIz_UIValue_Percent( 1.0f );
    popup.Layout.MaxHeight      = DenOfIz_UIValue_Points( 200 );
    popup.Layout.Padding        = DenOfIz_UILengthEdges_All( 4 );
    popup.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
    popup.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    popup.Style.BackgroundColor = Theme::Surface;
    popup.Style.BorderColor     = Theme::Border;
    popup.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    popup.Style.CornerRadius    = DenOfIz_UICorners_All( 8 );
    popup.Floating.Target       = DENOFIZ_UI_FLOATING_TARGET_ELEMENT;
    popup.Floating.TargetId     = m_qualityId;
    popup.Floating.Anchor       = DENOFIZ_UI_ANCHOR_TOP_LEFT;
    popup.Floating.TargetAnchor = DENOFIZ_UI_ANCHOR_BOTTOM_LEFT;
    popup.Floating.Offset       = DenOfIz_Float2{ 0.0f, 4.0f };
    popup.Floating.Flags        = DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT;
    popup.Transition            = FadeIn( 0.12f, DenOfIz_Float2{ 0.0f, -6.0f } );
    Open( popup );
    for ( uint32_t i = 0; i < NumQualities; ++i )
    {
        const uint32_t id       = DenOfIz_UI_CombineId( popup.Id, i );
        const bool     selected = i == m_quality;
        const bool     hovered  = Has( Interaction( id ).Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

        DenOfIz_UIElementDesc option = Row( );
        option.Id                    = id;
        option.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
        option.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 8, 6 );
        option.Style.BackgroundColor = selected ? Theme::AccentSoft : hovered ? Theme::Hover : Theme::Clear;
        option.Style.CornerRadius    = DenOfIz_UICorners_All( 4 );
        Open( option );
        Label( Qualities[ i ], 14, selected ? Theme::Accent : Theme::Text );
        Close( );
    }
    Close( );
}

void UIExample::Tooltip( ) const
{
    if ( !m_showTooltip )
    {
        return;
    }
    DenOfIz_UIElementDesc tip = Row( );
    tip.Id                    = DenOfIz_UI_CombineId( m_clickId, 1 );
    tip.Pointer               = DENOFIZ_UI_POINTER_MODE_PASSTHROUGH;
    tip.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 8, 5 );
    tip.Style.BackgroundColor = Theme::Dark;
    tip.Style.CornerRadius    = DenOfIz_UICorners_All( 5 );
    tip.Floating.Target       = DENOFIZ_UI_FLOATING_TARGET_ELEMENT;
    tip.Floating.TargetId     = m_clickId;
    tip.Floating.Anchor       = DENOFIZ_UI_ANCHOR_TOP_CENTER;
    tip.Floating.TargetAnchor = DENOFIZ_UI_ANCHOR_BOTTOM_CENTER;
    tip.Floating.Offset       = DenOfIz_Float2{ 0.0f, 6.0f };
    tip.Floating.Flags        = DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT;
    tip.Transition            = FadeIn( 0.12f, DenOfIz_Float2{ 0.0f, -4.0f } );
    Open( tip );
    Label( "Anchored to the button, drawn above the page", 12, Theme::OnAccent );
    Close( );
}

void UIExample::Dialog( )
{
    if ( m_dialogOpen && Has( Interaction( m_dialogId ).Flags, DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT ) )
    {
        m_dialogOpen   = false;
        m_dialogResult = "dismissed";
    }
    if ( !m_dialogOpen )
    {
        return;
    }

    DenOfIz_UIElementDesc backdrop    = Column( );
    backdrop.Pointer                  = DENOFIZ_UI_POINTER_MODE_BLOCK;
    backdrop.Layout.Width             = DenOfIz_UIValue_Percent( 1.0f );
    backdrop.Layout.Height            = DenOfIz_UIValue_Percent( 1.0f );
    backdrop.Layout.Justify           = DENOFIZ_UI_JUSTIFY_CENTER;
    backdrop.Layout.AlignItems        = DENOFIZ_UI_ALIGN_CENTER;
    backdrop.Style.BackgroundColor    = Theme::Backdrop;
    backdrop.Floating.Target          = DENOFIZ_UI_FLOATING_TARGET_VIEWPORT;
    backdrop.Floating.ZIndex          = 100;
    backdrop.Transition.Duration      = 0.2f;
    backdrop.Transition.Enter.Flags   = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT;
    backdrop.Transition.Enter.Opacity = 0.0f;
    backdrop.Transition.Exit          = backdrop.Transition.Enter;
    Open( backdrop );
    {
        DenOfIz_UIElementDesc dialog   = Column( 12 );
        dialog.Id                      = m_dialogId;
        dialog.Layout.Width            = DenOfIz_UIValue_Points( 400 );
        dialog.Layout.Padding          = DenOfIz_UILengthEdges_All( 20 );
        dialog.Style.BackgroundColor   = Theme::Surface;
        dialog.Style.BorderColor       = Theme::Border;
        dialog.Style.BorderWidth       = DenOfIz_UIEdges_All( 1 );
        dialog.Style.CornerRadius      = DenOfIz_UICorners_All( 12 );
        dialog.Transition              = FadeIn( 0.22f, DenOfIz_Float2{ 0.0f, 32.0f } );
        dialog.Transition.Flags        = DENOFIZ_UI_TRANSITION_ENTER_WITH_PARENT_BIT | DENOFIZ_UI_TRANSITION_EXIT_WITH_PARENT_BIT;
        dialog.Transition.Exit.Flags  |= DENOFIZ_UI_TRANSITION_STATE_SCALE_BIT;
        dialog.Transition.Exit.Scale   = 0.9f;
        Open( dialog );
        {
            Label( "Discard unsaved changes?", 18, Theme::Text );
            Label( "The dialog slides in, the backdrop fades on its own and both animate out after they stop being declared.", 14, Theme::Muted );

            DenOfIz_UIElementDesc actions = Row( 8 );
            actions.Layout.Justify        = DENOFIZ_UI_JUSTIFY_END;
            Open( actions );
            if ( Button( "Cancel", Id( "Cancel" ), false ) )
            {
                m_dialogOpen   = false;
                m_dialogResult = "cancelled";
            }
            if ( Button( "Discard", Id( "Discard" ) ) )
            {
                m_dialogOpen   = false;
                m_dialogResult = "discarded";
            }
            Close( );
        }
        Close( );
    }
    Close( );
}

void UIExample::TextField( )
{
    const DenOfIz_UIInteraction interaction = Interaction( m_fieldId );
    if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESS_STARTED_BIT ) )
    {
        m_selecting = interaction.ClickCount == 1;
        if ( interaction.ClickCount >= 2 )
        {
            m_anchor = 0;
            MoveCaret( static_cast<uint32_t>( m_text.size( ) ), true );
        }
        else
        {
            MoveCaret( DenOfIz_UI_GetTextOffsetAtPoint( m_ui, m_fieldTextId, DenOfIz_UI_GetPointerPosition( m_ui ) ), Shift( DenOfIz_InputSystem_GetModState( ) ) );
        }
    }
    else if ( m_selecting && Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT ) )
    {
        MoveCaret( DenOfIz_UI_GetTextOffsetAtPoint( m_ui, m_fieldTextId, DenOfIz_UI_GetPointerPosition( m_ui ) ), true );
    }
    const bool focused = m_focusId == m_fieldId;
    const bool hovered = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc field = Row( );
    field.Id                    = m_fieldId;
    field.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    field.Cursor                = DENOFIZ_SYSTEM_CURSOR_TEXT;
    field.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 10, 7 );
    field.Style.BackgroundColor = Theme::Surface;
    field.Style.BorderColor     = focused ? Theme::Accent : hovered ? Theme::BorderStrong : Theme::Border;
    field.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    field.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    field.Transition            = ColorTransition( DENOFIZ_UI_TRANSITION_PROPERTY_BORDER_COLOR_BIT );
    Open( field );
    {
        DenOfIz_UIElementDesc viewport = Row( );
        viewport.Id                    = m_fieldScrollId;
        viewport.Layout.Padding.Right  = DenOfIz_UILength_Points( 2 );
        viewport.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_SCROLL;
        viewport.Scroll.Scrollbar      = DENOFIZ_UI_SCROLLBAR_MODE_HIDDEN;
        Fill( viewport.Layout );
        Open( viewport );

        DenOfIz_UITextDesc text{ };
        text.Id              = m_fieldTextId;
        text.Color           = Theme::Text;
        text.Wrap            = DENOFIZ_UI_TEXT_WRAP_NONE;
        text.Selection.Start = std::min( m_caret, m_anchor );
        text.Selection.End   = std::max( m_caret, m_anchor );
        text.Selection.Color = Theme::Selection;
        text.Caret           = m_caret;
        text.CaretColor      = focused && std::fmod( m_blinkTimer, 1.06f ) < 0.53f ? Theme::Text : Theme::Clear;
        DenOfIz_UI_Text( m_ui, DenOfIz_StringView{ m_text.data( ), m_text.size( ) }, &text );
        if ( m_reveal )
        {
            DenOfIz_UI_ScrollCaretIntoView( m_ui, m_fieldTextId, m_caret );
            m_reveal = false;
        }
        Close( );
    }
    Close( );
}

void UIExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_Texture backBuffer = m_frame.BackBuffer;
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, backBuffer, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachment{ };
    attachment.Resource   = backBuffer;
    attachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    attachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    attachment.ClearColor = DenOfIz_Float4{ Theme::Background.R / 255.0f, Theme::Background.G / 255.0f, Theme::Background.B / 255.0f, 1.0f };

    const auto            width  = static_cast<float>( m_frame.Width );
    const auto            height = static_cast<float>( m_frame.Height );
    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachment;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.RenderAreaWidth           = width;
    renderingDesc.RenderAreaHeight          = height;
    renderingDesc.NumLayers                 = 1;

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, width, height );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, width, height );
    DenOfIz_UI_Render( m_ui, frameIndex, commandList );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, backBuffer, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );
}

void UIExample::HandleEvent( DenOfIz_Event &event )
{
    DenOfIz_UI_HandleEvent( m_ui, &event );

    const bool editing = DenOfIz_UI_GetFocus( m_ui ) == m_fieldId;
    switch ( event.Type )
    {
    case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN:
        m_focusVisible = false;
        break;
    case DENOFIZ_EVENT_TYPE_TEXT_INPUT:
        if ( editing )
        {
            Insert( event.Text.Text );
            return;
        }
        break;
    case DENOFIZ_EVENT_TYPE_KEY_DOWN:
        if ( HandleKeyDown( event.Key, editing ) )
        {
            return;
        }
        break;
    default:
        break;
    }
    IExample::HandleEvent( event );
}

bool UIExample::HandleKeyDown( const DenOfIz_KeyboardEventData &key, const bool editing )
{
    switch ( key.KeyCode )
    {
    case DENOFIZ_KEY_CODE_TAB:
        DenOfIz_UI_FocusNext( m_ui, Shift( key.Mod ) );
        m_focusVisible = true;
        return true;
    case DENOFIZ_KEY_CODE_F11:
        ToggleDebug( );
        return true;
    case DENOFIZ_KEY_CODE_ESCAPE:
        if ( m_dialogOpen )
        {
            m_dialogOpen   = false;
            m_dialogResult = "dismissed";
            return true;
        }
        if ( m_qualityOpen )
        {
            m_qualityOpen = false;
            return true;
        }
        if ( DenOfIz_UI_GetFocus( m_ui ) != 0 )
        {
            DenOfIz_UI_SetFocus( m_ui, 0 );
            return true;
        }
        return false;
    case DENOFIZ_KEY_CODE_RETURN:
    case DENOFIZ_KEY_CODE_KEYPAD_ENTER:
    case DENOFIZ_KEY_CODE_SPACE:
        if ( !editing )
        {
            m_keyActivate = true;
            return true;
        }
        break;
    default:
        break;
    }
    return editing && EditKey( key );
}

bool UIExample::EditKey( const DenOfIz_KeyboardEventData &key )
{
    const auto     size   = static_cast<uint32_t>( m_text.size( ) );
    const bool     extend = Shift( key.Mod );
    const uint32_t start  = std::min( m_caret, m_anchor );
    const uint32_t end    = std::max( m_caret, m_anchor );
    switch ( key.KeyCode )
    {
    case DENOFIZ_KEY_CODE_BACKSPACE:
        if ( !DeleteSelection( ) && m_caret > 0 )
        {
            const uint32_t previous = PrevCodepoint( m_text, m_caret );
            m_text.erase( previous, m_caret - previous );
            MoveCaret( previous, false );
        }
        return true;
    case DENOFIZ_KEY_CODE_DELETE:
        if ( !DeleteSelection( ) && m_caret < size )
        {
            m_text.erase( m_caret, NextCodepoint( m_text, m_caret ) - m_caret );
            MoveCaret( m_caret, false );
        }
        return true;
    case DENOFIZ_KEY_CODE_LEFT:
        MoveCaret( !extend && start != end ? start : PrevCodepoint( m_text, m_caret ), extend );
        return true;
    case DENOFIZ_KEY_CODE_RIGHT:
        MoveCaret( !extend && start != end ? end : NextCodepoint( m_text, m_caret ), extend );
        return true;
    case DENOFIZ_KEY_CODE_HOME:
        MoveCaret( 0, extend );
        return true;
    case DENOFIZ_KEY_CODE_END:
        MoveCaret( size, extend );
        return true;
    case DENOFIZ_KEY_CODE_A:
        if ( Shortcut( key.Mod ) )
        {
            m_anchor = 0;
            MoveCaret( size, true );
            return true;
        }
        return false;
    default:
        return false;
    }
}

void UIExample::Insert( const DenOfIz_StringView text )
{
    if ( text.Chars == nullptr || text.NumChars == 0 )
    {
        return;
    }
    DeleteSelection( );
    m_text.insert( m_caret, text.Chars, text.NumChars );
    std::replace_if( m_text.begin( ) + m_caret, m_text.begin( ) + m_caret + text.NumChars, []( const char c ) { return c == '\n' || c == '\r'; }, ' ' );
    MoveCaret( m_caret + static_cast<uint32_t>( text.NumChars ), false );
}

bool UIExample::DeleteSelection( )
{
    if ( m_caret == m_anchor )
    {
        return false;
    }
    const uint32_t start = std::min( m_caret, m_anchor );
    m_text.erase( start, std::max( m_caret, m_anchor ) - start );
    MoveCaret( start, false );
    return true;
}

void UIExample::MoveCaret( const uint32_t offset, const bool extend )
{
    m_caret = std::min( offset, static_cast<uint32_t>( m_text.size( ) ) );
    if ( !extend )
    {
        m_anchor = m_caret;
    }
    m_reveal     = true;
    m_blinkTimer = 0.0f;
}

uint32_t UIExample::Id( const char *name ) const
{
    return DenOfIz_UI_HashId( View( name ), 0 );
}

DenOfIz_UIInteraction UIExample::Interaction( const uint32_t id ) const
{
    DenOfIz_UIInteraction interaction{ };
    DenOfIz_UI_GetInteraction( m_ui, id, &interaction );
    return interaction;
}

bool UIExample::Activated( const uint32_t id, const DenOfIz_UIInteraction &interaction ) const
{
    return Has( interaction.Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) || ( m_keyActivate && m_focusId == id );
}

void UIExample::FocusRing( DenOfIz_UIElementDesc &desc ) const
{
    if ( m_focusVisible && desc.Id == m_focusId )
    {
        desc.Style.BorderColor = Theme::Dark;
    }
}

void UIExample::Open( const DenOfIz_UIElementDesc &desc ) const
{
    DenOfIz_UI_OpenElement( m_ui, &desc );
}

void UIExample::Close( ) const
{
    DenOfIz_UI_CloseElement( m_ui );
}

void UIExample::Label( const DenOfIz_StringView text, const float size, const DenOfIz_UIColor color ) const
{
    DenOfIz_UITextDesc desc{ };
    desc.Color    = color;
    desc.FontSize = size;
    DenOfIz_UI_Text( m_ui, text, &desc );
}

void UIExample::Label( const char *text, const float size, const DenOfIz_UIColor color ) const
{
    Label( View( text ), size, color );
}

DenOfIz_StringView UIExample::Format( const char *format, ... )
{
    va_list args;
    va_start( args, format );
    const int length = std::vsnprintf( m_format.data( ), m_format.size( ), format, args );
    va_end( args );
    return DenOfIz_StringView{ m_format.data( ), length < 0 ? 0 : std::min( static_cast<size_t>( length ), m_format.size( ) - 1 ) };
}

void UIExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    if ( m_textInput )
    {
        DenOfIz_InputSystem_StopTextInput( );
    }
    m_cube.reset( );
    DenOfIz_UI_Destroy( m_ui );
    m_ui = DENOFIZ_NULL_HANDLE;
    IExample::Quit( );
}
