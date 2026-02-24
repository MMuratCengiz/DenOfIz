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
#include <string>

namespace DenOfIz
{
    struct ProjectContextDesc
    {
        std::string IncludeRoot;
    };

    class ProjectContext
    {
        std::string m_includeRoot;

    public:
        explicit ProjectContext( const ProjectContextDesc &desc );

        std::string GetIncludeRoot( ) const;
        std::string ResolvePath( const char *path ) const;
        std::string Normalize( const std::string &path ) const;
        std::string Relativize( const std::string &path ) const;
        std::string Relativize( const std::string &root, const std::string &path ) const;
    };
} // namespace DenOfIz
