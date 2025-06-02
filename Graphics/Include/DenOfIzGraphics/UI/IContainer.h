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

#include <DenOfIzGraphics/UI/Widgets/Widget.h>

namespace DenOfIz
{
    /// Container widgets that can hold other widgets or Clay elements.
    ///
    /// Container widgets work like Clay's OpenElement/CloseElement pattern:
    /// - Call OpenContent() to begin adding content
    /// - Add widgets or Clay elements
    /// - Call CloseContent() to finish
    ///
    class DZ_API IContainer : public Widget
    {
    public:
        IContainer( IClayContext *clayContext, uint32_t id ) : Widget( clayContext, id )
        {
        }
        virtual ~IContainer( ) = default;
        virtual void OpenContent( )  = 0;
        virtual void CloseContent( ) = 0;
        void         CreateLayoutElement( ) override
        {
            OpenContent( );
            CloseContent( );
        }
    };

} // namespace DenOfIz
