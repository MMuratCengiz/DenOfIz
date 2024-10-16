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