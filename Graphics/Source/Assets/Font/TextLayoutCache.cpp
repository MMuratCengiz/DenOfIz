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

#include "DenOfIzGraphicsInternal/Assets/Font/TextLayoutCache.h"

using namespace DenOfIz;

TextLayout *TextLayoutCache::GetOrCreate( const uint64_t textHash, const uint16_t fontId, const uint32_t fontSize, Font *font, const DenOfIz_StringView &text )
{
    TextShapeCacheKey key{ };
    key.textHash  = textHash;
    key.fontId    = fontId;
    key.fontSize  = fontSize;
    key.direction = DENOFIZ_TEXT_DIRECTION_AUTO;
    key.scriptTag = DenOfIz_UInt4{ 'L', 'a', 't', 'n' };

    const auto it = m_cache.find( key );
    if ( it != m_cache.end( ) )
    {
        it->second.hitCount++;
        it->second.lastAccessOrder = ++m_accessCounter;
        return it->second.layout.get( );
    }

    EvictLowestHitCountIfNeeded( );

    CacheEntry entry;
    entry.layout          = std::make_unique<TextLayout>( TextLayoutDesc{ font } );
    entry.hitCount        = 1;
    entry.lastAccessOrder = ++m_accessCounter;

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text        = text;
    shapeDesc.FontSize    = fontSize;
    shapeDesc.Direction   = key.direction;
    shapeDesc.HbScriptTag = key.scriptTag;

    entry.layout->SetFont( font );
    entry.layout->ShapeText( shapeDesc );

    TextLayout *layoutPtr = entry.layout.get( );
    m_cache[ key ]        = std::move( entry );
    return layoutPtr;
}

void TextLayoutCache::Clear( )
{
    m_cache.clear( );
}

size_t TextLayoutCache::GetCacheSize( ) const
{
    return m_cache.size( );
}

uint64_t TextLayoutCache::HashString( const DenOfIz_StringView &str )
{
    // Simple FNV-1a hash
    uint64_t hash = 14695981039346656037ULL;
    for ( size_t i = 0; i < str.NumChars; ++i )
    {
        hash ^= static_cast<uint64_t>( str.Chars[ i ] );
        hash *= 1099511628211ULL;
    }
    return hash;
}

void TextLayoutCache::EvictLowestHitCountIfNeeded( )
{
    if ( m_cache.size( ) < MAX_CACHE_SIZE )
    {
        return;
    }

    auto     victimIt    = m_cache.end( );
    uint64_t lowestScore = UINT64_MAX;

    for ( auto it = m_cache.begin( ); it != m_cache.end( ); ++it )
    {
        const auto &entry = it->second;

        const uint64_t score = entry.hitCount * 1000 + ( m_accessCounter - entry.lastAccessOrder );
        if ( score < lowestScore )
        {
            lowestScore = score;
            victimIt    = it;
        }
    }

    if ( victimIt != m_cache.end( ) )
    {
        m_cache.erase( victimIt );
    }
}
