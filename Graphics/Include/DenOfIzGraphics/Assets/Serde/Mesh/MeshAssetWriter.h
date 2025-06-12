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

#include <map>
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "MeshAsset.h"

namespace DenOfIz
{
    struct DZ_API MeshAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class MeshAssetWriter
    {
        BinaryWriter       *m_writer;
        MeshAssetWriterDesc m_desc;
        MeshAsset const    *m_meshAsset;

        enum class State
        {
            Idle,
            ReadyToWriteData,
            WritingVertices,
            WritingIndices,
            WritingHulls,
            WritingDeltas,
            DataWritten,
            Finalized,
            SubMeshEnded,
            ExpectingMorphTarget,
            ExpectingIndices,
            ExpectingHulls
        };
        State m_state = State::Idle;

        uint64_t m_streamStartLocation      = 0;
        uint32_t m_expectedSubMeshCount     = 0;
        uint32_t m_expectedMorphTargetCount = 0;
        uint32_t m_writtenSubMeshCount{ };
        uint32_t m_writtenMorphTargetCount{ };

        uint32_t m_currentSubMeshIndex     = 0;
        uint32_t m_currentMorphTargetIndex = 0;
        uint32_t m_currentBVIndex          = 0;

        uint64_t m_numVertices = 0;
        uint64_t m_numIndices  = 0;
        uint64_t m_numDeltas   = 0;

        uint32_t m_vertexStride     = 0;
        uint32_t m_morphDeltaStride = 0;

        void CalculateStrides( );
        void WriteHeader( uint64_t totalNumBytes );
        void WriteTopLevelMetadata( );
        void WriteMetadataArrays( );
        void WriteSubMeshData( const SubMeshData &data ) const;
        void WriteMorphTargetData( const MorphTarget &data ) const;
        void WriteBoundingVolume( const BoundingVolume &bv ) const;
        void WriteVertexInternal( const MeshVertex &vertex ) const;
        void WriteMorphTargetDeltaInternal( const MorphTargetDelta &delta ) const;

    public:
        DZ_API explicit MeshAssetWriter( const MeshAssetWriterDesc &desc );
        DZ_API ~MeshAssetWriter( );

        DZ_API void Write( const MeshAsset &meshAssetData );

        DZ_API void AddVertex( const MeshVertex &vertex );
        DZ_API void AddIndex16( uint16_t index );
        DZ_API void AddIndex32( uint32_t index );
        DZ_API void AddConvexHullData( uint32_t boundingVolumeIndex, const ByteArrayView &vertexData );
        DZ_API void AddMorphTargetDelta( const MorphTargetDelta &delta );

        DZ_API void FinalizeAsset( );
    };
} // namespace DenOfIz
