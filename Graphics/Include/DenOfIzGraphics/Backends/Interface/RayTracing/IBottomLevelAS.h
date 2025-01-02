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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include "RayTracingData.h"

namespace DenOfIz
{
    enum class GeometryFlags
    {
        Opaque                      = 1 << 0,
        NoDuplicateAnyHitInvocation = 1 << 1
    };
    template class DZ_API BitSet<GeometryFlags>;

    struct DZ_API ASGeometryTriangleDesc
    {
        IBufferResource *VertexBuffer;
        uint32_t         VertexOffset;
        uint32_t         VertexStride;
        uint32_t         NumVertices;
        Format           VertexFormat;
        IBufferResource *IndexBuffer;
        uint32_t         IndexOffset;
        uint32_t         NumIndices;
        IndexType        IndexType;
    };

    struct DZ_API ASGeometryAABBDesc
    {
        IBufferResource *Buffer;
        uint32_t         Offset;
        uint32_t         Stride;
        uint32_t         NumAABBs;
    };

    struct DZ_API ASGeometryDesc
    {
        HitGroupType           Type;
        ASGeometryTriangleDesc Triangles;
        ASGeometryAABBDesc     AABBs;
        BitSet<GeometryFlags>  Flags;
    };

    template class DZ_API InteropArray<ASGeometryDesc>;

    struct DZ_API BottomLevelASDesc
    {
        InteropArray<ASGeometryDesc> Geometries;
        BitSet<ASBuildFlags>         BuildFlags;
    };

    class DZ_API IBottomLevelAS
    {
    public:
        virtual ~IBottomLevelAS( ) = default;
    };
} // namespace DenOfIz
