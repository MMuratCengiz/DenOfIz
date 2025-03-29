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
#include "MeshAsset.h"

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>

namespace DenOfIz
{
    struct DZ_API LoadToBufferDesc
    {
        AssetDataStream  Stream{ };
        IBufferResource *Buffer{ };
        uint32_t         Offset{ };
    };

    struct DZ_API LoadToMemoryDesc
    {
        AssetDataStream     Stream{ };
        InteropArray<Byte> *Memory{ };
        uint32_t            Offset{ };
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

    public:
        explicit MeshAssetReader( const MeshAssetReaderDesc &desc );
        ~MeshAssetReader( );
        MeshAsset ReadMetadata( );

        void LoadStreamToBuffer( const LoadToBufferDesc &desc );
        void LoadStreamToMemory( const LoadToMemoryDesc &desc );

        InteropArray<MeshVertex>       ReadVertices( const AssetDataStream &stream );
        InteropArray<uint32_t>         ReadIndices( const AssetDataStream &stream );
        InteropArray<MorphTargetDelta> ReadMorphTargets( const AssetDataStream &stream );
    };
} // namespace DenOfIz
