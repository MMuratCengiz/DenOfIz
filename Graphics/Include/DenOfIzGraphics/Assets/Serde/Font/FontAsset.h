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
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    struct DZ_API GlyphMetrics
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
    template class DZ_API InteropArray<GlyphMetrics>;

    struct DZ_API FontMetrics
    {
        uint32_t Ascent;
        uint32_t Descent;
        uint32_t LineGap;
        uint32_t LineHeight;
        uint32_t UnderlinePos;
        uint32_t UnderlineThickness;
    };

    struct DZ_API FontAsset : public AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString              FontPath;
        uint32_t                   PixelSize;
        bool                       AntiAliasing;
        uint32_t                   AtlasWidth;
        uint32_t                   AtlasHeight;
        uint32_t                   AtlasBitmapNumBytes;
        FontMetrics                Metrics;
        InteropArray<GlyphMetrics> GlyphData;
        InteropArray<UserProperty> UserProperties;
        AssetDataStream            AtlasBitmap;

        FontAsset( ) : AssetHeader( 0x544E4F465A44 /*DZFONT*/, Latest, 0 )
        {
            PixelSize       = 24;
            AntiAliasing    = true;
            AtlasWidth      = 512;
            AtlasHeight     = 512;
            AtlasBitmapNumBytes = 0;

            // Initialize metrics with default values
            Metrics.Ascent             = 0;
            Metrics.Descent            = 0;
            Metrics.LineGap            = 0;
            Metrics.LineHeight         = 0;
            Metrics.UnderlinePos       = 0;
            Metrics.UnderlineThickness = 0;
        }
    };
} // namespace DenOfIz
