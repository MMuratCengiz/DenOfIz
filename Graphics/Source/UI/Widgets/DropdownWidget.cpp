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

#include <DenOfIzGraphics/UI/Clay.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/Widgets/DropdownWidget.h>
#include <algorithm>

using namespace DenOfIz;

DropdownWidget::DropdownWidget( Clay *clay, uint32_t id, const InteropArray<InteropString> &options, const DropdownStyle &style ) :
    Widget( clay, id ), m_options( options ), m_style( style ), m_dropdownListId( clay->HashString( InteropString( "dropdown_list" ), 0, id ) )
{
    m_dropdownState.SelectedIndex = -1;
    m_dropdownState.IsOpen        = false;
    m_dropdownState.ScrollOffset  = 0.0f;

    m_widgetData.Type = ClayCustomWidgetType::Dropdown;
    m_widgetData.Data = &m_renderData;
}

void DropdownWidget::Update( float deltaTime )
{
    UpdateHoverState( );
}

void DropdownWidget::CreateLayoutElement( )
{
    m_dropdownState.SelectedIndex = m_selectedIndex;
    m_dropdownState.IsOpen        = m_isOpen;
    m_dropdownState.ScrollOffset  = m_scrollOffset;

    m_style.Options = m_options;

    m_renderData.State     = &m_dropdownState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;

    ClayElementDeclaration decl;
    decl.Id                   = m_id;
    decl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_style.ItemHeight );
    decl.Custom.CustomData    = &m_widgetData;

    m_clay->OpenElement( decl );
    m_clay->CloseElement( );
}

void DropdownWidget::Render( )
{
    m_dropdownState.SelectedIndex = m_selectedIndex;
    m_dropdownState.IsOpen        = m_isOpen;
    m_dropdownState.ScrollOffset  = m_scrollOffset;

    m_renderData.State     = &m_dropdownState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;
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
            const auto dropdownBounds = m_clay->GetElementBoundingBox( m_dropdownListId );
            if ( event.Button.X >= dropdownBounds.X && event.Button.X <= dropdownBounds.X + dropdownBounds.Width && event.Button.Y >= dropdownBounds.Y &&
                 event.Button.Y <= dropdownBounds.Y + dropdownBounds.Height )
            {
                const float   relativeY    = event.Button.Y - dropdownBounds.Y + m_scrollOffset;
                const int32_t clickedIndex = static_cast<int32_t>( relativeY / m_style.ItemHeight );

                if ( clickedIndex >= 0 && clickedIndex < static_cast<int32_t>( m_options.NumElements( ) ) )
                {
                    SetSelectedIndex( clickedIndex );
                    m_isOpen = false;
                }
            }
            else
            {
                m_isOpen = false;
            }
        }
    }
    else if ( event.Type == EventType::MouseWheel && m_dropdownState.IsOpen )
    {
        const auto dropdownBounds = m_clay->GetElementBoundingBox( m_dropdownListId );
        if ( event.Wheel.X >= dropdownBounds.X && event.Wheel.X <= dropdownBounds.X + dropdownBounds.Width && event.Wheel.Y >= dropdownBounds.Y &&
             event.Wheel.Y <= dropdownBounds.Y + dropdownBounds.Height )
        {
            const float totalHeight      = m_options.NumElements( ) * m_style.ItemHeight;
            const float maxScroll        = std::max( 0.0f, totalHeight - m_style.MaxDropdownHeight );
            m_scrollOffset               = std::clamp( m_scrollOffset - event.Wheel.Y * 20.0f, 0.0f, maxScroll );
            m_dropdownState.ScrollOffset = m_scrollOffset;
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
        m_selectedIndex               = index;
        m_dropdownState.SelectedIndex = index;
        if ( index >= 0 )
        {
            m_dropdownState.SelectedText = m_options.GetElement( index );
        }
        else
        {
            m_dropdownState.SelectedText = m_style.PlaceholderText;
        }
        m_selectionChanged = true;
    }
}

InteropString DropdownWidget::GetSelectedText( ) const
{
    return m_dropdownState.SelectedText;
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
        m_selectedIndex               = -1;
        m_dropdownState.SelectedIndex = -1;
        m_dropdownState.SelectedText  = m_style.PlaceholderText;
        m_selectionChanged            = true;
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
    const auto parentBounds = m_clay->GetElementBoundingBox( m_id );
    const auto viewportSize = m_clay->GetViewportSize( );

    const float spaceBelow     = viewportSize.Height - ( parentBounds.Y + parentBounds.Height );
    const float spaceAbove     = parentBounds.Y;
    const float dropdownHeight = std::min( m_style.MaxDropdownHeight, m_options.NumElements( ) * m_style.ItemHeight );

    ClayFloatingDesc floatingDesc;
    floatingDesc.Offset   = Float_2{ 0, 0 };
    floatingDesc.Expand   = ClayDimensions{ 1, 0 };
    floatingDesc.ZIndex   = 1000;
    floatingDesc.ParentId = m_id;

    if ( spaceBelow >= dropdownHeight || spaceBelow >= spaceAbove )
    {
        floatingDesc.ElementAttachPoint = ClayFloatingAttachPoint::LeftTop;
        floatingDesc.ParentAttachPoint  = ClayFloatingAttachPoint::LeftBottom;
    }
    else
    {
        floatingDesc.ElementAttachPoint = ClayFloatingAttachPoint::LeftBottom;
        floatingDesc.ParentAttachPoint  = ClayFloatingAttachPoint::LeftTop;
    }

    // Create scroll config
    ClayScrollDesc scrollDesc;
    scrollDesc.Vertical = true;

    ClayElementDeclaration listDecl;
    listDecl.Id                     = m_dropdownListId;
    listDecl.Floating               = floatingDesc;
    listDecl.Layout.Sizing.Width    = ClaySizingAxis::Fixed( 0 );
    listDecl.Layout.Sizing.Height   = ClaySizingAxis::Fixed( m_style.MaxDropdownHeight );
    listDecl.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    listDecl.BackgroundColor        = m_style.DropdownBgColor;
    listDecl.Border.Color           = m_style.BorderColor;
    listDecl.Border.Width           = ClayBorderWidth( 1 );
    listDecl.CornerRadius           = ClayCornerRadius( 4 );
    listDecl.Scroll                 = scrollDesc;

    m_clay->OpenElement( listDecl );

    for ( size_t i = 0; i < m_options.NumElements( ); ++i )
    {
        uint32_t itemId     = m_clay->HashString( "item", static_cast<uint32_t>( i ), m_dropdownListId );
        bool     isHovered  = m_clay->PointerOver( itemId );
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

        m_clay->OpenElement( itemDecl );

        ClayTextDesc textDesc;
        textDesc.TextColor = isSelected ? ClayColor( 255, 255, 255, 255 ) : m_style.TextColor;
        textDesc.FontId    = m_style.FontId;
        textDesc.FontSize  = m_style.FontSize;

        m_clay->Text( m_options.GetElement( i ), textDesc );
        m_clay->CloseElement( );
    }

    m_clay->CloseElement( );
}
