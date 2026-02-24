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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define DZ_API EMSCRIPTEN_KEEPALIVE
#elif defined( _WIN32 )
#ifdef DZ_GRAPHICS_EXPORTS
#define DZ_API __declspec( dllexport )
#elif defined( DZ_GRAPHICS_IMPORTS )
#define DZ_API __declspec( dllimport )
#endif
#endif

#ifndef DZ_API
#define DZ_API
#endif

#if _WIN32
#define SafeCopyString( dst, dstSize, src ) strcpy_s( dst, dstSize, src )
#elif __APPLE__ || __linux__
#define SafeCopyString( dst, dstSize, src ) strncpy( dst, src, dstSize )
#endif
