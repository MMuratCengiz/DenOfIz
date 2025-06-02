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

#pragma once

#include <memory>
#include <vector>
#include "ResizableContainerWidget.h"

namespace DenOfIz
{
    class ClayContext;
    enum class DockingSide : uint8_t
    {
        None,
        Left,
        Right,
        Top,
        Bottom,
        Center
    };

    enum class DockingMode : uint8_t
    {
        Floating,
        Docked,
        Tabbed
    };

    struct DockZone
    {
        DockingSide     Side;
        ClayBoundingBox Bounds;
        bool            IsHighlighted = false;
    };

    using DockableContainerStyle = ClayDockableContainerDesc;

    class DockingManager;

    class DockableContainerWidget : public Widget
    {
        ClayDockableContainerState                m_containerState;
        DockableContainerStyle                    m_style;
        DockingManager                           *m_dockingManager = nullptr;
        std::unique_ptr<ResizableContainerWidget> m_resizableContainer;
        bool                                      m_isClosed    = false;
        bool                                      m_contentOpen = false;

    public:
        DZ_API DockableContainerWidget( IClayContext *clayContext, uint32_t id, DockingManager *dockingManager );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API void CreateLayoutElement( ) override;
        DZ_API void OpenElement( const DockableContainerStyle &style );
        DZ_API void CloseElement( );

        DZ_API const DockableContainerStyle &GetStyle( ) const;

        DZ_API void        SetDockingMode( DockingMode mode );
        DZ_API DockingMode GetDockingMode( ) const;
        DZ_API void        SetDockedSide( DockingSide side );
        DZ_API DockingSide GetDockedSide( ) const;
        DZ_API void        SetFloatingPosition( Float_2 position );
        DZ_API Float_2     GetFloatingPosition( ) const;
        DZ_API void        SetFloatingSize( Float_2 size );
        DZ_API Float_2     GetFloatingSize( ) const;
        DZ_API bool        IsClosed( ) const;
        DZ_API void        Close( );
        DZ_API void        Show( );

    private:
        void            HandleTitleBarDrag( const Event &event );
        void            UpdateDockZones( float mouseX, float mouseY );
        bool            IsPointInTitleBar( float x, float y ) const;
        bool            IsPointInCloseButton( float x, float y ) const;
        ClayBoundingBox GetTitleBarBounds( ) const;
        ClayBoundingBox GetCloseButtonBounds( ) const;
    };

    class DockingManager
    {
        std::vector<DockableContainerWidget *> m_containers;
        std::vector<DockZone>                  m_dockZones;
        DockableContainerWidget               *m_draggingContainer = nullptr;
        ClayContext                           *m_clayContext;

    public:
        DZ_API DockingManager( ClayContext *clayContext );
        DZ_API ~DockingManager( );

        DZ_API void RegisterContainer( DockableContainerWidget *container );
        DZ_API void UnregisterContainer( DockableContainerWidget *container );
        DZ_API void Update( float deltaTime ) const;
        DZ_API void Render( );

        DZ_API void        StartDragging( DockableContainerWidget *container );
        DZ_API void        StopDragging( );
        DZ_API void        UpdateDraggedContainer( Float_2 mousePos );
        DZ_API DockingSide GetHoveredDockZone( Float_2 mousePos ) const;
        DZ_API void        DockContainer( DockableContainerWidget *container, DockingSide side );
        DZ_API void        UndockContainer( DockableContainerWidget *container );

    private:
        void            UpdateDockZones( Float_2 mousePos );
        void            RenderDockZones( ) const;
        ClayBoundingBox GetDockZoneBounds( DockingSide side ) const;
    };

} // namespace DenOfIz
