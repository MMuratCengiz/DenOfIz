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

#include "gtest/gtest.h"

#include "../../../../Internal/DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class TextureAssetSerdeTest : public testing::Test
{
    std::unique_ptr<TextureAsset> m_asset;

protected:
    static std::vector<Byte> CreateTestPixelData( const uint32_t width, const uint32_t height, const uint32_t mipLevel )
    {
        constexpr uint32_t pixelSize = 4;
        std::vector<Byte>  data( width * height * pixelSize );

        for ( uint32_t i = 0; i < data.size( ); ++i )
        {
            data[ i ] = static_cast<Byte>( ( i + mipLevel * 50 ) % 256 );
        }

        return data;
    }

    TextureAsset *CreateSampleTextureAsset( )
    {
        m_asset             = std::make_unique<TextureAsset>( );
        m_asset->Name       = "TestTexture";
        m_asset->Uri        = AssetUri::Create( "textures/TestTexture.dztex" );
        m_asset->SourcePath = "original/textures/test.dds";

        m_asset->Width     = 256;
        m_asset->Height    = 256;
        m_asset->Depth     = 1;
        m_asset->Format    = Format::R8G8B8A8Unorm;
        m_asset->Dimension = TextureDimension::Texture2D;

        m_asset->MipLevels    = 3;
        m_asset->ArraySize    = 1;
        m_asset->BitsPerPixel = 32;
        m_asset->BlockSize    = 1;
        m_asset->RowPitch     = m_asset->Width * 4;
        m_asset->NumRows      = m_asset->Height;
        m_asset->SlicePitch   = m_asset->RowPitch * m_asset->NumRows;

        m_asset->_Arena.EnsureCapacity( 2048 );
        DZArenaArrayHelper<TextureMipArray, TextureMip>::AllocateAndConstructArray( m_asset->_Arena, m_asset->Mips, m_asset->MipLevels );

        for ( uint32_t mip = 0; mip < m_asset->MipLevels; ++mip )
        {
            const uint32_t mipWidth  = m_asset->Width >> mip;
            const uint32_t mipHeight = m_asset->Height >> mip;

            TextureMip &mipDesc = m_asset->Mips.Elements[ mip ];
            mipDesc.Width       = mipWidth;
            mipDesc.Height      = mipHeight;
            mipDesc.MipIndex    = mip;
            mipDesc.ArrayIndex  = 0;
            mipDesc.RowPitch    = mipWidth * 4;
            mipDesc.NumRows     = mipHeight;
            mipDesc.SlicePitch  = mipDesc.RowPitch * mipDesc.NumRows;
            mipDesc.DataOffset  = 0;
        }

        return m_asset.get( );
    }
};

TEST_F( TextureAssetSerdeTest, WriteAndReadBack )
{
    BinaryContainer container;
    auto            sampleAsset = std::unique_ptr<TextureAsset>( CreateSampleTextureAsset( ) );

    {
        BinaryWriter       writer( container );
        TextureAssetWriter textureWriter( TextureAssetWriterDesc{ &writer } );
        textureWriter.Write( *sampleAsset );

        for ( uint32_t mip = 0; mip < sampleAsset->MipLevels; ++mip )
        {
            uint32_t mipWidth  = sampleAsset->Width >> mip;
            uint32_t mipHeight = sampleAsset->Height >> mip;

            std::vector<Byte> pixelData = CreateTestPixelData( mipWidth, mipHeight, mip );
            ByteArrayView     data{ };
            data.Elements    = pixelData.data( );
            data.NumElements = pixelData.size( );
            textureWriter.AddPixelData( data, mip, 0 );
        }

        textureWriter.End( );
    }
    BinaryReader       reader( container );
    TextureAssetReader textureReader( TextureAssetReaderDesc{ &reader } );
    auto               readAsset = std::unique_ptr<TextureAsset>( textureReader.Read( ) );

    ASSERT_EQ( readAsset->Magic, TextureAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, TextureAsset::Latest );
    ASSERT_STREQ( readAsset->Name.Get( ), sampleAsset->Name.Get( ) );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->SourcePath.Get( ), sampleAsset->SourcePath.Get( ) );

    ASSERT_EQ( readAsset->Width, sampleAsset->Width );
    ASSERT_EQ( readAsset->Height, sampleAsset->Height );
    ASSERT_EQ( readAsset->Depth, sampleAsset->Depth );
    ASSERT_EQ( readAsset->Format, sampleAsset->Format );
    ASSERT_EQ( readAsset->Dimension, sampleAsset->Dimension );

    ASSERT_EQ( readAsset->MipLevels, sampleAsset->MipLevels );
    ASSERT_EQ( readAsset->ArraySize, sampleAsset->ArraySize );
    ASSERT_EQ( readAsset->BitsPerPixel, sampleAsset->BitsPerPixel );
    ASSERT_EQ( readAsset->BlockSize, sampleAsset->BlockSize );
    ASSERT_EQ( readAsset->RowPitch, sampleAsset->RowPitch );
    ASSERT_EQ( readAsset->NumRows, sampleAsset->NumRows );
    ASSERT_EQ( readAsset->SlicePitch, sampleAsset->SlicePitch );

    ASSERT_EQ( readAsset->Mips.NumElements, sampleAsset->MipLevels );

    for ( uint32_t i = 0; i < readAsset->Mips.NumElements; ++i )
    {
        const TextureMip &readMip = readAsset->Mips.Elements[ i ];

        ASSERT_EQ( readMip.MipIndex, i );
        ASSERT_EQ( readMip.ArrayIndex, 0 );
        ASSERT_EQ( readMip.Width, sampleAsset->Width >> i );
        ASSERT_EQ( readMip.Height, sampleAsset->Height >> i );
        ASSERT_EQ( readMip.RowPitch, ( sampleAsset->Width >> i ) * 4 );
        ASSERT_EQ( readMip.NumRows, sampleAsset->Height >> i );
        ASSERT_EQ( readMip.SlicePitch, readMip.RowPitch * readMip.NumRows );

        if ( i > 0 )
        {
            ASSERT_GT( readMip.DataOffset, 0 );
        }
        else
        {
            ASSERT_EQ( readMip.DataOffset, 0 );
        }
    }

    ASSERT_GT( readAsset->Data.NumBytes, 0 );

    for ( uint32_t mip = 0; mip < readAsset->MipLevels; ++mip )
    {
        ByteArray readMipData = textureReader.ReadRaw( mip, 0 );

        const TextureMip &mipDesc = readAsset->Mips.Elements[ mip ];
        ASSERT_EQ( readMipData.NumElements, mipDesc.SlicePitch );

        auto      expectedDataVec = CreateTestPixelData( mipDesc.Width, mipDesc.Height, mip );
        ByteArray expectedData{ };
        expectedData.Elements    = expectedDataVec.data( );
        expectedData.NumElements = expectedDataVec.size( );
        for ( const size_t checkPoints[] = { 0, 16, 64, static_cast<size_t>( mipDesc.SlicePitch / 2 ), static_cast<size_t>( mipDesc.SlicePitch - 1 ) }; size_t point : checkPoints )
        {
            if ( point < readMipData.NumElements )
            {
                ASSERT_EQ( readMipData.Elements[ point ], expectedData.Elements[ point ] ) << "Data mismatch at mip " << mip << " offset " << point;
            }
        }
    }
}
