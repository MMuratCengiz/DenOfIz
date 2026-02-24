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

// ReSharper disable once CppUnusedIncludeDirective
#if defined( _WIN32 ) || defined( XBOX )
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined( __ANDROID__ )
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#elif defined( __linux__ ) && !defined( VK_USE_PLATFORM_GGP )
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined( NX64 )
#define VK_USE_PLATFORM_VI_NN
#endif
#define VK_NO_PROTOTYPES
#include <vk_mem_alloc.h>
#include <volk.h>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifndef VK_CHECK_RESULT
#define VK_CHECK_RESULT( R )                                                                                                                                                       \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        VkResult result = ( R );                                                                                                                                                   \
        if ( result != VK_SUCCESS )                                                                                                                                                \
        {                                                                                                                                                                          \
            spdlog::error( "[Vulkan] VK_CHECK_RESULT failed at {}:{} with result: {}", __FILE__, __LINE__, (int)result );                                                          \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( 0 )
#endif

#undef Bool
#undef None
#undef Success
#undef Always
#undef Font
#undef CurrentTime
#undef InputFocus
