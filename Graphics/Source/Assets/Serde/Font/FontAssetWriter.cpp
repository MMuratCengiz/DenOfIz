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

void FontAssetWriter::Write( const FontAsset &fontAsset )
{
    m_fontAsset           = fontAsset;
    m_streamStartLocation = m_writer->Position( );

    WriteHeader( 0 );
    WriteMetadata( fontAsset );
    WriteGlyph( fontAsset );

    AssetWriterHelpers::WriteProperties( m_writer, fontAsset.UserProperties );
    m_writer->WriteUInt64( fontAsset.NumAtlasDataBytes );
    m_writer->WriteBytes( fontAsset.AtlasData );
}

void FontAssetWriter::End( ) const
{
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
    m_writer->WriteString( m_fontAsset.Uri.ToInteropString( ) );
}

void FontAssetWriter::WriteMetadata( const FontAsset &fontAsset ) const
{
    m_writer->WriteUInt64( fontAsset.DataNumBytes );
    m_writer->WriteBytes( fontAsset.Data );

    m_writer->WriteUInt32( fontAsset.InitialFontSize );
    m_writer->WriteUInt32( fontAsset.AtlasWidth );
    m_writer->WriteUInt32( fontAsset.AtlasHeight );

    m_writer->WriteUInt32( fontAsset.Metrics.Ascent );
    m_writer->WriteUInt32( fontAsset.Metrics.Descent );
    m_writer->WriteUInt32( fontAsset.Metrics.LineGap );
    m_writer->WriteUInt32( fontAsset.Metrics.LineHeight );
    m_writer->WriteUInt32( fontAsset.Metrics.UnderlinePos );
    m_writer->WriteUInt32( fontAsset.Metrics.UnderlineThickness );
}

void FontAssetWriter::WriteGlyph( const FontAsset &fontAsset ) const
{
    const uint32_t numGlyphs = fontAsset.Glyphs.NumElements( );
    m_writer->WriteUInt32( numGlyphs );
    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        const FontGlyph &glyph = fontAsset.Glyphs.GetElement( i );
        m_writer->WriteUInt32( glyph.CodePoint );
        m_writer->WriteDouble( glyph.Bounds.XMin );
        m_writer->WriteDouble( glyph.Bounds.YMin );
        m_writer->WriteDouble( glyph.Bounds.XMax );
        m_writer->WriteDouble( glyph.Bounds.YMax );
        m_writer->WriteUInt32( glyph.Width );
        m_writer->WriteUInt32( glyph.Height );
        m_writer->WriteUInt32( glyph.BearingX );
        m_writer->WriteUInt32( glyph.BearingY );
        m_writer->WriteUInt32( glyph.XAdvance );
        m_writer->WriteUInt32( glyph.YAdvance );
        m_writer->WriteUInt32( glyph.AtlasX );
        m_writer->WriteUInt32( glyph.AtlasY );
        m_writer->WriteUInt32( glyph.Pitch );
        m_writer->WriteBytes( glyph.Data );
    }
}
