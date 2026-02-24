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

#include "DenOfIzBindings/Core/Dzir.h"
#include "EmitterStream.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace DenOfIz
{
    struct CSharpEmitterConfig
    {
        std::string Namespace        = "DenOfIz";
        std::string LibraryName      = "DenOfIzGraphics";
        bool        GenerateNative   = true;
        bool        GenerateStructs  = true;
        bool        GenerateEnums    = true;
        bool        GenerateWrappers = false;
    };

    class CSharpEmitter
    {
        const DzirLibrary    *m_library;
        CSharpEmitterConfig   m_config;
        std::filesystem::path m_outputDir;

    public:
        CSharpEmitter( const DzirLibrary *library, const CSharpEmitterConfig &config );

        void SetOutputDirectory( const std::filesystem::path &outputDir );
        void Emit( );

    private:
        void EmitNativeFile( );
        void EmitEnumsFile( );
        void EmitStructsFile( );
        void EmitHelpersFile( );
        void EmitStructExtensionsFile( const std::unordered_map<std::string, std::vector<const DzirFunction *>> &structFuncs );
        void EmitWrapperFile( const DzirOwnerType *ownerType );
        void EmitOrphanHandleWrappers( );
        void EmitGlobalFunctionsFile( );

        void EmitEnum( EmitterStream &stream, const DzirEnum *enumType );
        void EmitStruct( EmitterStream &stream, const DzirStruct *structType );
        void EmitNativeFunction( EmitterStream &stream, const DzirFunction *function );

        void EmitWrapperClass( EmitterStream &stream, const DzirOwnerType *ownerType );
        void EmitWrapperConstructor( EmitterStream &stream, const DzirOwnerType *ownerType );
        void EmitWrapperDestructor( EmitterStream &stream, const DzirOwnerType *ownerType );
        void EmitWrapperMethod( EmitterStream &stream, const DzirFunction *function );
        void EmitWrapperFactoryMethod( EmitterStream &stream, const DzirFunction *function );
        void EmitStaticMethod( EmitterStream &stream, const DzirFunction *function );

        std::string GetNativeReturnType( const DzirFunction *function ) const;
        std::string GetNativeParameterList( const DzirFunction *function ) const;
        std::string GetWrapperReturnType( const DzirFunction *function ) const;
        std::string GetWrapperParameterList( const DzirFunction *function, bool skipSelf, bool includeOutputAsOut = false ) const;
        std::string GetNativeCallArguments( const DzirFunction *function, bool includeSelf, bool usePinnedBuffers = false ) const;

        std::string FieldTypeToCSharp( const DzirFieldType &type, bool forNative ) const;
        std::string GetMarshalAttribute( const DzirFieldType &type ) const;
        bool        IsHandleTypeName( const std::string &shortName ) const;
        bool        IsStructTypeName( const std::string &shortName ) const;

        static std::string ToPascalCase( const std::string &name );
        static std::string ToCamelCase( const std::string &name );
    };

} // namespace DenOfIz
