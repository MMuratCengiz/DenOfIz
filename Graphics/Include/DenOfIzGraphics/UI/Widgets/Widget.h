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

#include <DenOfIzGraphics/Input/Event.h>
#include <DenOfIzGraphics/UI/ClayData.h>

namespace DenOfIz
{
    class Clay;

    class Widget
    {
    protected:
        uint32_t m_id;
        Clay    *m_clay;
        bool     m_isHovered = false;
        bool     m_isFocused = false;

    public:
        DZ_API Widget( Clay *clay, uint32_t id );
        DZ_API virtual ~Widget( );

        DZ_API virtual void Update( float deltaTime )         = 0;
        DZ_API virtual void CreateLayoutElement( )            = 0;
        DZ_API virtual void Render( )                         = 0;
        DZ_API virtual void HandleEvent( const Event &event ) = 0;

        DZ_API uint32_t GetId( ) const;
        DZ_API bool     IsHovered( ) const;
        DZ_API bool     IsFocused( ) const;

        DZ_API void UpdateHoverState( bool hovered );
        DZ_API void UpdateHoverState( );

        DZ_API ClayBoundingBox GetBoundingBox( ) const;
    };

    struct WidgetRenderData
    {
        ClayCustomWidgetType Type;
        Widget              *WidgetPtr;
    };

} // namespace DenOfIz
