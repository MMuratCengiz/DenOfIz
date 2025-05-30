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
#include <DenOfIzGraphics/UI/UITextVertexCache.h>

using namespace DenOfIz;

CachedTextVertices *UITextVertexCache::GetOrCreateCachedTextVertices( const TextVertexCacheKey &key, uint32_t currentFrame )
{
    const auto it = m_cache.find( key );
    if ( it != m_cache.end( ) )
    {
        it->second->lastUsedFrame = currentFrame;
        return it->second.get( );
    }

    auto cached                   = std::make_unique<CachedTextVertices>( );
    cached->lastUsedFrame         = currentFrame;
    CachedTextVertices *cachedPtr = cached.get( );
    m_cache[ key ]                = std::move( cached );
    return std::move( cachedPtr );
}

void UITextVertexCache::Cleanup( uint32_t currentFrame, uint32_t maxAge )
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

void UITextVertexCache::Clear( )
{
    m_cache.clear( );
}

size_t UITextVertexCache::GetCacheSize( ) const
{
    return m_cache.size( );
}

TextVertexCacheKey UITextVertexCache::CreateTextVertexKey( const Clay_RenderCommand *command, float effectiveScale, float adjustedY, float dpiScale )
{
    const auto &data   = command->renderData.text;
    const auto &bounds = command->boundingBox;

    TextVertexCacheKey key{ };
    key.textHash       = TextLayoutCache::HashString( data.stringContents.chars, data.stringContents.length );
    key.fontId         = data.fontId;
    key.fontSize       = data.fontSize > 0 ? static_cast<uint32_t>( data.fontSize * dpiScale ) : 0;
    key.posX           = bounds.x * dpiScale;
    key.posY           = adjustedY;
    key.colorRGBA      = ColorToRGBA( data.textColor );
    key.letterSpacing  = data.letterSpacing * dpiScale;
    key.lineHeight     = data.lineHeight;
    key.effectiveScale = effectiveScale;
    return key;
}

uint32_t UITextVertexCache::ColorToRGBA( const Clay_Color &color )
{
    return static_cast<uint32_t>( color.r ) << 24 | static_cast<uint32_t>( color.g ) << 16 | static_cast<uint32_t>( color.b ) << 8 | static_cast<uint32_t>( color.a );
}
