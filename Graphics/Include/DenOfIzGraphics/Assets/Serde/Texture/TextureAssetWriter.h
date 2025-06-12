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

#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "TextureAsset.h"

namespace DenOfIz
{
    struct DZ_API TextureAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class TextureAssetWriter
    {
        BinaryWriter                 *m_writer;
        TextureAssetWriterDesc        m_desc;
        std::unique_ptr<TextureAsset> m_textureAsset;
        uint64_t                      m_assetDataStreamPosition = 0;
        std::vector<uint64_t>         m_textureMipPositions;

        uint64_t m_streamStartLocation = 0;
        uint32_t m_lastMipIndex        = 0;
        uint32_t m_lastArrayIndex      = 0;
        bool     m_isFirstMip          = true;

        void WriteHeader( uint64_t totalNumBytes ) const;
        void WriteMipInfo( const TextureMip &mip ) const;
        void ValidateMipRange( uint32_t mipIndex, uint32_t arrayLayer );

    public:
        DZ_API explicit TextureAssetWriter( const TextureAssetWriterDesc &desc );
        DZ_API ~TextureAssetWriter( );

        DZ_API void Write( const TextureAsset &textureAsset );
        // Stream write bytes to a specific mip & array level, could be called multiple times for the same mip & array level
        DZ_API void AddPixelData( const ByteArrayView &bytes, uint32_t mipIndex = 0, uint32_t arrayLayer = 0 );
        DZ_API void End( ) const;
    };
} // namespace DenOfIz
