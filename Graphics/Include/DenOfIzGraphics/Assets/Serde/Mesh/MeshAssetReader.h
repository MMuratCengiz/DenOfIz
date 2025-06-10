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

#include <cstdint>
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "MeshAsset.h"

namespace DenOfIz
{

    struct DZ_API LoadToMemoryDesc
    {
        AssetDataStream Stream{ };
        ByteArray       Memory{ };
        uint32_t        DstMemoryOffset{ };
    };

    struct DZ_API MeshAssetReaderDesc
    {
        BinaryReader *Reader;
    };

    class MeshAssetReader
    {
        BinaryReader       *m_reader;
        MeshAssetReaderDesc m_desc;
        MeshAsset           m_meshAsset;
        bool                m_metadataRead         = false;
        uint64_t            m_dataBlockStartOffset = 0;

        SubMeshData                    ReadCompleteSubMeshData( ) const;
        MorphTarget                    ReadCompleteMorphTargetData( ) const;
        BoundingVolume                 ReadBoundingVolume( ) const;
        [[nodiscard]] MeshVertex       ReadSingleVertex( ) const;
        [[nodiscard]] MorphTargetDelta ReadSingleMorphTargetDelta( ) const;

    public:
        DZ_API explicit MeshAssetReader( const MeshAssetReaderDesc &desc );
        DZ_API ~MeshAssetReader( );
        DZ_API MeshAsset                      Read( );
        [[nodiscard]] DZ_API const MeshAsset &GetMetadata( ) const;

        DZ_API void LoadStreamToMemory( LoadToMemoryDesc &desc ) const;

        [[nodiscard]] DZ_API MeshVertexArray       ReadVertices( const AssetDataStream &stream ) const;
        [[nodiscard]] DZ_API UInt16Array           ReadIndices16( const AssetDataStream &stream ) const;
        [[nodiscard]] DZ_API UInt32Array           ReadIndices32( const AssetDataStream &stream ) const;
        [[nodiscard]] DZ_API MorphTargetDeltaArray ReadMorphTargetDeltas( const AssetDataStream &stream ) const;
        [[nodiscard]] DZ_API ByteArray             ReadConvexHullData( const AssetDataStream &stream ) const; // Todo maybe use proper types

        [[nodiscard]] DZ_API uint32_t VertexEntryNumBytes( ) const;
        [[nodiscard]] DZ_API uint32_t MorphDeltaEntryNumBytes( ) const;
    };
} // namespace DenOfIz
