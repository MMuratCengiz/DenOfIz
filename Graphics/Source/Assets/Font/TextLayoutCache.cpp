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

#include <DenOfIzGraphics/Assets/Font/TextLayoutCache.h>

using namespace DenOfIz;

TextLayout *TextLayoutCache::GetOrCreate( const uint64_t textHash, const uint16_t fontId, const uint32_t fontSize, Font *font, const char *text, const size_t length,
                                          const uint32_t currentFrame )
{
    TextShapeCacheKey key{ };
    key.textHash  = textHash;
    key.fontId    = fontId;
    key.fontSize  = fontSize;
    key.direction = TextDirection::Auto;
    key.scriptTag = UInt32_4{ 'L', 'a', 't', 'n' };

    const auto it = m_cache.find( key );
    if ( it != m_cache.end( ) )
    {
        it->second->lastUsedFrame = currentFrame;
        return it->second->layout.get( );
    }

    auto cachedLayout           = std::make_unique<CachedLayout>( );
    cachedLayout->layout        = std::make_unique<TextLayout>( TextLayoutDesc{ font } );
    cachedLayout->lastUsedFrame = currentFrame;

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text        = InteropString( text, length );
    shapeDesc.FontSize    = fontSize;
    shapeDesc.Direction   = key.direction;
    shapeDesc.HbScriptTag = key.scriptTag;

    cachedLayout->layout->ShapeText( shapeDesc );

    TextLayout *layoutPtr = cachedLayout->layout.get( );
    m_cache[ key ]        = std::move( cachedLayout );
    return layoutPtr;
}

TextShapeCacheKey TextLayoutCache::CreateKey( const char *text, const size_t length, const uint16_t fontId, const uint32_t fontSize, const TextDirection direction ) const
{
    TextShapeCacheKey key{ };
    key.textHash  = HashString( text, length );
    key.fontId    = fontId;
    key.fontSize  = fontSize;
    key.direction = direction;
    key.scriptTag = UInt32_4{ 'L', 'a', 't', 'n' };
    return key;
}

void TextLayoutCache::Cleanup( const uint32_t currentFrame, const uint32_t maxAge )
{
    auto it = m_cache.begin( );
    while ( it != m_cache.end( ) )
    {
        if ( currentFrame - it->second->lastUsedFrame > maxAge )
        {
            it = m_cache.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

void TextLayoutCache::Clear( )
{
    m_cache.clear( );
}

size_t TextLayoutCache::GetCacheSize( ) const
{
    return m_cache.size( );
}

uint64_t TextLayoutCache::HashString( const char *str, const size_t length )
{
    // Simple FNV-1a hash
    uint64_t hash = 14695981039346656037ULL;
    for ( size_t i = 0; i < length; ++i )
    {
        hash ^= static_cast<uint64_t>( str[ i ] );
        hash *= 1099511628211ULL;
    }
    return hash;
}
