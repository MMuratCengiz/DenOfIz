/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzBindings/Core/Dzir.h"
#include "EmitterStream.h"

#include <filesystem>
#include <string>

namespace DenOfIz
{
    struct CSharpEmitterConfig
    {
        std::string Namespace   = "DenOfIz";
        std::string LibraryName = "DenOfIzGraphics";
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

        void EmitEnum( EmitterStream &stream, const DzirEnum *enumType );
        void EmitStruct( EmitterStream &stream, const DzirStruct *structType );
        void EmitNativeFunction( EmitterStream &stream, const DzirFunction *function );

        std::string GetNativeReturnType( const DzirFunction *function ) const;
        std::string GetNativeParameterList( const DzirFunction *function ) const;

        std::string FieldTypeToCSharp( const DzirFieldType &type, bool forNative ) const;
        std::string GetMarshalAttribute( const DzirFieldType &type ) const;
    };

} // namespace DenOfIz
