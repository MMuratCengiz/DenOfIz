// Blazar Engine - 3D Game Engine
// Copyright (c) 2020-2021 Muhammed Murat Cengiz
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include <cstdarg>
#include <filesystem>
#include <fstream>

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

using namespace DenOfIz;

std::string Utilities::ReadFile( const std::string &filename )
{
    std::ifstream file( filename, std::ios::ate | std::ios::binary );

    if ( !file.is_open( ) )
    {
        throw std::runtime_error( "failed to open file!" );
    }

    std::string data;

    file.seekg( 0, std::ios::end );
    data.reserve( file.tellg( ) );
    file.seekg( 0, std::ios::beg );
    data.assign( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>( ) );

    return std::move( data );
}

std::string Utilities::GetFileDirectory( const std::string &file, const bool includeFinalSep )
{
    const size_t sepUnixIdx = file.find_last_of( "/\\" );
    const size_t sepWinIdx  = file.find_last_of( "\\\\" );

    const int finalSepSub = includeFinalSep ? 1 : 0;

    if ( sepUnixIdx != -1 )
    {
        return file.substr( 0, sepUnixIdx + finalSepSub );
    }
    if ( sepWinIdx != -1 )
    {
        return file.substr( 0, sepWinIdx - finalSepSub );
    }

    return file;
}

std::string Utilities::GetFilename( const std::string &file )
{
    const size_t sepUnixIdx = file.find_last_of( "/\\" );
    const size_t sepWinIdx  = file.find_last_of( "\\\\" );

    if ( sepUnixIdx != -1 )
    {
        return file.substr( sepUnixIdx + 1 );
    }
    if ( sepWinIdx != -1 )
    {
        return file.substr( 0, sepWinIdx + 1 );
    }

    return file;
}

std::string Utilities::CombineDirectories( const std::string &directory, const std::string &file )
{
    const std::string dir = GetFileDirectory( directory );
    const std::string f   = GetFilename( file );

    return dir + f;
}

std::string Utilities::AppPath( const std::string &resourcePath )
{
    const std::filesystem::path testRel( resourcePath );
    if ( testRel.is_absolute( ) )
    {
        return resourcePath;
    }
#if __APPLE__
    CFBundleRef mainBundle   = CFBundleGetMainBundle( );
    CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
    char        path[ PATH_MAX ];
    if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, (UInt8 *)path, PATH_MAX ) )
    {
        CFRelease( resourcesURL );
        return std::string( path ) + "/" + resourcePath;
    }
    CFRelease( resourcesURL );
    spdlog::warn( "Unable to load file: {}", resourcePath );
    return "";
#else
    return resourcePath;
#endif
}

uint32_t Utilities::Align( const uint32_t value, const uint32_t alignment )
{
    return ( value + alignment - 1 ) & ~( alignment - 1 );
}

uint32_t Utilities::HashInts( uint32_t args, ... )
{
    va_list ap;
    va_start( ap, args );

    uint32_t hash = 2166136261u;

    for ( uint32_t i = 0; i < args; i++ )
    {
        constexpr uint32_t prime = 16777619u;
        const uint32_t     value = va_arg( ap, uint32_t );
        hash ^= value;
        hash *= prime;
    }

    va_end( ap );
    return hash;
}
