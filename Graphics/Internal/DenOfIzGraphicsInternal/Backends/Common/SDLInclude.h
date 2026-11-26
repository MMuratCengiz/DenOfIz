/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#define WINDOW_MANAGER_SDL // SDL is the only window manager for now

#ifdef WINDOW_MANAGER_SDL

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#ifdef BUILD_VK
#include <SDL3/SDL_vulkan.h>
#endif
#include <SDL3/SDL_system.h>

// Cleanup X11 macro pollution
#undef Bool
#undef None
#undef Success
#undef Always
#undef Font
#undef CurrentTime
#undef InputFocus

#endif
