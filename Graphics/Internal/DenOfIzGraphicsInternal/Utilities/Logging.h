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

#include <cstdlib>

#define SPDLOG_NO_EXCEPTIONS
#define FMT_EXCEPTIONS 0
#include <spdlog/spdlog.h>

#ifdef _MSC_VER
#define DZ_DEBUG_BREAK( ) __debugbreak( )
#elif defined( __GNUC__ ) || defined( __clang__ )
#if defined( __i386__ ) || defined( __x86_64__ )
#define DZ_DEBUG_BREAK( ) __asm__ __volatile__( "int $0x03" )
#elif defined( __arm__ )
#define DZ_DEBUG_BREAK( ) __asm__ __volatile__( ".inst 0xe7f001f0" )
#elif defined( __aarch64__ )
#define DZ_DEBUG_BREAK( ) __asm__ __volatile__( ".inst 0xd4200000" )
#elif defined( EMSCRIPTEN )
#include <emscripten.h>
#define DZ_DEBUG_BREAK( )                                                                                                                                                          \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        EM_ASM( { debugger; } );                                                                                                                                                   \
        abort( );                                                                                                                                                                  \
    }                                                                                                                                                                              \
    while ( 0 )
#else
#define DZ_DEBUG_BREAK( ) abort( )
#endif
#else
#define DZ_DEBUG_BREAK( ) abort( )
#endif

#define DZ_RETURN_IF( condition )                                                                                                                                                  \
    if ( condition )                                                                                                                                                               \
    return

#define DZ_ASSERTM( exp, msg )                                                                                                                                                     \
    if ( !( exp ) )                                                                                                                                                                \
    spdlog::debug( msg )

#ifdef NDEBUG
#define DZ_NOT_NULL( exp )                                                                                                                                                         \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( !exp )                                                                                                                                                                \
        {                                                                                                                                                                          \
            spdlog::critical( "{} is required but was null at {}:{}", #exp, __FILE__, __LINE__ );                                                                                  \
            spdlog::shutdown( );                                                                                                                                                   \
            abort( );                                                                                                                                                              \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( 0 )
#else
#define DZ_NOT_NULL( exp )                                                                                                                                                         \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( !exp )                                                                                                                                                                \
        {                                                                                                                                                                          \
            spdlog::critical( "{} is required but was null at {}:{}", #exp, __FILE__, __LINE__ );                                                                                  \
            DZ_DEBUG_BREAK( );                                                                                                                                                     \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( 0 )
#endif
