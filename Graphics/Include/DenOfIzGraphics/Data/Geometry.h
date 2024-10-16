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

#include <DenOfIzGraphics/Utilities/BitSet.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DirectXMath.h>
#include <cmath>
#include <cstdint>
#include <map>
#include <numbers>
#include <vector>

namespace DenOfIz
{
    enum class BuildDesc
    {
        BuildNormal    = 1 << 0,
        BuildTangent   = 1 << 1,
        BuildBitangent = 1 << 2,
        BuildTexCoord  = 1 << 3,
        RightHanded    = 1 << 4,
        InvertNormals  = 1 << 5,
    };

    template class DZ_API DenOfIz::BitSet<BuildDesc>;

    struct DZ_API QuadDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Width;
        float             Height;
    };

    struct DZ_API BoxDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Width;
        float             Height;
        float             Depth;
    };

    struct DZ_API SphereDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{ };
        size_t            Tessellation{ };
    };

    struct DZ_API GeoSphereDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{ };
        size_t            Tessellation{ };
    };

    struct DZ_API CylinderDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{ };
        float             Height{ };
        size_t            Tessellation{ };
    };

    struct DZ_API ConeDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{ };
        float             Height{ };
        size_t            Tessellation{ };
    };

    struct DZ_API TorusDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Diameter{ };
        float             Thickness{ };
        size_t            Tessellation{ };
    };

    struct DZ_API TetrahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{ };
    };

    struct DZ_API OctahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{ };
    };

    struct DZ_API DodecahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{ };
    };

    struct DZ_API IcosahedronDesc
    {
        BitSet<BuildDesc> BuildDesc;
        float             Size{ };
    };

    struct DZ_API GeometryPositionVertexData
    {
        float X;
        float Y;
        float Z;
    };

    struct DZ_API GeometryNormalVertexData
    {
        float X;
        float Y;
        float Z;
    };

    struct DZ_API GeometryTextureCoordinateVertexData
    {
        float U;
        float V;
    };

    struct DZ_API GeometryVertexData
    {
        GeometryPositionVertexData          Position{ };
        GeometryNormalVertexData            Normal{ };
        GeometryTextureCoordinateVertexData TextureCoordinate{ };
    };

    struct DZ_API GeometryData
    {
        std::vector<GeometryVertexData> Vertices;
        std::vector<uint32_t>           Indices;
    };

    class DZ_API Geometry
    {
    public:
        Geometry( )  = delete;
        ~Geometry( ) = delete;

        static GeometryData BuildQuad( const QuadDesc &desc );
        static GeometryData BuildBox( const BoxDesc &desc );
        static GeometryData BuildSphere( const SphereDesc &desc );
        static GeometryData BuildGeoSphere( const GeoSphereDesc &desc );
        static GeometryData BuildCylinder( const CylinderDesc &desc );
        static GeometryData BuildCone( const ConeDesc &desc );
        static GeometryData BuildTorus( const TorusDesc &desc );
        static GeometryData BuildTetrahedron( const TetrahedronDesc &tetrahedronDesc );
        static GeometryData BuildOctahedron( const OctahedronDesc &octahedronDesc );
        static GeometryData BuildDodecahedron( const DodecahedronDesc &dodecahedronDesc );
        static GeometryData BuildIcosahedron( const IcosahedronDesc &desc );
    };
} // namespace DenOfIz

template class DZ_API std::vector<DenOfIz::GeometryVertexData>;
template class DZ_API std::vector<uint32_t>;