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
#include <DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>

using namespace DenOfIz;

FontAssetWriter::FontAssetWriter( const FontAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    DZ_NOT_NULL( m_writer );
}

FontAssetWriter::~FontAssetWriter( ) = default;

void FontAssetWriter::Write( const FontAsset &fontAsset, const InteropArray<Byte> &atlasBitmap )
{
    m_fontAsset           = fontAsset;
    m_streamStartLocation = m_writer->Position( );

    WriteHeader( 0 );
    WriteMetadata( fontAsset );
    WriteGlyphData( fontAsset );

    AssetWriterHelpers::WriteProperties( m_writer, fontAsset.UserProperties );

    if ( atlasBitmap.NumElements( ) > 0 )
    {
        const uint32_t streamOffset = m_writer->Position( ) - m_streamStartLocation + sizeof( AssetDataStream );
        AssetWriterHelpers::WriteAssetDataStream( m_writer, { streamOffset, atlasBitmap.NumElements( ) } );
        m_writer->WriteBytes( atlasBitmap );
    }

    const auto totalNumBytes   = m_writer->Position( ) - m_streamStartLocation;
    const auto currentPosition = m_writer->Position( );
    m_writer->Seek( m_streamStartLocation );
    WriteHeader( totalNumBytes );
    m_writer->Seek( currentPosition );
}

void FontAssetWriter::WriteHeader( const uint64_t totalNumBytes ) const
{
    m_writer->WriteUInt64( m_fontAsset.Magic );
    m_writer->WriteUInt32( m_fontAsset.Version );
    m_writer->WriteUInt64( totalNumBytes );
    m_writer->WriteString( m_fontAsset.Uri.ToString( ) );
}

void FontAssetWriter::WriteMetadata( const FontAsset &fontAsset ) const
{
    m_writer->WriteString( fontAsset.FontPath );

    m_writer->WriteUInt32( fontAsset.PixelSize );
    m_writer->WriteByte( fontAsset.AntiAliasing ? 1 : 0 );
    m_writer->WriteUInt32( fontAsset.AtlasWidth );
    m_writer->WriteUInt32( fontAsset.AtlasHeight );

    m_writer->WriteUInt32( fontAsset.Metrics.Ascent );
    m_writer->WriteUInt32( fontAsset.Metrics.Descent );
    m_writer->WriteUInt32( fontAsset.Metrics.LineGap );
    m_writer->WriteUInt32( fontAsset.Metrics.LineHeight );
    m_writer->WriteUInt32( fontAsset.Metrics.UnderlinePos );
    m_writer->WriteUInt32( fontAsset.Metrics.UnderlineThickness );
}

void FontAssetWriter::WriteGlyphData( const FontAsset &fontAsset ) const
{
    const uint32_t numGlyphs = fontAsset.GlyphData.NumElements( );
    m_writer->WriteUInt32( numGlyphs );
    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        const GlyphMetrics &metrics = fontAsset.GlyphData.GetElement( i );
        m_writer->WriteUInt32( metrics.CodePoint );
        m_writer->WriteUInt32( metrics.Width );
        m_writer->WriteUInt32( metrics.Height );
        m_writer->WriteUInt32( metrics.BearingX );
        m_writer->WriteUInt32( metrics.BearingY );
        m_writer->WriteUInt32( metrics.Advance );
        m_writer->WriteUInt32( metrics.AtlasX );
        m_writer->WriteUInt32( metrics.AtlasY );
    }
}
