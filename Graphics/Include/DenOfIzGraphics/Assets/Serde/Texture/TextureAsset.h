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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    enum class TextureDimension
    {
        Undefined,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
    };

    struct DZ_API TextureMip
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t MipIndex;
        uint32_t ArrayIndex;
        uint32_t RowPitch;
        uint32_t NumRows;
        uint32_t SlicePitch;
        uint32_t DataOffset; // Offset starting for the beginning of the stream
    };

    struct DZ_API TextureMipArray
    {
        TextureMip *Elements;
        size_t      NumElements;
    };

    struct DZ_API TextureAsset : AssetHeader, NonCopyable
    {
        DZArena _Arena{ sizeof( TextureAsset ) };

        static constexpr uint32_t Latest = 1;

        InteropString Name;
        InteropString SourcePath;

        uint32_t         Width     = 0;
        uint32_t         Height    = 0;
        uint32_t         Depth     = 1;
        Format           Format    = Format::Undefined;
        TextureDimension Dimension = TextureDimension::Texture2D;

        uint32_t MipLevels = 1;
        uint32_t ArraySize = 1;

        uint32_t BitsPerPixel = 0;
        uint32_t BlockSize    = 1;

        uint32_t RowPitch   = 0;
        uint32_t NumRows    = 0;
        uint32_t SlicePitch = 0;

        TextureMipArray Mips;
        AssetDataStream Data;

        TextureAsset( ) : AssetHeader( 0x445A544558 /* 'DZTEX' */, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dztex";
        }
    };
} // namespace DenOfIz
