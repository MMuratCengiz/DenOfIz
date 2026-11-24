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

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/UI/UI.h"

namespace DenOfIz
{
    struct UIShapeVertex
    {
        float X;
        float Y;
        float Coverage;
    };

    struct UIShape
    {
        std::vector<UIShapeVertex> Vertices;
        std::vector<uint32_t>      Indices;
        uint64_t                   LastUsedFrame = 0;
    };

    struct UIShapeKey
    {
        float             Width;
        float             Height;
        DenOfIz_UICorners Radius;
        DenOfIz_UIEdges   Border;
        float             Feather;
        bool              IsBorder;

        bool operator==( const UIShapeKey &other ) const;
    };

    struct UIShapeKeyHash
    {
        size_t operator( )( const UIShapeKey &key ) const;
    };

    class UIGeometry
    {
    public:
        static void Fill( float width, float height, const DenOfIz_UICorners &radius, float feather, UIShape &outShape );
        static void Border( float width, float height, const DenOfIz_UIEdges &border, const DenOfIz_UICorners &radius, float feather, UIShape &outShape );
    };

    class UIShapeCache
    {
        std::unordered_map<UIShapeKey, UIShape, UIShapeKeyHash> m_shapes;

    public:
        const UIShape &GetFill( float width, float height, const DenOfIz_UICorners &radius, float feather, uint64_t frame );
        const UIShape &GetBorder( float width, float height, const DenOfIz_UIEdges &border, const DenOfIz_UICorners &radius, float feather, uint64_t frame );
        void           Evict( uint64_t frame, uint64_t maxAge );
        void           Clear( );
    };
} // namespace DenOfIz
