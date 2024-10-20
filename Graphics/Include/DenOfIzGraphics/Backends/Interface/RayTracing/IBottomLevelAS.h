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

    enum class ASGeometryType
    {
        Triangles,
        AABBs
    };

    struct DZ_API ASGeometryDesc
    {
        ASGeometryType   Type;
        IBufferResource *VertexBuffer;
        uint32_t         VertexOffset;
        uint32_t         VertexStride;
        uint32_t         NumVertices;
        Format           VertexFormat;
        IBufferResource *IndexBuffer;
        uint32_t         IndexOffset;
        uint32_t         NumIndices;
        IndexType        IndexType;
        uint32_t         PrimitiveCount;
        bool             IsOpaque;
    };
    template class DZ_API InteropArray<ASGeometryDesc>;

    struct DZ_API BottomLevelASDesc
    {
        InteropArray<ASGeometryDesc> Geometries;
        uint32_t                     Flags;
        ASBuildFlags                 BuildFlags;
    };

    class DZ_API IBottomLevelAS
    {
    public:
        virtual ~IBottomLevelAS( )                           = default;
        virtual void Update( const BottomLevelASDesc &desc ) = 0;
    };
} // namespace DenOfIz
