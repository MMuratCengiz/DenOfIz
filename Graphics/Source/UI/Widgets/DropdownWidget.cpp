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

#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/Widgets/DropdownWidget.h>
#include <algorithm>

using namespace DenOfIz;

DropdownWidget::DropdownWidget( IClayContext *clayContext, uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style ) :
    Widget( clayContext, id ), m_options( options ), m_style( style ), m_dropdownListId( clayContext->HashString( InteropString( "dropdown_list" ), 0, id ) )
{
}

void DropdownWidget::Update( float deltaTime )
{
    UpdateHoverState( );
    m_dropdownListCreatedThisFrame = false;
}

void DropdownWidget::CreateLayoutElement( )
{
    m_style.Options = m_options;

    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.ItemHeight );
    decl.Layout.Padding       = m_style.Padding;
    decl.Custom.CustomData    = this;
    decl.BackgroundColor      = m_style.BackgroundColor;
    decl.Border.Color         = m_style.BorderColor;
    decl.Border.Width         = ClayBorderWidth( 1 );

    m_clayContext->OpenElement( decl );

    const InteropString &displayText = GetSelectedText( );
    const ClayColor     &textColor   = m_selectedIndex >= 0 ? m_style.TextColor : m_style.PlaceholderColor;

    if ( !displayText.IsEmpty( ) )
    {
        ClayTextDesc textDesc;
        textDesc.TextColor = textColor;
        textDesc.FontId    = m_style.FontId;
        textDesc.FontSize  = m_style.FontSize;

        m_clayContext->Text( displayText, textDesc );
    }

    m_clayContext->CloseElement( );
    if ( m_isOpen && !m_dropdownListCreatedThisFrame )
    {
        RenderDropdownList( );
        m_dropdownListCreatedThisFrame = true;
    }
}

void DropdownWidget::Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch )
{
    constexpr float arrowSize = 8.0f;
    const float     arrowX    = boundingBox.X + boundingBox.Width - m_style.Padding.Right - arrowSize;
    const float     arrowY    = boundingBox.Y + ( boundingBox.Height - arrowSize ) * 0.5f;

    ClayBoundingBox arrowBounds;
    arrowBounds.X      = arrowX;
    arrowBounds.Y      = arrowY;
    arrowBounds.Width  = arrowSize;
    arrowBounds.Height = arrowSize;

    AddRectangle( renderBatch, arrowBounds, m_style.TextColor );
}

void DropdownWidget::HandleEvent( const Event &event )
{
    m_selectionChanged = false;

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        if ( m_isHovered )
        {
            m_isOpen = !m_isOpen;
        }
        else if ( m_isOpen )
        {
            bool clickedOnItem = false;
            for ( size_t i = 0; i < m_options.NumElements( ); ++i )
            {
                uint32_t itemId = m_clayContext->HashString( "item", static_cast<uint32_t>( i ), m_dropdownListId );
                if ( m_clayContext->PointerOver( itemId ) )
                {
                    SetSelectedIndex( static_cast<int32_t>( i ) );
                    m_isOpen      = false;
                    clickedOnItem = true;
                    break;
                }
            }
            if ( !clickedOnItem && !m_clayContext->PointerOver( m_dropdownListId ) )
            {
                m_isOpen = false;
            }
        }
    }
    else if ( event.Type == EventType::MouseWheel && m_isOpen )
    {
        const auto dropdownBounds = m_clayContext->GetElementBoundingBox( m_dropdownListId );
        if ( event.Wheel.X >= dropdownBounds.X && event.Wheel.X <= dropdownBounds.X + dropdownBounds.Width && event.Wheel.Y >= dropdownBounds.Y &&
             event.Wheel.Y <= dropdownBounds.Y + dropdownBounds.Height )
        {
            const float totalHeight = m_options.NumElements( ) * m_style.ItemHeight;
            const float maxScroll   = std::max( 0.0f, totalHeight - m_style.MaxDropdownHeight );
            m_scrollOffset          = std::clamp( m_scrollOffset - event.Wheel.Y * 20.0f, 0.0f, maxScroll );
        }
    }
}
int32_t DropdownWidget::GetSelectedIndex( ) const
{
    return m_selectedIndex;
}

void DropdownWidget::SetSelectedIndex( const int32_t index )
{
    if ( index >= -1 && index < static_cast<int32_t>( m_options.NumElements( ) ) && index != m_selectedIndex )
    {
        m_selectedIndex    = index;
        m_selectionChanged = true;
    }
}

InteropString DropdownWidget::GetSelectedText( ) const
{
    return m_selectedIndex >= 0 ? m_options.GetElement( m_selectedIndex ) : m_style.PlaceholderText;
}

bool DropdownWidget::WasSelectionChanged( ) const
{
    return m_selectionChanged;
}

void DropdownWidget::ClearSelectionChangedEvent( )
{
    m_selectionChanged = false;
}

bool DropdownWidget::IsOpen( ) const
{
    return m_isOpen;
}

void DropdownWidget::SetOpen( const bool open )
{
    m_isOpen = open;
}

void DropdownWidget::SetOptions( const InteropArray<InteropString> &options )
{
    m_options = options;
    if ( m_selectedIndex >= static_cast<int32_t>( options.NumElements( ) ) )
    {
        m_selectedIndex    = -1;
        m_selectionChanged = true;
    }
}

const InteropArray<InteropString> &DropdownWidget::GetOptions( ) const
{
    return m_options;
}

void DropdownWidget::SetStyle( const DropdownStyle &style )
{
    m_style = style;
}

const DropdownStyle &DropdownWidget::GetStyle( ) const
{
    return m_style;
}

void DropdownWidget::RenderDropdownList( )
{
    if ( m_dropdownListCreatedThisFrame )
    {
        return;
    }
    m_dropdownListCreatedThisFrame = true;
    const auto parentBounds        = m_clayContext->GetElementBoundingBox( m_id );
    const auto viewportSize        = m_clayContext->GetViewportSize( );

    const float spaceBelow           = viewportSize.Height - ( parentBounds.Y + parentBounds.Height );
    const float actualDropdownHeight = m_options.NumElements( ) * m_style.ItemHeight;
    const float dropdownHeight       = std::min( m_style.MaxDropdownHeight, actualDropdownHeight );

    ClayFloatingDesc floatingDesc;
    floatingDesc.Offset   = Float_2{ -1, 0 };
    floatingDesc.Expand   = ClayDimensions{ 2, 0 };
    floatingDesc.ZIndex   = 1000; // High z-index to ensure dropdown renders above everything
    floatingDesc.ParentId = 0;
    floatingDesc.AttachTo = ClayFloatingAttachTo::Parent;

    if ( spaceBelow > 10 )
    {
        floatingDesc.ElementAttachPoint = ClayFloatingAttachPoint::LeftTop;
        floatingDesc.ParentAttachPoint  = ClayFloatingAttachPoint::LeftBottom;
    }
    else
    {
        floatingDesc.ElementAttachPoint = ClayFloatingAttachPoint::LeftBottom;
        floatingDesc.ParentAttachPoint  = ClayFloatingAttachPoint::LeftTop;
    }

    ClayScrollDesc scrollDesc;
    scrollDesc.Vertical = actualDropdownHeight > m_style.MaxDropdownHeight;

    ClayElementDeclaration listDecl;
    listDecl.Id                     = m_dropdownListId;
    listDecl.Floating               = floatingDesc;
    listDecl.Layout.Sizing.Width    = ClaySizingAxis::Fixed( parentBounds.Width ); // Use parent's exact width
    listDecl.Layout.Sizing.Height   = ClaySizingAxis::Fixed( dropdownHeight );
    listDecl.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    listDecl.BackgroundColor        = m_style.DropdownBgColor;
    listDecl.Border.Color           = m_style.BorderColor;
    listDecl.Border.Width           = ClayBorderWidth( 1 );
    listDecl.CornerRadius           = ClayCornerRadius( 4 );
    listDecl.Scroll                 = scrollDesc;

    m_clayContext->OpenElement( listDecl );
    for ( size_t i = 0; i < m_options.NumElements( ); ++i )
    {
        uint32_t itemId     = m_clayContext->HashString( "item", static_cast<uint32_t>( i ), m_dropdownListId );
        bool     isHovered  = m_clayContext->PointerOver( itemId );
        bool     isSelected = static_cast<int32_t>( i ) == m_selectedIndex;

        ClayColor itemColor = m_style.BackgroundColor;
        if ( isSelected )
        {
            itemColor = m_style.SelectedColor;
        }
        else if ( isHovered )
        {
            itemColor = m_style.HoverColor;
        }

        ClayElementDeclaration itemDecl;
        itemDecl.Id                   = itemId;
        itemDecl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
        itemDecl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.ItemHeight );
        itemDecl.Layout.Padding       = m_style.Padding;
        itemDecl.BackgroundColor      = itemColor;

        m_clayContext->OpenElement( itemDecl );

        ClayTextDesc textDesc;
        textDesc.TextColor = isSelected ? ClayColor( 255, 255, 255, 255 ) : m_style.TextColor;
        textDesc.FontId    = m_style.FontId;
        textDesc.FontSize  = m_style.FontSize;

        m_clayContext->Text( m_options.GetElement( i ), textDesc );
        m_clayContext->CloseElement( );
    }

    m_clayContext->CloseElement( );
}
