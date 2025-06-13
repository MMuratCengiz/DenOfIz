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

#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "TextureAsset.h"

#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"

namespace DenOfIz
{
    struct DZ_API TextureAssetReaderDesc
    {
        BinaryReader *Reader;
    };

    struct DZ_API LoadIntoGpuTextureDesc
    {
        ICommandList     *CommandList;
        IBufferResource  *StagingBuffer; // Should be large enough to hold all mip layers
        ITextureResource *Texture;
    };

    class TextureAssetReader
    {
        BinaryReader *m_reader;
        TextureAsset *m_textureAsset;
        bool          m_textureRead = false;

        TextureMip FindMip( const uint32_t mipLevel, const uint32_t arrayLayer ) const;

    public:
        DZ_API explicit TextureAssetReader( const TextureAssetReaderDesc &desc );
        DZ_API ~TextureAssetReader( );

        DZ_API TextureAsset *Read( );
        DZ_API void          LoadIntoGpuTexture( const LoadIntoGpuTextureDesc &desc ) const;
        DZ_API ByteArray     ReadRaw( const uint32_t mipLevel = 0, const uint32_t arrayLayer = 0 ) const;
        DZ_API uint64_t      AlignedTotalNumBytes( const DeviceConstants &constants ) const;
    };
} // namespace DenOfIz
