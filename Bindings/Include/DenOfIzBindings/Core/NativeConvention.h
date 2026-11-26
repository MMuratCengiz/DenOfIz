/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
