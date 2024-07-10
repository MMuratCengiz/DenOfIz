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

#include <DenOfIzCore/BitSet.h>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

namespace DenOfIz
{
    enum class BuildDesc : uint32_t
    {
        Normal    = 1 << 0,
        Tangent   = 1 << 1,
        Bitangent = 1 << 2,
        TexCoord  = 1 << 3,
        All       = Normal | Tangent | Bitangent | TexCoord
    };

    struct CubeDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Width;
        float             Height;
        float             Depth;
    };

    struct SphereDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Radius;
    };

    struct CylinderDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Radius;
        float             Height;
    };

    struct ConeDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Radius;
        float             Height;
    };

    struct TorusDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Radius;
        float             TubeRadius;
    };

    struct PlaneDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Width;
        float             Height;
    };

    struct PrimitiveData
    {
        std::vector<float>    Vertices;
        std::vector<uint32_t> Indices;
    };

    class PrimitiveBuilder
    {
    public:
        PrimitiveBuilder()  = delete;
        ~PrimitiveBuilder() = delete;

        static PrimitiveData BuildCube(const CubeDesc &desc);
        static PrimitiveData BuildSphere(const SphereDesc &desc);
        static PrimitiveData BuildCylinder(const CylinderDesc &desc);
        static PrimitiveData BuildCone(const ConeDesc &desc);
        static PrimitiveData BuildTorus(const TorusDesc &desc);
        static PrimitiveData BuildPlane(const PlaneDesc &desc);
    };
} // namespace DenOfIz
