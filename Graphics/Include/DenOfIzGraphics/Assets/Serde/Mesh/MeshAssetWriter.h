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
    struct DZ_API MeshAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API MeshAssetWriter
    {
        BinaryWriter       *m_writer;
        MeshAssetWriterDesc m_desc;

    public:
        explicit MeshAssetWriter( const MeshAssetWriterDesc &desc );
        ~MeshAssetWriter( );

        /// Write general metadata for the MeshAsset, must be written first
        void WriteMetadata( const MeshAsset &meshData );

        void AddVertex( uint32_t subMeshIndex, const MeshVertex &vertex );
        void AddIndex16( uint32_t subMeshIndex, uint16_t index );
        void AddIndex32( uint32_t subMeshIndex, uint32_t index );
        void AddMorphTargetDelta( uint32_t morphTargetIndex, const MorphTargetDelta &vertexDelta );

        void Finalize( );
    };
} // namespace DenOfIz
