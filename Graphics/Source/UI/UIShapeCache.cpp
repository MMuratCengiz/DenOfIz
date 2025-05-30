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

#include <DenOfIzGraphics/UI/UIShapeCache.h>

using namespace DenOfIz;

CachedShape *UIShapeCache::GetOrCreateCachedShape( const ShapeCacheKey &key, const uint32_t currentFrame )
{
    const auto it = m_cache.find( key );
    if ( it != m_cache.end( ) )
    {
        it->second->lastUsedFrame = currentFrame;
        return it->second.get( );
    }

    auto cached            = std::make_unique<CachedShape>( );
    cached->lastUsedFrame  = currentFrame;
    CachedShape *cachedPtr = cached.get( );
    m_cache[ key ]         = std::move( cached );
    return std::move( cachedPtr );
}

void UIShapeCache::Cleanup( const uint32_t currentFrame, const uint32_t maxAge )
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

void UIShapeCache::Clear( )
{
    m_cache.clear( );
}

size_t UIShapeCache::GetCacheSize( ) const
{
    return m_cache.size( );
}

ShapeCacheKey UIShapeCache::CreateRectangleKey( const Clay_RenderCommand *command )
{
    const auto &data   = command->renderData.rectangle;
    const auto &bounds = command->boundingBox;

    ShapeCacheKey key{ };
    key.x            = bounds.x;
    key.y            = bounds.y;
    key.width        = bounds.width;
    key.height       = bounds.height;
    key.colorRGBA    = ColorToRGBA( data.backgroundColor );
    key.textureIndex = 0;
    key.shapeType    = 0; // Rectangle

    key.cornerRadius[ 0 ] = data.cornerRadius.topLeft;
    key.cornerRadius[ 1 ] = data.cornerRadius.topRight;
    key.cornerRadius[ 2 ] = data.cornerRadius.bottomRight;
    key.cornerRadius[ 3 ] = data.cornerRadius.bottomLeft;

    key.borderWidth[ 0 ] = 0;
    key.borderWidth[ 1 ] = 0;
    key.borderWidth[ 2 ] = 0;
    key.borderWidth[ 3 ] = 0;
    return key;
}

ShapeCacheKey UIShapeCache::CreateBorderKey( const Clay_RenderCommand *command )
{
    const auto &data   = command->renderData.border;
    const auto &bounds = command->boundingBox;

    ShapeCacheKey key{ };
    key.x            = bounds.x;
    key.y            = bounds.y;
    key.width        = bounds.width;
    key.height       = bounds.height;
    key.colorRGBA    = ColorToRGBA( data.color );
    key.textureIndex = 0;
    key.shapeType    = 1; // Border

    key.cornerRadius[ 0 ] = data.cornerRadius.topLeft;
    key.cornerRadius[ 1 ] = data.cornerRadius.topRight;
    key.cornerRadius[ 2 ] = data.cornerRadius.bottomRight;
    key.cornerRadius[ 3 ] = data.cornerRadius.bottomLeft;

    key.borderWidth[ 0 ] = data.width.top;
    key.borderWidth[ 1 ] = data.width.right;
    key.borderWidth[ 2 ] = data.width.bottom;
    key.borderWidth[ 3 ] = data.width.left;
    return key;
}

uint32_t UIShapeCache::ColorToRGBA( const Clay_Color &color )
{
    return static_cast<uint32_t>( color.r ) << 24 | static_cast<uint32_t>( color.g ) << 16 | static_cast<uint32_t>( color.b ) << 8 | static_cast<uint32_t>( color.a );
}
