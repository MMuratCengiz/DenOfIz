/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "Dzir.h"

#include <functional>

namespace DenOfIz
{
    struct DzirTypeVisitorDesc
    {
        std::function<void( PrimitiveKind )>         OnPrimitive;
        std::function<void( const DzirEnumRef & )>   OnEnumRef;
        std::function<void( const DzirStructRef & )> OnStructRef;
        std::function<void( const DzirHandleRef & )> OnHandleRef;
        std::function<void( const DzirPointer & )>   OnPointer;
        std::function<void( const DzirArray & )>     OnArray;
    };

    class DzirTypeVisitor
    {
    public:
        static void Visit( const DzirFieldType &type, const DzirTypeVisitorDesc &desc );
    };
} // namespace DenOfIz
