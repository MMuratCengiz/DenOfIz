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

#include "IBufferResource.h"

namespace DenOfIz
{
    struct AccelerationStructureGeometryDesc
    {
        enum class GeometryType
        {
            Triangles,
            AABBs
        };

        GeometryType     Type;
        IBufferResource *VertexBuffer;
        uint64_t         VertexOffset;
        uint32_t         VertexStride;
        IBufferResource *IndexBuffer;
        uint64_t         IndexOffset;
        uint32_t         IndexCount;
        uint32_t         VertexCount;
        uint32_t         PrimitiveCount;
        bool             IsOpaque;
    };

    struct AccelerationStructureInstanceDesc
    {
        IBufferResource *InstanceBuffer;
        uint64_t         InstanceOffset;
        uint32_t         InstanceCount;
        uint32_t         Flags;
    };

    struct AccelerationStructureBottomLevelDesc
    {
        std::vector<AccelerationStructureGeometryDesc> Geometries;
        uint32_t                                       Flags;
    };

    struct AccelerationStructureTopLevelDesc
    {
        std::vector<AccelerationStructureInstanceDesc> Instances;
        uint32_t                                       Flags;
    };

    struct AccelerationStructureDesc
    {
        AccelerationStructureTopLevelDesc    TopLevelDesc;
        AccelerationStructureBottomLevelDesc BottomLevelDesc;
    };

    class IRayTracingAccelerationStructure
    {
    public:
        virtual ~IRayTracingAccelerationStructure( ) = default;

        virtual void BuildAccelerationStructure( const AccelerationStructureDesc &desc )  = 0;
        virtual void UpdateAccelerationStructure( const AccelerationStructureDesc &desc ) = 0;
    };
} // namespace DenOfIz
