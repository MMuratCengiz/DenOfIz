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

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

TextureAssetReader::TextureAssetReader( const TextureAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for TextureAssetReader" );
    }
}

TextureAssetReader::~TextureAssetReader( ) = default;

TextureMip TextureAssetReader::FindMip( const uint32_t mipLevel, const uint32_t arrayLayer )
{
    for ( uint32_t i = 0; i < m_textureAsset->Mips.NumElements; ++i )
    {
        if ( const TextureMip &mip = m_textureAsset->Mips.Elements[ i ]; mip.MipIndex == mipLevel && mip.ArrayIndex == arrayLayer )
        {
            return mip;
        }
    }
    return { };
}

TextureAsset *TextureAssetReader::Read( )
{
    if ( m_textureRead )
    {
        return m_textureAsset;
    }
    m_textureRead        = true;
    m_textureAsset       = new TextureAsset( );
    m_textureAsset->Magic = m_reader->ReadUInt64( );
    if ( m_textureAsset->Magic != TextureAsset{ }.Magic )
    {
        spdlog::critical( "Invalid TextureAsset magic number." );
    }

    m_textureAsset->Version = m_reader->ReadUInt32( );
    if ( m_textureAsset->Version > TextureAsset::Latest )
    {
        spdlog::warn( "TextureAsset version mismatch." );
    }
    m_textureAsset->NumBytes = m_reader->ReadUInt64( );
    m_textureAsset->Uri      = AssetUri::Parse( m_reader->ReadString( ) );
    m_textureAsset->_Arena.EnsureCapacity( m_textureAsset->NumBytes );

    m_textureAsset->Name         = m_reader->ReadString( );
    m_textureAsset->SourcePath   = m_reader->ReadString( );
    m_textureAsset->Width        = m_reader->ReadUInt32( );
    m_textureAsset->Height       = m_reader->ReadUInt32( );
    m_textureAsset->Depth        = m_reader->ReadUInt32( );
    m_textureAsset->Format       = static_cast<Format>( m_reader->ReadUInt32( ) );
    m_textureAsset->Dimension    = static_cast<TextureDimension>( m_reader->ReadUInt32( ) );
    m_textureAsset->MipLevels    = m_reader->ReadUInt32( );
    m_textureAsset->ArraySize    = m_reader->ReadUInt32( );
    m_textureAsset->BitsPerPixel = m_reader->ReadUInt32( );
    m_textureAsset->BlockSize    = m_reader->ReadUInt32( );
    m_textureAsset->RowPitch     = m_reader->ReadUInt32( );
    m_textureAsset->NumRows      = m_reader->ReadUInt32( );
    m_textureAsset->SlicePitch   = m_reader->ReadUInt32( );

    const uint32_t numMips = m_reader->ReadUInt32( );
    DZArenaArrayHelper<TextureMipArray, TextureMip>::AllocateAndConstructArray( m_textureAsset->_Arena, m_textureAsset->Mips, numMips );

    for ( uint32_t i = 0; i < numMips; ++i )
    {
        TextureMip &mip = m_textureAsset->Mips.Elements[ i ];
        mip.Width       = m_reader->ReadUInt32( );
        mip.Height      = m_reader->ReadUInt32( );
        mip.MipIndex    = m_reader->ReadUInt32( );
        mip.ArrayIndex  = m_reader->ReadUInt32( );
        mip.RowPitch    = m_reader->ReadUInt32( );
        mip.NumRows     = m_reader->ReadUInt32( );
        mip.SlicePitch  = m_reader->ReadUInt32( );
        mip.DataOffset  = m_reader->ReadUInt32( );
    }

    m_textureAsset->Data = AssetReaderHelpers::ReadAssetDataStream( m_reader );
    return m_textureAsset;
}

ByteArray TextureAssetReader::ReadRaw( const uint32_t mipLevel, const uint32_t arrayLayer )
{
    if ( mipLevel >= m_textureAsset->MipLevels || arrayLayer >= m_textureAsset->ArraySize )
    {
        spdlog::critical( "Invalid mip level or array layer requested" );
        return { };
    }

    const TextureMip mip    = FindMip( mipLevel, arrayLayer );
    const uint64_t   offset = m_textureAsset->Data.Offset + mip.DataOffset;

    m_reader->Seek( offset );
    ByteArray mipData{ };
    mipData.Elements    = static_cast<Byte *>( std::malloc( mip.SlicePitch ) );
    mipData.NumElements = mip.SlicePitch;
    if ( const uint32_t bytesRead = m_reader->Read( mipData, 0, mip.SlicePitch ); bytesRead != mip.SlicePitch )
    {
        spdlog::error( "Could not read complete mip data. Expected {} bytes, got {}", mip.SlicePitch, bytesRead );
    }

    return mipData;
}

void TextureAssetReader::LoadIntoGpuTexture( const LoadIntoGpuTextureDesc &desc )
{
    if ( !desc.CommandList || !desc.Texture )
    {
        spdlog::critical( "CommandList and Texture are required for LoadIntoGpuTexture" );
        return;
    }

    constexpr uint32_t batchSize = 1024;
    std::vector<Byte>  buffer( batchSize );
    const ByteArray    bufferArray{ buffer.data( ), buffer.size( ) };

    const auto stagingBuffer  = desc.StagingBuffer;
    uint64_t   remainingBytes = m_textureAsset->Data.NumBytes;
    auto       mappedMemory   = static_cast<Byte *>( stagingBuffer->MapMemory( ) );

    m_reader->Seek( m_textureAsset->Data.Offset );
    while ( remainingBytes > 0 )
    {
        const uint32_t bytesToRead = static_cast<uint32_t>( std::min( static_cast<uint64_t>( batchSize ), remainingBytes ) );
        const uint32_t bytesRead   = m_reader->Read( bufferArray, 0, bytesToRead );

        if ( bytesRead != bytesToRead )
        {
            spdlog::error( "Failed to read expected number of bytes. Expected: {} , Read: {}", bytesToRead, bytesRead );
            break;
        }

        memcpy( mappedMemory, buffer.data( ), bytesRead );
        mappedMemory += bytesRead;
        remainingBytes -= bytesRead;
    }

    stagingBuffer->UnmapMemory( );

    for ( uint32_t i = 0; i < m_textureAsset->Mips.NumElements; ++i )
    {
        const TextureMip mip = m_textureAsset->Mips.Elements[ i ];

        CopyBufferToTextureDesc copyDesc{ };
        copyDesc.DstTexture = desc.Texture;
        copyDesc.SrcBuffer  = stagingBuffer;
        copyDesc.SrcOffset  = mip.DataOffset;
        copyDesc.DstX       = 0;
        copyDesc.DstY       = 0;
        copyDesc.DstZ       = 0;
        copyDesc.Format     = desc.Texture->GetFormat( );
        copyDesc.MipLevel   = mip.MipIndex;
        copyDesc.ArrayLayer = mip.ArrayIndex;
        copyDesc.RowPitch   = mip.RowPitch;
        copyDesc.NumRows    = mip.NumRows;

        desc.CommandList->CopyBufferToTexture( copyDesc );
    }
}

uint64_t TextureAssetReader::AlignedTotalNumBytes( const DeviceConstants &constants )
{
    uint64_t totalNumBytes = 0;
    for ( uint32_t i = 0; i < m_textureAsset->Mips.NumElements; ++i )
    {
        const TextureMip &mip               = m_textureAsset->Mips.Elements[ i ];
        const uint32_t    alignedRowPitch   = Utilities::Align( mip.RowPitch, constants.BufferTextureRowAlignment );
        const uint32_t    alignedSlicePitch = Utilities::Align( alignedRowPitch * mip.NumRows, constants.BufferTextureAlignment );
        totalNumBytes += alignedSlicePitch;
    }
    return totalNumBytes;
}
