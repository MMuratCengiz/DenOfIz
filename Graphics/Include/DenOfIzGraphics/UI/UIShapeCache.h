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

#include <DenOfIzGraphics/UI/UIShapes.h>
#include <clay.h>
#include <memory>
#include <unordered_map>

namespace DenOfIz
{
    // Intentionally Not DZ_API internal use
    struct ShapeCacheKey
    {
        float    x, y, width, height;
        uint32_t colorRGBA;
        uint32_t textureIndex;
        float    cornerRadius[ 4 ]; // tl, tr, br, bl
        float    borderWidth[ 4 ];  // top, right, bottom, left
        uint32_t shapeType;         // 0=rectangle, 1=border

        bool operator==( const ShapeCacheKey &other ) const
        {
            return x == other.x && y == other.y && width == other.width && height == other.height && colorRGBA == other.colorRGBA && textureIndex == other.textureIndex &&
                   cornerRadius[ 0 ] == other.cornerRadius[ 0 ] && cornerRadius[ 1 ] == other.cornerRadius[ 1 ] && cornerRadius[ 2 ] == other.cornerRadius[ 2 ] &&
                   cornerRadius[ 3 ] == other.cornerRadius[ 3 ] && borderWidth[ 0 ] == other.borderWidth[ 0 ] && borderWidth[ 1 ] == other.borderWidth[ 1 ] &&
                   borderWidth[ 2 ] == other.borderWidth[ 2 ] && borderWidth[ 3 ] == other.borderWidth[ 3 ] && shapeType == other.shapeType;
        }
    };

    struct ShapeCacheKeyHash
    {
        std::size_t operator( )( const ShapeCacheKey &key ) const
        {
            const std::size_t h1 = std::hash<float>{ }( key.x );
            const std::size_t h2 = std::hash<float>{ }( key.y );
            const std::size_t h3 = std::hash<float>{ }( key.width );
            const std::size_t h4 = std::hash<float>{ }( key.height );
            const std::size_t h5 = std::hash<uint32_t>{ }( key.colorRGBA );
            const std::size_t h6 = std::hash<uint32_t>{ }( key.textureIndex );
            const std::size_t h7 = std::hash<uint32_t>{ }( key.shapeType );
            return h1 ^ h2 << 1 ^ h3 << 2 ^ h4 << 3 ^ h5 << 4 ^ h6 << 5 ^ h7 << 6;
        }
    };

    struct CachedShape
    {
        InteropArray<UIVertex> vertices;
        InteropArray<uint32_t> indices;
        uint32_t               lastUsedFrame = 0;
    };

    class UIShapeCache
    {
    public:
        UIShapeCache( )  = default;
        ~UIShapeCache( ) = default;

        CachedShape *GetOrCreateCachedShape( const ShapeCacheKey &key, uint32_t currentFrame );
        void         Cleanup( uint32_t currentFrame, uint32_t maxAge = 1024 );
        void         Clear( );
        size_t       GetCacheSize( ) const;

        static ShapeCacheKey CreateRectangleKey( const Clay_RenderCommand *command );
        static ShapeCacheKey CreateBorderKey( const Clay_RenderCommand *command );
        static uint32_t      ColorToRGBA( const Clay_Color &color );

    private:
        std::unordered_map<ShapeCacheKey, std::unique_ptr<CachedShape>, ShapeCacheKeyHash> m_cache;
    };

} // namespace DenOfIz
