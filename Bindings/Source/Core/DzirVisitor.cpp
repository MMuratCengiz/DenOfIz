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

#include "DenOfIzBindings/Core/DzirVisitor.h"

#include <algorithm>
#include <ranges>

using namespace DenOfIz;

DzirVisitor::DzirVisitor( const DzirVisitorDesc &desc, const DzirLibrary *library ) :
    m_onPrimitive( desc.OnPrimitive ), m_onEnumRef( desc.OnEnumRef ), m_onStructRef( desc.OnStructRef ), m_onHandleRef( desc.OnHandleRef ), m_onPointer( desc.OnPointer ),
    m_onArray( desc.OnArray ), m_onEnumContainer( desc.OnEnumContainer ), m_onStructContainer( desc.OnStructContainer ), m_onHandleTypeContainer( desc.OnHandleTypeContainer ),
    m_onOwnerTypeContainer( desc.OnOwnerTypeContainer ), m_onFunction( desc.OnFunction ), m_onField( desc.OnField ), m_onParameter( desc.OnParameter ),
    m_onInclude( desc.OnInclude ), m_context( ), m_library( library )
{
    m_context.UserData = desc.UserData;
}

void DzirVisitor::Visit( const DzirHeader *header )
{
    if ( header == nullptr )
    {
        return;
    }

    m_context.CurrentHeader = header;

    for ( const auto *include : header->Includes )
    {
        VisitInclude( include );
    }

    std::vector<std::pair<int, void *>> orderedDeclarations;

    for ( const auto *enumDecl : header->Enums )
    {
        if ( enumDecl->Order >= 0 )
        {
            orderedDeclarations.push_back( { enumDecl->Order, const_cast<DzirEnum *>( enumDecl ) } );
        }
    }

    for ( const auto *structDecl : header->Structs )
    {
        if ( structDecl->Order >= 0 )
        {
            orderedDeclarations.push_back( { structDecl->Order, const_cast<DzirStruct *>( structDecl ) } );
        }
    }

    for ( const auto *handleDecl : header->HandleTypes )
    {
        if ( handleDecl->Order >= 0 )
        {
            orderedDeclarations.push_back( { handleDecl->Order, const_cast<DzirHandleType *>( handleDecl ) } );
        }
    }

    for ( const auto *functionDecl : header->Functions )
    {
        if ( functionDecl->Order >= 0 )
        {
            orderedDeclarations.push_back( { functionDecl->Order, const_cast<DzirFunction *>( functionDecl ) } );
        }
    }

    std::sort( orderedDeclarations.begin( ), orderedDeclarations.end( ), []( const auto &a, const auto &b ) { return a.first < b.first; } );

    for ( const auto &ptr : orderedDeclarations | std::views::values )
    {
        auto enumPtr   = static_cast<DzirEnum *>( ptr );
        auto structPtr = static_cast<DzirStruct *>( ptr );
        auto handlePtr = static_cast<DzirHandleType *>( ptr );
        auto funcPtr   = static_cast<DzirFunction *>( ptr );

        if ( std::ranges::find( header->Enums, enumPtr ) != header->Enums.end( ) )
        {
            VisitEnum( enumPtr );
        }
        else if ( std::ranges::find( header->Structs.begin( ), header->Structs.end( ), structPtr ) != header->Structs.end( ) )
        {
            VisitStruct( structPtr );
        }
        else if ( std::ranges::find( header->HandleTypes.begin( ), header->HandleTypes.end( ), handlePtr ) != header->HandleTypes.end( ) )
        {
            VisitHandleType( handlePtr );
        }
        else if ( std::ranges::find( header->Functions.begin( ), header->Functions.end( ), funcPtr ) != header->Functions.end( ) )
        {
            VisitFunction( funcPtr );
        }
    }
}

void DzirVisitor::VisitOwnerTypes( )
{
    for ( const auto &[ name, ownerType ] : m_library->GetOwnerTypes( ) )
    {
        VisitOwnerType( ownerType );
    }
}

void DzirVisitor::VisitInclude( const DzirInclude *include )
{
    if ( include == nullptr )
    {
        return;
    }

    if ( m_onInclude )
    {
        m_onInclude( include, m_context );
    }
}

void DzirVisitor::VisitFieldType( const DzirFieldType &type )
{
    std::visit(
        [ this ]<typename T0>( T0 &&arg )
        {
            using T = std::decay_t<T0>;

            if constexpr ( std::is_same_v<T, PrimitiveKind> )
            {
                if ( m_onPrimitive )
                {
                    m_onPrimitive( arg, m_context );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirEnumRef> )
            {
                if ( m_onEnumRef )
                {
                    m_onEnumRef( arg, m_context );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirStructRef> )
            {
                if ( m_onStructRef )
                {
                    m_onStructRef( arg, m_context );
                }
            }
            else if constexpr ( std::is_same_v<T, DzirHandleRef> )
            {
                if ( m_onHandleRef )
                {
                    m_onHandleRef( arg, m_context );
                }
            }
            else if constexpr ( std::is_same_v<T, std::unique_ptr<DzirPointer>> )
            {
                if ( arg && m_onPointer )
                {
                    if ( m_onPointer( *arg, m_context ) && arg->Pointee )
                    {
                        VisitFieldType( *arg->Pointee );
                    }
                }
            }
            else if constexpr ( std::is_same_v<T, std::unique_ptr<DzirArray>> )
            {
                if ( arg && m_onArray )
                {
                    if ( m_onArray( *arg, m_context ) && arg->ElementType )
                    {
                        VisitFieldType( *arg->ElementType );
                    }
                }
            }
        },
        type );
}

void DzirVisitor::VisitEnum( const DzirEnum *enumType )
{
    if ( enumType == nullptr )
    {
        return;
    }

    const DzirEnum *previousParentEnum = m_context.ParentEnum;
    m_context.ParentEnum               = enumType;

    if ( m_onEnumContainer )
    {
        m_onEnumContainer( enumType, m_context );
    }

    m_context.ParentEnum = previousParentEnum;
}

void DzirVisitor::VisitStruct( const DzirStruct *structType )
{
    if ( structType == nullptr )
    {
        return;
    }

    const DzirStruct *previousParentStruct = m_context.ParentStruct;
    m_context.ParentStruct                 = structType;

    if ( m_onStructContainer && !m_onStructContainer( structType, m_context ) )
    {
        m_context.ParentStruct = previousParentStruct;
        return;
    }

    for ( const auto *field : structType->Fields )
    {
        VisitField( field );
    }

    m_context.ParentStruct = previousParentStruct;
}

void DzirVisitor::VisitHandleType( const DzirHandleType *handleType )
{
    if ( handleType == nullptr )
    {
        return;
    }

    const DzirHandleType *previousParentHandle = m_context.ParentHandleType;
    m_context.ParentHandleType                 = handleType;

    if ( m_onHandleTypeContainer )
    {
        m_onHandleTypeContainer( handleType, m_context );
    }

    m_context.ParentHandleType = previousParentHandle;
}

void DzirVisitor::VisitOwnerType( const DzirOwnerType *ownerType )
{
    if ( ownerType == nullptr )
    {
        return;
    }

    const DzirOwnerType *previousParentOwner = m_context.ParentOwnerType;
    m_context.ParentOwnerType                = ownerType;

    if ( m_onOwnerTypeContainer && !m_onOwnerTypeContainer( ownerType, m_context ) )
    {
        m_context.ParentOwnerType = previousParentOwner;
        return;
    }

    if ( ownerType->Constructor )
    {
        VisitFunction( ownerType->Constructor );
    }

    if ( ownerType->Destructor )
    {
        VisitFunction( ownerType->Destructor );
    }

    for ( const auto *method : ownerType->Methods )
    {
        VisitFunction( method );
    }

    for ( const auto *factory : ownerType->FactoryMethods )
    {
        VisitFunction( factory );
    }

    m_context.ParentOwnerType = previousParentOwner;
}

void DzirVisitor::VisitField( const DzirField *field )
{
    if ( field == nullptr )
    {
        return;
    }

    if ( m_onField && !m_onField( field, m_context ) )
    {
        return;
    }

    VisitFieldType( field->Type );
}

void DzirVisitor::VisitFunction( const DzirFunction *function )
{
    if ( function == nullptr )
    {
        return;
    }

    const DzirFunction *previousParentFunction = m_context.ParentFunction;
    m_context.ParentFunction                   = function;

    if ( m_onFunction && !m_onFunction( function, m_context ) )
    {
        m_context.ParentFunction = previousParentFunction;
        return;
    }

    VisitFieldType( function->ReturnType );

    for ( const auto *parameter : function->Parameters )
    {
        VisitParameter( parameter );
    }

    m_context.ParentFunction = previousParentFunction;
}

void DzirVisitor::VisitParameter( const DzirParameter *parameter )
{
    if ( parameter == nullptr )
    {
        return;
    }

    if ( m_onParameter && !m_onParameter( parameter, m_context ) )
    {
        return;
    }

    VisitFieldType( parameter->Type );
}
