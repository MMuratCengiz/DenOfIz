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
    struct DZ_API MeshWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API MeshWriter
    {
        BinaryWriter         *m_writer;
        uint64_t              m_streamOffset;
        uint32_t              m_currentVertexCount;
        uint32_t              m_currentVertexIndex;
        uint32_t              m_currentIndexCount;
        uint32_t              m_currentIndexIndex;
        uint32_t              m_currentSubMeshIndex;
        uint32_t              m_currentLODLevel;
        bool                  m_activeStreamMode;
        std::vector<uint64_t> m_streamOffsets; // Tracks offsets for each stream

        void WriteMorphTarget( const MorphTarget &morphTarget );
        void WriteJointData( const JointData &jointData );
        void WriteUserProperty( const UserProperty &property );
        void WriteBoundingVolume( const BoundingVolume &boundingVolume );

        void                   WriteSubMeshHeader( const SubMeshData &subMesh, bool placeholder = false );
        [[nodiscard]] uint32_t CalculateVertexStride( const SubMeshData &subMesh ) const;
        [[nodiscard]] uint32_t CalculateSubMeshHeaderSize( const SubMeshData &subMesh ) const;
        [[nodiscard]] uint32_t CalculateSubMeshHeaderBaseSize( ) const;

    public:
        explicit MeshWriter( const MeshWriterDesc &desc );
        ~MeshWriter( );

        void WriteMeshData( const MeshData &meshData );

        void StartVertexStream( const SubMeshData &subMesh );
        void EndVertexStream( );
        void StartIndexStream( const SubMeshData &subMesh );
        void EndIndexStream( );

        void BeginWriteStream( AssetDataStream &stream );
        void EndWriteStream( );
        void WriteStreamData( const AssetDataStream &stream, const InteropArray<Byte> &data );

        void WriteMeshVertex( const MeshVertex &vertexData );
        void WriteIndex( uint32_t index );

        void BeginLODLevel( uint32_t level );
        void EndLODLevel( );

        void WriteSubMeshInstance( const SubMeshInstance &instance );

        void WritePositionStream( const AssetDataStream &stream, const InteropArray<Float3> &positions );
        void WriteNormalStream( const AssetDataStream &stream, const InteropArray<Float3> &normals );
        void WriteTangentStream( const AssetDataStream &stream, const InteropArray<Float4> &tangents );

        [[nodiscard]] uint32_t GetCurrentVertexIndex( ) const
        {
            return m_currentVertexIndex;
        }
        [[nodiscard]] uint32_t GetCurrentVertexCount( ) const
        {
            return m_currentVertexCount;
        }
        [[nodiscard]] uint32_t GetCurrentIndexCount( ) const
        {
            return m_currentIndexCount;
        }
        [[nodiscard]] uint32_t GetCurrentSubMeshIndex( ) const
        {
            return m_currentSubMeshIndex;
        }
    };
} // namespace DenOfIz
