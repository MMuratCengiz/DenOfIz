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
#include "DenOfIzGraphics/UI/IClayContext.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

#include <DirectXMath.h>

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

        static void GenerateRectangle( const GenerateRectangleDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices, uint32_t baseVertex = 0 );
        static void GenerateRoundedRectangle( const GenerateRoundedRectangleDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices,
                                              uint32_t baseVertex = 0 );
        static void GenerateBorder( const GenerateBorderDesc &desc, InteropArray<UIVertex> *outVertices, InteropArray<uint32_t> *outIndices, uint32_t baseVertex = 0 );

    private:
        static DirectX::XMFLOAT4 ClayColorToFloat4( const Clay_Color &color );

        static void AddVertex( InteropArray<UIVertex> *vertices, float x, float y, float z, float u, float v, const DirectX::XMFLOAT4 &color, uint32_t textureIndex );
        static void AddTriangle( InteropArray<uint32_t> *indices, uint32_t v0, uint32_t v1, uint32_t v2 );
        static void AddQuad( InteropArray<uint32_t> *indices, uint32_t topLeft, uint32_t topRight, uint32_t bottomRight, uint32_t bottomLeft );
    };

} // namespace DenOfIz
