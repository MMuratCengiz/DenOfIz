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

#include <DenOfIzGraphics/Utilities/BitSet.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    enum class ASBuildFlags
    {
        None            = 1 << 1,
        AllowUpdate     = 1 << 2,
        AllowCompaction = 1 << 3,
        PreferFastTrace = 1 << 4,
        PreferFastBuild = 1 << 5,
        LowMemory       = 1 << 6,
        FastTrace       = 1 << 7,
        FastBuild       = 1 << 8,
        MinimizeMemory  = 1 << 9,
        PerformUpdate   = 1 << 10,
    };
    template class DZ_API BitSet<ASBuildFlags>;

#ifdef BUILD_METAL
    constexpr uint32_t TriangleIntersectionShader   = 0;
    constexpr uint32_t ProceduralIntersectionShader = 1;
#endif
} // namespace DenOfIz
