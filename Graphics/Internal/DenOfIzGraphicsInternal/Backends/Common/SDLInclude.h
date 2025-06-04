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

#define WINDOW_MANAGER_SDL // SDL is the only window manager for now

#ifdef WINDOW_MANAGER_SDL

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#ifdef BUILD_VK
#include <SDL2/SDL_vulkan.h>
#endif
#include <SDL2/SDL_syswm.h>

// Cleanup X11 macro pollution
#undef Bool
#undef None
#undef Success
#undef Always
#undef Font
#undef CurrentTime
#undef InputFocus

#endif
