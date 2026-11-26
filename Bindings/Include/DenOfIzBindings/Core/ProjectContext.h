/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
