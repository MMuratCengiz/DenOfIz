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

#include "DenOfIzGraphics/Assets/FileSystem/FSConfig.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <filesystem>
#include <mutex>

using namespace DenOfIz;

FSDesc FSConfig::m_profileConfig;

void FSConfig::InitDefaults( )
{
    Init( { "Default", BundleResourcePath( ) } );
}

void FSConfig::Init( const FSDesc &config )
{
    static std::mutex mutex;
    std::lock_guard   guard( mutex );
    if ( !m_profileConfig.AssetPath.IsEmpty( ) )
    {
        LOG( WARNING ) << "FSConfig already initialized with asset path: " << m_profileConfig.AssetPath.Get( )
                       << ". Overriding this value is not recommended. You should initialize this class with the correct config at application startup.";
    }
    m_profileConfig = config;
}

InteropString FSConfig::AssetPath( )
{
    return m_profileConfig.AssetPath;
}

InteropString FSConfig::BundleResourcePath( )
{
#ifdef __APPLE__
    const CFBundleRef mainBundle   = CFBundleGetMainBundle( );
    const CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );

    if ( char path[ PATH_MAX ]; CFURLGetFileSystemRepresentation( resourcesURL, TRUE, reinterpret_cast<UInt8 *>( path ), PATH_MAX ) )
    {
        CFRelease( resourcesURL );
        const std::filesystem::path basePath( path );
        return basePath.c_str( );
    }
#endif
    return "";
}
