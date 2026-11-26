/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstdint>
#include <string>

namespace DenOfIz
{

    class Utilities
    {
        Utilities( ) = default;

    public:
        static std::string ReadFile( const std::string &filename );
        static std::string GetFileDirectory( const std::string &file, bool includeFinalSep = true );
        static std::string GetFilename( const std::string &file );
        static std::string CombineDirectories( const std::string &directory, const std::string &file );
        static std::string AppPath( const std::string &resourcePath );
        static uint32_t    Align( const uint32_t value, const uint32_t alignment );
        static uint32_t    HashInts( uint32_t args, ... );
    };

} // namespace DenOfIz
