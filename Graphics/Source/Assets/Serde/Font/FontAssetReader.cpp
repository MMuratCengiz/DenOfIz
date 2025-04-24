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

    m_fontAsset.AtlasWidth  = m_reader->ReadUInt32( );
    m_fontAsset.AtlasHeight = m_reader->ReadUInt32( );

    m_fontAsset.Metrics.Ascent             = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.Descent            = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.LineGap            = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.LineHeight         = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.UnderlinePos       = m_reader->ReadUInt32( );
    m_fontAsset.Metrics.UnderlineThickness = m_reader->ReadUInt32( );

    const uint32_t numGlyphs = m_reader->ReadUInt32( );
    m_fontAsset.Glyphs.Resize( numGlyphs );

    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        FontGlyph &glyph  = m_fontAsset.Glyphs.GetElement( i );
        glyph.CodePoint   = m_reader->ReadUInt32( );
        glyph.Bounds.XMin = m_reader->ReadUInt32( );
        glyph.Bounds.YMin = m_reader->ReadUInt32( );
        glyph.Bounds.XMax = m_reader->ReadUInt32( );
        glyph.Bounds.YMax = m_reader->ReadUInt32( );
        glyph.Width       = m_reader->ReadUInt32( );
        glyph.Height      = m_reader->ReadUInt32( );
        glyph.BearingX    = m_reader->ReadUInt32( );
        glyph.BearingY    = m_reader->ReadUInt32( );
        glyph.AdvanceX    = m_reader->ReadUInt32( );
        glyph.AdvanceY    = m_reader->ReadUInt32( );
        glyph.AtlasX      = m_reader->ReadUInt32( );
        glyph.AtlasY      = m_reader->ReadUInt32( );
        glyph.Pitch       = m_reader->ReadUInt32( );
        glyph.Data        = m_reader->ReadBytes( glyph.Pitch * glyph.Height );
    }

    m_fontAsset.UserProperties = AssetReaderHelpers::ReadUserProperties( m_reader );
    m_fontAsset.AtlasData      = AssetReaderHelpers::ReadAssetDataStream( m_reader );

    m_assetRead = true;
    return m_fontAsset;
}

void FontAssetReader::LoadAtlasIntoGpuTexture( const LoadAtlasIntoGpuTextureDesc &desc ) const
{
    if ( !desc.CommandList || !desc.Texture )
    {
        LOG( FATAL ) << "CommandList and Texture are required for LoadIntoGpuTexture";
        return;
    }

    constexpr uint32_t batchSize = 1024;
    InteropArray<Byte> buffer( batchSize );

    const auto stagingBuffer  = desc.StagingBuffer;
    uint64_t   remainingBytes = m_fontAsset.AtlasData.NumBytes;
    auto       mappedMemory   = static_cast<Byte *>( stagingBuffer->MapMemory( ) );

    m_reader->Seek( m_fontAsset.AtlasData.Offset );
    while ( remainingBytes > 0 )
    {
        const uint32_t bytesToRead = static_cast<uint32_t>( std::min( static_cast<uint64_t>( batchSize ), remainingBytes ) );
        const uint32_t bytesRead   = m_reader->Read( buffer, 0, bytesToRead );

        if ( bytesRead != bytesToRead )
        {
            LOG( ERROR ) << "Failed to read expected number of bytes. Expected: " << bytesToRead << ", Read: " << bytesRead;
            break;
        }

        memcpy( mappedMemory, buffer.Data( ), bytesRead );

        mappedMemory += bytesRead;
        remainingBytes -= bytesRead;
    }

    stagingBuffer->UnmapMemory( );

    CopyBufferToTextureDesc copyDesc{ };
    copyDesc.DstTexture = desc.Texture;
    copyDesc.SrcBuffer  = stagingBuffer;
    copyDesc.Format     = desc.Texture->GetFormat( );
    copyDesc.MipLevel   = 0;
    copyDesc.ArrayLayer = 0;
    copyDesc.RowPitch   = m_fontAsset.AtlasWidth;
    copyDesc.NumRows    = m_fontAsset.AtlasHeight;

    desc.CommandList->CopyBufferToTexture( copyDesc );
}

InteropArray<Byte> FontAssetReader::ReadAtlasData( )
{
    if ( !m_assetRead )
    {
        Read( );
    }

    m_reader->Seek( m_streamStartOffset + m_fontAsset.AtlasData.Offset );
    InteropArray<Byte> atlasBitmap;
    atlasBitmap.Resize( m_fontAsset.AtlasData.NumBytes );

    if ( m_fontAsset.AtlasData.NumBytes > 0 )
    {
        return m_reader->ReadBytes( m_fontAsset.AtlasData.NumBytes );
    }

    return { };
}

uint64_t FontAssetReader::AtlasBitmapNumBytes( ) const
{
    return m_fontAsset.AtlasData.NumBytes;
}
