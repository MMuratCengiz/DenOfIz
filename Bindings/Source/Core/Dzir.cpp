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

#include "DenOfIzBindings/Core/Dzir.h"

#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

using namespace DenOfIz;

static constexpr const char *DENOFIZ_PREFIX     = "DenOfIz_";
static constexpr size_t      DENOFIZ_PREFIX_LEN = 8;

DzirLibrary::DzirLibrary( const DZIndex *index ) : m_storage( std::make_unique<Storage>( ) )
{
    for ( const auto &tu : index->GetTranslationUnits( ) )
    {
        VisitTranslationUnit( tu );
    }

    ClassifyAndGroupFunctions( );
}

const std::vector<DzirHeader *> &DzirLibrary::GetHeaders( ) const
{
    return m_headers;
}

const std::unordered_map<std::string, DzirEnum *> &DzirLibrary::GetEnumMap( ) const
{
    return m_enumMap;
}

const std::unordered_map<std::string, DzirStruct *> &DzirLibrary::GetStructMap( ) const
{
    return m_structMap;
}

const std::unordered_set<std::string> &DzirLibrary::GetHandleTypeNames( ) const
{
    return m_handleTypeNames;
}

const std::unordered_map<std::string, DzirOwnerType *> &DzirLibrary::GetOwnerTypes( ) const
{
    return m_ownerTypes;
}

const std::vector<DzirFunction *> &DzirLibrary::GetGlobalFunctions( ) const
{
    return m_globalFunctions;
}

DzirEnum *DzirLibrary::ResolveEnum( const std::string &name ) const
{
    const auto it = m_enumMap.find( name );
    return it != m_enumMap.end( ) ? it->second : nullptr;
}

DzirStruct *DzirLibrary::ResolveStruct( const std::string &name ) const
{
    const auto it = m_structMap.find( name );
    return it != m_structMap.end( ) ? it->second : nullptr;
}

bool DzirLibrary::IsHandleType( const std::string &name ) const
{
    return m_handleTypeNames.contains( name );
}

void DzirLibrary::VisitTranslationUnit( const DZTranslationUnit *tu )
{
    DzirHeader *header = NewHeader( );
    header->Path       = tu->RelPath;

    for ( const auto &dzInclude : tu->Includes )
    {
        DzirInclude *include = NewInclude( );
        include->IsSystem    = dzInclude.IsSystem;
        include->Path        = dzInclude.RelPath.empty( ) ? dzInclude.FullPath : dzInclude.RelPath;
        header->Includes.push_back( include );
    }

    m_headers.push_back( header );

    const CXCursor cursor = clang_getTranslationUnitCursor( tu->TranslationUnit );

    struct CursorVisitContext
    {
        DzirLibrary *Self;
        DzirHeader  *Header;
    };

    CursorVisitContext context{ this, header };

    clang_visitChildren(
        cursor,
        []( CXCursor currentCursor, CXCursor parent, const CXClientData clientData )
        {
            const auto *ctx = static_cast<CursorVisitContext *>( clientData );
            return ctx->Self->VisitCursor( currentCursor, parent, ctx->Header );
        },
        &context );
}

CXChildVisitResult DzirLibrary::VisitCursor( const CXCursor &cursor, const CXCursor &parent, DzirHeader *header )
{
    (void)parent;

    const CXSourceLocation location = clang_getCursorLocation( cursor );
    if ( clang_Location_isFromMainFile( location ) == 0 )
    {
        return CXChildVisit_Continue;
    }

    const CXCursorKind kind = clang_getCursorKind( cursor );

    if ( kind == CXCursor_LinkageSpec )
    {
        return CXChildVisit_Recurse;
    }

    VisitTopLevelDeclaration( cursor, header );
    return CXChildVisit_Continue;
}

void DzirLibrary::VisitTopLevelDeclaration( const CXCursor &cursor, DzirHeader *header )
{
    const CXCursorKind kind = clang_getCursorKind( cursor );

    switch ( kind )
    {
    case CXCursor_EnumDecl:
        if ( clang_isCursorDefinition( cursor ) != 0 )
        {
            PopulateEnum( cursor, header );
        }
        break;
    case CXCursor_StructDecl:
        if ( clang_isCursorDefinition( cursor ) != 0 )
        {
            PopulateStruct( cursor, header );
        }
        break;
    case CXCursor_TypedefDecl:
        {
            std::string typedefName = GetCursorTypeName( cursor );
            PopulateTypedef( cursor, header );
            break;
        }
    case CXCursor_MacroExpansion:
        {
            std::string macroName = ToString( clang_getCursorSpelling( cursor ) );
            if ( macroName == "DENOFIZ_DEFINE_HANDLE" )
            {
                CXSourceRange range     = clang_getCursorExtent( cursor );
                CXToken      *tokens    = nullptr;
                unsigned      numTokens = 0;
                clang_tokenize( clang_Cursor_getTranslationUnit( cursor ), range, &tokens, &numTokens );

                if ( numTokens >= 4 )
                {
                    std::string handleName = ToString( clang_getTokenSpelling( clang_Cursor_getTranslationUnit( cursor ), tokens[ 2 ] ) );

                    if ( StartsWith( handleName, DENOFIZ_PREFIX ) )
                    {
                        DzirHandleType *handleType = NewHandleType( );
                        handleType->FullName       = handleName;
                        handleType->ShortName      = StripPrefix( handleName );
                        handleType->Order          = m_declarationOrder++;

                        header->HandleTypes.push_back( handleType );
                        m_handleTypeNames.insert( handleName );
                    }
                }

                if ( tokens )
                {
                    clang_disposeTokens( clang_Cursor_getTranslationUnit( cursor ), tokens, numTokens );
                }
            }
            break;
        }
    case CXCursor_FunctionDecl:
        PopulateFunction( cursor, header );
        break;
    default:
        break;
    }
}

std::string DzirLibrary::ToString( const CXString cxString ) const
{
    const char *data   = clang_getCString( cxString );
    std::string result = data ? data : "";
    clang_disposeString( cxString );
    return result;
}

std::string DzirLibrary::GetCursorTypeName( const CXCursor &cursor ) const
{
    return ToString( clang_getCursorSpelling( cursor ) );
}

void DzirLibrary::PopulateEnum( const CXCursor &cursor, DzirHeader *header )
{
    std::string fullName = GetCursorTypeName( cursor );
    if ( fullName.empty( ) )
    {
        return;
    }

    DzirEnum *enumInfo  = NewEnum( );
    enumInfo->FullName  = fullName;
    enumInfo->ShortName = StripPrefix( fullName );
    enumInfo->Order     = m_declarationOrder++;

    PopulateEnumValues( cursor, enumInfo );

    header->Enums.push_back( enumInfo );
    m_enumMap[ fullName ] = enumInfo;
}

void DzirLibrary::PopulateEnumValues( const CXCursor &cursor, DzirEnum *enumInfo )
{
    struct EnumValueContext
    {
        DzirLibrary *Self;
        DzirEnum    *EnumInfo;
    };

    EnumValueContext context{ this, enumInfo };

    clang_visitChildren(
        cursor,
        []( CXCursor childCursor, CXCursor parentCursor, const CXClientData clientData )
        {
            (void)parentCursor;
            const auto *ctx = static_cast<EnumValueContext *>( clientData );

            if ( clang_getCursorKind( childCursor ) != CXCursor_EnumConstantDecl )
            {
                return CXChildVisit_Continue;
            }

            DzirEnumValue *value = ctx->Self->NewEnumValue( );
            value->Name          = ctx->Self->GetCursorTypeName( childCursor );
            value->Value         = static_cast<uint64_t>( clang_getEnumConstantDeclValue( childCursor ) );
            ctx->EnumInfo->Values.push_back( value );

            return CXChildVisit_Continue;
        },
        &context );
}

void DzirLibrary::PopulateStruct( const CXCursor &cursor, DzirHeader *header )
{
    std::string fullName = GetCursorTypeName( cursor );
    if ( fullName.empty( ) )
    {
        return;
    }

    DzirStruct *structInfo  = NewStruct( );
    structInfo->FullName    = fullName;
    structInfo->ShortName   = StripPrefix( fullName );
    structInfo->IsArray     = EndsWith( fullName, "Array" ) && !EndsWith( fullName, "ArrayView" );
    structInfo->IsArrayView = EndsWith( fullName, "ArrayView" );
    structInfo->Order       = m_declarationOrder++;

    if ( structInfo->IsArray || structInfo->IsArrayView )
    {
        std::string suffix           = structInfo->IsArrayView ? "ArrayView" : "Array";
        structInfo->ArrayElementType = fullName.substr( 0, fullName.size( ) - suffix.size( ) );
    }

    PopulateStructFields( cursor, structInfo );

    header->Structs.push_back( structInfo );
    m_structMap[ fullName ] = structInfo;
}

void DzirLibrary::PopulateStructFields( const CXCursor &cursor, DzirStruct *structInfo )
{
    struct StructFieldContext
    {
        DzirLibrary *Self;
        DzirStruct  *StructInfo;
    };

    StructFieldContext context{ this, structInfo };

    clang_visitChildren(
        cursor,
        []( CXCursor childCursor, CXCursor parentCursor, const CXClientData clientData )
        {
            (void)parentCursor;
            const auto *ctx = static_cast<StructFieldContext *>( clientData );

            if ( clang_getCursorKind( childCursor ) == CXCursor_FieldDecl )
            {
                ctx->Self->PopulateField( childCursor, ctx->StructInfo->Fields );
            }

            return CXChildVisit_Continue;
        },
        &context );
}

void DzirLibrary::PopulateField( const CXCursor &cursor, std::vector<DzirField *> &outFields )
{
    std::string fieldName = GetCursorTypeName( cursor );
    if ( fieldName.empty( ) )
    {
        return;
    }

    DzirField *field = NewField( );
    field->Name      = fieldName;
    field->Type      = BuildFieldType( clang_getCursorType( cursor ) );

    outFields.push_back( field );
}

void DzirLibrary::PopulateTypedef( const CXCursor &cursor, DzirHeader *header )
{
    if ( !IsHandleTypedef( cursor ) )
    {
        return;
    }

    std::string fullName = GetCursorTypeName( cursor );
    if ( fullName.empty( ) || !StartsWith( fullName, DENOFIZ_PREFIX ) )
    {
        return;
    }

    DzirHandleType *handleType = NewHandleType( );
    handleType->FullName       = fullName;
    handleType->ShortName      = StripPrefix( fullName );
    handleType->Order          = m_declarationOrder++;

    header->HandleTypes.push_back( handleType );
    m_handleTypeNames.insert( fullName );
}

bool DzirLibrary::IsHandleTypedef( const CXCursor &cursor ) const
{
    const CXType    underlyingType = clang_getTypedefDeclUnderlyingType( cursor );
    const CXType    canonical      = clang_getCanonicalType( underlyingType );
    const long long size           = clang_Type_getSizeOf( canonical );

    // Check if the underlying type spelling is uint64_t (handles are defined as typedef uint64_t X)
    std::string underlyingSpelling = ToString( clang_getTypeSpelling( underlyingType ) );

    if ( underlyingSpelling == "uint64_t" || size == 8 )
    {
        if ( canonical.kind == CXType_ULongLong || canonical.kind == CXType_LongLong || canonical.kind == CXType_ULong || canonical.kind == CXType_Long )
        {
            return true;
        }
    }

    return false;
}

void DzirLibrary::PopulateFunction( const CXCursor &cursor, DzirHeader *header )
{
    std::string fullName = GetCursorTypeName( cursor );
    if ( fullName.empty( ) || !StartsWith( fullName, DENOFIZ_PREFIX ) )
    {
        return;
    }

    DzirFunction *func = NewFunction( );
    func->ParsedName   = ParseFunctionName( fullName );
    func->Order        = m_declarationOrder++;

    const CXType resultType = clang_getCursorResultType( cursor );
    if ( resultType.kind != CXType_Invalid )
    {
        func->ReturnType = BuildFieldType( resultType );
    }
    else
    {
        func->ReturnType = PrimitiveKind::Void;
    }

    PopulateFunctionParameters( cursor, func );

    header->Functions.push_back( func );
}

void DzirLibrary::PopulateFunctionParameters( const CXCursor &cursor, DzirFunction *function )
{
    const int argCount = clang_Cursor_getNumArguments( cursor );
    if ( argCount <= 0 )
    {
        return;
    }

    function->Parameters.reserve( static_cast<size_t>( argCount ) );

    for ( int i = 0; i < argCount; ++i )
    {
        const CXCursor argument = clang_Cursor_getArgument( cursor, i );
        if ( clang_Cursor_isNull( argument ) != 0 )
        {
            continue;
        }

        DzirParameter *param = NewParameter( );
        param->Name          = GetCursorTypeName( argument );
        param->IsOutput      = false;
        param->IsBuffer      = false;

        const CXType paramType = clang_getCursorType( argument );
        param->Type            = BuildFieldType( paramType );

        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
        {
            if ( *ptr && ( *ptr )->Attributes == POINTER_ATTRIBUTE_NONE )
            {
                if ( StartsWith( param->Name, "out" ) || StartsWith( param->Name, "Out" ) )
                {
                    if ( ( *ptr )->Pointee )
                    {
                        if ( const auto *prim = std::get_if<PrimitiveKind>( ( *ptr )->Pointee.get( ) ) )
                        {
                            if ( *prim == PrimitiveKind::Void )
                            {
                                param->IsBuffer = true;
                            }
                            else
                            {
                                param->IsOutput = true;
                            }
                        }
                        else if ( std::get_if<std::unique_ptr<DzirPointer>>( ( *ptr )->Pointee.get( ) ) )
                        {
                            param->IsOutput = true;
                        }
                        else
                        {
                            param->IsOutput = true;
                        }
                    }
                    else
                    {
                        param->IsOutput = true;
                    }
                }
            }
        }

        function->Parameters.push_back( param );
    }
}

DzirFieldType DzirLibrary::BuildFieldType( const CXType &cxType ) const
{
    if ( cxType.kind == CXType_Invalid )
    {
        return PrimitiveKind::Void;
    }

    const CXType canonical = clang_getCanonicalType( cxType );

    std::string typeSpelling = ToString( clang_getTypeSpelling( cxType ) );
    if ( StartsWith( typeSpelling, "const " ) )
    {
        typeSpelling = typeSpelling.substr( 6 );
    }

    if ( m_handleTypeNames.contains( typeSpelling ) )
    {
        return DzirHandleRef{ typeSpelling };
    }

    switch ( canonical.kind )
    {
    case CXType_Pointer:
    case CXType_LValueReference:
    case CXType_RValueReference:
        {
            if ( auto ptrInfo = BuildPointerType( cxType, canonical ) )
            {
                return std::unique_ptr<DzirPointer>( ptrInfo.release( ) );
            }
            return PrimitiveKind::Void;
        }
    case CXType_ConstantArray:
    case CXType_IncompleteArray:
    case CXType_VariableArray:
    case CXType_DependentSizedArray:
        {
            if ( auto arrayInfo = BuildArrayType( canonical ) )
            {
                return std::unique_ptr<DzirArray>( arrayInfo.release( ) );
            }
            return PrimitiveKind::Void;
        }
    case CXType_Record:
        {
            const CXCursor decl = clang_getTypeDeclaration( canonical );
            if ( !clang_Cursor_isNull( decl ) )
            {
                std::string recordName = ToString( clang_getCursorSpelling( decl ) );
                if ( !recordName.empty( ) )
                {
                    return DzirStructRef{ recordName };
                }
            }
            return PrimitiveKind::Void;
        }
    case CXType_Enum:
        {
            const CXCursor decl = clang_getTypeDeclaration( canonical );
            if ( !clang_Cursor_isNull( decl ) )
            {
                std::string enumName = ToString( clang_getCursorSpelling( decl ) );
                if ( !enumName.empty( ) )
                {
                    return DzirEnumRef{ enumName };
                }
            }
            return PrimitiveKind::Void;
        }
    default:
        return MapBuiltinType( canonical );
    }
}

PrimitiveKind DzirLibrary::MapBuiltinType( const CXType &cxType ) const
{
    switch ( cxType.kind )
    {
    case CXType_Void:
        return PrimitiveKind::Void;
    case CXType_Bool:
        return PrimitiveKind::Bool;
    case CXType_SChar:
    case CXType_Char_S:
        return PrimitiveKind::Char;
    case CXType_UChar:
    case CXType_Char_U:
        return PrimitiveKind::UChar;
    case CXType_Short:
        return PrimitiveKind::I16;
    case CXType_UShort:
        return PrimitiveKind::U16;
    case CXType_Int:
        return PrimitiveKind::I32;
    case CXType_UInt:
        return PrimitiveKind::U32;
    case CXType_Long:
        {
            const long long size = clang_Type_getSizeOf( cxType );
            return size == 8 ? PrimitiveKind::I64 : PrimitiveKind::I32;
        }
    case CXType_ULong:
        {
            const long long size = clang_Type_getSizeOf( cxType );
            return size == 8 ? PrimitiveKind::U64 : PrimitiveKind::U32;
        }
    case CXType_LongLong:
        return PrimitiveKind::I64;
    case CXType_ULongLong:
        return PrimitiveKind::U64;
    case CXType_Float:
        return PrimitiveKind::F32;
    case CXType_Double:
    case CXType_LongDouble:
        return PrimitiveKind::F64;
    default:
        break;
    }
    return PrimitiveKind::Void;
}

std::unique_ptr<DzirPointer> DzirLibrary::BuildPointerType( const CXType &cxType, const CXType &canonical ) const
{
    auto pointer = std::make_unique<DzirPointer>( );

    switch ( cxType.kind )
    {
    case CXType_LValueReference:
        pointer->Kind = PointerKind::LValueReference;
        break;
    case CXType_RValueReference:
        pointer->Kind = PointerKind::RValueReference;
        break;
    default:
        pointer->Kind = PointerKind::Pointer;
        break;
    }

    CXType pointeeType = clang_getPointeeType( cxType );
    if ( pointeeType.kind == CXType_Invalid )
    {
        pointeeType = clang_getPointeeType( canonical );
    }

    const CXType pointeeCanonical = clang_getCanonicalType( pointeeType );

    PointerAttributes pointerAttributes = POINTER_ATTRIBUTE_NONE;
    if ( pointeeType.kind != CXType_Invalid )
    {
        if ( clang_isConstQualifiedType( pointeeType ) != 0 || clang_isConstQualifiedType( pointeeCanonical ) != 0 )
        {
            pointerAttributes = static_cast<PointerAttributes>( pointerAttributes | POINTER_ATTRIBUTE_CONST );
        }
    }
    pointer->Attributes = pointerAttributes;

    if ( pointeeType.kind != CXType_Invalid )
    {
        pointer->Pointee = std::make_unique<DzirFieldType>( BuildFieldType( pointeeType ) );
    }

    return pointer;
}

std::unique_ptr<DzirArray> DzirLibrary::BuildArrayType( const CXType &cxType ) const
{
    auto array         = std::make_unique<DzirArray>( );
    array->ElementType = std::make_unique<DzirFieldType>( BuildFieldType( clang_getArrayElementType( cxType ) ) );

    const long long size = clang_getArraySize( cxType );
    array->NumElements   = size >= 0 ? static_cast<uint32_t>( size ) : 0;

    return array;
}

bool DzirLibrary::HasDzApiSpecifier( const CXCursor &cursor ) const
{
    bool hasDzApi = false;

    clang_visitChildren(
        cursor,
        []( CXCursor child, CXCursor parent, const CXClientData clientData )
        {
            (void)parent;
            const CXCursorKind kind = clang_getCursorKind( child );

            if ( kind == CXCursor_DLLImport || kind == CXCursor_DLLExport )
            {
                *static_cast<bool *>( clientData ) = true;
                return CXChildVisit_Break;
            }

            if ( kind != CXCursor_UnexposedAttr && kind != CXCursor_AnnotateAttr )
            {
                return CXChildVisit_Continue;
            }

            const CXString attrName = clang_getCursorDisplayName( child );
            const char    *name     = clang_getCString( attrName );

            if ( name != nullptr && ( std::strstr( name, "DZ_API" ) != nullptr || std::strstr( name, "dllexport" ) != nullptr || std::strstr( name, "dllimport" ) != nullptr ) )
            {
                *static_cast<bool *>( clientData ) = true;
                clang_disposeString( attrName );
                return CXChildVisit_Break;
            }

            clang_disposeString( attrName );
            return CXChildVisit_Continue;
        },
        &hasDzApi );

    return hasDzApi;
}

void DzirLibrary::ClassifyAndGroupFunctions( )
{
    std::vector<DzirFunction *> allFunctions;
    for ( DzirHeader *header : m_headers )
    {
        for ( DzirFunction *func : header->Functions )
        {
            allFunctions.push_back( func );
        }
    }

    for ( DzirFunction *func : allFunctions )
    {
        func->Kind = ClassifyFunction( func );

        if ( func->Kind == FunctionKind::FactoryMethod )
        {
            func->FactoryReturnType = GetOutParameterHandleType( func );
        }
    }

    for ( DzirFunction *func : allFunctions )
    {
        const std::string &ownerType = func->ParsedName.OwnerType;

        if ( ownerType.empty( ) )
        {
            m_globalFunctions.push_back( func );
            continue;
        }

        std::string fullOwnerName = std::string( DENOFIZ_PREFIX ) + ownerType;

        if ( !IsHandleType( fullOwnerName ) )
        {
            m_globalFunctions.push_back( func );
            continue;
        }

        auto it = m_ownerTypes.find( fullOwnerName );
        if ( it == m_ownerTypes.end( ) )
        {
            DzirOwnerType *owner = NewOwnerType( );
            owner->FullName      = fullOwnerName;
            owner->ShortName     = ownerType;
            owner->HandleType    = nullptr;
            owner->Constructor   = nullptr;
            owner->Destructor    = nullptr;

            m_ownerTypes[ fullOwnerName ] = owner;
            it                            = m_ownerTypes.find( fullOwnerName );
        }

        DzirOwnerType *owner = it->second;

        switch ( func->Kind )
        {
        case FunctionKind::Constructor:
            owner->Constructor = func;
            break;
        case FunctionKind::Destructor:
            owner->Destructor = func;
            break;
        case FunctionKind::FactoryMethod:
            owner->FactoryMethods.push_back( func );
            break;
        case FunctionKind::InstanceMethod:
            owner->Methods.push_back( func );
            break;
        case FunctionKind::StaticFunction:
            m_globalFunctions.push_back( func );
            break;
        }
    }
}

DzirParsedFunctionName DzirLibrary::ParseFunctionName( const std::string &fullName ) const
{
    DzirParsedFunctionName result;
    result.FullName = fullName;

    if ( !StartsWith( fullName, DENOFIZ_PREFIX ) )
    {
        result.MethodName = fullName;
        return result;
    }

    std::string withoutPrefix = fullName.substr( DENOFIZ_PREFIX_LEN );

    size_t underscorePos = withoutPrefix.find( '_' );
    if ( underscorePos == std::string::npos )
    {
        result.MethodName = withoutPrefix;
        return result;
    }

    result.OwnerType  = withoutPrefix.substr( 0, underscorePos );
    result.MethodName = withoutPrefix.substr( underscorePos + 1 );

    return result;
}

FunctionKind DzirLibrary::ClassifyFunction( DzirFunction *func ) const
{
    const std::string &methodName = func->ParsedName.MethodName;
    const std::string &ownerType  = func->ParsedName.OwnerType;

    if ( ownerType.empty( ) )
    {
        return FunctionKind::StaticFunction;
    }

    std::string fullOwnerName = std::string( DENOFIZ_PREFIX ) + ownerType;
    bool        ownerIsHandle = IsHandleType( fullOwnerName );

    if ( methodName == "Create" || methodName == "CreateDefault" )
    {
        if ( IsHandleFieldType( func->ReturnType ) )
        {
            std::string returnTypeName = GetHandleTypeName( func->ReturnType );
            if ( returnTypeName == fullOwnerName )
            {
                return FunctionKind::Constructor;
            }
        }
    }

    if ( methodName == "Destroy" )
    {
        if ( !func->Parameters.empty( ) )
        {
            const DzirParameter *firstParam = func->Parameters[ 0 ];
            if ( IsHandleFieldType( firstParam->Type ) )
            {
                std::string paramTypeName = GetHandleTypeName( firstParam->Type );
                if ( paramTypeName == fullOwnerName )
                {
                    return FunctionKind::Destructor;
                }
            }
        }
    }

    if ( StartsWith( methodName, "Create" ) && ownerType == "LogicalDevice" )
    {
        std::string outHandleType = GetOutParameterHandleType( func );
        if ( !outHandleType.empty( ) && outHandleType != fullOwnerName )
        {
            return FunctionKind::FactoryMethod;
        }
    }

    if ( ownerIsHandle && !func->Parameters.empty( ) )
    {
        const DzirParameter *firstParam = func->Parameters[ 0 ];
        if ( IsHandleFieldType( firstParam->Type ) )
        {
            std::string paramTypeName = GetHandleTypeName( firstParam->Type );
            if ( paramTypeName == fullOwnerName )
            {
                return FunctionKind::InstanceMethod;
            }
        }
    }

    return FunctionKind::StaticFunction;
}

std::string DzirLibrary::GetOutParameterHandleType( DzirFunction *func ) const
{
    for ( const DzirParameter *param : func->Parameters )
    {
        if ( !param->IsOutput )
        {
            continue;
        }

        if ( const auto *ptr = std::get_if<std::unique_ptr<DzirPointer>>( &param->Type ) )
        {
            if ( *ptr && ( *ptr )->Pointee )
            {
                if ( const auto *handleRef = std::get_if<DzirHandleRef>( ( *ptr )->Pointee.get( ) ) )
                {
                    return handleRef->Name;
                }
            }
        }
    }

    return "";
}

bool DzirLibrary::IsHandleFieldType( const DzirFieldType &type ) const
{
    if ( const auto *handleRef = std::get_if<DzirHandleRef>( &type ) )
    {
        return true;
    }
    return false;
}

std::string DzirLibrary::GetHandleTypeName( const DzirFieldType &type ) const
{
    if ( const auto *handleRef = std::get_if<DzirHandleRef>( &type ) )
    {
        return handleRef->Name;
    }
    return "";
}

std::string DzirLibrary::StripPrefix( const std::string &name )
{
    if ( StartsWith( name, DENOFIZ_PREFIX ) )
    {
        return name.substr( DENOFIZ_PREFIX_LEN );
    }
    return name;
}

bool DzirLibrary::EndsWith( const std::string &str, const std::string &suffix )
{
    if ( suffix.size( ) > str.size( ) )
    {
        return false;
    }
    return str.compare( str.size( ) - suffix.size( ), suffix.size( ), suffix ) == 0;
}

bool DzirLibrary::StartsWith( const std::string &str, const std::string &prefix )
{
    if ( prefix.size( ) > str.size( ) )
    {
        return false;
    }
    return str.compare( 0, prefix.size( ), prefix ) == 0;
}

DzirHeader *DzirLibrary::NewHeader( )
{
    return m_storage->Headers.emplace_back( std::make_unique<DzirHeader>( ) ).get( );
}

DzirInclude *DzirLibrary::NewInclude( )
{
    return m_storage->Includes.emplace_back( std::make_unique<DzirInclude>( ) ).get( );
}

DzirEnumValue *DzirLibrary::NewEnumValue( )
{
    return m_storage->EnumValues.emplace_back( std::make_unique<DzirEnumValue>( ) ).get( );
}

DzirField *DzirLibrary::NewField( )
{
    return m_storage->Fields.emplace_back( std::make_unique<DzirField>( ) ).get( );
}

DzirParameter *DzirLibrary::NewParameter( )
{
    return m_storage->Parameters.emplace_back( std::make_unique<DzirParameter>( ) ).get( );
}

DzirFunction *DzirLibrary::NewFunction( )
{
    return m_storage->Functions.emplace_back( std::make_unique<DzirFunction>( ) ).get( );
}

DzirEnum *DzirLibrary::NewEnum( )
{
    return m_storage->Enums.emplace_back( std::make_unique<DzirEnum>( ) ).get( );
}

DzirStruct *DzirLibrary::NewStruct( )
{
    return m_storage->Structs.emplace_back( std::make_unique<DzirStruct>( ) ).get( );
}

DzirHandleType *DzirLibrary::NewHandleType( )
{
    return m_storage->HandleTypes.emplace_back( std::make_unique<DzirHandleType>( ) ).get( );
}

DzirOwnerType *DzirLibrary::NewOwnerType( )
{
    return m_storage->OwnerTypes.emplace_back( std::make_unique<DzirOwnerType>( ) ).get( );
}
