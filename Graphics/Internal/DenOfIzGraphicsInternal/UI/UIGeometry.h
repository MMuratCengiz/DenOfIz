/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
