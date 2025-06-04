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

#include "DenOfIzGraphics/Utilities/Engine.h"
#include <thorvg.h>
#include <thread>
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void Engine::Init( const EngineDesc &desc )
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

    tvg::Initializer::init( tvg::CanvasEngine::Sw, std::thread::hardware_concurrency( ) );
    std::atexit( [] { tvg::Initializer::term( tvg::CanvasEngine::Sw ); } );

    FLAGS_alsologtostderr           = true;
    FLAGS_colorlogtostdout          = true;
    FLAGS_max_log_size              = 50;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_logfile_mode              = 0644;
    FLAGS_logcleansecs              = 60 * 60 * 24;

    switch ( desc.LogLevel )
    {
    case LogLevel::Info:
        FLAGS_minloglevel = google::GLOG_INFO;
        break;
    case LogLevel::Warning:
        FLAGS_minloglevel = google::GLOG_WARNING;
        break;
    case LogLevel::Error:
        FLAGS_minloglevel = google::GLOG_ERROR;
        break;
    case LogLevel::Fatal:
        FLAGS_minloglevel = google::GLOG_FATAL;
        break;
    }

    const auto logFile = desc.LogFile.Get( );
    google::InitGoogleLogging( "DenOfIz" );
    google::SetLogDestination( google::GLOG_INFO, logFile );
    google::SetLogDestination( google::GLOG_WARNING, logFile );
    google::SetLogDestination( google::GLOG_ERROR, logFile );
    google::SetLogDestination( google::GLOG_FATAL, logFile );
}

void Engine::Shutdown( )
{
    google::ShutdownGoogleLogging( );
}
