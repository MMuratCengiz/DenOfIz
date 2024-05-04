// Blazar Engine - 3D Game Engine
// Copyright (c) 2020-2021 Muhammed Murat Cengiz
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#endif

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <stdlib.h>

#ifdef _WIN32
#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifndef NOMINMAX
#define NOMINMAX
#include <windows.h>
#endif
#endif

#ifndef __APPLE_CC__
#include <malloc.h>
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include <memory>
#include <cstring>
#include "Time.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stb_image.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define PATH( P ) std::string("") + P

#define ENTITY_CAST( instance ) std::dynamic_pointer_cast< IGameEntity >( instance )

#define while_false( statement ) do { statement } while ( false )
#define ASSERT_M( val, message ) while_false( if ( !( val ) ) { throw std::runtime_error( message ); } )
#define ASSERT( val ) ASSERT_M( val, "assert val failed!" )

#define NOT_NULL( val ) ASSERT_M( val != nullptr, "val cannot be null!" )
#define VkCheckResult( R ) ASSERT( R == vk::Result::eSuccess )
#define IS_NULL( val ) ( val == nullptr )

#define FUNCTION_BREAK( condition ) if ( condition ) return
#define SKIP_ITERATION_IF( condition ) if ( condition ) continue;

#define VK_CORRECTION_MATRIX glm::mat4(  1.0f,  0.0f, 0.0f, 0.0f, \
                                         0.0f, -1.0f, 0.0f, 0.0f, \
                                         0.0f,  0.0f, 0.5f, 0.0f, \
                                         0.0f,  0.0f, 0.5f, 1.0f)

#include "Constants.h"

struct Unit { };
template<typename T>
struct Result {
	bool Success;
	std::string Message;
	T Result;
};

#define Success(Return) \
{ \
	.Success = true, \
	.Message = "", \
	.Result = Return, \
}

#define Error(ErrorMessage) \
{ \
	.Success = false, \
	.Message = ErrorMessage \
}

#include <boost/noncopyable.hpp>