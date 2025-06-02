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

#include "Widget.h"

namespace DenOfIz
{
    enum class ResizeDirection : uint8_t
    {
        None,
        North,
        South,
        East,
        West,
        NorthEast,
        NorthWest,
        SouthEast,
        SouthWest
    };

    using ResizableContainerStyle = ClayResizableContainerDesc;

    class ResizableContainerWidget : public Widget
    {
        ClayResizableContainerState m_containerState;
        ResizableContainerStyle     m_style;
        bool                        m_sizeChanged = false;
        bool                        m_contentOpen = false;
        Float_2                     m_position = { 600.0f, 100.0f }; // Default position

    public:
        DZ_API ResizableContainerWidget( IClayContext *clayContext, uint32_t id );

        DZ_API void Update( float deltaTime ) override;
        DZ_API void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) override;
        DZ_API void HandleEvent( const Event &event ) override;

        DZ_API void CreateLayoutElement() override;
        DZ_API void OpenElement( const ResizableContainerStyle &style );
        DZ_API void CloseElement( );

        DZ_API void    SetSize( float width, float height );
        DZ_API Float_2 GetSize( ) const;
        DZ_API bool    WasSizeChanged( ) const;
        DZ_API void    ClearSizeChangedEvent( );

        DZ_API const ResizableContainerStyle &GetStyle( ) const;
        
        DZ_API void    SetPosition( const Float_2 &position );
        DZ_API Float_2 GetPosition( ) const;

    private:
        ResizeDirection GetResizeDirectionAtPoint( float x, float y ) const;
        void            UpdateResizing( float mouseX, float mouseY );
        bool            IsPointInResizeHandle( float x, float y, ResizeDirection direction ) const;
        ClayBoundingBox GetResizeHandleBounds( ResizeDirection direction ) const;
    };

} // namespace DenOfIz
