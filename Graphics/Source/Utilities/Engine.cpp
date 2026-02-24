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

#define FMT_EXCEPTIONS 0

#include "DenOfIzGraphics/Utilities/Engine.h"
#include <thread>
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>

extern "C"
{

    void DenOfIz_Engine_Init( const DenOfIz_EngineDesc *desc )
    {
        if ( desc == NULL )
        {
            DenOfIz_FSConfig_InitDefaults( );
        }
        else
        {
            if ( desc->FS.AssetPath.Chars != NULL && desc->FS.AssetPath.NumChars > 0 )
            {
                DenOfIz_FSConfig_Init( &desc->FS );
            }
            else
            {
                DenOfIz_FSConfig_InitDefaults( );
            }
        }

#ifdef LINUX
        SDL_SetHint( SDL_HINT_VIDEO_WAYLAND_MODE_EMULATION, "0" );
        SDL_SetHint( SDL_HINT_VIDEO_WAYLAND_PREFER_LIBDECOR, "1" );
#endif
#ifdef WINDOW_MANAGER_SDL
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS );
#endif
        std::vector<spdlog::sink_ptr> sinks;

        const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>( );
        console_sink->set_color_mode( spdlog::color_mode::always );
        sinks.push_back( console_sink );

        std::string logFile = "DenOfIz.log";
        if ( desc != NULL && desc->LogFile.Chars != NULL && desc->LogFile.NumChars > 0 )
        {
            logFile = std::string( desc->LogFile.Chars, desc->LogFile.Chars + desc->LogFile.NumChars );
        }
        const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>( logFile, true );
        sinks.push_back( file_sink );

        const auto logger = std::make_shared<spdlog::logger>( "DenOfIz", begin( sinks ), end( sinks ) );
        if ( desc != NULL )
        {
            switch ( desc->LogLevel )
            {
            case DENOFIZ_LOG_LEVEL_INFO:
                logger->set_level( spdlog::level::info );
                break;
            case DENOFIZ_LOG_LEVEL_WARNING:
                logger->set_level( spdlog::level::warn );
                break;
            case DENOFIZ_LOG_LEVEL_ERROR:
                logger->set_level( spdlog::level::err );
                break;
            case DENOFIZ_LOG_LEVEL_FATAL:
                logger->set_level( spdlog::level::critical );
                break;
            }
        }
        else
        {
            logger->set_level( spdlog::level::info );
        }

        logger->set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v" );
        spdlog::set_default_logger( logger );
        logger->flush_on( spdlog::level::err );
    }

    void DenOfIz_Engine_Shutdown( )
    {
        SDL_Quit( );
#ifndef EMSCRIPTEN
        spdlog::shutdown( );
#endif
    }
}
