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

#include <DenOfIzGraphics/Assets/Font/Font.h>
#include <DenOfIzGraphics/Assets/Font/TextLayout.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <memory>
#include <unordered_map>

// buggy here for some reason
// clang-format off
namespace DenOfIz
{
    struct DZ_API TextShapeCacheKey
    {
        uint64_t      textHash;
        uint16_t      fontId;
        uint32_t      fontSize;
        TextDirection direction;
        UInt32_4      scriptTag;

        bool operator==( const TextShapeCacheKey &other ) const
        {
            return textHash == other.textHash && fontId == other.fontId && fontSize == other.fontSize && direction == other.direction && scriptTag.X == other.scriptTag.X &&
                   scriptTag.Y == other.scriptTag.Y && scriptTag.Z == other.scriptTag.Z && scriptTag.W == other.scriptTag.W;
        }
    };

    struct DZ_API TextShapeCacheKeyHash{ std::size_t operator( )( const TextShapeCacheKey &key ) const
    {
        const std::size_t h1 = std::hash<uint64_t>{ }( key.textHash );
        const std::size_t h2 = std::hash<uint16_t>{ }( key.fontId );
        const std::size_t h3 = std::hash<uint32_t>{ }( key.fontSize );
        return h1 ^ h2 << 1 ^ h3 << 2;
} // namespace DenOfIz
}
;
// clang-format on

/// Caches frequently used text, TextLayout is a cheap class in general so we keep track of dimensions of every text we come across,
class DZ_API TextLayoutCache
{
public:
    TextLayoutCache( )  = default;
    ~TextLayoutCache( ) = default;

    TextLayoutCache( const TextLayoutCache & )             = delete;
    TextLayoutCache  &operator=( const TextLayoutCache & ) = delete;
    TextLayout       *GetOrCreate( uint64_t textHash, uint16_t fontId, uint32_t fontSize, Font *font, const char *text, size_t length, uint32_t currentFrame );
    TextShapeCacheKey CreateKey( const char *text, size_t length, uint16_t fontId, uint32_t fontSize, TextDirection direction = TextDirection::Auto ) const;

    void Cleanup( uint32_t currentFrame, uint32_t maxAge = 300 );
    void Clear( );

    size_t          GetCacheSize( ) const;
    static uint64_t HashString( const char *str, size_t length );

private:
    struct CachedLayout
    {
        std::unique_ptr<TextLayout> layout;
        uint32_t                    lastUsedFrame = 0;
    };

    std::unordered_map<TextShapeCacheKey, std::unique_ptr<CachedLayout>, TextShapeCacheKeyHash> m_cache;
};

} // namespace DenOfIz
