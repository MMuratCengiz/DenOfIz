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

#include <DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h>
#include <algorithm>

using namespace DenOfIz;

ResizableContainerWidget::ResizableContainerWidget( IClayContext *clayContext, const uint32_t id, const ResizableContainerStyle &style ) :
    IContainer( clayContext, id ), m_style( style )
{
    m_containerState.Width           = style.MinWidth + 100.0f;
    m_containerState.Height          = style.MinHeight + 50.0f;
    m_containerState.ResizeDirection = static_cast<uint8_t>( ResizeDirection::None );
}

void ResizableContainerWidget::Update( const float deltaTime )
{
    UpdateHoverState( );
    m_sizeChanged = false;
    m_contentOpen = false;
}

void ResizableContainerWidget::OpenElement( )
{
    if ( m_contentOpen )
    {
        return;
    }

    ClayElementDeclaration decl;
    decl.Id                     = m_id;
    decl.Layout.Sizing.Width    = ClaySizingAxis::Fixed( m_containerState.Width );
    decl.Layout.Sizing.Height   = ClaySizingAxis::Fixed( m_containerState.Height );
    decl.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;
    decl.Custom.CustomData      = this;
    decl.BackgroundColor        = m_style.BackgroundColor;
    decl.Border.Color           = m_style.BorderColor;
    decl.Border.Width           = ClayBorderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );
    decl.CornerRadius           = ClayCornerRadius( 4 );

    decl.Floating.AttachTo = ClayFloatingAttachTo::Root;
    decl.Floating.Offset   = m_position;
    decl.Floating.ZIndex   = 400.0f;

    m_clayContext->OpenElement( decl );

    if ( m_style.ShowTitleBar )
    {
        ClayElementDeclaration titleBarDecl;
        titleBarDecl.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
        titleBarDecl.Layout.Sizing.Height    = ClaySizingAxis::Fixed( m_style.TitleBarHeight );
        titleBarDecl.Layout.LayoutDirection  = ClayLayoutDirection::LeftToRight;
        titleBarDecl.Layout.Padding          = ClayPadding( 8 );
        titleBarDecl.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
        titleBarDecl.BackgroundColor         = m_style.TitleBarColor;

        m_clayContext->OpenElement( titleBarDecl );

        ClayTextDesc titleTextDesc;
        titleTextDesc.TextColor = m_style.TitleTextColor;
        titleTextDesc.FontId    = m_style.FontId;
        titleTextDesc.FontSize  = m_style.FontSize;
        m_clayContext->Text( m_style.Title, titleTextDesc );

        m_clayContext->CloseElement( );
    }

    ClayElementDeclaration contentDecl;
    contentDecl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    contentDecl.Layout.Sizing.Height = ClaySizingAxis::Grow( );
    contentDecl.Layout.Padding       = ClayPadding( 8 );
    contentDecl.BackgroundColor      = m_style.BackgroundColor;
    contentDecl.Border.Color         = m_style.BorderColor;
    contentDecl.Border.Width         = ClayBorderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );

    m_clayContext->OpenElement( contentDecl );
    m_contentOpen = true;
}

void ResizableContainerWidget::CloseElement( )
{
    if ( !m_contentOpen )
    {
        return;
    }

    m_clayContext->CloseElement( ); // Close content
    m_clayContext->CloseElement( ); // Close container
    m_contentOpen = false;
}

void ResizableContainerWidget::Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch )
{
    ClayBoundingBox containerBounds;
    containerBounds.X      = boundingBox.X;
    containerBounds.Y      = boundingBox.Y;
    containerBounds.Width  = boundingBox.Width;
    containerBounds.Height = boundingBox.Height;

    AddRectangle( renderBatch, containerBounds, m_style.BackgroundColor );

    ClayBorderWidth borderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );
    AddBorder( renderBatch, containerBounds, m_style.BorderColor, borderWidth );

    // TODO: Add resize handles rendering
}

void ResizableContainerWidget::HandleEvent( const Event &event )
{
    if ( !m_style.EnableResize )
    {
        return;
    }

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        const float mouseX = static_cast<float>( event.Button.X );
        const float mouseY = static_cast<float>( event.Button.Y );

        const ResizeDirection direction = GetResizeDirectionAtPoint( mouseX, mouseY );
        if ( direction != ResizeDirection::None )
        {
            m_containerState.IsResizing      = true;
            m_containerState.ResizeDirection = static_cast<uint8_t>( direction );
            m_containerState.ResizeStartPos  = Float_2{ mouseX, mouseY };
            m_containerState.InitialSize     = Float_2{ m_containerState.Width, m_containerState.Height };

            const auto bounds                = GetBoundingBox( );
            m_containerState.InitialPosition = Float_2{ bounds.X, bounds.Y };
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        if ( m_containerState.IsResizing )
        {
            m_containerState.IsResizing      = false;
            m_containerState.ResizeDirection = static_cast<uint8_t>( ResizeDirection::None );
        }
    }
    else if ( event.Type == EventType::MouseMotion && m_containerState.IsResizing )
    {
        UpdateResizing( static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) );
    }
}

void ResizableContainerWidget::SetSize( const float width, const float height )
{
    const float newWidth  = std::clamp( width, m_style.MinWidth, m_style.MaxWidth );
    const float newHeight = std::clamp( height, m_style.MinHeight, m_style.MaxHeight );

    if ( std::abs( m_containerState.Width - newWidth ) > 0.1f || std::abs( m_containerState.Height - newHeight ) > 0.1f )
    {
        m_containerState.Width  = newWidth;
        m_containerState.Height = newHeight;
        m_sizeChanged           = true;
    }
}

Float_2 ResizableContainerWidget::GetSize( ) const
{
    return Float_2{ m_containerState.Width, m_containerState.Height };
}

bool ResizableContainerWidget::WasSizeChanged( ) const
{
    return m_sizeChanged;
}

void ResizableContainerWidget::ClearSizeChangedEvent( )
{
    m_sizeChanged = false;
}

void ResizableContainerWidget::SetStyle( const ResizableContainerStyle &style )
{
    m_style = style;
}

const ResizableContainerStyle &ResizableContainerWidget::GetStyle( ) const
{
    return m_style;
}

ResizeDirection ResizableContainerWidget::GetResizeDirectionAtPoint( const float x, const float y ) const
{
    const auto  bounds     = GetBoundingBox( );
    const float handleSize = m_style.ResizeHandleSize;

    const bool nearLeft   = x >= bounds.X && x <= bounds.X + handleSize;
    const bool nearRight  = x >= bounds.X + bounds.Width - handleSize && x <= bounds.X + bounds.Width;
    const bool nearTop    = y >= bounds.Y && y <= bounds.Y + handleSize;
    const bool nearBottom = y >= bounds.Y + bounds.Height - handleSize && y <= bounds.Y + bounds.Height;
    if ( nearLeft && nearTop )
    {
        return ResizeDirection::NorthWest;
    }
    if ( nearRight && nearTop )
    {
        return ResizeDirection::NorthEast;
    }
    if ( nearLeft && nearBottom )
    {
        return ResizeDirection::SouthWest;
    }
    if ( nearRight && nearBottom )
    {
        return ResizeDirection::SouthEast;
    }
    if ( nearLeft )
    {
        return ResizeDirection::West;
    }
    if ( nearRight )
    {
        return ResizeDirection::East;
    }
    if ( nearTop )
    {
        return ResizeDirection::North;
    }
    if ( nearBottom )
    {
        return ResizeDirection::South;
    }
    return ResizeDirection::None;
}

void ResizableContainerWidget::UpdateResizing( const float mouseX, const float mouseY )
{
    const Float_2 delta = Float_2{ mouseX - m_containerState.ResizeStartPos.X, mouseY - m_containerState.ResizeStartPos.Y };

    float newWidth  = m_containerState.InitialSize.X;
    float newHeight = m_containerState.InitialSize.Y;

    switch ( static_cast<ResizeDirection>( m_containerState.ResizeDirection ) )
    {
    case ResizeDirection::East:
        newWidth += delta.X;
        break;
    case ResizeDirection::West:
        newWidth -= delta.X;
        break;
    case ResizeDirection::North:
        newHeight -= delta.Y;
        break;
    case ResizeDirection::South:
        newHeight += delta.Y;
        break;
    case ResizeDirection::NorthEast:
        newWidth += delta.X;
        newHeight -= delta.Y;
        break;
    case ResizeDirection::NorthWest:
        newWidth -= delta.X;
        newHeight -= delta.Y;
        break;
    case ResizeDirection::SouthEast:
        newWidth += delta.X;
        newHeight += delta.Y;
        break;
    case ResizeDirection::SouthWest:
        newWidth -= delta.X;
        newHeight += delta.Y;
        break;
    default:
        break;
    }

    SetSize( newWidth, newHeight );
}

bool ResizableContainerWidget::IsPointInResizeHandle( const float x, const float y, const ResizeDirection direction ) const
{
    const ClayBoundingBox handleBounds = GetResizeHandleBounds( direction );
    return x >= handleBounds.X && x <= handleBounds.X + handleBounds.Width && y >= handleBounds.Y && y <= handleBounds.Y + handleBounds.Height;
}

ClayBoundingBox ResizableContainerWidget::GetResizeHandleBounds( const ResizeDirection direction ) const
{
    const auto  bounds     = GetBoundingBox( );
    const float handleSize = m_style.ResizeHandleSize;

    ClayBoundingBox handleBounds;

    switch ( direction )
    {
    case ResizeDirection::North:
        handleBounds = { bounds.X, bounds.Y, bounds.Width, handleSize };
        break;
    case ResizeDirection::South:
        handleBounds = { bounds.X, bounds.Y + bounds.Height - handleSize, bounds.Width, handleSize };
        break;
    case ResizeDirection::East:
        handleBounds = { bounds.X + bounds.Width - handleSize, bounds.Y, handleSize, bounds.Height };
        break;
    case ResizeDirection::West:
        handleBounds = { bounds.X, bounds.Y, handleSize, bounds.Height };
        break;
    case ResizeDirection::NorthEast:
        handleBounds = { bounds.X + bounds.Width - handleSize, bounds.Y, handleSize, handleSize };
        break;
    case ResizeDirection::NorthWest:
        handleBounds = { bounds.X, bounds.Y, handleSize, handleSize };
        break;
    case ResizeDirection::SouthEast:
        handleBounds = { bounds.X + bounds.Width - handleSize, bounds.Y + bounds.Height - handleSize, handleSize, handleSize };
        break;
    case ResizeDirection::SouthWest:
        handleBounds = { bounds.X, bounds.Y + bounds.Height - handleSize, handleSize, handleSize };
        break;
    default:
        handleBounds = { 0, 0, 0, 0 };
        break;
    }

    return handleBounds;
}

void ResizableContainerWidget::SetPosition( const Float_2 &position )
{
    m_position = position;
}

Float_2 ResizableContainerWidget::GetPosition( ) const
{
    return m_position;
}
