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

// This file contains modified code from the DirectX Tool Kit, released under the MIT License:
//--------------------------------------------------------------------------------------
// File: Geometry.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <DenOfIzCore/BitSet.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdint>
#include <map>
#include <numbers>
#include <vector>
#include "VertexTypes.h"

namespace DenOfIz
{
    typedef uint32_t geometry_index_t;

    enum class BuildDesc : uint32_t
    {
        BuildNormal    = 1 << 0,
        BuildTangent   = 1 << 1,
        BuildBitangent = 1 << 2,
        BuildTexCoord  = 1 << 3,
        RightHanded    = 1 << 4,
        InvertNormals  = 1 << 5,
    };

    struct QuadDesc
    {
        BitSet<BuildDesc> BuildDesc;
        XMFLOAT2          Size;
    };

    struct BoxDesc
    {
        BitSet<BuildDesc> BuildDesc;
        XMFLOAT3          Size;
    };

    struct SphereDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{};
        size_t            Tessellation{};
    };

    struct GeoSphereDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{};
        size_t            Tessellation{};
    };

    struct CylinderDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{};
        float             Height{};
        size_t            Tessellation{};
    };

    struct ConeDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{};
        float             Height{};
        size_t            Tessellation{};
    };

    struct TorusDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{};
        float             Thickness{};
        size_t            Tessellation{};
    };

    struct TetrahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{};
    };

    struct OctahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{};
    };

    struct DodecahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{};
    };

    struct IcosahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{};
    };

    struct GeometryData
    {
        std::vector<VertexPositionNormalTexture> Vertices;
        std::vector<geometry_index_t>            Indices;

        [[nodiscard]] size_t SizeOfVertices() const
        {
            return Vertices.size() * sizeof(VertexPositionNormalTexture);
        }

        [[nodiscard]] size_t SizeOfIndices() const
        {
            return Indices.size() * sizeof(geometry_index_t);
        }
    };

    class Geometry
    {
    public:
        Geometry()  = delete;
        ~Geometry() = delete;

        static GeometryData BuildQuad(const QuadDesc &desc);
        static GeometryData BuildBox(const BoxDesc &desc);
        static GeometryData BuildSphere(const SphereDesc &desc);
        static GeometryData BuildGeoSphere(const GeoSphereDesc &desc);
        static GeometryData BuildCylinder(const CylinderDesc &desc);
        static GeometryData BuildCone(const ConeDesc &desc);
        static GeometryData BuildTorus(const TorusDesc &desc);
        static GeometryData BuildTetrahedron(const TetrahedronDesc &tetrahedronDesc);
        static GeometryData BuildOctahedron(const OctahedronDesc &octahedronDesc);
        static GeometryData BuildDodecahedron(const DodecahedronDesc &dodecahedronDesc);
        static GeometryData BuildIcosahedron(const IcosahedronDesc &desc);
    };
} // namespace DenOfIz
