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
#include <DenOfIzGraphics/Assets/Serde/Common/AssetReaderHelpers.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>

using namespace DenOfIz;

FontAssetReader::FontAssetReader( const FontAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    DZ_NOT_NULL( m_reader );
}

FontAssetReader::~FontAssetReader( ) = default;

FontAsset FontAssetReader::Read( )
{
    if ( m_assetRead )
    {
        return m_fontAsset;
    }
    m_streamStartOffset = m_reader->Position( );
    m_fontAsset.Magic   = m_reader->ReadUInt32( );
    if ( m_fontAsset.Magic != 0x544E4F465A44 ) // DZFONT
    {
        LOG( ERROR ) << "Invalid font asset magic word";
        return m_fontAsset;
    }
    m_fontAsset.Version  = m_reader->ReadUInt32( );
    m_fontAsset.NumBytes = m_reader->ReadUInt64( );
    m_fontAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );

    m_fontAsset.FontPath     = m_reader->ReadString( );
    m_fontAsset.PixelSize    = m_reader->ReadUInt32( );
    m_fontAsset.AntiAliasing = m_reader->ReadByte( ) == 1;
    m_fontAsset.AtlasWidth   = m_reader->ReadUInt32( );
    m_fontAsset.AtlasHeight  = m_reader->ReadUInt32( );

    m_fontAsset.Metrics.Ascent             = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.Descent            = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.LineGap            = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.LineHeight         = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.UnderlinePos       = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.UnderlineThickness = m_reader->ReadUInt32( );

    const uint32_t numGlyphs = m_reader->ReadUInt32( );
    m_fontAsset.GlyphData.Resize( numGlyphs );

    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        GlyphMetrics &metrics = m_fontAsset.GlyphData.GetElement( i );
        metrics.CodePoint     = m_reader->ReadUInt32( );
        metrics.Width         = m_reader->ReadUInt32( );
        metrics.Height        = m_reader->ReadUInt32( );
        metrics.BearingX      = m_reader->ReadUInt32( );
        metrics.BearingY      = m_reader->ReadUInt32( );
        metrics.Advance       = m_reader->ReadUInt32( );
        metrics.AtlasX        = m_reader->ReadUInt32( );
        metrics.AtlasY        = m_reader->ReadUInt32( );
    }

    m_fontAsset.UserProperties = AssetReaderHelpers::ReadUserProperties( m_reader );

    m_assetRead = true;
    return m_fontAsset;
}

InteropArray<Byte> FontAssetReader::ReadAtlasBitmap( )
{
    if ( !m_assetRead )
    {
        Read( );
    }

    m_reader->Seek( m_streamStartOffset + m_fontAsset.AtlasBitmap.Offset );
    InteropArray<Byte> atlasBitmap;
    atlasBitmap.Resize( m_fontAsset.AtlasBitmap.NumBytes );

    if ( m_fontAsset.AtlasBitmap.NumBytes > 0 )
    {
        return m_reader->ReadBytes( m_fontAsset.AtlasBitmap.NumBytes );
    }

    return { };
}

uint64_t FontAssetReader::AtlasBitmapNumBytes( ) const
{
    return m_fontAsset.AtlasBitmap.NumBytes;
}
