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
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>

using namespace DenOfIz;

TextureAssetWriter::TextureAssetWriter( const TextureAssetWriterDesc &desc ) : m_writer( desc.Writer ), m_desc( desc )
{
    if ( !m_writer )
    {
        LOG( FATAL ) << "BinaryWriter cannot be null for TextureAssetWriter";
    }
}

TextureAssetWriter::~TextureAssetWriter( ) = default;

void TextureAssetWriter::WriteHeader( const uint64_t totalNumBytes ) const
{
    m_writer->WriteUInt64( m_textureAsset.Magic );
    m_writer->WriteUInt32( m_textureAsset.Version );
    m_writer->WriteUInt64( totalNumBytes );
    m_writer->WriteString( m_textureAsset.Uri.ToString( ) );
}
void TextureAssetWriter::WriteMipInfo( const TextureMip &mip ) const
{
    m_writer->WriteUInt32( mip.Width );
    m_writer->WriteUInt32( mip.Height );
    m_writer->WriteUInt32( mip.MipIndex );
    m_writer->WriteUInt32( mip.ArrayIndex );
    m_writer->WriteUInt32( mip.RowPitch );
    m_writer->WriteUInt32( mip.NumRows );
    m_writer->WriteUInt32( mip.SlicePitch );
    m_writer->WriteUInt32( mip.DataOffset );
}

void TextureAssetWriter::ValidateMipRange( const uint32_t mipIndex, const uint32_t arrayLayer )
{
    if ( mipIndex >= m_textureAsset.MipLevels || arrayLayer >= m_textureAsset.ArraySize )
    {
        LOG( FATAL ) << "Attempted to add more Mip or Array data than expected.";
    }
    if ( !m_isFirstMip )
    {
        if ( mipIndex < m_lastMipIndex || arrayLayer < m_lastArrayIndex ||
             ( mipIndex == m_lastMipIndex && ( arrayLayer != m_lastArrayIndex || arrayLayer != m_lastArrayIndex + 1 ) ) )
        {
            LOG( FATAL ) << "Attempting to write mip data out of order expected either mipLevel[" << m_lastMipIndex << " (+1)] or arrayIndex[" << m_lastArrayIndex << " (+1)]";
        }
        else if ( mipIndex == m_lastMipIndex + 1 && arrayLayer != 0 )
        {
            LOG( FATAL ) << "Attempting to write mip data out of order expected array index to be 0.";
        }
    }
    else
    {
        if ( mipIndex != 0 || arrayLayer != 0 )
        {
            LOG( FATAL ) << "Attempting to write mip data out of order expected mip level to be 0 and array index to be 0.";
        }
    }
    m_isFirstMip = false;
}

void TextureAssetWriter::Write( const TextureAsset &textureAsset )
{
    m_textureAsset        = textureAsset;
    m_streamStartLocation = m_writer->Position( );
    WriteHeader( 0 );
    m_writer->WriteString( m_textureAsset.Name );
    m_writer->WriteString( m_textureAsset.SourcePath );
    m_writer->WriteUInt32( m_textureAsset.Width );
    m_writer->WriteUInt32( m_textureAsset.Height );
    m_writer->WriteUInt32( m_textureAsset.Depth );
    m_writer->WriteUInt32( static_cast<uint32_t>( m_textureAsset.Format ) );
    m_writer->WriteUInt32( static_cast<uint32_t>( m_textureAsset.Dimension ) );
    m_writer->WriteUInt32( m_textureAsset.MipLevels );
    m_writer->WriteUInt32( m_textureAsset.ArraySize );
    m_writer->WriteUInt32( m_textureAsset.BitsPerPixel );
    m_writer->WriteUInt32( m_textureAsset.BlockSize );
    m_writer->WriteUInt32( m_textureAsset.RowPitch );
    m_writer->WriteUInt32( m_textureAsset.NumRows );
    m_writer->WriteUInt32( m_textureAsset.SlicePitch );
    m_writer->WriteUInt32( m_textureAsset.Mips.NumElements( ) );
    m_textureMipPositions.resize( m_textureAsset.Mips.NumElements( ) );
    for ( int i = 0; i < m_textureAsset.Mips.NumElements( ); ++i )
    {
        m_textureMipPositions[ i ] = m_writer->Position( );
        WriteMipInfo( m_textureAsset.Mips.GetElement( i ) );
    }

    AssetWriterHelpers::WriteAssetDataStream( m_writer, { 0, 0 } );
    m_assetDataStreamPosition = m_writer->Position( );
}

void TextureAssetWriter::AddPixelData( const InteropArray<Byte> &bytes, const uint32_t mipIndex, const uint32_t arrayLayer )
{
    ValidateMipRange( mipIndex, arrayLayer );

    const uint32_t currentDataOffset = m_writer->Position( ) - m_assetDataStreamPosition;
    for ( size_t i = 0; i < m_textureAsset.Mips.NumElements( ); ++i )
    {
        if ( TextureMip &mip = m_textureAsset.Mips.GetElement( i ); mip.MipIndex == mipIndex && mip.ArrayIndex == arrayLayer )
        {
            if ( mip.DataOffset == 0 )
            {
                mip.DataOffset = currentDataOffset;

                const uint64_t currentPos = m_writer->Position( );
                m_writer->Seek( m_textureMipPositions[ i ] + offsetof( TextureMip, DataOffset ) );
                m_writer->WriteUInt32( currentDataOffset );
                m_writer->Seek( currentPos );
            }
            break;
        }
    }

    m_lastMipIndex   = mipIndex;
    m_lastArrayIndex = arrayLayer;
    m_writer->WriteBytes( bytes );
}

void TextureAssetWriter::Finalize( ) const
{
    const uint64_t currentPos = m_writer->Position( );

    m_writer->Seek( m_assetDataStreamPosition - sizeof( AssetDataStream ) );
    const AssetDataStream stream = { m_assetDataStreamPosition, currentPos - m_assetDataStreamPosition };
    AssetWriterHelpers::WriteAssetDataStream( m_writer, stream );

    m_writer->Seek( m_streamStartLocation );
    WriteHeader( currentPos - m_streamStartLocation );
    m_writer->Seek( currentPos );
    m_writer->Flush( );
}
