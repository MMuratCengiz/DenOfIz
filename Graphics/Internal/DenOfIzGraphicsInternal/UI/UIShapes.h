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

#include <clay.h>
#include <vector>
#include "DenOfIzGraphicsInternal/UI/IClayContext.h"

namespace DenOfIz
{

    class UIShapes
    {
    public:
        struct GenerateRectangleDesc
        {
            Clay_BoundingBox Bounds;
            Clay_Color       Color;
            uint32_t         TextureIndex = 0; // 0 is solid color
        };

        struct GenerateRoundedRectangleDesc
        {
            Clay_BoundingBox  Bounds;
            Clay_Color        Color;
            Clay_CornerRadius CornerRadius;
            uint32_t          TextureIndex      = 0;
            uint32_t          SegmentsPerCorner = 8;
        };

        struct GenerateBorderDesc
        {
            Clay_BoundingBox  Bounds;
            Clay_Color        Color;
            Clay_BorderWidth  BorderWidth;
            Clay_CornerRadius CornerRadius;
            uint32_t          SegmentsPerCorner = 8;
        };

        static void GenerateRectangle( const GenerateRectangleDesc &desc, std::vector<UIVertex> *outVertices, std::vector<uint32_t> *outIndices, uint32_t baseVertex = 0 );
        static void GenerateRoundedRectangle( const GenerateRoundedRectangleDesc &desc, std::vector<UIVertex> *outVertices, std::vector<uint32_t> *outIndices,
                                              uint32_t baseVertex = 0 );
        static void GenerateBorder( const GenerateBorderDesc &desc, std::vector<UIVertex> *outVertices, std::vector<uint32_t> *outIndices, uint32_t baseVertex = 0 );
    };

} // namespace DenOfIz
