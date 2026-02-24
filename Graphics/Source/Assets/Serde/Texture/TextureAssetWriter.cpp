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

#include <algorithm>
#include <cstddef>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define TEXTURE_ASSET_WRITER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextureAssetWriter, handle )

namespace DenOfIz
{
    class TextureAssetWriter
    {
        DenOfIz_BinaryWriter            m_writer;
        DenOfIz_TextureAssetWriterDesc  m_desc;
        DenOfIz_TextureAsset            m_textureAsset            = DENOFIZ_NULL_HANDLE;
        bool                            m_ownsTextureAsset        = false;
        uint64_t                        m_assetDataStreamPosition = 0;
        std::vector<uint64_t>           m_textureMipPositions;
        std::vector<DenOfIz_TextureMip> m_mipsStorage;

        uint64_t m_streamStartLocation = 0;
        uint32_t m_lastMipIndex        = 0;
        uint32_t m_lastArrayIndex      = 0;
        bool     m_isFirstMip          = true;

        void WriteHeader( uint64_t totalNumBytes ) const
        {
            DenOfIz_BinaryWriter_WriteUInt64( m_writer, DenOfIz_TextureAsset_Magic( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_Version( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt64( m_writer, totalNumBytes );
            const DenOfIz_StringView path = DenOfIz_TextureAsset_Path( m_textureAsset );
            DenOfIz_BinaryWriter_WriteString( m_writer, path );
        }

        void WriteMipInfo( const DenOfIz_TextureMip &mip ) const
        {
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.Width );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.Height );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.MipIndex );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.ArrayIndex );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.RowPitch );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.NumRows );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.SlicePitch );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, mip.DataOffset );
        }

        void ValidateMipRange( const uint32_t mipIndex, const uint32_t arrayLayer )
        {
            if ( mipIndex >= DenOfIz_TextureAsset_MipLevels( m_textureAsset ) || arrayLayer >= DenOfIz_TextureAsset_ArraySize( m_textureAsset ) )
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

    public:
        explicit TextureAssetWriter( const DenOfIz_TextureAssetWriterDesc &desc ) : m_writer( desc.Writer ), m_desc( desc )
        {
            if ( !DENOFIZ_HANDLE_IS_VALID( m_writer ) )
            {
                spdlog::critical( "BinaryWriter cannot be null for TextureAssetWriter" );
            }
        }

        ~TextureAssetWriter( )
        {
            if ( m_ownsTextureAsset && DENOFIZ_HANDLE_IS_VALID( m_textureAsset ) )
            {
                DenOfIz_TextureAsset_Destroy( m_textureAsset );
            }
        }

        void Write( DenOfIz_TextureAsset textureAsset )
        {
            m_textureAsset     = DenOfIz_TextureAsset_Create( );
            m_ownsTextureAsset = true;

            DenOfIz_TextureAsset_SetName( m_textureAsset, DenOfIz_TextureAsset_Name( textureAsset ) );
            DenOfIz_TextureAsset_SetSourcePath( m_textureAsset, DenOfIz_TextureAsset_SourcePath( textureAsset ) );
            DenOfIz_TextureAsset_SetWidth( m_textureAsset, DenOfIz_TextureAsset_Width( textureAsset ) );
            DenOfIz_TextureAsset_SetHeight( m_textureAsset, DenOfIz_TextureAsset_Height( textureAsset ) );
            DenOfIz_TextureAsset_SetDepth( m_textureAsset, DenOfIz_TextureAsset_Depth( textureAsset ) );
            DenOfIz_TextureAsset_SetFormat( m_textureAsset, DenOfIz_TextureAsset_GetFormat( textureAsset ) );
            DenOfIz_TextureAsset_SetDimension( m_textureAsset, DenOfIz_TextureAsset_GetDimension( textureAsset ) );
            DenOfIz_TextureAsset_SetMipLevels( m_textureAsset, DenOfIz_TextureAsset_MipLevels( textureAsset ) );
            DenOfIz_TextureAsset_SetArraySize( m_textureAsset, DenOfIz_TextureAsset_ArraySize( textureAsset ) );
            DenOfIz_TextureAsset_SetBitsPerPixel( m_textureAsset, DenOfIz_TextureAsset_BitsPerPixel( textureAsset ) );
            DenOfIz_TextureAsset_SetBlockSize( m_textureAsset, DenOfIz_TextureAsset_BlockSize( textureAsset ) );
            DenOfIz_TextureAsset_SetRowPitch( m_textureAsset, DenOfIz_TextureAsset_RowPitch( textureAsset ) );
            DenOfIz_TextureAsset_SetNumRows( m_textureAsset, DenOfIz_TextureAsset_NumRows( textureAsset ) );
            DenOfIz_TextureAsset_SetSlicePitch( m_textureAsset, DenOfIz_TextureAsset_SlicePitch( textureAsset ) );

            const DenOfIz_TextureMipArray mips = DenOfIz_TextureAsset_Mips( textureAsset );
            if ( mips.NumElements > 0 )
            {
                DenOfIz_TextureAsset_SetMips( m_textureAsset, mips.Elements, mips.NumElements );
            }
            DenOfIz_TextureAsset_SetData( m_textureAsset, DenOfIz_TextureAsset_Data( textureAsset ) );

            m_streamStartLocation = DenOfIz_BinaryWriter_Position( m_writer );
            WriteHeader( 0 );
            const DenOfIz_StringView name       = DenOfIz_TextureAsset_Name( m_textureAsset );
            const DenOfIz_StringView sourcePath = DenOfIz_TextureAsset_SourcePath( m_textureAsset );
            DenOfIz_BinaryWriter_WriteString( m_writer, name );
            DenOfIz_BinaryWriter_WriteString( m_writer, sourcePath );

            if ( DenOfIz_TextureAsset_Width( m_textureAsset ) == 0 || DenOfIz_TextureAsset_Height( m_textureAsset ) == 0 )
            {
                spdlog::warn( "TextureAssetWriter: Texture dimensions are zero, which may indicate uninitialized data" );
            }

            DenOfIz_Format format = DenOfIz_TextureAsset_GetFormat( m_textureAsset );
            if ( format == DENOFIZ_FORMAT_UNDEFINED )
            {
                spdlog::warn( "TextureAssetWriter: Texture format is undefined, defaulting to R8G8B8A8Unorm" );
                DenOfIz_TextureAsset_SetFormat( m_textureAsset, DENOFIZ_FORMAT_R8G8B8A8_UNORM );
                format = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
            }

            uint32_t bitsPerPixel = DenOfIz_TextureAsset_BitsPerPixel( m_textureAsset );
            if ( bitsPerPixel == 0 )
            {
                spdlog::warn( "TextureAssetWriter: BitsPerPixel is 0, attempting to set based on format" );
                switch ( format )
                {
                case DENOFIZ_FORMAT_R8G8B8A8_UNORM:
                case DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB:
                case DENOFIZ_FORMAT_R8G8B8A8_UINT:
                case DENOFIZ_FORMAT_R8G8B8A8_SNORM:
                case DENOFIZ_FORMAT_R8G8B8A8_SINT:
                case DENOFIZ_FORMAT_B8G8R8A8_UNORM:
                    bitsPerPixel = 32;
                    break;
                case DENOFIZ_FORMAT_BC1_UNORM:
                case DENOFIZ_FORMAT_BC1_UNORM_SRGB:
                    bitsPerPixel = 4;
                    break;
                case DENOFIZ_FORMAT_BC2_UNORM:
                case DENOFIZ_FORMAT_BC2_UNORM_SRGB:
                case DENOFIZ_FORMAT_BC3_UNORM:
                case DENOFIZ_FORMAT_BC3_UNORM_SRGB:
                    bitsPerPixel = 8;
                    break;
                case DENOFIZ_FORMAT_BC4_UNORM:
                case DENOFIZ_FORMAT_BC4_SNORM:
                    bitsPerPixel = 4;
                    break;
                case DENOFIZ_FORMAT_BC5_UNORM:
                case DENOFIZ_FORMAT_BC5_SNORM:
                case DENOFIZ_FORMAT_BC7_UNORM:
                    bitsPerPixel = 8;
                    break;
                default:
                    bitsPerPixel = 32;
                    break;
                }
                DenOfIz_TextureAsset_SetBitsPerPixel( m_textureAsset, bitsPerPixel );
            }

            uint32_t blockSize = DenOfIz_TextureAsset_BlockSize( m_textureAsset );
            if ( blockSize == 0 )
            {
                if ( DenOfIz_Format_IsBC( format ) )
                {
                    blockSize = 4;
                }
                else
                {
                    blockSize = 1;
                }
                DenOfIz_TextureAsset_SetBlockSize( m_textureAsset, blockSize );
            }

            uint32_t rowPitch = DenOfIz_TextureAsset_RowPitch( m_textureAsset );
            if ( rowPitch == 0 )
            {
                if ( format >= DENOFIZ_FORMAT_BC1_UNORM && format <= DENOFIZ_FORMAT_BC7_UNORM )
                {
                    const uint32_t blockWidth = ( DenOfIz_TextureAsset_Width( m_textureAsset ) + blockSize - 1 ) / blockSize;
                    rowPitch                  = blockWidth * ( bitsPerPixel / 8 );
                }
                else
                {
                    rowPitch = DenOfIz_TextureAsset_Width( m_textureAsset ) * ( bitsPerPixel / 8 );
                }
                DenOfIz_TextureAsset_SetRowPitch( m_textureAsset, rowPitch );
            }

            uint32_t numRows = DenOfIz_TextureAsset_NumRows( m_textureAsset );
            if ( numRows == 0 )
            {
                spdlog::warn( "TextureAssetWriter: NumRows is 0, calculating based on height and format" );
                if ( format >= DENOFIZ_FORMAT_BC1_UNORM && format <= DENOFIZ_FORMAT_BC7_UNORM )
                {
                    numRows = ( DenOfIz_TextureAsset_Height( m_textureAsset ) + blockSize - 1 ) / blockSize;
                }
                else
                {
                    numRows = DenOfIz_TextureAsset_Height( m_textureAsset );
                }
                DenOfIz_TextureAsset_SetNumRows( m_textureAsset, numRows );
            }

            uint32_t slicePitch = DenOfIz_TextureAsset_SlicePitch( m_textureAsset );
            if ( slicePitch == 0 )
            {
                slicePitch = rowPitch * numRows;
                DenOfIz_TextureAsset_SetSlicePitch( m_textureAsset, slicePitch );
            }

            if ( DenOfIz_TextureAsset_NumMips( m_textureAsset ) == 0 )
            {
                spdlog::critical( "TextureAssetWriter: No mip levels provided. Importers must provide pre-allocated mips" );
            }

            DenOfIz_BinaryWriter_WriteUInt32( m_writer, std::max( 1u, DenOfIz_TextureAsset_Width( m_textureAsset ) ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, std::max( 1u, DenOfIz_TextureAsset_Height( m_textureAsset ) ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_Depth( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( DenOfIz_TextureAsset_GetFormat( m_textureAsset ) ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( DenOfIz_TextureAsset_GetDimension( m_textureAsset ) ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_MipLevels( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_ArraySize( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_BitsPerPixel( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_BlockSize( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_RowPitch( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_NumRows( m_textureAsset ) );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_TextureAsset_SlicePitch( m_textureAsset ) );

            const DenOfIz_TextureMipArray writerMips = DenOfIz_TextureAsset_Mips( m_textureAsset );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( writerMips.NumElements ) );

            m_mipsStorage.assign( writerMips.Elements, writerMips.Elements + writerMips.NumElements );
            m_textureMipPositions.resize( writerMips.NumElements );
            for ( size_t i = 0; i < writerMips.NumElements; ++i )
            {
                m_textureMipPositions[ i ] = DenOfIz_BinaryWriter_Position( m_writer );
                WriteMipInfo( writerMips.Elements[ i ] );
            }

            AssetWriterHelpers::WriteAssetDataStream( m_writer, { 0, 0 } );
            m_assetDataStreamPosition = DenOfIz_BinaryWriter_Position( m_writer );
        }

        void AddPixelData( const DenOfIz_ByteArrayView &bytes, const uint32_t mipIndex, const uint32_t arrayLayer )
        {
            ValidateMipRange( mipIndex, arrayLayer );

            const uint32_t currentDataOffset = static_cast<uint32_t>( DenOfIz_BinaryWriter_Position( m_writer ) - m_assetDataStreamPosition );
            bool           mipFound          = false;

            for ( size_t i = 0; i < m_mipsStorage.size( ); ++i )
            {
                if ( DenOfIz_TextureMip &mip = m_mipsStorage[ i ]; mip.MipIndex == mipIndex && mip.ArrayIndex == arrayLayer )
                {
                    if ( mip.DataOffset == 0 )
                    {
                        mip.DataOffset = currentDataOffset;

                        const uint64_t currentPos = DenOfIz_BinaryWriter_Position( m_writer );
                        DenOfIz_BinaryWriter_Seek( m_writer, m_textureMipPositions[ i ] + offsetof( DenOfIz_TextureMip, DataOffset ) );
                        DenOfIz_BinaryWriter_WriteUInt32( m_writer, currentDataOffset );
                        DenOfIz_BinaryWriter_Seek( m_writer, currentPos );
                    }
                    mipFound = true;
                    break;
                }
            }

            DenOfIz_TextureAsset_SetMips( m_textureAsset, m_mipsStorage.data( ), m_mipsStorage.size( ) );

            if ( !mipFound )
            {
                spdlog::critical( "TextureAssetWriter: Mip level {} array layer {} not found in pre-allocated mips array", mipIndex, arrayLayer );
            }

            m_lastMipIndex   = mipIndex;
            m_lastArrayIndex = arrayLayer;
            DenOfIz_BinaryWriter_WriteBytes( m_writer, bytes );
        }

        void End( ) const
        {
            const uint64_t currentPos = DenOfIz_BinaryWriter_Position( m_writer );

            DenOfIz_BinaryWriter_Seek( m_writer, m_assetDataStreamPosition - sizeof( DenOfIz_AssetDataStream ) );
            const DenOfIz_AssetDataStream stream = { m_assetDataStreamPosition, currentPos - m_assetDataStreamPosition };
            AssetWriterHelpers::WriteAssetDataStream( m_writer, stream );

            DenOfIz_BinaryWriter_Seek( m_writer, m_streamStartLocation );
            WriteHeader( currentPos - m_streamStartLocation );
            DenOfIz_BinaryWriter_Seek( m_writer, currentPos );
            DenOfIz_BinaryWriter_Flush( m_writer );
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_TextureAssetWriter DenOfIz_TextureAssetWriter_Create( const DenOfIz_TextureAssetWriterDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *writer = new DenOfIz::TextureAssetWriter( *desc );
        return DENOFIZ_TO_HANDLE( writer );
    }

    void DenOfIz_TextureAssetWriter_Destroy( DenOfIz_TextureAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        delete TEXTURE_ASSET_WRITER_IMPL( writer );
    }

    void DenOfIz_TextureAssetWriter_Write( DenOfIz_TextureAssetWriter writer, DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) || !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_WRITER_IMPL( writer )->Write( textureAsset );
    }

    void DenOfIz_TextureAssetWriter_AddPixelData( DenOfIz_TextureAssetWriter writer, const DenOfIz_ByteArrayView *bytes, uint32_t mipIndex, uint32_t arrayLayer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) || bytes == NULL )
        {
            return;
        }
        TEXTURE_ASSET_WRITER_IMPL( writer )->AddPixelData( *bytes, mipIndex, arrayLayer );
    }

    void DenOfIz_TextureAssetWriter_End( DenOfIz_TextureAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        TEXTURE_ASSET_WRITER_IMPL( writer )->End( );
    }
}
