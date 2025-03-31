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

#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include "MeshAsset.h"

namespace DenOfIz
{
    struct DZ_API LoadToBufferDesc
    {
        AssetDataStream    Stream{ };
        BatchResourceCopy *BatchCopy = nullptr;
        IBufferResource   *Buffer{ };
        uint32_t           DstBufferOffset{ };
    };

    struct DZ_API LoadToMemoryDesc
    {
        AssetDataStream     Stream{ };
        InteropArray<Byte> *Memory{ };
        uint32_t            DstMemoryOffset{ };
    };

    struct DZ_API MeshAssetReaderDesc
    {
        BinaryReader *Reader;
    };

    class DZ_API MeshAssetReader
    {
        BinaryReader       *m_reader;
        MeshAssetReaderDesc m_desc;
        MeshAsset           m_meshData;
        bool                m_metadataRead         = false;
        uint64_t            m_dataBlockStartOffset = 0;

        SubMeshData                    ReadCompleteSubMeshData( ) const;
        MorphTarget                    ReadCompleteMorphTargetData( ) const;
        UserProperty                   ReadCompleteUserPropertyData( ) const;
        BoundingVolume                 ReadBoundingVolume( ) const;
        void                           ReadUserPropertyContent( UserProperty &prop ) const;
        [[nodiscard]] MeshVertex       ReadSingleVertex( ) const;
        [[nodiscard]] MorphTargetDelta ReadSingleMorphTargetDelta( ) const;

    public:
        explicit MeshAssetReader( const MeshAssetReaderDesc &desc );
        ~MeshAssetReader( );
        MeshAsset                      ReadMetadata( );
        [[nodiscard]] const MeshAsset &GetMetadata( ) const;

        // Load Raw Bytes
        void LoadStreamToBuffer( const LoadToBufferDesc &desc ) const;
        void LoadStreamToMemory( const LoadToMemoryDesc &desc ) const;

        // Read parsed
        [[nodiscard]] InteropArray<MeshVertex>       ReadVertices( const AssetDataStream &stream ) const;
        [[nodiscard]] InteropArray<uint16_t>         ReadIndices16( const AssetDataStream &stream ) const;
        [[nodiscard]] InteropArray<uint32_t>         ReadIndices32( const AssetDataStream &stream ) const;
        [[nodiscard]] InteropArray<MorphTargetDelta> ReadMorphTargetDeltas( const AssetDataStream &stream ) const;
        [[nodiscard]] InteropArray<Byte>             ReadConvexHullData( const AssetDataStream &stream ) const; // Todo maybe use proper types

        // Number of bytes of a single Vertex/MorphDelta
        [[nodiscard]] uint32_t         VertexEntryNumBytes( ) const;
        [[nodiscard]] uint32_t         MorphDeltaEntryNumBytes( ) const;
    };
} // namespace DenOfIz
