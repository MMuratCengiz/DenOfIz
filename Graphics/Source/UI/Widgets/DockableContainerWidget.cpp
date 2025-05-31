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
#include <DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;

DockableContainerWidget::DockableContainerWidget( Clay *clay, const uint32_t id, DockingManager *dockingManager, const DockableContainerStyle &style ) :
    Widget( clay, id ), m_style( style ), m_dockingManager( dockingManager )
{
    m_containerState.FloatingSize = Float_2{ 300.0f, 200.0f };
    m_containerState.Mode         = static_cast<uint8_t>( DockingMode::Floating );
    m_containerState.DockedSide   = static_cast<uint8_t>( DockingSide::None );

    m_widgetData.Type = ClayCustomWidgetType::DockableContainer;
    m_widgetData.Data = &m_renderData;

    if ( m_dockingManager )
    {
        m_dockingManager->RegisterContainer( this );
    }

    if ( m_style.AllowResize )
    {
        ResizableContainerStyle resizableStyle;
        resizableStyle.MinWidth     = m_style.MinWidth;
        resizableStyle.MinHeight    = m_style.MinHeight;
        resizableStyle.ShowTitleBar = false; // We'll handle our own title bar
        resizableStyle.EnableResize = true;

        m_resizableContainer = std::make_unique<ResizableContainerWidget>( clay, id + 1, resizableStyle );
    }
}

void DockableContainerWidget::Update( const float deltaTime )
{
    UpdateHoverState( );

    if ( m_resizableContainer && static_cast<DockingMode>( m_containerState.Mode ) == DockingMode::Floating )
    {
        m_resizableContainer->Update( deltaTime );

        if ( m_resizableContainer->WasSizeChanged( ) )
        {
            const auto newSize            = m_resizableContainer->GetSize( );
            m_containerState.FloatingSize = newSize;
            m_resizableContainer->ClearSizeChangedEvent( );
        }
    }
}

void DockableContainerWidget::CreateLayoutElement( )
{
    if ( m_isClosed )
    {
        return;
    }

    m_renderData.State     = &m_containerState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;

    ClayElementDeclaration decl;
    decl.Id                = m_id;
    decl.Custom.CustomData = &m_widgetData;

    if ( static_cast<DockingMode>( m_containerState.Mode ) == DockingMode::Floating )
    {
        // Floating mode - use fixed positioning
        decl.Layout.Sizing.Width  = ClaySizingAxis::Fixed( m_containerState.FloatingSize.X );
        decl.Layout.Sizing.Height = ClaySizingAxis::Fixed( m_containerState.FloatingSize.Y );
        decl.Floating.AttachTo    = ClayFloatingAttachTo::Root;
        decl.Floating.Offset      = m_containerState.FloatingPosition;
        decl.Floating.ZIndex      = 100.0f;
    }
    else
    {
        // Docked mode - size based on parent
        decl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
        decl.Layout.Sizing.Height = ClaySizingAxis::Grow( );
    }

    decl.Layout.LayoutDirection = ClayLayoutDirection::TopToBottom;

    m_clay->OpenElement( decl );

    ClayElementDeclaration titleBarDecl;
    titleBarDecl.Layout.Sizing.Width     = ClaySizingAxis::Grow( );
    titleBarDecl.Layout.Sizing.Height    = ClaySizingAxis::Fixed( m_style.TitleBarHeight );
    titleBarDecl.Layout.LayoutDirection  = ClayLayoutDirection::LeftToRight;
    titleBarDecl.Layout.Padding          = ClayPadding( 8 );
    titleBarDecl.Layout.ChildAlignment.Y = ClayAlignmentY::Center;
    titleBarDecl.BackgroundColor         = m_style.TitleBarColor;
    titleBarDecl.Border.Color            = m_style.BorderColor;
    titleBarDecl.Border.Width            = ClayBorderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );

    m_clay->OpenElement( titleBarDecl );

    ClayTextDesc titleTextDesc;
    titleTextDesc.TextColor = m_style.TitleTextColor;
    titleTextDesc.FontId    = m_style.FontId;
    titleTextDesc.FontSize  = m_style.FontSize;
    m_clay->Text( m_style.Title, titleTextDesc );

    ClayElementDeclaration spacerDecl;
    spacerDecl.Layout.Sizing.Width = ClaySizingAxis::Grow( );
    m_clay->OpenElement( spacerDecl );
    m_clay->CloseElement( );

    if ( m_style.ShowCloseButton )
    {
        ClayElementDeclaration closeButtonDecl;
        closeButtonDecl.Layout.Sizing.Width     = ClaySizingAxis::Fixed( 20.0f );
        closeButtonDecl.Layout.Sizing.Height    = ClaySizingAxis::Fixed( 20.0f );
        closeButtonDecl.BackgroundColor         = ClayColor( 220, 220, 220, 255 );
        closeButtonDecl.Layout.ChildAlignment.X = ClayAlignmentX::Center;
        closeButtonDecl.Layout.ChildAlignment.Y = ClayAlignmentY::Center;

        m_clay->OpenElement( closeButtonDecl );

        ClayTextDesc closeTextDesc;
        closeTextDesc.TextColor = ClayColor( 0, 0, 0, 255 );
        closeTextDesc.FontSize  = 12;
        m_clay->Text( InteropString( "×" ), closeTextDesc );

        m_clay->CloseElement( );
    }

    m_clay->CloseElement( );

    ClayElementDeclaration contentDecl;
    contentDecl.Layout.Sizing.Width  = ClaySizingAxis::Grow( );
    contentDecl.Layout.Sizing.Height = ClaySizingAxis::Grow( );
    contentDecl.Layout.Padding       = ClayPadding( 8 );
    contentDecl.BackgroundColor      = m_style.BackgroundColor;
    contentDecl.Border.Color         = m_style.BorderColor;
    contentDecl.Border.Width         = ClayBorderWidth( static_cast<uint16_t>( m_style.BorderWidth ) );

    m_clay->OpenElement( contentDecl );
    if ( m_contentRenderer )
    {
        m_contentRenderer( );
    }

    m_clay->CloseElement( );
    m_clay->CloseElement( );
}

void DockableContainerWidget::Render( )
{
    if ( m_isClosed )
    {
        return;
    }

    m_renderData.State     = &m_containerState;
    m_renderData.Desc      = m_style;
    m_renderData.ElementId = m_id;
}

void DockableContainerWidget::HandleEvent( const Event &event )
{
    if ( m_isClosed )
    {
        return;
    }

    if ( event.Type == EventType::MouseButtonDown && event.Button.Button == MouseButton::Left )
    {
        const float mouseX = static_cast<float>( event.Button.X );
        const float mouseY = static_cast<float>( event.Button.Y );
        if ( m_style.ShowCloseButton && IsPointInCloseButton( mouseX, mouseY ) )
        {
            Close( );
            return;
        }

        // Check title bar for dragging
        if ( IsPointInTitleBar( mouseX, mouseY ) && m_style.AllowUndock )
        {
            m_containerState.IsDragging   = true;
            m_containerState.DragStartPos = Float_2{ mouseX, mouseY };

            const auto bounds           = GetBoundingBox( );
            m_containerState.DragOffset = Float_2{ mouseX - bounds.X, mouseY - bounds.Y };

            if ( m_dockingManager )
            {
                m_dockingManager->StartDragging( this );
            }
        }
    }
    else if ( event.Type == EventType::MouseButtonUp && event.Button.Button == MouseButton::Left )
    {
        if ( m_containerState.IsDragging )
        {
            m_containerState.IsDragging = false;

            if ( m_dockingManager )
            {
                const Float_2     mousePos{ static_cast<float>( event.Button.X ), static_cast<float>( event.Button.Y ) };
                const DockingSide hoveredZone = m_dockingManager->GetHoveredDockZone( mousePos );

                if ( hoveredZone != DockingSide::None )
                {
                    m_dockingManager->DockContainer( this, hoveredZone );
                }
                else if ( static_cast<DockingMode>( m_containerState.Mode ) == DockingMode::Docked )
                {
                    m_dockingManager->UndockContainer( this );
                }

                m_dockingManager->StopDragging( );
            }
        }
    }
    else if ( event.Type == EventType::MouseMotion && m_containerState.IsDragging )
    {
        const Float_2 mousePos{ static_cast<float>( event.Motion.X ), static_cast<float>( event.Motion.Y ) };
        m_containerState.FloatingPosition = Float_2{ mousePos.X - m_containerState.DragOffset.X, mousePos.Y - m_containerState.DragOffset.Y };

        if ( m_dockingManager )
        {
            m_dockingManager->UpdateDraggedContainer( mousePos );
        }
    }

    if ( m_resizableContainer && static_cast<DockingMode>( m_containerState.Mode ) == DockingMode::Floating )
    {
        m_resizableContainer->HandleEvent( event );
    }
}

void DockableContainerWidget::SetContentRenderer( std::function<void( )> renderer )
{
    m_contentRenderer = std::move( renderer );
}

void DockableContainerWidget::SetStyle( const DockableContainerStyle &style )
{
    m_style = style;
}

const DockableContainerStyle &DockableContainerWidget::GetStyle( ) const
{
    return m_style;
}

void DockableContainerWidget::SetDockingMode( const DockingMode mode )
{
    m_containerState.Mode = static_cast<uint8_t>( mode );
}

DockingMode DockableContainerWidget::GetDockingMode( ) const
{
    return static_cast<DockingMode>( m_containerState.Mode );
}

void DockableContainerWidget::SetDockedSide( const DockingSide side )
{
    m_containerState.DockedSide = static_cast<uint8_t>( side );
    if ( side != DockingSide::None )
    {
        m_containerState.Mode = static_cast<uint8_t>( DockingMode::Docked );
    }
}

DockingSide DockableContainerWidget::GetDockedSide( ) const
{
    return static_cast<DockingSide>( m_containerState.DockedSide );
}

void DockableContainerWidget::SetFloatingPosition( const Float_2 position )
{
    m_containerState.FloatingPosition = position;
}

Float_2 DockableContainerWidget::GetFloatingPosition( ) const
{
    return m_containerState.FloatingPosition;
}

void DockableContainerWidget::SetFloatingSize( const Float_2 size )
{
    m_containerState.FloatingSize = size;
}

Float_2 DockableContainerWidget::GetFloatingSize( ) const
{
    return m_containerState.FloatingSize;
}

bool DockableContainerWidget::IsClosed( ) const
{
    return m_isClosed;
}

void DockableContainerWidget::Close( )
{
    m_isClosed = true;
}

void DockableContainerWidget::Show( )
{
    m_isClosed = false;
}

void DockableContainerWidget::HandleTitleBarDrag( const Event &event )
{
    // TODO Implementation handled in main HandleEvent method
}

void DockableContainerWidget::UpdateDockZones( const float mouseX, const float mouseY )
{
    // Implementation handled by DockingManager
}

bool DockableContainerWidget::IsPointInTitleBar( const float x, const float y ) const
{
    const ClayBoundingBox titleBarBounds = GetTitleBarBounds( );
    return x >= titleBarBounds.X && x <= titleBarBounds.X + titleBarBounds.Width && y >= titleBarBounds.Y && y <= titleBarBounds.Y + titleBarBounds.Height;
}

bool DockableContainerWidget::IsPointInCloseButton( const float x, const float y ) const
{
    const ClayBoundingBox closeButtonBounds = GetCloseButtonBounds( );
    return x >= closeButtonBounds.X && x <= closeButtonBounds.X + closeButtonBounds.Width && y >= closeButtonBounds.Y && y <= closeButtonBounds.Y + closeButtonBounds.Height;
}

ClayBoundingBox DockableContainerWidget::GetTitleBarBounds( ) const
{
    const auto bounds = GetBoundingBox( );
    return ClayBoundingBox{ bounds.X, bounds.Y, bounds.Width, m_style.TitleBarHeight };
}

ClayBoundingBox DockableContainerWidget::GetCloseButtonBounds( ) const
{
    const auto bounds = GetBoundingBox( );
    return ClayBoundingBox{ bounds.X + bounds.Width - 28.0f, // 20px button + 8px padding
                            bounds.Y + ( m_style.TitleBarHeight - 20.0f ) * 0.5f, 20.0f, 20.0f };
}

DockingManager::DockingManager( Clay *clay ) : m_clay( clay )
{
}

DockingManager::~DockingManager( )
{
}

void DockingManager::RegisterContainer( DockableContainerWidget *container )
{
    if ( std::find( m_containers.begin( ), m_containers.end( ), container ) == m_containers.end( ) )
    {
        m_containers.push_back( container );
    }
}

void DockingManager::UnregisterContainer( DockableContainerWidget *container )
{
    m_containers.erase( std::remove( m_containers.begin( ), m_containers.end( ), container ), m_containers.end( ) );
}

void DockingManager::Update( const float deltaTime ) const
{
    // Update all containers
    for ( auto *container : m_containers )
    {
        if ( container )
        {
            container->Update( deltaTime );
        }
    }
}

void DockingManager::Render( )
{
    // Render dock zones if dragging
    if ( m_draggingContainer )
    {
        RenderDockZones( );
    }
}

void DockingManager::StartDragging( DockableContainerWidget *container )
{
    m_draggingContainer = container;
}

void DockingManager::StopDragging( )
{
    m_draggingContainer = nullptr;
    m_dockZones.clear( );
}

void DockingManager::UpdateDraggedContainer( const Float_2 mousePos )
{
    if ( m_draggingContainer )
    {
        UpdateDockZones( mousePos );
    }
}

DockingSide DockingManager::GetHoveredDockZone( const Float_2 mousePos ) const
{
    for ( const auto &zone : m_dockZones )
    {
        if ( mousePos.X >= zone.Bounds.X && mousePos.X <= zone.Bounds.X + zone.Bounds.Width && mousePos.Y >= zone.Bounds.Y && mousePos.Y <= zone.Bounds.Y + zone.Bounds.Height )
        {
            return zone.Side;
        }
    }
    return DockingSide::None;
}

void DockingManager::DockContainer( DockableContainerWidget *container, const DockingSide side )
{
    if ( container )
    {
        container->SetDockingMode( DockingMode::Docked );
        container->SetDockedSide( side );
    }
}

void DockingManager::UndockContainer( DockableContainerWidget *container )
{
    if ( container )
    {
        container->SetDockingMode( DockingMode::Floating );
        container->SetDockedSide( DockingSide::None );
    }
}

void DockingManager::UpdateDockZones( const Float_2 mousePos )
{
    m_dockZones.clear( );

    // Create dock zones for viewport edges
    const auto      viewport = m_clay->GetViewportSize( );
    constexpr float zoneSize = 100.0f;

    // Left zone
    m_dockZones.push_back( DockZone{ DockingSide::Left, ClayBoundingBox{ 0, 0, zoneSize, viewport.Height } } );

    // Right zone
    m_dockZones.push_back( DockZone{ DockingSide::Right, ClayBoundingBox{ viewport.Width - zoneSize, 0, zoneSize, viewport.Height } } );

    // Top zone
    m_dockZones.push_back( DockZone{ DockingSide::Top, ClayBoundingBox{ 0, 0, viewport.Width, zoneSize } } );

    // Bottom zone
    m_dockZones.push_back( DockZone{ DockingSide::Bottom, ClayBoundingBox{ 0, viewport.Height - zoneSize, viewport.Width, zoneSize } } );

    // Update hover state
    for ( auto &zone : m_dockZones )
    {
        zone.IsHighlighted =
            ( mousePos.X >= zone.Bounds.X && mousePos.X <= zone.Bounds.X + zone.Bounds.Width && mousePos.Y >= zone.Bounds.Y && mousePos.Y <= zone.Bounds.Y + zone.Bounds.Height );
    }
}

void DockingManager::RenderDockZones( ) const
{
    for ( const auto &zone : m_dockZones )
    {
        if ( zone.IsHighlighted )
        {
            ClayElementDeclaration zoneDecl;
            zoneDecl.Layout.Sizing.Width  = ClaySizingAxis::Fixed( zone.Bounds.Width );
            zoneDecl.Layout.Sizing.Height = ClaySizingAxis::Fixed( zone.Bounds.Height );
            zoneDecl.Floating.AttachTo    = ClayFloatingAttachTo::Root;
            zoneDecl.Floating.Offset      = Float_2{ zone.Bounds.X, zone.Bounds.Y };
            zoneDecl.Floating.ZIndex      = 200.0f;
            zoneDecl.BackgroundColor      = ClayColor( 0, 120, 215, 100 );

            m_clay->OpenElement( zoneDecl );
            m_clay->CloseElement( );
        }
    }
}

ClayBoundingBox DockingManager::GetDockZoneBounds( const DockingSide side ) const
{
    const auto      viewport = m_clay->GetViewportSize( );
    constexpr float zoneSize = 100.0f;

    switch ( side )
    {
    case DockingSide::Left:
        return ClayBoundingBox{ 0, 0, zoneSize, viewport.Height };
    case DockingSide::Right:
        return ClayBoundingBox{ viewport.Width - zoneSize, 0, zoneSize, viewport.Height };
    case DockingSide::Top:
        return ClayBoundingBox{ 0, 0, viewport.Width, zoneSize };
    case DockingSide::Bottom:
        return ClayBoundingBox{ 0, viewport.Height - zoneSize, viewport.Width, zoneSize };
    default:
        return ClayBoundingBox{ 0, 0, 0, 0 };
    }
}
