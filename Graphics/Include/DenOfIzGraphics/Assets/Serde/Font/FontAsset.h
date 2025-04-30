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

namespace DenOfIz
{
    enum class AntiAliasingMode
    {
        None,
        Grayscale,
        Subpixel
    };
    struct DZ_API GlyphBounds
    {
        double XMin = 0;
        double YMin = 0;
        double XMax = 0;
        double YMax = 0;
    };
    struct DZ_API FontGlyph
    {
        uint32_t           CodePoint = 0;
        GlyphBounds        Bounds    = { };
        uint32_t           Width     = 0;
        uint32_t           Height    = 0;
        uint32_t           BearingX  = 0;
        uint32_t           BearingY  = 0;
        uint32_t           XAdvance  = 0;
        uint32_t           YAdvance  = 0;
        uint32_t           AtlasX    = 0;
        uint32_t           AtlasY    = 0;
        uint32_t           Pitch     = 0;
        InteropArray<Byte> Data; // MTSDF, RGBA
    };
    template class DZ_API InteropArray<FontGlyph>;

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
        static constexpr uint32_t Latest      = 1;
        static constexpr uint32_t NumChannels = 4;

        uint64_t                   DataNumBytes = 0;
        InteropArray<Byte>         Data;
        uint32_t                   InitialFontSize;
        AntiAliasingMode           AntiAliasingMode;
        uint32_t                   AtlasWidth;
        uint32_t                   AtlasHeight;
        FontMetrics                Metrics;
        InteropArray<FontGlyph>    Glyphs;
        InteropArray<UserProperty> UserProperties;
        uint64_t                   NumAtlasDataBytes = 0;
        InteropArray<Byte>         AtlasData; // RGBA (MTSDF format)

        FontAsset( ) : AssetHeader( 0x544E4F465A44 /*DZFONT*/, Latest, 0 )
        {
            InitialFontSize = 36;
            AtlasWidth      = 512;
            AtlasHeight     = 512;

            Metrics.Ascent             = 0;
            Metrics.Descent            = 0;
            Metrics.LineGap            = 0;
            Metrics.LineHeight         = 0;
            Metrics.UnderlinePos       = 0;
            Metrics.UnderlineThickness = 0;
        }
    };
} // namespace DenOfIz
