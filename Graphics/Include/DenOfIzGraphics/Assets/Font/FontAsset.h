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

#include <DenOfIzGraphics/Assets/Serde/Asset.h>

#include "DenOfIzGraphics/Assets/Bundle/Bundle.h"

namespace DenOfIz
{
    struct GlyphMetrics
    {
        uint32_t CodePoint;
        uint32_t Width;
        uint32_t Height;
        uint32_t BearingX;
        uint32_t BearingY;
        uint32_t Advance;
        uint32_t AtlasX;
        uint32_t AtlasY;
    };

    struct FontAsset : public AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString                              FontPath;
        uint32_t                                   PixelSize;
        bool                                       AntiAliasing;
        uint32_t                                   AtlasWidth;
        uint32_t                                   AtlasHeight;
        std::vector<uint8_t>                       AtlasBitmap;
        std::unordered_map<uint32_t, GlyphMetrics> GlyphCache;

        FontAsset( ) : AssetHeader( 0x544E4F465A44 /*DZFONT*/, Latest, 0 )
        {
            PixelSize    = 24;
            AntiAliasing = true;
            AtlasWidth   = 512;
            AtlasHeight  = 512;
        }

        void ReserveAtlasBitmap( )
        {
            AtlasBitmap.resize( AtlasWidth * AtlasHeight, 0 );
        }

        void ClearAtlasBitmap( )
        {
            std::memset( AtlasBitmap.data( ), 0, AtlasBitmap.size( ) );
        }
    };
} // namespace DenOfIz
