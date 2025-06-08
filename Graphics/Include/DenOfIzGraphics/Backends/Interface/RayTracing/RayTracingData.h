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

#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    enum class HitGroupType
    {
        Triangles,
        AABBs
    };

    namespace ASBuildFlags
    {
        constexpr uint32_t None            = 1 << 1;
        constexpr uint32_t AllowUpdate     = 1 << 2;
        constexpr uint32_t AllowCompaction = 1 << 3;
        constexpr uint32_t PreferFastTrace = 1 << 4;
        constexpr uint32_t PreferFastBuild = 1 << 5;
        constexpr uint32_t LowMemory       = 1 << 6;
        constexpr uint32_t FastTrace       = 1 << 7;
        constexpr uint32_t FastBuild       = 1 << 8;
        constexpr uint32_t MinimizeMemory  = 1 << 9;
        constexpr uint32_t PerformUpdate   = 1 << 10;
    } // namespace ASBuildFlags

#ifdef BUILD_METAL
    constexpr uint32_t TriangleIntersectionShader   = 0;
    constexpr uint32_t ProceduralIntersectionShader = 1;
#endif
} // namespace DenOfIz
