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

#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include "TextureAsset.h"

namespace DenOfIz
{
    struct DZ_API TextureAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API TextureAssetWriter
    {
        BinaryWriter *m_writer;
        uint64_t      m_dataFieldOffset = 0;
        uint64_t      m_streamOffset    = 0;
        uint64_t      m_dataNumBytes    = 0;

    public:
        explicit TextureAssetWriter( const TextureAssetWriterDesc &desc );
        ~TextureAssetWriter( );

        void Write( const TextureAsset &textureAsset );
        void AddDataBytes( const InteropArray<Byte> &bytes );
        void Finalize( ) const;
    };
} // namespace DenOfIz
