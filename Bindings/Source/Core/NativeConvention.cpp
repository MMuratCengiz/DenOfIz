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

#include "DenOfIzBindings/Core/NativeConvention.h"
#include "DenOfIzBindings/Core/DzirTypeVisitor.h"

using namespace DenOfIz;

static constexpr const char *DENOFIZ_PREFIX     = "DenOfIz_";
static constexpr size_t      DENOFIZ_PREFIX_LEN = 8;

std::string NativeConvention::PrimitiveTypeToString( const PrimitiveKind kind )
{
    switch ( kind )
    {
    case PrimitiveKind::Void:
        return "void";
    case PrimitiveKind::Bool:
        return "bool";
    case PrimitiveKind::Char:
        return "sbyte";
    case PrimitiveKind::UChar:
        return "byte";
    case PrimitiveKind::I16:
        return "short";
    case PrimitiveKind::U16:
        return "ushort";
    case PrimitiveKind::I32:
        return "int";
    case PrimitiveKind::U32:
        return "uint";
    case PrimitiveKind::I64:
        return "long";
    case PrimitiveKind::U64:
        return "ulong";
    case PrimitiveKind::F32:
        return "float";
    case PrimitiveKind::F64:
        return "double";
    case PrimitiveKind::Size:
        return "nuint";
    }
    return "void";
}

std::string NativeConvention::TypeToNativeString( const DzirLibrary *library, const DzirFieldType &field )
{
    std::string result = "void";

    DzirTypeVisitorDesc desc;

    desc.OnPrimitive = [ &result ]( const PrimitiveKind kind ) { result = PrimitiveTypeToString( kind ); };

    desc.OnEnumRef = [ &result ]( const DzirEnumRef &ref ) { result = StripPrefix( ref.Name ); };

    desc.OnStructRef = [ &result ]( const DzirStructRef &ref ) { result = StripPrefix( ref.Name ); };

    desc.OnHandleRef = [ &result ]( const DzirHandleRef &ref ) { result = "ulong"; };

    desc.OnPointer = [ library, &result ]( const DzirPointer &ptr )
    {
        if ( ptr.Pointee )
        {
            const auto *handleRef = std::get_if<DzirHandleRef>( ptr.Pointee.get( ) );
            if ( handleRef )
            {
                result = "out ulong";
                return;
            }

            const auto *structRef = std::get_if<DzirStructRef>( ptr.Pointee.get( ) );
            if ( structRef )
            {
                bool isConst = ( ptr.Attributes & POINTER_ATTRIBUTE_CONST ) != 0;
                result       = ( isConst ? "in " : "ref " ) + StripPrefix( structRef->Name );
                return;
            }

            const auto *primitive = std::get_if<PrimitiveKind>( ptr.Pointee.get( ) );
            if ( primitive && *primitive == PrimitiveKind::Void )
            {
                result = "IntPtr";
                return;
            }
        }
        result = "IntPtr";
    };

    desc.OnArray = [ library, &result ]( const DzirArray &arr ) { result = "IntPtr"; };

    DzirTypeVisitor::Visit( field, desc );
    return result;
}

std::string NativeConvention::TypeToCSharpString( const DzirLibrary *library, const DzirFieldType &field )
{
    std::string result = "void";

    DzirTypeVisitorDesc desc;

    desc.OnPrimitive = [ &result ]( const PrimitiveKind kind ) { result = PrimitiveTypeToString( kind ); };

    desc.OnEnumRef = [ &result ]( const DzirEnumRef &ref ) { result = StripPrefix( ref.Name ); };

    desc.OnStructRef = [ library, &result ]( const DzirStructRef &ref )
    {
        const DzirStruct *structType = library->ResolveStruct( ref.Name );
        if ( structType && ( structType->IsArray || structType->IsArrayView ) )
        {
            result = StripPrefix( ref.Name );
        }
        else
        {
            result = StripPrefix( ref.Name );
        }
    };

    desc.OnHandleRef = [ &result ]( const DzirHandleRef &ref ) { result = StripPrefix( ref.Name ); };

    desc.OnPointer = [ library, &result ]( const DzirPointer &ptr )
    {
        if ( ptr.Pointee )
        {
            result = TypeToCSharpString( library, *ptr.Pointee );
        }
        else
        {
            result = "IntPtr";
        }
    };

    desc.OnArray = [ library, &result ]( const DzirArray &arr )
    {
        if ( arr.ElementType )
        {
            result = TypeToCSharpString( library, *arr.ElementType ) + "[]";
        }
        else
        {
            result = "IntPtr";
        }
    };

    DzirTypeVisitor::Visit( field, desc );
    return result;
}

std::string NativeConvention::TypeToCSharpMarshalAttribute( const DzirLibrary *library, const DzirFieldType &field )
{
    std::string result;

    DzirTypeVisitorDesc desc;

    desc.OnPrimitive = [ &result ]( const PrimitiveKind kind )
    {
        if ( kind == PrimitiveKind::Bool )
        {
            result = "[MarshalAs(UnmanagedType.I1)]";
        }
    };

    DzirTypeVisitor::Visit( field, desc );
    return result;
}

bool NativeConvention::IsEnumType( const DzirLibrary *library, const DzirFieldType &field )
{
    bool isEnum = false;

    DzirTypeVisitorDesc desc;
    desc.OnEnumRef = [ &isEnum ]( const DzirEnumRef &ref ) { isEnum = true; };

    DzirTypeVisitor::Visit( field, desc );
    return isEnum;
}

bool NativeConvention::IsStructType( const DzirLibrary *library, const DzirFieldType &field )
{
    bool isStruct = false;

    DzirTypeVisitorDesc desc;
    desc.OnStructRef = [ &isStruct ]( const DzirStructRef &ref ) { isStruct = true; };

    DzirTypeVisitor::Visit( field, desc );
    return isStruct;
}

bool NativeConvention::IsHandleType( const DzirLibrary *library, const DzirFieldType &field )
{
    bool isHandle = false;

    DzirTypeVisitorDesc desc;
    desc.OnHandleRef = [ &isHandle ]( const DzirHandleRef &ref ) { isHandle = true; };

    DzirTypeVisitor::Visit( field, desc );
    return isHandle;
}

bool NativeConvention::IsPointerType( const DzirFieldType &field )
{
    bool isPointer = false;

    DzirTypeVisitorDesc desc;
    desc.OnPointer = [ &isPointer ]( const DzirPointer &ptr ) { isPointer = true; };

    DzirTypeVisitor::Visit( field, desc );
    return isPointer;
}

bool NativeConvention::IsArrayType( const DzirLibrary *library, const DzirFieldType &field )
{
    if ( const auto *structRef = std::get_if<DzirStructRef>( &field ) )
    {
        const DzirStruct *structType = library->ResolveStruct( structRef->Name );
        if ( structType )
        {
            return structType->IsArray || structType->IsArrayView;
        }
    }
    return false;
}

std::string NativeConvention::StripPrefix( const std::string &name )
{
    if ( name.size( ) > DENOFIZ_PREFIX_LEN && name.compare( 0, DENOFIZ_PREFIX_LEN, DENOFIZ_PREFIX ) == 0 )
    {
        return name.substr( DENOFIZ_PREFIX_LEN );
    }
    return name;
}
