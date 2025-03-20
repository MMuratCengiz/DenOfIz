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

namespace DenOfIz
{
    class DZ_API MeshAssetReaderCallback
    {
    public:
        virtual ~MeshAssetReaderCallback( ) = default;

    private:
        virtual void OnVertex( const MeshVertex &meshVertex )
        {
        }
        virtual void OnIndex( uint32_t index )
        {
        }
        virtual void OnMorphPositionDeltas( const Float3 &positionDeltas )
        {
        }
        virtual void OnMorphNormalDeltas( const Float3 &normalDeltas )
        {
        }
        virtual void OnMorphTangentDeltas( const Float3 &tangentDeltas )
        {
        }
    };

    struct DZ_API MeshAssetReaderDesc
    {
        BinaryReader            *Reader;
        MeshAssetReaderCallback *Callback;
    };

    class DZ_API MeshAssetReader
    {
        BinaryReader *m_reader;
        MeshData      m_meshData;

    public:
        explicit MeshAssetReader( const MeshAssetReaderDesc &desc );
        ~MeshAssetReader( );
        MeshData ReadMeshData( );
    };
} // namespace DenOfIz
