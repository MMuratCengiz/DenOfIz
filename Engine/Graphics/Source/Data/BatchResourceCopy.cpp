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

#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#endif
/* TODO !THEFORGE! Uncomment once The reference is removed.
#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>
#endif
*/

using namespace DenOfIz;

BatchResourceCopy::BatchResourceCopy( ILogicalDevice *device ) : m_device( device )
{
    m_commandListPool = m_device->CreateCommandListPool( { QueueType::Copy } );
    DZ_ASSERTM( m_commandListPool->GetCommandLists( ).size( ) > 0, "Command list pool did not produce any command lists." );

    m_copyCommandList = m_commandListPool->GetCommandLists( )[ 0 ];
    m_executeFence    = m_device->CreateFence( );
}

BatchResourceCopy::~BatchResourceCopy( )
{
    m_executeFence.reset( );
    m_commandListPool.reset( );
}

void BatchResourceCopy::Begin( )
{
    m_copyCommandList->Begin( );
}

void BatchResourceCopy::CopyToGPUBuffer( const CopyToGpuBufferDesc &copyDesc )
{
    BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.NumBytes     = Utilities::Align( copyDesc.NumBytes, m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );

    auto stagingBuffer = m_device->CreateBufferResource( "StagingBuffer", stagingBufferDesc );
    memcpy( stagingBuffer->MapMemory( ), copyDesc.Data, copyDesc.NumBytes );
    stagingBuffer->UnmapMemory( );

    CopyBufferRegionDesc copyBufferRegionDesc{ };
    copyBufferRegionDesc.DstBuffer = copyDesc.DstBuffer;
    copyBufferRegionDesc.SrcBuffer = stagingBuffer.get( );
    copyBufferRegionDesc.NumBytes  = copyDesc.NumBytes;

    CopyBufferRegion( copyBufferRegionDesc );

    std::lock_guard<std::mutex> lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::move( stagingBuffer ) );
}

void BatchResourceCopy::CopyBufferRegion( const CopyBufferRegionDesc &copyDesc )
{
    m_copyCommandList->CopyBufferRegion( copyDesc );
}

void BatchResourceCopy::CopyTextureRegion( const CopyTextureRegionDesc &copyDesc )
{
    m_copyCommandList->CopyTextureRegion( copyDesc );
}

void BatchResourceCopy::CopyDataToTexture( const CopyDataToTextureDesc &copyDesc )
{
    BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.NumBytes     = copyDesc.NumBytes;

    auto  stagingBuffer = m_device->CreateBufferResource( "StagingBuffer", stagingBufferDesc );
    void *dst           = stagingBuffer->MapMemory( );
    memcpy( dst, copyDesc.Data, copyDesc.NumBytes );
    stagingBuffer->UnmapMemory( );

    CopyBufferToTextureDesc copyBufferToTextureDesc{ };
    copyBufferToTextureDesc.DstTexture = copyDesc.DstTexture;
    copyBufferToTextureDesc.SrcBuffer  = stagingBuffer.get( );
    copyBufferToTextureDesc.Format     = FormatToTypeless( copyDesc.DstTexture->GetFormat( ) );
    copyBufferToTextureDesc.MipLevel   = copyDesc.MipLevel;
    copyBufferToTextureDesc.ArrayLayer = copyDesc.ArrayLayer;
    m_copyCommandList->CopyBufferToTexture( copyBufferToTextureDesc );

    std::lock_guard<std::mutex> lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::move( stagingBuffer ) );
}

std::unique_ptr<ITextureResource> BatchResourceCopy::CreateAndLoadTexture( const std::string &resourceName, const std::string &file )
{
    Texture texture( file );

    TextureDesc textureDesc{ };
    textureDesc.HeapType     = HeapType::GPU;
    textureDesc.Descriptor   = ResourceDescriptor::Texture;
    textureDesc.InitialState = ResourceState::CopyDst;
    textureDesc.Width        = texture.Width;
    textureDesc.Height       = texture.Height;
    textureDesc.Format       = texture.Format;
    textureDesc.Depth        = texture.Depth;
    textureDesc.ArraySize    = texture.ArraySize;
    textureDesc.MipLevels    = texture.MipLevels;

    auto outTex = m_device->CreateTextureResource( resourceName, textureDesc );
    LoadTextureInternal( texture, outTex.get( ) );
    return std::move( outTex );
}

void BatchResourceCopy::LoadTexture( const LoadTextureDesc &loadDesc )
{
    Texture texture( loadDesc.File );
    LoadTextureInternal( texture, loadDesc.DstTexture );
}

void BatchResourceCopy::End( ISemaphore *notify )
{
    ExecuteDesc desc{ };

    desc.Notify = m_executeFence.get( );
    if ( notify )
    {
        desc.NotifySemaphores.push_back( notify );
    }

    m_copyCommandList->Execute( desc );
    m_cleanResourcesFuture = std::async( std::launch::async, [ this ]( ) { CleanResources( ); } );
}

void BatchResourceCopy::CleanResources( )
{
    std::lock_guard<std::mutex> lock( m_resourceCleanLock );
    m_executeFence->Wait( );
    m_resourcesToClean.clear( );

    for ( auto &texture : m_freeTextures )
    {
        free( texture );
    }
}

void BatchResourceCopy::LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture )
{
    BufferDesc stagingBufferDesc   = { };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;

    for ( uint32_t i = 0; i < texture.MipLevels; ++i )
    {
        uint32_t mipRowPitch   = Utilities::Align( std::max( 1u, texture.RowPitch >> i ), m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
        uint32_t mipNumRows    = std::max( 1u, texture.NumRows >> i );
        uint32_t mipSlicePitch = Utilities::Align( texture.Depth * mipRowPitch * mipNumRows, m_device->DeviceInfo( ).Constants.BufferTextureAlignment );
        stagingBufferDesc.NumBytes += mipSlicePitch;
    }

    auto  stagingBuffer       = m_device->CreateBufferResource( "StagingBuffer", stagingBufferDesc );
    Byte *stagingMappedMemory = (Byte *)stagingBuffer->MapMemory( );

    texture.StreamMipData(
        [ & ]( MipData mipData )
        {
            CopyTextureToMemoryAligned( texture, mipData, stagingMappedMemory + mipData.DataOffset );

            CopyBufferToTextureDesc copyBufferToTextureDesc{ };
            copyBufferToTextureDesc.DstTexture = dstTexture;
            copyBufferToTextureDesc.SrcBuffer  = stagingBuffer.get( );
            copyBufferToTextureDesc.SrcOffset  = mipData.DataOffset;
            copyBufferToTextureDesc.Format     = dstTexture->GetFormat( );
            copyBufferToTextureDesc.MipLevel   = mipData.MipIndex;
            copyBufferToTextureDesc.ArrayLayer = mipData.ArrayIndex;
            copyBufferToTextureDesc.RowPitch   = mipData.RowPitch;
            copyBufferToTextureDesc.NumRows    = mipData.NumRows;
            m_copyCommandList->CopyBufferToTexture( copyBufferToTextureDesc );
        } );

    stagingBuffer->UnmapMemory( );
    std::lock_guard<std::mutex> lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::move( stagingBuffer ) );
}

void BatchResourceCopy::CopyTextureToMemoryAligned( const Texture &texture, const MipData &mipData, Byte *dst )
{
    uint32_t alignedRowPitch   = Utilities::Align( mipData.RowPitch, m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
    uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * mipData.NumRows, GetSubresourceAlignment( texture.BitsPerPixel ) );

    const Byte *pSrcData = texture.Data.data( ) + mipData.DataOffset;
    for ( uint32_t z = 0; z < texture.ArraySize; ++z )
    {
        auto dstSlice = dst + alignedSlicePitch * z;
        auto srcSlice = pSrcData + mipData.SlicePitch * z;
        for ( uint32_t y = 0; y < mipData.NumRows; ++y )
        {
            memcpy( dstSlice + alignedRowPitch * y, srcSlice + mipData.RowPitch * y, mipData.RowPitch );
        }
    }
}

uint32_t BatchResourceCopy::GetSubresourceAlignment( uint32_t bitSize )
{
    uint32_t blockSize = std::max( 1u, bitSize >> 3 );
    uint32_t alignment = Utilities::Align( m_device->DeviceInfo( ).Constants.BufferTextureAlignment, blockSize );
    return Utilities::Align( alignment, m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
}
