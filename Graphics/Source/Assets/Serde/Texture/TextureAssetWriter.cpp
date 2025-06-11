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

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h"

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

TextureAssetWriter::TextureAssetWriter( const TextureAssetWriterDesc &desc ) : m_writer( desc.Writer ), m_desc( desc )
{
    if ( !m_writer )
    {
        spdlog::critical( "BinaryWriter cannot be null for TextureAssetWriter" );
    }
}

TextureAssetWriter::~TextureAssetWriter( ) = default;

void TextureAssetWriter::WriteHeader( const uint64_t totalNumBytes ) const
{
    m_writer->WriteUInt64( m_textureAsset.Magic );
    m_writer->WriteUInt32( m_textureAsset.Version );
    m_writer->WriteUInt64( totalNumBytes );
    m_writer->WriteString( m_textureAsset.Uri.ToInteropString( ) );
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
        spdlog::critical( "Attempted to add more Mip or Array data than expected." );
    }
    if ( !m_isFirstMip )
    {
        if ( mipIndex < m_lastMipIndex || arrayLayer < m_lastArrayIndex ||
             ( mipIndex == m_lastMipIndex && ( arrayLayer != m_lastArrayIndex || arrayLayer != m_lastArrayIndex + 1 ) ) )
        {
            spdlog::critical( "Attempting to write mip data out of order expected either mipLevel[ {} (+1)] or arrayIndex[{} (+1)]", m_lastMipIndex, m_lastArrayIndex );
        }
        else if ( mipIndex == m_lastMipIndex + 1 && arrayLayer != 0 )
        {
            spdlog::critical( "Attempting to write mip data out of order expected array index to be 0." );
        }
    }
    else
    {
        if ( mipIndex != 0 || arrayLayer != 0 )
        {
            spdlog::critical( "Attempting to write mip data out of order expected mip level to be 0 and array index to be 0." );
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
    if ( m_textureAsset.Width == 0 || m_textureAsset.Height == 0 )
    {
        spdlog::warn( "TextureAssetWriter: Texture dimensions are zero, which may indicate uninitialized data" );
    }
    if ( m_textureAsset.Format == Format::Undefined )
    {
        spdlog::warn( "TextureAssetWriter: Texture format is undefined, defaulting to R8G8B8A8Unorm" );
        m_textureAsset.Format = Format::R8G8B8A8Unorm;
    }
    if ( m_textureAsset.BitsPerPixel == 0 )
    {
        spdlog::warn( "TextureAssetWriter: BitsPerPixel is 0, attempting to set based on format" );
        switch ( m_textureAsset.Format )
        {
        case Format::R8G8B8A8Unorm:
        case Format::R8G8B8A8UnormSrgb:
        case Format::R8G8B8A8Uint:
        case Format::R8G8B8A8Snorm:
        case Format::R8G8B8A8Sint:
        case Format::B8G8R8A8Unorm:
            m_textureAsset.BitsPerPixel = 32;
            break;
        case Format::BC1Unorm:
        case Format::BC1UnormSrgb:
            m_textureAsset.BitsPerPixel = 4;
            break;
        case Format::BC2Unorm:
        case Format::BC2UnormSrgb:
        case Format::BC3Unorm:
        case Format::BC3UnormSrgb:
            m_textureAsset.BitsPerPixel = 8;
            break;
        case Format::BC4Unorm:
        case Format::BC4Snorm:
            m_textureAsset.BitsPerPixel = 4;
            break;
        case Format::BC5Unorm:
        case Format::BC5Snorm:
        case Format::BC7Unorm:
            m_textureAsset.BitsPerPixel = 8;
            break;
        default:
            m_textureAsset.BitsPerPixel = 32;
            break;
        }
    }

    if ( m_textureAsset.BlockSize == 0 )
    {
        if ( IsFormatBC( m_textureAsset.Format ) )
        {
            m_textureAsset.BlockSize = 4;
        }
        else
        {
            m_textureAsset.BlockSize = 1;
        }
    }

    if ( m_textureAsset.RowPitch == 0 )
    {
        if ( m_textureAsset.Format >= Format::BC1Unorm && m_textureAsset.Format <= Format::BC7Unorm )
        {
            const uint32_t blockWidth = ( m_textureAsset.Width + m_textureAsset.BlockSize - 1 ) / m_textureAsset.BlockSize;
            m_textureAsset.RowPitch   = blockWidth * ( m_textureAsset.BitsPerPixel / 8 );
        }
        else
        {
            m_textureAsset.RowPitch = m_textureAsset.Width * ( m_textureAsset.BitsPerPixel / 8 );
        }
    }

    if ( m_textureAsset.NumRows == 0 )
    {
        spdlog::warn( "TextureAssetWriter: NumRows is 0, calculating based on height and format" );
        if ( m_textureAsset.Format >= Format::BC1Unorm && m_textureAsset.Format <= Format::BC7Unorm )
        {
            m_textureAsset.NumRows = ( m_textureAsset.Height + m_textureAsset.BlockSize - 1 ) / m_textureAsset.BlockSize;
        }
        else
        {
            m_textureAsset.NumRows = m_textureAsset.Height;
        }
    }

    if ( m_textureAsset.SlicePitch == 0 )
    {
        m_textureAsset.SlicePitch = m_textureAsset.RowPitch * m_textureAsset.NumRows;
    }

    if ( m_textureAsset.Mips.NumElements( ) == 0 )
    {
        spdlog::warn( "TextureAssetWriter: No mip levels found, creating default mip level" );
        TextureMip mip{ };
        mip.Width      = std::max( 1u, m_textureAsset.Width );
        mip.Height     = std::max( 1u, m_textureAsset.Height );
        mip.MipIndex   = 0;
        mip.ArrayIndex = 0;
        mip.RowPitch   = m_textureAsset.RowPitch;
        mip.NumRows    = m_textureAsset.NumRows;
        mip.SlicePitch = m_textureAsset.SlicePitch;
        mip.DataOffset = 0;

        m_textureAsset.Mips.Resize( 1 );
        m_textureAsset.Mips.Elements[ 0 ] = mip;
    }

    m_writer->WriteUInt32( std::max( 1u, m_textureAsset.Width ) );
    m_writer->WriteUInt32( std::max( 1u, m_textureAsset.Height ) );
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
        WriteMipInfo( m_textureAsset.Mips.Elements[ i ] );
    }

    AssetWriterHelpers::WriteAssetDataStream( m_writer, { 0, 0 } );
    m_assetDataStreamPosition = m_writer->Position( );
}

void TextureAssetWriter::AddPixelData( const ByteArrayView &bytes, const uint32_t mipIndex, const uint32_t arrayLayer )
{
    ValidateMipRange( mipIndex, arrayLayer );

    const uint32_t currentDataOffset = m_writer->Position( ) - m_assetDataStreamPosition;
    bool           mipFound          = false;

    for ( size_t i = 0; i < m_textureAsset.Mips.NumElements( ); ++i )
    {
        if ( TextureMip &mip = m_textureAsset.Mips.Elements[ i ]; mip.MipIndex == mipIndex && mip.ArrayIndex == arrayLayer )
        {
            if ( mip.DataOffset == 0 )
            {
                mip.DataOffset = currentDataOffset;

                const uint64_t currentPos = m_writer->Position( );
                m_writer->Seek( m_textureMipPositions[ i ] + offsetof( TextureMip, DataOffset ) );
                m_writer->WriteUInt32( currentDataOffset );
                m_writer->Seek( currentPos );
            }
            mipFound = true;
            break;
        }
    }

    // If mip was not found, add a new mip entry (unless we're already at max mips)
    if ( !mipFound && ( mipIndex < m_textureAsset.MipLevels ) && ( arrayLayer < m_textureAsset.ArraySize ) )
    {
        spdlog::warn( "TextureAssetWriter: Adding missing mip entry for level {} , array layer {}", mipIndex, arrayLayer );
        const uint32_t mipWidth  = std::max( 1u, m_textureAsset.Width >> mipIndex );
        const uint32_t mipHeight = std::max( 1u, m_textureAsset.Height >> mipIndex );

        uint32_t mipRowPitch, mipNumRows, mipSlicePitch;
        if ( IsFormatBC( m_textureAsset.Format ) )
        {
            const uint32_t blockWidth  = ( mipWidth + m_textureAsset.BlockSize - 1 ) / m_textureAsset.BlockSize;
            const uint32_t blockHeight = ( mipHeight + m_textureAsset.BlockSize - 1 ) / m_textureAsset.BlockSize;

            mipRowPitch   = blockWidth * ( m_textureAsset.BitsPerPixel / 8 );
            mipNumRows    = blockHeight;
            mipSlicePitch = mipRowPitch * mipNumRows;
        }
        else
        {
            mipRowPitch   = mipWidth * ( m_textureAsset.BitsPerPixel / 8 );
            mipNumRows    = mipHeight;
            mipSlicePitch = mipRowPitch * mipNumRows;
        }

        TextureMip newMip{ };
        newMip.Width      = mipWidth;
        newMip.Height     = mipHeight;
        newMip.MipIndex   = mipIndex;
        newMip.ArrayIndex = arrayLayer;
        newMip.RowPitch   = mipRowPitch;
        newMip.NumRows    = mipNumRows;
        newMip.SlicePitch = mipSlicePitch;
        newMip.DataOffset = currentDataOffset;
        m_textureAsset.Mips.AddElement( newMip );
        m_textureMipPositions.push_back( m_writer->Position( ) );
        WriteMipInfo( newMip );
    }

    m_lastMipIndex   = mipIndex;
    m_lastArrayIndex = arrayLayer;
    m_writer->WriteBytes( bytes );
}

void TextureAssetWriter::End( ) const
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
