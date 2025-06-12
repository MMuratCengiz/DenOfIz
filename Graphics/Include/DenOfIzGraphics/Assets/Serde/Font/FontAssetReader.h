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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"

namespace DenOfIz
{
    struct DZ_API FontAssetReaderDesc
    {
        BinaryReader *Reader;
    };

    struct DZ_API LoadAtlasIntoGpuTextureDesc
    {
        ILogicalDevice   *Device;
        ICommandList     *CommandList;
        IBufferResource  *StagingBuffer;
        ITextureResource *Texture;
    };

    class FontAssetReader
    {
        BinaryReader *m_reader;
        FontAsset    *m_fontAsset;
        bool          m_assetRead         = false;
        uint64_t      m_streamStartOffset = 0;

    public:
        DZ_API explicit FontAssetReader( const FontAssetReaderDesc &desc );
        DZ_API ~FontAssetReader( );

        DZ_API FontAsset  *Read( );
        DZ_API static void LoadAtlasIntoGpuTexture( const FontAsset &fontAsset, const LoadAtlasIntoGpuTextureDesc &desc );
    };
} // namespace DenOfIz
