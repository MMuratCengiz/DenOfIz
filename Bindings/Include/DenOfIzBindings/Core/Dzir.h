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

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "DZIndex.h"

namespace DenOfIz
{
    enum class PrimitiveKind : uint16_t
    {
        Void,
        Bool,
        Char,
        UChar,
        I16,
        U16,
        I32,
        U32,
        I64,
        U64,
        F32,
        F64,
        Size,
    };

    enum class PointerKind : uint8_t
    {
        Pointer,
        LValueReference,
        RValueReference,
    };

    enum PointerAttributes
    {
        POINTER_ATTRIBUTE_NONE     = 0,
        POINTER_ATTRIBUTE_CONST    = 1 << 0,
        POINTER_ATTRIBUTE_VOLATILE = 1 << 1,
        POINTER_ATTRIBUTE_RESTRICT = 1 << 2,
    };

    enum class FunctionKind : uint8_t
    {
        Constructor,
        Destructor,
        FactoryMethod,
        InstanceMethod,
        StaticFunction,
    };

    struct DzirEnumRef
    {
        std::string Name;
    };

    struct DzirStructRef
    {
        std::string Name;
    };

    struct DzirHandleRef
    {
        std::string Name;
    };

    struct DzirPointer;
    struct DzirArray;

    using DzirFieldType = std::variant<PrimitiveKind, DzirEnumRef, DzirStructRef, DzirHandleRef, std::unique_ptr<DzirPointer>, std::unique_ptr<DzirArray>>;

    struct DzirPointer
    {
        std::unique_ptr<DzirFieldType> Pointee;
        PointerKind                    Kind;
        PointerAttributes              Attributes;
    };

    struct DzirArray
    {
        std::unique_ptr<DzirFieldType> ElementType;
        uint32_t                       NumElements;
    };

    struct DzirEnumValue
    {
        std::string   Name;
        std::uint64_t Value;
        std::string   Comment;
    };

    struct DzirEnum
    {
        std::string                  FullName;
        std::string                  ShortName;
        std::vector<DzirEnumValue *> Values;
        std::string                  Comment;
        int                          Order = -1;
    };

    struct DzirField
    {
        std::string   Name;
        DzirFieldType Type;
        std::string   DefaultValue;
        std::string   Comment;
    };

    struct DzirStruct
    {
        std::string              FullName;
        std::string              ShortName;
        std::vector<DzirField *> Fields;
        std::string              Comment;
        bool                     IsArray;
        bool                     IsArrayView;
        std::string              ArrayElementType;
        int                      Order = -1;
    };

    struct DzirHandleType
    {
        std::string FullName;
        std::string ShortName;
        std::string Comment;
        int         Order = -1;
    };

    struct DzirParameter
    {
        std::string   Name;
        DzirFieldType Type;
        bool          IsOutput;
        bool          IsBuffer;
        std::string   DefaultValue;
        std::string   Comment;
    };

    struct DzirParsedFunctionName
    {
        std::string FullName;
        std::string OwnerType;
        std::string MethodName;
    };

    struct DzirFunction
    {
        DzirParsedFunctionName       ParsedName;
        FunctionKind                 Kind;
        DzirFieldType                ReturnType;
        std::vector<DzirParameter *> Parameters;
        std::string                  FactoryReturnType;
        std::string                  Comment;
        int                          Order = -1;
    };

    struct DzirOwnerType
    {
        std::string                 FullName;
        std::string                 ShortName;
        DzirHandleType             *HandleType;
        DzirFunction               *Constructor;
        DzirFunction               *Destructor;
        std::vector<DzirFunction *> Methods;
        std::vector<DzirFunction *> FactoryMethods;
    };

    struct DzirInclude
    {
        bool        IsSystem;
        std::string Path;
    };

    struct DzirHeader
    {
        std::string                   Path;
        std::vector<DzirInclude *>    Includes;
        std::vector<DzirEnum *>       Enums;
        std::vector<DzirStruct *>     Structs;
        std::vector<DzirHandleType *> HandleTypes;
        std::vector<DzirFunction *>   Functions;
    };

    class DzirLibrary
    {
        struct Storage
        {
            std::vector<std::unique_ptr<DzirHeader>>     Headers;
            std::vector<std::unique_ptr<std::string>>    Strings;
            std::vector<std::unique_ptr<DzirInclude>>    Includes;
            std::vector<std::unique_ptr<DzirEnumValue>>  EnumValues;
            std::vector<std::unique_ptr<DzirField>>      Fields;
            std::vector<std::unique_ptr<DzirParameter>>  Parameters;
            std::vector<std::unique_ptr<DzirFunction>>   Functions;
            std::vector<std::unique_ptr<DzirEnum>>       Enums;
            std::vector<std::unique_ptr<DzirStruct>>     Structs;
            std::vector<std::unique_ptr<DzirHandleType>> HandleTypes;
            std::vector<std::unique_ptr<DzirOwnerType>>  OwnerTypes;
        };

        std::unique_ptr<Storage>                      m_storage;
        std::vector<DzirHeader *>                     m_headers;
        std::unordered_map<std::string, DzirEnum *>   m_enumMap;
        std::unordered_map<std::string, DzirStruct *> m_structMap;
        std::unordered_set<std::string>               m_handleTypeNames;

        std::unordered_map<std::string, DzirOwnerType *> m_ownerTypes;
        std::vector<DzirFunction *>                      m_globalFunctions;
        int                                              m_declarationOrder = 0;

    public:
        explicit DzirLibrary( const DZIndex *index );

        const std::vector<DzirHeader *>                        &GetHeaders( ) const;
        const std::unordered_map<std::string, DzirEnum *>      &GetEnumMap( ) const;
        const std::unordered_map<std::string, DzirStruct *>    &GetStructMap( ) const;
        const std::unordered_set<std::string>                  &GetHandleTypeNames( ) const;
        const std::unordered_map<std::string, DzirOwnerType *> &GetOwnerTypes( ) const;
        const std::vector<DzirFunction *>                      &GetGlobalFunctions( ) const;

        DzirEnum   *ResolveEnum( const std::string &name ) const;
        DzirStruct *ResolveStruct( const std::string &name ) const;
        bool        IsHandleType( const std::string &name ) const;

    private:
        void               VisitTranslationUnit( const DZTranslationUnit *tu );
        CXChildVisitResult VisitCursor( const CXCursor &cursor, const CXCursor &parent, DzirHeader *header );
        void               VisitTopLevelDeclaration( const CXCursor &cursor, DzirHeader *header );

        std::string ToString( CXString cxString ) const;
        std::string GetCursorTypeName( const CXCursor &cursor ) const;

        void PopulateEnum( const CXCursor &cursor, DzirHeader *header );
        void PopulateEnumValues( const CXCursor &cursor, DzirEnum *enumInfo );

        void PopulateStruct( const CXCursor &cursor, DzirHeader *header );
        void PopulateStructFields( const CXCursor &cursor, DzirStruct *structInfo );
        void PopulateField( const CXCursor &cursor, std::vector<DzirField *> &outFields );

        void PopulateTypedef( const CXCursor &cursor, DzirHeader *header );
        bool IsHandleTypedef( const CXCursor &cursor ) const;

        void PopulateFunction( const CXCursor &cursor, DzirHeader *header );
        void PopulateFunctionParameters( const CXCursor &cursor, DzirFunction *function );

        DzirFieldType                BuildFieldType( const CXType &cxType ) const;
        PrimitiveKind                MapBuiltinType( const CXType &cxType ) const;
        std::unique_ptr<DzirPointer> BuildPointerType( const CXType &cxType, const CXType &canonical ) const;
        std::unique_ptr<DzirArray>   BuildArrayType( const CXType &cxType ) const;

        bool HasDzApiSpecifier( const CXCursor &cursor ) const;

        void                   ClassifyAndGroupFunctions( );
        DzirParsedFunctionName ParseFunctionName( const std::string &fullName ) const;
        FunctionKind           ClassifyFunction( DzirFunction *func ) const;
        std::string            GetOutParameterHandleType( DzirFunction *func ) const;
        bool                   IsHandleFieldType( const DzirFieldType &type ) const;
        std::string            GetHandleTypeName( const DzirFieldType &type ) const;

        static std::string StripPrefix( const std::string &name );
        static bool        EndsWith( const std::string &str, const std::string &suffix );
        static bool        StartsWith( const std::string &str, const std::string &prefix );

        DzirHeader     *NewHeader( );
        DzirInclude    *NewInclude( );
        DzirEnumValue  *NewEnumValue( );
        DzirField      *NewField( );
        DzirParameter  *NewParameter( );
        DzirFunction   *NewFunction( );
        DzirEnum       *NewEnum( );
        DzirStruct     *NewStruct( );
        DzirHandleType *NewHandleType( );
        DzirOwnerType  *NewOwnerType( );
    };

} // namespace DenOfIz
