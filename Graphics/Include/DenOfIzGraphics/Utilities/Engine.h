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

// Resolve an odd Swig error
#ifdef __linux__
#undef __valid
#include <chrono>
#endif

// Init OS specific common includes, to make sure they are loaded first
#include "Common_Apple.h"
#include "Common_Windows.h"
#include "DenOfIzGraphics/Assets/FileSystem/FSConfig.h"

#include "glog/logging.h"
#include "DenOfIzGraphics/Backends/Common/SDLInclude.h"

namespace DenOfIz
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    struct DZ_API EngineDesc
    {
        LogLevel      LogLevel  = LogLevel::Info;
        InteropString LogFile   = "DenOfIz.log";
        FSDesc        FS = { };
    };

    class DZ_API Engine
    {
    public:
        static void Init( const EngineDesc &desc = { } )
        {
            if ( !desc.FS.AssetPath.IsEmpty( ) )
            {
                FSConfig::Init( desc.FS );
            }
            else
            {
                FSConfig::InitDefaults( );
            }

#ifdef WINDOW_MANAGER_SDL
            SDL_SetMainReady( );
            SDL_Init( SDL_INIT_VIDEO | SDL_INIT_SENSOR | SDL_INIT_GAMECONTROLLER );
            std::atexit( SDL_Quit );
#endif

            switch ( desc.LogLevel )
            {
            case LogLevel::Info:
                google::LogAtLevel( google::GLOG_INFO, "DenOfIz.log" );
                break;
            case LogLevel::Warning:
                google::LogAtLevel( google::GLOG_WARNING, "DenOfIz.log" );
                break;
            case LogLevel::Error:
                google::LogAtLevel( google::GLOG_ERROR, "DenOfIz.log" );
                break;
            case LogLevel::Fatal:
                google::LogAtLevel( google::GLOG_FATAL, "DenOfIz.log" );
                break;
            }

            FLAGS_alsologtostderr  = true;
            FLAGS_colorlogtostdout = true;

            const auto logFile = desc.LogFile.Get( );
            google::InitGoogleLogging( "DenOfIz" );
            google::SetLogDestination( google::GLOG_INFO, logFile );
            google::SetLogDestination( google::GLOG_WARNING, logFile );
            google::SetLogDestination( google::GLOG_ERROR, logFile );
            google::SetLogDestination( google::GLOG_FATAL, logFile );
        }

        static void Shutdown( )
        {
        }
    };
} // namespace DenOfIz
