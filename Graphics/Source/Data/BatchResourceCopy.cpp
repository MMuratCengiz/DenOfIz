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

/* TODO !THEFORGE! Uncomment once The reference is removed.
#ifndef TINYGLTF_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>
#endif
*/

using namespace DenOfIz;

BatchResourceCopy::BatchResourceCopy( ILogicalDevice *device, const bool issueBarriers ) : m_device( device ), m_issueBarriers( issueBarriers )
{
    m_commandListPool = std::unique_ptr<ICommandListPool>( m_device->CreateCommandListPool( { QueueType::Copy } ) );
    DZ_ASSERTM( m_commandListPool->GetCommandLists( ).NumElements( ) != 0, "Command list pool did not produce any command lists." );

    m_copyCommandList = m_commandListPool->GetCommandLists( ).GetElement( 0 );
    m_executeFence    = std::unique_ptr<IFence>( m_device->CreateFence( ) );

    if ( m_issueBarriers )
    {
        CommandListPoolDesc poolDesc{ };
        poolDesc.QueueType       = QueueType::Graphics;
        poolDesc.NumCommandLists = 1;
        m_syncCommandPool        = std::unique_ptr<ICommandListPool>( m_device->CreateCommandListPool( poolDesc ) );

        m_syncCommandList = m_syncCommandPool->GetCommandLists( ).GetElement( 0 );
        m_batchCopyWait   = std::unique_ptr<ISemaphore>( m_device->CreateSemaphore( ) );
        m_syncWait        = std::unique_ptr<IFence>( m_device->CreateFence( ) );
    }
}

BatchResourceCopy::~BatchResourceCopy( )
{
    if ( m_issueBarriers )
    {
        m_cleanResourcesFuture.wait( );
        m_executeFence.reset( );
    }
    m_commandListPool.reset( );
}

void BatchResourceCopy::Begin( ) const
{
    m_copyCommandList->Begin( );
    if ( m_issueBarriers )
    {
        m_syncCommandList->Begin( );
    }
}

void BatchResourceCopy::CopyToGPUBuffer( const CopyToGpuBufferDesc &copyDesc )
{
    BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.NumBytes     = Utilities::Align( copyDesc.NumBytes, m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
    stagingBufferDesc.DebugName    = "CopyToGPUBuffer_StagingBuffer";

    const auto stagingBuffer = m_device->CreateBufferResource( stagingBufferDesc );
    memcpy( stagingBuffer->MapMemory( ), copyDesc.Data, copyDesc.NumBytes );
    stagingBuffer->UnmapMemory( );

    CopyBufferRegionDesc copyBufferRegionDesc{ };
    copyBufferRegionDesc.DstBuffer = copyDesc.DstBuffer;
    copyBufferRegionDesc.SrcBuffer = stagingBuffer;
    copyBufferRegionDesc.NumBytes  = copyDesc.NumBytes;

    CopyBufferRegion( copyBufferRegionDesc );

    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::unique_ptr<IBufferResource>( stagingBuffer ) );
}

void BatchResourceCopy::CopyBufferRegion( const CopyBufferRegionDesc &copyDesc ) const
{
    m_copyCommandList->CopyBufferRegion( copyDesc );
}

void BatchResourceCopy::CopyTextureRegion( const CopyTextureRegionDesc &copyDesc ) const
{
    m_copyCommandList->CopyTextureRegion( copyDesc );
}

void BatchResourceCopy::CopyDataToTexture( const CopyDataToTextureDesc &copyDesc )
{
    BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.NumBytes     = copyDesc.NumBytes;
    stagingBufferDesc.DebugName    = "CopyDataToTexture_StagingBuffer";

    const auto stagingBuffer = m_device->CreateBufferResource( stagingBufferDesc );
    void      *dst           = stagingBuffer->MapMemory( );
    memcpy( dst, copyDesc.Data, copyDesc.NumBytes );
    stagingBuffer->UnmapMemory( );

    CopyBufferToTextureDesc copyBufferToTextureDesc{ };
    copyBufferToTextureDesc.DstTexture = copyDesc.DstTexture;
    copyBufferToTextureDesc.SrcBuffer  = stagingBuffer;
    copyBufferToTextureDesc.Format     = FormatToTypeless( copyDesc.DstTexture->GetFormat( ) );
    copyBufferToTextureDesc.MipLevel   = copyDesc.MipLevel;
    copyBufferToTextureDesc.ArrayLayer = copyDesc.ArrayLayer;
    m_copyCommandList->CopyBufferToTexture( copyBufferToTextureDesc );

    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::unique_ptr<IBufferResource>( stagingBuffer ) );
}

ITextureResource *BatchResourceCopy::CreateAndLoadTexture( const InteropString &file )
{
    Texture texture( file.Get( ) );

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
    textureDesc.DebugName.Append( "CreateAndLoadTexture(" ).Append( file.Get( ) ).Append( ")" );

    auto outTex = m_device->CreateTextureResource( textureDesc );
    LoadTextureInternal( texture, outTex );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.TextureBarrier( { .Resource = outTex, .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }
    return outTex;
}

IBufferResource *BatchResourceCopy::CreateUniformBuffer( const void *data, const uint32_t numBytes )
{
    BufferDesc bufferDesc{ };
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    bufferDesc.InitialState = ResourceState::CopyDst;
    bufferDesc.NumBytes     = numBytes;
    bufferDesc.DebugName    = NextId( "Uniform" );

    const auto buffer = m_device->CreateBufferResource( bufferDesc );

    CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer = buffer;
    copyDesc.Data      = data;
    copyDesc.NumBytes  = numBytes;
    CopyToGPUBuffer( copyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = buffer, .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }

    return buffer;
}

[[nodiscard]] IBufferResource *BatchResourceCopy::CreateGeometryVertexBuffer( const GeometryData &geometryData )
{
    BufferDesc vBufferDesc{ };
    vBufferDesc.HeapType     = HeapType::GPU;
    vBufferDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vBufferDesc.InitialState = ResourceState::CopyDst;
    vBufferDesc.NumBytes     = geometryData.Vertices.NumElements( ) * sizeof( GeometryVertexData );
    vBufferDesc.DebugName    = NextId( "Vertex" );

    const auto vertexBuffer = m_device->CreateBufferResource( vBufferDesc );

    CopyToGpuBufferDesc vbCopyDesc{ };
    vbCopyDesc.DstBuffer = vertexBuffer;
    vbCopyDesc.Data      = geometryData.Vertices.Data( );
    vbCopyDesc.NumBytes  = geometryData.Vertices.NumElements( ) * sizeof( GeometryVertexData );
    CopyToGPUBuffer( vbCopyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = vertexBuffer, .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }

    return vertexBuffer;
}

[[nodiscard]] IBufferResource *BatchResourceCopy::CreateGeometryIndexBuffer( const GeometryData &geometryData )
{
    BufferDesc iBufferDesc{ };
    iBufferDesc.HeapType        = HeapType::GPU;
    iBufferDesc.Descriptor      = ResourceDescriptor::IndexBuffer;
    iBufferDesc.InitialState    = ResourceState::CopyDst;
    iBufferDesc.NumBytes        = geometryData.Indices.NumElements( ) * sizeof( uint32_t );
    const char *indexBufferName = "IndexBuffer";
    iBufferDesc.DebugName       = NextId( indexBufferName );

    const auto indexBuffer = m_device->CreateBufferResource( iBufferDesc );

    CopyToGpuBufferDesc ibCopyDesc{ };
    ibCopyDesc.DstBuffer = indexBuffer;
    ibCopyDesc.Data      = geometryData.Indices.Data( );
    ibCopyDesc.NumBytes  = geometryData.Indices.NumElements( ) * sizeof( uint32_t );
    CopyToGPUBuffer( ibCopyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = indexBuffer, .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }
    return indexBuffer;
}

void BatchResourceCopy::LoadTexture( const LoadTextureDesc &loadDesc )
{
    const Texture texture( loadDesc.File.Get( ) );
    LoadTextureInternal( texture, loadDesc.DstTexture );
}

void BatchResourceCopy::Submit( ISemaphore *notify )
{
    ExecuteDesc desc{ };

    m_executeFence->Reset( );
    desc.Notify = m_executeFence.get( );
    desc.NotifySemaphores.AddElement( m_batchCopyWait.get( ) );
    if ( notify )
    {
        desc.NotifySemaphores.AddElement( notify );
    }

    m_copyCommandList->Execute( desc );
    m_cleanResourcesFuture = std::async( std::launch::async, [ this ] { CleanResources( ); } );

    if ( m_issueBarriers )
    {
        m_syncWait->Reset( );
        ExecuteDesc executeDesc{ };
        executeDesc.WaitOnSemaphores.AddElement( m_batchCopyWait.get( ) );
        executeDesc.Notify = m_syncWait.get( );
        m_syncCommandList->Execute( executeDesc );
        m_syncWait->Wait( );
    }
}

void BatchResourceCopy::CleanResources( )
{
    std::lock_guard lock( m_resourceCleanLock );
    m_executeFence->Wait( );
    m_resourcesToClean.clear( );

    for ( const auto &texture : m_freeTextures )
    {
        free( texture );
    }
}

void BatchResourceCopy::LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture )
{
    BufferDesc stagingBufferDesc   = { };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.DebugName    = "LoadTexture_StagingBuffer";

    for ( uint32_t i = 0; i < texture.MipLevels; ++i )
    {
        const uint32_t mipRowPitch   = Utilities::Align( std::max( 1u, texture.RowPitch >> i ), m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
        const uint32_t mipNumRows    = std::max( 1u, texture.NumRows >> i );
        const uint32_t mipSlicePitch = Utilities::Align( texture.Depth * mipRowPitch * mipNumRows, m_device->DeviceInfo( ).Constants.BufferTextureAlignment );
        stagingBufferDesc.NumBytes += mipSlicePitch;
    }

    const auto stagingBuffer       = m_device->CreateBufferResource( stagingBufferDesc );
    Byte      *stagingMappedMemory = static_cast<Byte *>( stagingBuffer->MapMemory( ) );

    texture.StreamMipData(
        [ & ]( const MipData &mipData )
        {
            CopyTextureToMemoryAligned( texture, mipData, stagingMappedMemory + mipData.DataOffset );

            CopyBufferToTextureDesc copyBufferToTextureDesc{ };
            copyBufferToTextureDesc.DstTexture = dstTexture;
            copyBufferToTextureDesc.SrcBuffer  = stagingBuffer;
            copyBufferToTextureDesc.SrcOffset  = mipData.DataOffset;
            copyBufferToTextureDesc.Format     = dstTexture->GetFormat( );
            copyBufferToTextureDesc.MipLevel   = mipData.MipIndex;
            copyBufferToTextureDesc.ArrayLayer = mipData.ArrayIndex;
            copyBufferToTextureDesc.RowPitch   = mipData.RowPitch;
            copyBufferToTextureDesc.NumRows    = mipData.NumRows;
            m_copyCommandList->CopyBufferToTexture( copyBufferToTextureDesc );
        } );

    stagingBuffer->UnmapMemory( );
    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::unique_ptr<IBufferResource>( stagingBuffer ) );
}

void BatchResourceCopy::CopyTextureToMemoryAligned( const Texture &texture, const MipData &mipData, Byte *dst ) const
{
    const uint32_t alignedRowPitch   = Utilities::Align( mipData.RowPitch, m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
    const uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * mipData.NumRows, GetSubresourceAlignment( texture.BitsPerPixel ) );

    const Byte *pSrcData = texture.Data.data( ) + mipData.DataOffset;
    for ( uint32_t z = 0; z < texture.ArraySize; ++z )
    {
        const auto dstSlice = dst + alignedSlicePitch * z;
        const auto srcSlice = pSrcData + mipData.SlicePitch * z;
        for ( uint32_t y = 0; y < mipData.NumRows; ++y )
        {
            memcpy( dstSlice + alignedRowPitch * y, srcSlice + mipData.RowPitch * y, mipData.RowPitch );
        }
    }
}

uint32_t BatchResourceCopy::GetSubresourceAlignment( const uint32_t bitSize ) const
{
    const uint32_t blockSize = std::max( 1u, bitSize >> 3 );
    const uint32_t alignment = Utilities::Align( m_device->DeviceInfo( ).Constants.BufferTextureAlignment, blockSize );
    return Utilities::Align( alignment, m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
}

const char *BatchResourceCopy::NextId( const char *prefix )
{
#ifndef NDEBUG
    static std::atomic<unsigned int> idCounter( 0 );
    const int                        next = idCounter.fetch_add( 1, std::memory_order_relaxed );
    return ( std::string( prefix ) + "_BatchResourceCopyResource#" + std::to_string( next ) ).c_str( );
#else
    return prefix + "_BatchResourceCopyResource";
#endif
}
