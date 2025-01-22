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
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{

    struct DZ_API Vec3
    {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
    };

    struct DZ_API Vec2
    {
        float X{ 0 };
        float Y{ 0 };
    };

    struct DZ_API Vec4Int
    {
        uint32_t X{ 0 };
        uint32_t Y{ 0 };
        uint32_t Z{ 0 };
        uint32_t W{ 0 };
    };

    struct DZ_API Vec4Float
    {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
        float W{ 0 };
    };

    struct DZ_API MeshVertex
    {
        Vec3      Position;
        Vec3      Normal;
        Vec2      TexCoord;
        Vec3      Tangent;
        Vec3      Bitangent;
        Vec4Float Color;
        Vec4Int   BoneIndices;
        Vec4Float BoneWeights;
    };

    struct DZ_API SubMesh
    {
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t NumVertices;
        uint32_t NumIndices;
        uint32_t MaterialIndex;
    };

    struct DZ_API MeshBufferSizes
    {
        uint32_t TotalVertices;
        uint32_t TotalIndices;
        uint32_t NumSubMeshes;
    };

    class DZ_API MeshStreamCallback
    {
    public:
        virtual ~MeshStreamCallback( ) = default;
        virtual void OnBegin( const MeshBufferSizes &sizes ) {};
        virtual void OnVertexData( const InteropArray<MeshVertex> &vertices, uint32_t startIndex ) {}
        virtual void OnIndexData( const InteropArray<uint32_t> &indices, uint32_t startIndex ) {}
        virtual void OnComplete( const InteropArray<SubMesh> &subMeshes ) {}
    };

} // namespace DenOfIz
