/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
