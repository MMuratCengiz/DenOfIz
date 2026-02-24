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

#include "Dzir.h"

namespace DenOfIz
{
    class NativeConvention
    {
    public:
        static std::string PrimitiveTypeToString( PrimitiveKind kind );
        static std::string TypeToNativeString( const DzirLibrary *library, const DzirFieldType &field );
        static std::string TypeToCSharpString( const DzirLibrary *library, const DzirFieldType &field );
        static std::string TypeToCSharpMarshalAttribute( const DzirLibrary *library, const DzirFieldType &field );

        static bool IsEnumType( const DzirLibrary *library, const DzirFieldType &field );
        static bool IsStructType( const DzirLibrary *library, const DzirFieldType &field );
        static bool IsHandleType( const DzirLibrary *library, const DzirFieldType &field );
        static bool IsPointerType( const DzirFieldType &field );
        static bool IsArrayType( const DzirLibrary *library, const DzirFieldType &field );

        static std::string StripPrefix( const std::string &name );
    };
} // namespace DenOfIz
