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

#include <functional>

#include "Dzir.h"

namespace DenOfIz
{
    struct DzirVisitContext
    {
        const DzirHeader     *CurrentHeader;
        const DzirEnum       *ParentEnum;
        const DzirStruct     *ParentStruct;
        const DzirHandleType *ParentHandleType;
        const DzirOwnerType  *ParentOwnerType;
        const DzirFunction   *ParentFunction;
        void                 *UserData;

        DzirVisitContext( ) :
            CurrentHeader( nullptr ), ParentEnum( nullptr ), ParentStruct( nullptr ), ParentHandleType( nullptr ), ParentOwnerType( nullptr ), ParentFunction( nullptr ),
            UserData( nullptr )
        {
        }
    };

    using OnPrimitiveCallback = std::function<void( PrimitiveKind, DzirVisitContext & )>;
    using OnEnumRefCallback   = std::function<void( const DzirEnumRef &, DzirVisitContext & )>;
    using OnStructRefCallback = std::function<void( const DzirStructRef &, DzirVisitContext & )>;
    using OnHandleRefCallback = std::function<void( const DzirHandleRef &, DzirVisitContext & )>;
    using OnPointerCallback   = std::function<bool( const DzirPointer &, DzirVisitContext & )>;
    using OnArrayCallback     = std::function<bool( const DzirArray &, DzirVisitContext & )>;

    using OnEnumContainerCallback       = std::function<bool( const DzirEnum *, DzirVisitContext & )>;
    using OnStructContainerCallback     = std::function<bool( const DzirStruct *, DzirVisitContext & )>;
    using OnHandleTypeContainerCallback = std::function<bool( const DzirHandleType *, DzirVisitContext & )>;
    using OnOwnerTypeContainerCallback  = std::function<bool( const DzirOwnerType *, DzirVisitContext & )>;
    using OnFunctionCallback            = std::function<bool( const DzirFunction *, DzirVisitContext & )>;

    using OnFieldCallback     = std::function<bool( const DzirField *, DzirVisitContext & )>;
    using OnParameterCallback = std::function<bool( const DzirParameter *, DzirVisitContext & )>;
    using OnIncludeCallback   = std::function<bool( const DzirInclude *, DzirVisitContext & )>;

    struct DzirVisitorDesc
    {
        OnPrimitiveCallback OnPrimitive;
        OnEnumRefCallback   OnEnumRef;
        OnStructRefCallback OnStructRef;
        OnHandleRefCallback OnHandleRef;
        OnPointerCallback   OnPointer;
        OnArrayCallback     OnArray;

        OnEnumContainerCallback       OnEnumContainer;
        OnStructContainerCallback     OnStructContainer;
        OnHandleTypeContainerCallback OnHandleTypeContainer;
        OnOwnerTypeContainerCallback  OnOwnerTypeContainer;
        OnFunctionCallback            OnFunction;

        OnFieldCallback     OnField;
        OnParameterCallback OnParameter;
        OnIncludeCallback   OnInclude;

        void *UserData;
    };

    class DzirVisitor
    {
        OnPrimitiveCallback           m_onPrimitive;
        OnEnumRefCallback             m_onEnumRef;
        OnStructRefCallback           m_onStructRef;
        OnHandleRefCallback           m_onHandleRef;
        OnPointerCallback             m_onPointer;
        OnArrayCallback               m_onArray;
        OnEnumContainerCallback       m_onEnumContainer;
        OnStructContainerCallback     m_onStructContainer;
        OnHandleTypeContainerCallback m_onHandleTypeContainer;
        OnOwnerTypeContainerCallback  m_onOwnerTypeContainer;
        OnFunctionCallback            m_onFunction;
        OnFieldCallback               m_onField;
        OnParameterCallback           m_onParameter;
        OnIncludeCallback             m_onInclude;
        DzirVisitContext              m_context;
        const DzirLibrary            *m_library;

    public:
        explicit DzirVisitor( const DzirVisitorDesc &desc, const DzirLibrary *library );
        ~DzirVisitor( ) = default;

        void Visit( const DzirHeader *header );
        void VisitOwnerTypes( );

    private:
        void VisitInclude( const DzirInclude *include );
        void VisitFieldType( const DzirFieldType &type );
        void VisitEnum( const DzirEnum *enumType );
        void VisitStruct( const DzirStruct *structType );
        void VisitHandleType( const DzirHandleType *handleType );
        void VisitOwnerType( const DzirOwnerType *ownerType );
        void VisitFunction( const DzirFunction *function );
        void VisitField( const DzirField *field );
        void VisitParameter( const DzirParameter *parameter );
    };
} // namespace DenOfIz
