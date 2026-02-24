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
#include <filesystem>
#include <mutex>
#include <string>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace
{
    struct ProfileConfig
    {
        std::string Name;
        std::string AssetPath;
    };

    ProfileConfig g_profileConfig;

    std::string ToOwnedString( const DenOfIz_StringView view )
    {
        if ( view.Chars == NULL || view.NumChars == 0 )
        {
            return { };
        }
        return std::string( view.Chars, view.Chars + view.NumChars );
    }
} // namespace

extern "C"
{

    void DenOfIz_FSConfig_InitDefaults( )
    {
        DenOfIz_FSDesc defaultConfig{ };
        defaultConfig.Name      = DENOFIZ_STRING( "Default" );
        defaultConfig.AssetPath = DenOfIz_FSConfig_BundleResourcePath( );
        DenOfIz_FSConfig_Init( &defaultConfig );
    }

    void DenOfIz_FSConfig_Init( const DenOfIz_FSDesc *config )
    {
        if ( config == NULL )
        {
            return;
        }

        static std::mutex mutex;
        std::lock_guard   guard( mutex );
        if ( !g_profileConfig.AssetPath.empty( ) )
        {
            spdlog::warn(
                "FSConfig already initialized with asset path: {} . Overriding this value is not recommended. You should initialize this class with the correct config at "
                "application startup.",
                g_profileConfig.AssetPath.c_str( ) );
        }
        g_profileConfig.Name      = ToOwnedString( config->Name );
        g_profileConfig.AssetPath = ToOwnedString( config->AssetPath );
    }

    DenOfIz_StringView DenOfIz_FSConfig_AssetPath( )
    {
        return { g_profileConfig.AssetPath.c_str( ), static_cast<uint32_t>( g_profileConfig.AssetPath.size( ) ) };
    }

    DenOfIz_StringView DenOfIz_FSConfig_BundleResourcePath( )
    {
#ifdef __APPLE__
        static std::once_flag onceFlag;
        static std::string    bundleResourcePath;

        std::call_once( onceFlag,
                        [ & ]( )
                        {
                            const CFBundleRef mainBundle   = CFBundleGetMainBundle( );
                            const CFURLRef    resourcesURL = CFBundleCopyResourcesDirectoryURL( mainBundle );
                            if ( resourcesURL == NULL )
                            {
                                return;
                            }
                            char path[ PATH_MAX ];
                            if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, reinterpret_cast<UInt8 *>( path ), PATH_MAX ) )
                            {
                                const std::filesystem::path basePath( path );
                                bundleResourcePath = basePath.string( );
                            }
                            CFRelease( resourcesURL );
                        } );

        return { bundleResourcePath.c_str( ), static_cast<uint32_t>( bundleResourcePath.size( ) ) };
#endif
        return { "", 0 };
    }
}
