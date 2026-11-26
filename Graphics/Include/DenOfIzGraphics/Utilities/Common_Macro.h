/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#ifdef _WIN32
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
