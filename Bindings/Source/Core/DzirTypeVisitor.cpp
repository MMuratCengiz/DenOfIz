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

#include "DenOfIzBindings/Core/DzirTypeVisitor.h"

using namespace DenOfIz;

void DzirTypeVisitor::Visit( const DzirFieldType &type, const DzirTypeVisitorDesc &desc )
{
    std::visit(
        [ &desc ]<typename T0>( const T0 &arg )
        {
            using T = std::decay_t<T0>;

            if constexpr ( std::is_same_v<T, PrimitiveKind> )
            {
                if ( desc.OnPrimitive )
                {
                    desc.OnPrimitive( arg );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirEnumRef> )
            {
                if ( desc.OnEnumRef )
                {
                    desc.OnEnumRef( arg );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirStructRef> )
            {
                if ( desc.OnStructRef )
                {
                    desc.OnStructRef( arg );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirHandleRef> )
            {
                if ( desc.OnHandleRef )
                {
                    desc.OnHandleRef( arg );
                }
            }
            else if constexpr ( std::is_same_v<T, std::unique_ptr<DzirPointer>> )
            {
                if ( arg && desc.OnPointer )
                {
                    desc.OnPointer( *arg );
                }
            }
            else if constexpr ( std::is_same_v<T, std::unique_ptr<DzirArray>> )
            {
                if ( arg && desc.OnArray )
                {
                    desc.OnArray( *arg );
                }
            }
        },
        type );
}
