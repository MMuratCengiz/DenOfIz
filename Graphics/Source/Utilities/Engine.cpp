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
#include <cstring>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
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

    std::vector<spdlog::sink_ptr> sinks;

    const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>( );
    console_sink->set_color_mode( spdlog::color_mode::always );
    sinks.push_back( console_sink );

    const auto logFile = desc.LogFile.Get( );
    if ( logFile && std::strlen( logFile ) > 0 )
    {
        const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>( logFile, true );
        sinks.push_back( file_sink );
    }

    const auto logger = std::make_shared<spdlog::logger>( "DenOfIz", begin( sinks ), end( sinks ) );
    switch ( desc.LogLevel )
    {
    case LogLevel::Info:
        logger->set_level( spdlog::level::info );
        break;
    case LogLevel::Warning:
        logger->set_level( spdlog::level::warn );
        break;
    case LogLevel::Error:
        logger->set_level( spdlog::level::err );
        break;
    case LogLevel::Fatal:
        logger->set_level( spdlog::level::critical );
        break;
    }

    logger->set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v" );
    spdlog::set_default_logger( logger );
    logger->flush_on( spdlog::level::err );
}

void Engine::Shutdown( )
{
    spdlog::shutdown( );
}
