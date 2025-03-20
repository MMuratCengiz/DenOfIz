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
#include "MeshAsset.h"

#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>

namespace DenOfIz
{
    enum class ActiveMeshStream
    {
        Vertices,
        Indices,
        MorphPositionDeltas,
        MorphNormalDeltas,
        MorphTangentDeltas
    };
    struct DZ_API MeshWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API MeshWriter
    {
        BinaryWriter *m_writer;

    public:
        explicit MeshWriter( const MeshWriterDesc &desc );
        ~MeshWriter( );

        void WriteMeshData( const MeshData &meshData );
        // A stream can only be activated once
        void SetActiveMeshStream( ActiveMeshStream stream );
        void AddVertex( const MeshVertex &vertex );
        void AddIndex( uint32_t index );
        void AddMorphPositionDelta( const Float3 &vertexDeltas );
        void AddMorphNormalDelta( const Float3 &normalDeltas );
        void AddMorphTangentDelta( const Float3 &tangentDeltas );
    };
} // namespace DenOfIz
