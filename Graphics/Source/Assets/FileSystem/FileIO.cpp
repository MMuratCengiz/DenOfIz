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

#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include <fstream>
#include <spdlog/spdlog.h>

#include "DenOfIzGraphics/Assets/FileSystem/FSConfig.h"
#include "DenOfIzGraphics/Utilities/Common.h"

using namespace DenOfIz;

uint64_t FileIO::GetFileNumBytes( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );
    std::error_code   ec;
    const auto        numBytes = std::filesystem::file_size( resolvedPath, ec );
    if ( ec )
    {
        spdlog::error( "Failed to get file size: {}", ec.message( ) );
        return 0;
    }
    return numBytes;
}

bool FileIO::ReadFile( const InteropString &path, const ByteArray &buffer )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );
    std::ifstream     file( resolvedPath, std::ios::binary | std::ios::ate );
    if ( !file )
    {
        spdlog::error( "Failed to open file: {}", resolvedPath );
        return false;
    }
    const size_t fileSize = file.tellg( );
    if ( buffer.NumElements < fileSize )
    {
        spdlog::error( "Failed to read file: buffer is too small, use GetFileNumBytes( ) to correctly allocate memory" );
        return false;
    }
    file.seekg( 0, std::ios::beg );
    file.read( reinterpret_cast<char *>( buffer.Elements ), fileSize );
    return true;
}

void FileIO::WriteFile( const InteropString &path, const ByteArrayView &data )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::ofstream file( resolvedPath, std::ios::binary );
    if ( !file )
    {
        throw std::runtime_error( "Failed to create file: " + resolvedPath );
    }

    file.write( reinterpret_cast<const char *>( data.Elements ), data.NumElements );

    if ( !file )
    {
        throw std::runtime_error( "Failed to write file: " + resolvedPath );
    }
}

bool FileIO::FileExists( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );
    return std::filesystem::exists( resolvedPath );
}

bool FileIO::CreateDirectories( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::error_code ec;
    const bool      success = std::filesystem::create_directories( resolvedPath, ec );
    return success && !ec;
}

bool FileIO::Remove( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::error_code ec;
    const bool      success = std::filesystem::remove( resolvedPath, ec );
    return success && !ec;
}

bool FileIO::RemoveAll( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::error_code ec;
    const auto      count = std::filesystem::remove_all( resolvedPath, ec );
    return count > 0 && !ec;
}

InteropString FileIO::GetAbsolutePath( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::error_code ec;
    const auto      absolutePath = std::filesystem::absolute( resolvedPath, ec );

    if ( ec )
    {
        throw std::runtime_error( "Failed to get absolute path: " + resolvedPath + " - " + ec.message( ) );
    }

    return { absolutePath.string( ).c_str( ) };
}

InteropString FileIO::GetResourcePath( const InteropString &path )
{
    return { PlatformResourcePath( path.Get( ) ).c_str( ) };
}

std::string FileIO::PlatformResourcePath( const std::string &resourcePath )
{
    const auto assetPath = FSConfig::AssetPath( );
    if ( assetPath.NumChars( ) == 0 )
    {
        return resourcePath;
    }
    const std::filesystem::path basePath( assetPath.Get( ) );
    const std::filesystem::path resPath( resourcePath );

    if ( const std::filesystem::path fsPath( resourcePath ); fsPath.is_absolute( ) )
    {
        return resourcePath;
    }

    return ( basePath / resPath ).string( );
}
