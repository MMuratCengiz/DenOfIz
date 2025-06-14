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

#include <exception>
#include <stdexcept>
#include <spdlog/spdlog.h>

// Direct spdlog usage - no more glog-style macros
// Use spdlog::info(), spdlog::warn(), spdlog::error(), spdlog::critical()
// For debug logging, use spdlog::debug()

// Custom macros
#define DZ_RETURN_IF( condition )                                                                                                                                                  \
    if ( condition )                                                                                                                                                               \
    return
#define DZ_ASSERTM( exp, msg )                                                                                                                                                     \
    if ( !( exp ) )                                                                                                                                                                \
    spdlog::debug( msg )
#define DZ_NOT_NULL( exp )                                                                                                                                                         \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( !exp )                                                                                                                                                                \
        {                                                                                                                                                                          \
            spdlog::critical( "{} is required but was null.", #exp );                                                                                                              \
            throw std::runtime_error( "Null Pointer Exception: " #exp " is required but was null." );                                                                              \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( 0 )
