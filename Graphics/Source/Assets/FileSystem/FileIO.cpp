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

#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Utilities/Common.h>
#include <fstream>

using namespace DenOfIz;

InteropArray<Byte> FileIO::ReadFile( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::ifstream file( resolvedPath, std::ios::binary | std::ios::ate );
    if ( !file )
    {
        throw std::runtime_error( "Failed to open file: " + resolvedPath );
    }

    const size_t fileSize = file.tellg( );
    file.seekg( 0, std::ios::beg );

    InteropArray<Byte> buffer( fileSize );
    file.read( reinterpret_cast<char *>( buffer.Data( ) ), fileSize );

    if ( !file )
    {
        throw std::runtime_error( "Failed to read file: " + resolvedPath );
    }

    return buffer;
}

InteropArray<Byte> FileIO::ReadTextFile( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::ifstream file( resolvedPath, std::ios::binary | std::ios::ate );
    if ( !file )
    {
        throw std::runtime_error( "Failed to open file: " + resolvedPath );
    }

    const size_t fileSize = file.tellg( );
    file.seekg( 0, std::ios::beg );

    InteropArray<Byte> buffer( fileSize + 1 ); // +1 = '/0'
    file.read( reinterpret_cast<char *>( buffer.Data( ) ), fileSize );

    if ( !file )
    {
        throw std::runtime_error( "Failed to read file: " + resolvedPath );
    }

    buffer.GetElement( fileSize ) = '\0';
    return buffer;
}

void FileIO::WriteFile( const InteropString &path, const InteropArray<Byte> &data )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::ofstream file( resolvedPath, std::ios::binary );
    if ( !file )
    {
        throw std::runtime_error( "Failed to create file: " + resolvedPath );
    }

    file.write( reinterpret_cast<const char *>( data.Data( ) ), data.NumElements( ) );

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

size_t FileIO::GetFileSize( const InteropString &path )
{
    const std::string resolvedPath = PlatformResourcePath( path.Get( ) );

    std::error_code ec;
    const auto      size = std::filesystem::file_size( resolvedPath, ec );

    if ( ec )
    {
        throw std::runtime_error( "Failed to get file size: " + resolvedPath + " - " + ec.message( ) );
    }

    return size;
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
    if ( const std::filesystem::path fsPath( resourcePath ); fsPath.is_absolute( ) )
    {
        return resourcePath;
    }

#ifdef __APPLE__
    CFBundleRef mainBundle   = CFBundleGetMainBundle( );
    CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
    char        path[ PATH_MAX ];

    if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, (UInt8 *)path, PATH_MAX ) )
    {
        CFRelease( resourcesURL );
        return std::string( path ) + "/" + resourcePath;
    }

    CFRelease( resourcesURL );
    LOG( WARNING ) << "Unable to resolve resource path: " << resourcePath;
#endif
    return resourcePath;
}
