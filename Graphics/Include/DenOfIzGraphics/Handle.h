/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
