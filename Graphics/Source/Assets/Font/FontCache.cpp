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
#include <DenOfIzGraphics/Assets/Font/FontCache.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <algorithm>

using namespace DenOfIz;

FontCache::FontCache( const FontAsset &fontAsset ) : m_fontAsset( fontAsset )
{
    // For MSDF, we store RGB (3 channels) instead of just grayscale
    m_atlasBitmap.resize( fontAsset.AtlasWidth * fontAsset.AtlasHeight * 3, 0 );
}

void FontCache::InitializeAtlasBitmap( const InteropArray<Byte> &initialBitmap )
{
    if ( initialBitmap.NumElements( ) > 0 )
    {
        const size_t bytesToCopy = std::min( initialBitmap.NumElements( ), m_atlasBitmap.size( ) );
        std::memcpy( m_atlasBitmap.data( ), initialBitmap.Data( ), bytesToCopy );
    }
    else
    {
        // Zero out the bitmap
        std::memset( m_atlasBitmap.data( ), 0, m_atlasBitmap.size( ) );
    }
}

const FontAsset &FontCache::GetFontAsset( ) const
{
    return m_fontAsset;
}

const std::vector<uint8_t> &FontCache::GetAtlasBitmap( ) const
{
    return m_atlasBitmap;
}

void FontCache::AddGlyph( const AddGlyphDesc &desc )
{
    if ( HasGlyph( desc.CodePoint ) )
    {
        return;
    }

    if ( desc.Width == 0 || desc.Height == 0 )
    {
        GlyphMetrics metrics{ };
        metrics.CodePoint = desc.CodePoint;
        metrics.Width     = 0;
        metrics.Height    = 0;
        metrics.BearingX  = desc.BearingX;
        metrics.BearingY  = desc.BearingY;
        metrics.AtlasX    = 0;
        metrics.AtlasY    = 0;

        const uint32_t currentSize = m_fontAsset.GlyphData.NumElements( );
        m_fontAsset.GlyphData.Resize( currentSize + 1 );
        m_fontAsset.GlyphData.SetElement( currentSize, metrics );

        return;
    }

    const Rect rect = AllocateSpace( desc.Width, desc.Height );
    CopyMsdfDataToAtlas( rect, desc.MsdfData, desc.MsdfPitch );

    GlyphMetrics metrics{ };
    metrics.CodePoint = desc.CodePoint;
    metrics.Width     = desc.Width;
    metrics.Height    = desc.Height;
    metrics.BearingX  = desc.BearingX;
    metrics.BearingY  = desc.BearingY;
    metrics.AtlasX    = rect.X;
    metrics.AtlasY    = rect.Y;

    const uint32_t currentSize = m_fontAsset.GlyphData.NumElements( );
    m_fontAsset.GlyphData.Resize( currentSize + 1 );
    m_fontAsset.GlyphData.SetElement( currentSize, metrics );

    m_atlasNeedsUpdate = true;
}

bool FontCache::HasGlyph( const uint32_t codePoint ) const
{
    for ( uint32_t i = 0; i < m_fontAsset.GlyphData.NumElements( ); ++i )
    {
        if ( m_fontAsset.GlyphData.GetElement( i ).CodePoint == codePoint )
        {
            return true;
        }
    }
    return false;
}

const GlyphMetrics *FontCache::GetGlyphMetrics( const uint32_t codePoint ) const
{
    for ( uint32_t i = 0; i < m_fontAsset.GlyphData.NumElements( ); ++i )
    {
        if ( m_fontAsset.GlyphData.GetElement( i ).CodePoint == codePoint )
        {
            return &m_fontAsset.GlyphData.GetElement( i );
        }
    }
    return nullptr;
}

void FontCache::ClearAtlasBitmap( )
{
    std::memset( m_atlasBitmap.data( ), 0, m_atlasBitmap.size( ) );
    m_atlasNeedsUpdate = true;
}

void FontCache::ResizeAtlas( const uint32_t newWidth, const uint32_t newHeight )
{
    if ( newWidth == m_fontAsset.AtlasWidth && newHeight == m_fontAsset.AtlasHeight )
    {
        return;
    }

    std::vector<uint8_t> newBitmap( newWidth * newHeight * 3 /*rgb*/, 0 );
    m_fontAsset.AtlasWidth  = newWidth;
    m_fontAsset.AtlasHeight = newHeight;
    m_fontAsset.GlyphData.Resize( 0 );
    m_atlasBitmap   = std::move( newBitmap );
    m_currentAtlasX = 0;
    m_currentAtlasY = 0;
    m_rowHeight     = 0;

    m_atlasNeedsUpdate = true;
}

bool FontCache::AtlasNeedsUpdate( ) const
{
    return m_atlasNeedsUpdate;
}

void FontCache::MarkAtlasUpdated( )
{
    m_atlasNeedsUpdate = false;
}

FontCache::Rect FontCache::AllocateSpace( const uint32_t width, const uint32_t height )
{
    if ( m_currentAtlasX + width > m_fontAsset.AtlasWidth )
    {
        m_currentAtlasX = 0;
        m_currentAtlasY += m_rowHeight;
        m_rowHeight = 0;
    }

    if ( m_currentAtlasY + height > m_fontAsset.AtlasHeight )
    {
        ResizeAtlas( m_fontAsset.AtlasWidth, m_fontAsset.AtlasHeight * 2 );
        m_currentAtlasX = 0;
        m_currentAtlasY = 0;
        m_rowHeight     = 0;
    }

    Rect rect{ };
    rect.X      = m_currentAtlasX;
    rect.Y      = m_currentAtlasY;
    rect.Width  = width;
    rect.Height = height;

    m_currentAtlasX += width;
    m_rowHeight = std::max( m_rowHeight, height );
    return rect;
}

void FontCache::CopyMsdfDataToAtlas( const Rect &rect, const InteropArray<Byte> &msdfData, const uint32_t msdfPitch )
{
    for ( uint32_t y = 0; y < rect.Height; y++ )
    {
        for ( uint32_t x = 0; x < rect.Width; x++ )
        {
            const uint32_t atlasOffset = ( ( rect.Y + y ) * m_fontAsset.AtlasWidth + ( rect.X + x ) ) * 3;
            if ( atlasOffset + 2 >= m_atlasBitmap.size( ) )
            {
                LOG( ERROR ) << "Atlas offset out of bounds: " << atlasOffset << " >= " << m_atlasBitmap.size( );
                continue;
            }

            const uint32_t srcOffset         = y * msdfPitch + x * 3;
            m_atlasBitmap[ atlasOffset ]     = msdfData.GetElement( srcOffset );     // R
            m_atlasBitmap[ atlasOffset + 1 ] = msdfData.GetElement( srcOffset + 1 ); // G
            m_atlasBitmap[ atlasOffset + 2 ] = msdfData.GetElement( srcOffset + 2 ); // B
        }
    }
}
