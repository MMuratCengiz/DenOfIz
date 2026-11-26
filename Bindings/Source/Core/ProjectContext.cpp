/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzBindings/Core/ProjectContext.h"

#include <filesystem>

using namespace DenOfIz;

ProjectContext::ProjectContext( const ProjectContextDesc &desc ) : m_includeRoot( desc.IncludeRoot )
{
}

std::string ProjectContext::GetIncludeRoot( ) const
{
    return m_includeRoot;
}

std::string ProjectContext::ResolvePath( const char *path ) const
{
    std::error_code ec;
    auto            p = std::filesystem::path( path );
    if ( !p.is_absolute( ) )
    {
        p = m_includeRoot / p;
    }
    const auto canon = std::filesystem::weakly_canonical( p, ec );
    if ( !ec )
    {
        return canon.generic_string( );
    }
    return p.lexically_normal( ).generic_string( );
}

std::string ProjectContext::Normalize( const std::string &path ) const
{
    std::error_code ec;
    const auto      p     = std::filesystem::path( path );
    const auto      canon = std::filesystem::weakly_canonical( p, ec );
    if ( !ec )
    {
        return canon.generic_string( );
    }
    return p.lexically_normal( ).generic_string( );
}

std::string ProjectContext::Relativize( const std::string &path ) const
{
    return Relativize( m_includeRoot, path );
}

std::string ProjectContext::Relativize( const std::string &root, const std::string &path ) const
{
    std::error_code ec;
    auto            rootFs = std::filesystem::weakly_canonical( root, ec );
    if ( ec )
    {
        return path;
    }
    auto       a  = std::filesystem::path( path );
    const auto ca = std::filesystem::weakly_canonical( a, ec );
    if ( !ec )
    {
        a = ca;
    }
    const auto rel = std::filesystem::relative( a, root, ec );
    if ( !ec )
    {
        return rel.lexically_normal( ).generic_string( );
    }
    return a.lexically_normal( ).generic_string( );
}
