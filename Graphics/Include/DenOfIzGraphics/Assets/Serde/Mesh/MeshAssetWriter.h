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
        None,
        Vertices,
        Indices,
    };
    struct DZ_API MeshAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API MeshAssetWriter
    {
        BinaryWriter       *m_writer;
        ActiveMeshStream    m_activeStream;
        MeshAssetWriterDesc m_desc;

    public:
        explicit MeshAssetWriter( const MeshAssetWriterDesc &desc );
        ~MeshAssetWriter( );

        void WriteMeshData( const MeshAsset &meshData );
        // A stream can only be activated once
        void SetActiveMeshStream( ActiveMeshStream stream );
        void AddVertex( const MeshVertex &vertex ) const;
        void AddIndex( uint32_t index ) const;
    };
} // namespace DenOfIz
