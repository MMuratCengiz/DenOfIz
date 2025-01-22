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

#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>

using namespace DenOfIz;

std::string PathResolver::ResolvePath( const std::string &path )
{
    std::filesystem::path testPath( path );
    if ( testPath.is_absolute( ) )
    {
        return path;
    }

#ifdef __APPLE__
    CFBundleRef mainBundle   = CFBundleGetMainBundle( );
    CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
    char        resolvedPath[ PATH_MAX ];

    if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, (UInt8 *)resolvedPath, PATH_MAX ) )
    {
        CFRelease( resourcesURL );
        return std::string( resolvedPath ) + "/" + path;
    }

    CFRelease( resourcesURL );
    LOG( WARNING ) << "Unable to resolve path: " << path;
    return "";
#else
    return path;
#endif
}

std::string PathResolver::ResolveBundlePath( const std::string &bundlePath )
{
    std::filesystem::path testPath( bundlePath );
    if ( testPath.is_absolute( ) )
    {
        return bundlePath;
    }

#ifdef __APPLE__
    // For bundles, we might want to store them in a specific directory
    // within the app bundle's Resources
    CFBundleRef mainBundle   = CFBundleGetMainBundle( );
    CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
    char        resolvedPath[ PATH_MAX ];

    if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, (UInt8 *)resolvedPath, PATH_MAX ) )
    {
        CFRelease( resourcesURL );
        return std::string( resolvedPath ) + "/Bundles/" + bundlePath;
    }

    CFRelease( resourcesURL );
    LOG( WARNING ) << "Unable to resolve bundle path: " << bundlePath;
    return "";
#else
    return bundlePath;
#endif
}
