/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#ifdef _WIN32
#ifdef DZ_EXAMPLES_EXPORTS
#define DZ_EXAMPLES_API __declspec( dllexport )
#elif DZ_EXAMPLES_IMPORTS
#define DZ_EXAMPLES_API __declspec( dllimport )
#endif
#endif

#ifndef DZ_EXAMPLES_API
#define DZ_EXAMPLES_API
#endif
