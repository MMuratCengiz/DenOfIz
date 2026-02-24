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

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DENOFIZ_DEFINE_HANDLE( object ) typedef uint64_t object;

#define DENOFIZ_NULL_HANDLE ( (uint64_t)0 )

#define DENOFIZ_HANDLE_IS_VALID( handle ) ( ( handle ) != DENOFIZ_NULL_HANDLE )

#define DENOFIZ_TO_HANDLE( pointer ) ( (uint64_t)(uintptr_t)( pointer ) )
#define DENOFIZ_FROM_HANDLE( CppType, handle ) ( (CppType *)(uintptr_t)( handle ) )

#ifdef __cplusplus
}
#endif
