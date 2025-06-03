/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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
