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
#include <DenOfIzGraphics/UI/Widgets/Widget.h>

using namespace DenOfIz;

Widget::Widget( Clay *clay, const uint32_t id ) : m_id( id ), m_clay( clay )
{
}

Widget::~Widget( )
{
}

uint32_t Widget::GetId( ) const
{
    return m_id;
}

bool Widget::IsHovered( ) const
{
    return m_isHovered;
}

bool Widget::IsFocused( ) const
{
    return m_isFocused;
}

void Widget::UpdateHoverState( const bool hovered )
{
    m_isHovered = hovered;
}

void Widget::UpdateHoverState( )
{
    m_isHovered = m_clay->PointerOver( m_id );
}

ClayBoundingBox Widget::GetBoundingBox( ) const
{
    return m_clay->GetElementBoundingBox( m_id );
}
