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
#include <memory>
#include <unordered_map>
#include "DenOfIzGraphicsInternal/UI/UIShapes.h"

namespace DenOfIz
{
    // Intentionally Not DZ_API internal use
    // clang-format off
    struct TextVertexCacheKey
    {
        uint64_t textHash;
        uint16_t fontId;
        uint32_t fontSize;
        float    posX, posY;
        uint32_t colorRGBA;
        float    letterSpacing;
        float    lineHeight;
        float    effectiveScale;

        bool operator==(const TextVertexCacheKey &other) const
        {
            return textHash == other.textHash && fontId == other.fontId && fontSize == other.fontSize &&
                   posX == other.posX && posY == other.posY && colorRGBA == other.colorRGBA &&
                   letterSpacing == other.letterSpacing && lineHeight == other.lineHeight &&
                   effectiveScale == other.effectiveScale;
        }
    };

    struct TextVertexCacheKeyHash
    {
        std::size_t operator()(const TextVertexCacheKey &key) const
        {
            const std::size_t h1 = std::hash<uint64_t>{ }( key.textHash );
            const std::size_t h2 = std::hash<uint16_t>{ }( key.fontId );
            const std::size_t h3     = std::hash<uint32_t>{ }( key.fontSize );
            const std::size_t h4     = std::hash<float>{ }( key.posX );
            const std::size_t h5     = std::hash<float>{ }( key.posY );
            const std::size_t h6     = std::hash<uint32_t>{ }( key.colorRGBA );
            const std::size_t h7 = std::hash<float>{}(key.effectiveScale);
            return h1 ^ h2 << 1 ^ h3 << 2 ^ h4 << 3 ^ h5 << 4 ^ h6 << 5 ^ h7 << 6;
        }
    };

    struct CachedTextVertices
    {
        InteropArray<UIVertex> vertices;
        InteropArray<uint32_t> indices;
        uint32_t               lastUsedFrame = 0;
    };
    // clang-format on

    class UITextVertexCache
    {
    public:
        UITextVertexCache( )  = default;
        ~UITextVertexCache( ) = default;

        CachedTextVertices *GetOrCreateCachedTextVertices( const TextVertexCacheKey &key, uint32_t currentFrame );
        void                Cleanup( uint32_t currentFrame, uint32_t maxAge = 120 );
        void                Clear( );
        size_t              GetCacheSize( ) const;

        static TextVertexCacheKey CreateTextVertexKey( const Clay_RenderCommand *command, float effectiveScale, float adjustedY, float dpiScale );
        static uint32_t           ColorToRGBA( const Clay_Color &color );

    private:
        std::unordered_map<TextVertexCacheKey, std::unique_ptr<CachedTextVertices>, TextVertexCacheKeyHash> m_cache;
    };

} // namespace DenOfIz
