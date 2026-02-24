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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define TEXTURE_ASSET_READER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextureAssetReader, handle )
#define TEXTURE_ASSET_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextureAsset, handle )

namespace DenOfIz
{
    class TextureAssetReader
    {
        DenOfIz_BinaryReader m_reader;
        DenOfIz_TextureAsset m_textureAsset = DENOFIZ_NULL_HANDLE;
        bool                 m_textureRead  = false;

        DenOfIz_TextureMip FindMip( const uint32_t mipLevel, const uint32_t arrayLayer ) const
        {
            const DenOfIz_TextureMipArray mips = DenOfIz_TextureAsset_Mips( m_textureAsset );
            for ( uint32_t i = 0; i < mips.NumElements; ++i )
            {
                if ( const DenOfIz_TextureMip &mip = mips.Elements[ i ]; mip.MipIndex == mipLevel && mip.ArrayIndex == arrayLayer )
                {
                    return mip;
                }
            }
            return { };
        }

    public:
        static constexpr uint32_t TextureAssetLatestVersion = 1;

        explicit TextureAssetReader( const DenOfIz_TextureAssetReaderDesc &desc ) : m_reader( desc.Reader )
        {
            if ( !DENOFIZ_HANDLE_IS_VALID( m_reader ) )
            {
                spdlog::critical( "BinaryReader cannot be null for TextureAssetReader" );
            }
        }

        ~TextureAssetReader( )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( m_textureAsset ) )
            {
                DenOfIz_TextureAsset_Destroy( m_textureAsset );
            }
        }

        DenOfIz_TextureAsset Read( )
        {
            if ( m_textureRead )
            {
                return m_textureAsset;
            }
            m_textureRead  = true;
            m_textureAsset = DenOfIz_TextureAsset_Create( );

            const uint64_t magic = DenOfIz_BinaryReader_ReadUInt64( m_reader );
            if ( magic != DenOfIz_TextureAsset_Magic( m_textureAsset ) )
            {
                spdlog::critical( "Invalid TextureAsset magic number." );
            }

            const uint32_t version = DenOfIz_BinaryReader_ReadUInt32( m_reader );
            if ( version > TextureAssetLatestVersion )
            {
                spdlog::warn( "TextureAsset version mismatch." );
            }
            DenOfIz_TextureAsset_SetVersion( m_textureAsset, version );
            DenOfIz_TextureAsset_SetNumBytes( m_textureAsset, DenOfIz_BinaryReader_ReadUInt64( m_reader ) );
            DenOfIz_TextureAsset_SetPath( m_textureAsset, DenOfIz_StringView( DenOfIz_BinaryReader_ReadString( m_reader ) ) );
            DenOfIz_TextureAsset_SetName( m_textureAsset, DenOfIz_StringView( DenOfIz_BinaryReader_ReadString( m_reader ) ) );
            DenOfIz_TextureAsset_SetSourcePath( m_textureAsset, DenOfIz_StringView( DenOfIz_BinaryReader_ReadString( m_reader ) ) );
            DenOfIz_TextureAsset_SetWidth( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetHeight( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetDepth( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetFormat( m_textureAsset, static_cast<DenOfIz_Format>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) ) );
            DenOfIz_TextureAsset_SetDimension( m_textureAsset, static_cast<DenOfIz_TextureDimension>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) ) );
            DenOfIz_TextureAsset_SetMipLevels( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetArraySize( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetBitsPerPixel( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetBlockSize( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetRowPitch( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetNumRows( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
            DenOfIz_TextureAsset_SetSlicePitch( m_textureAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );

            const uint32_t numMips = DenOfIz_BinaryReader_ReadUInt32( m_reader );
            DenOfIz_TextureAsset_ReserveMips( m_textureAsset, numMips );

            for ( uint32_t i = 0; i < numMips; ++i )
            {
                DenOfIz_TextureMip mip{ };
                mip.Width      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.Height     = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.MipIndex   = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.ArrayIndex = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.RowPitch   = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.NumRows    = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.SlicePitch = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                mip.DataOffset = DenOfIz_BinaryReader_ReadUInt32( m_reader );
                DenOfIz_TextureAsset_AddMip( m_textureAsset, &mip );
            }

            DenOfIz_TextureAsset_SetData( m_textureAsset, AssetReaderHelpers::ReadAssetDataStream( m_reader ) );
            return m_textureAsset;
        }

        DenOfIz_ByteArray ReadRaw( const uint32_t mipLevel, const uint32_t arrayLayer ) const
        {
            if ( mipLevel >= DenOfIz_TextureAsset_MipLevels( m_textureAsset ) || arrayLayer >= DenOfIz_TextureAsset_ArraySize( m_textureAsset ) )
            {
                spdlog::critical( "Invalid mip level or array layer requested" );
                return { };
            }

            const DenOfIz_TextureMip      mip    = FindMip( mipLevel, arrayLayer );
            const DenOfIz_AssetDataStream data   = DenOfIz_TextureAsset_Data( m_textureAsset );
            const uint64_t                offset = data.Offset + mip.DataOffset;

            DenOfIz_BinaryReader_Seek( m_reader, offset );
            DenOfIz_ByteArray mipData{ };
            mipData.Elements    = static_cast<Byte *>( std::malloc( mip.SlicePitch ) );
            mipData.NumElements = mip.SlicePitch;
            if ( const uint32_t bytesRead = DenOfIz_BinaryReader_Read( m_reader, mipData, 0, mip.SlicePitch ); bytesRead != mip.SlicePitch )
            {
                spdlog::error( "Could not read complete mip data. Expected {} bytes, got {}", mip.SlicePitch, bytesRead );
            }

            return mipData;
        }

        void LoadIntoGpuTexture( const DenOfIz_LoadIntoGpuTextureDesc &desc ) const
        {
            if ( !DENOFIZ_HANDLE_IS_VALID( desc.CommandList ) || !DENOFIZ_HANDLE_IS_VALID( desc.Texture ) )
            {
                spdlog::critical( "CommandList and Texture are required for LoadIntoGpuTexture" );
                return;
            }

            constexpr uint32_t      batchSize = 1024;
            std::vector<Byte>       buffer( batchSize );
            const DenOfIz_ByteArray bufferArray{ buffer.data( ), buffer.size( ) };

            const DenOfIz_AssetDataStream data          = DenOfIz_TextureAsset_Data( m_textureAsset );
            const auto                    stagingBuffer = desc.StagingBuffer;
            size_t                        numBufferBytes;
            DenOfIz_Buffer_NumBytes( desc.StagingBuffer, &numBufferBytes );
            uint64_t remainingBytes = std::min<uint64_t>( numBufferBytes, data.NumBytes );
            Byte    *mappedMemory;
            void    *mappedVoid = NULL;
            DenOfIz_Buffer_MapMemory( stagingBuffer, &mappedVoid );
            mappedMemory = static_cast<Byte *>( mappedVoid );
            DenOfIz_BinaryReader_Seek( m_reader, data.Offset );
            while ( remainingBytes > 0 )
            {
                const uint32_t bytesToRead = static_cast<uint32_t>( std::min( static_cast<uint64_t>( batchSize ), remainingBytes ) );
                const uint32_t bytesRead   = DenOfIz_BinaryReader_Read( m_reader, bufferArray, 0, bytesToRead );

                if ( bytesRead != bytesToRead )
                {
                    spdlog::error( "Failed to read expected number of bytes. Expected: {} , Read: {}", bytesToRead, bytesRead );
                    break;
                }

                memcpy( mappedMemory, buffer.data( ), bytesRead );
                mappedMemory += bytesRead;
                remainingBytes -= bytesRead;
            }

            DenOfIz_Buffer_UnmapMemory( stagingBuffer );

            const DenOfIz_TextureMipArray mips = DenOfIz_TextureAsset_Mips( m_textureAsset );
            for ( uint32_t i = 0; i < mips.NumElements; ++i )
            {
                const DenOfIz_TextureMip mip = mips.Elements[ i ];

                DenOfIz_CopyBufferToTextureDesc copyDesc{ };
                copyDesc.DstTexture = desc.Texture;
                copyDesc.SrcBuffer  = stagingBuffer;
                copyDesc.SrcOffset  = mip.DataOffset;
                copyDesc.DstX       = 0;
                copyDesc.DstY       = 0;
                copyDesc.DstZ       = 0;
                DenOfIz_TextureResource_GetFormat( desc.Texture, &copyDesc.Format );
                copyDesc.MipLevel   = mip.MipIndex;
                copyDesc.ArrayLayer = mip.ArrayIndex;
                copyDesc.RowPitch   = mip.RowPitch;
                copyDesc.NumRows    = mip.NumRows;

                DenOfIz_CommandList_CopyBufferToTexture( desc.CommandList, &copyDesc );
            }
        }

        uint64_t AlignedTotalNumBytes( const DenOfIz_DeviceConstants &constants ) const
        {
            uint64_t                      totalNumBytes = 0;
            const DenOfIz_TextureMipArray mips          = DenOfIz_TextureAsset_Mips( m_textureAsset );
            for ( uint32_t i = 0; i < mips.NumElements; ++i )
            {
                const DenOfIz_TextureMip &mip               = mips.Elements[ i ];
                const uint32_t            alignedRowPitch   = Utilities::Align( mip.RowPitch, constants.BufferTextureRowAlignment );
                const uint32_t            alignedSlicePitch = Utilities::Align( alignedRowPitch * mip.NumRows, constants.BufferTextureAlignment );
                totalNumBytes += alignedSlicePitch;
            }
            return totalNumBytes;
        }

        DenOfIz_TextureAsset GetTextureAsset( ) const
        {
            return m_textureAsset;
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_TextureAssetReader DenOfIz_TextureAssetReader_Create( const DenOfIz_TextureAssetReaderDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *reader = new DenOfIz::TextureAssetReader( *desc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    void DenOfIz_TextureAssetReader_Destroy( DenOfIz_TextureAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        delete TEXTURE_ASSET_READER_IMPL( reader );
    }

    DenOfIz_TextureAsset DenOfIz_TextureAssetReader_Read( DenOfIz_TextureAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return TEXTURE_ASSET_READER_IMPL( reader )->Read( );
    }

    void DenOfIz_TextureAssetReader_LoadIntoGpuTexture( DenOfIz_TextureAssetReader reader, const DenOfIz_LoadIntoGpuTextureDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) || desc == NULL )
        {
            return;
        }
        TEXTURE_ASSET_READER_IMPL( reader )->LoadIntoGpuTexture( *desc );
    }

    DenOfIz_ByteArray DenOfIz_TextureAssetReader_ReadRaw( DenOfIz_TextureAssetReader reader, uint32_t mipLevel, uint32_t arrayLayer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_ASSET_READER_IMPL( reader )->ReadRaw( mipLevel, arrayLayer );
    }

    uint64_t DenOfIz_TextureAssetReader_AlignedTotalNumBytes( DenOfIz_TextureAssetReader reader, const DenOfIz_DeviceConstants *constants )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) || constants == NULL )
        {
            return 0;
        }
        return TEXTURE_ASSET_READER_IMPL( reader )->AlignedTotalNumBytes( *constants );
    }
}
