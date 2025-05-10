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


using namespace DenOfIz;

BatchResourceCopy::BatchResourceCopy( ILogicalDevice *device, const bool issueBarriers ) : m_device( device ), m_issueBarriers( issueBarriers )
{
    m_copyQueue = std::unique_ptr<ICommandQueue>( m_device->CreateCommandQueue( CommandQueueDesc{ QueueType::Copy } ) );

    m_commandListPool = std::unique_ptr<ICommandListPool>( m_device->CreateCommandListPool( { m_copyQueue.get( ) } ) );
    auto commandLists = m_commandListPool->GetCommandLists( );
    DZ_ASSERTM( commandLists.NumElements( ) != 0, "Command list pool did not produce any command lists." );

    m_copyCommandList = commandLists.GetElement( 0 );
    m_executeFence    = std::unique_ptr<IFence>( m_device->CreateFence( ) );

    if ( m_issueBarriers )
    {
        m_syncQueue = std::unique_ptr<ICommandQueue>( m_device->CreateCommandQueue( CommandQueueDesc{ QueueType::Graphics } ) );

        CommandListPoolDesc poolDesc{ };
        poolDesc.CommandQueue    = m_syncQueue.get( );
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
    stagingBufferDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingBufferDesc.NumBytes     = Utilities::Align( copyDesc.Data.NumElements( ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
    stagingBufferDesc.DebugName    = "CopyToGPUBuffer_StagingBuffer";

    const auto stagingBuffer = m_device->CreateBufferResource( stagingBufferDesc );
    memcpy( stagingBuffer->MapMemory( ), copyDesc.Data.Data( ), copyDesc.Data.NumElements( ) );
    stagingBuffer->UnmapMemory( );

    CopyBufferRegionDesc copyBufferRegionDesc{ };
    copyBufferRegionDesc.DstBuffer = copyDesc.DstBuffer;
    copyBufferRegionDesc.SrcBuffer = stagingBuffer;
    copyBufferRegionDesc.DstOffset = copyDesc.DstBufferOffset;
    copyBufferRegionDesc.NumBytes  = copyDesc.Data.NumElements( );

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
    stagingBufferDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingBufferDesc.NumBytes     = copyDesc.Data.NumElements( );
    stagingBufferDesc.DebugName    = "CopyDataToTexture_StagingBuffer";

    const auto stagingBuffer = m_device->CreateBufferResource( stagingBufferDesc );
    void      *dst           = stagingBuffer->MapMemory( );
    memcpy( dst, copyDesc.Data.Data( ), copyDesc.Data.NumElements( ) );
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
    textureDesc.InitialUsage = ResourceUsage::CopyDst;
    textureDesc.Width        = texture.Width;
    textureDesc.Height       = texture.Height;
    textureDesc.Format       = texture.Format;
    textureDesc.Depth        = texture.Depth;
    textureDesc.ArraySize    = texture.ArraySize;
    textureDesc.MipLevels    = texture.MipLevels;
    textureDesc.DebugName    = InteropString( "CreateAndLoadTexture(" ).Append( file.Get( ) ).Append( ")" );

    auto outTex = m_device->CreateTextureResource( textureDesc );
    LoadTextureInternal( texture, outTex );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.TextureBarrier( { .Resource = outTex, .OldState = ResourceUsage::Common, .NewState = ResourceUsage::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }
    return outTex;
}

ITextureResource *BatchResourceCopy::CreateAndLoadAssetTexture( const CreateAssetTextureDesc &loadDesc )
{
    const TextureAsset &textureAsset = loadDesc.Reader->Read( );

    TextureDesc textureDesc{ };
    textureDesc.HeapType     = HeapType::GPU;
    textureDesc.Descriptor   = ResourceDescriptor::Texture;
    textureDesc.InitialUsage = ResourceUsage::CopyDst;
    textureDesc.Width        = textureAsset.Width;
    textureDesc.Height       = textureAsset.Height;
    textureDesc.Depth        = textureAsset.Depth;
    textureDesc.Format       = textureAsset.Format;
    textureDesc.ArraySize    = textureAsset.ArraySize;
    textureDesc.MipLevels    = textureAsset.MipLevels;

    BitSet descriptors = ResourceDescriptor::Texture;
    if ( textureAsset.Dimension == TextureDimension::TextureCube )
    {
        descriptors |= ResourceDescriptor::TextureCube;
    }

    descriptors |= loadDesc.AdditionalDescriptors;
    textureDesc.Descriptor = descriptors;

    BitSet<ResourceUsage> usages = BitSet( ResourceUsage::ShaderResource ) | ResourceUsage::CopyDst;
    usages |= loadDesc.AdditionalUsages;
    textureDesc.Usages = usages;

    if ( !loadDesc.DebugName.IsEmpty( ) )
    {
        textureDesc.DebugName = loadDesc.DebugName;
    }
    else
    {
        textureDesc.DebugName = "TextureFromAsset:";
        if ( !textureAsset.Name.IsEmpty( ) )
        {
            textureDesc.DebugName.Append( textureAsset.Name.Get( ) );
        }
        else if ( !textureAsset.SourcePath.IsEmpty( ) )
        {
            textureDesc.DebugName.Append( textureAsset.SourcePath.Get( ) );
        }
    }

    ITextureResource *texture = m_device->CreateTextureResource( textureDesc );

    LoadAssetTextureDesc loadAssetTextureDesc{ };
    loadAssetTextureDesc.Reader     = loadDesc.Reader;
    loadAssetTextureDesc.DstTexture = texture;
    LoadAssetTexture( loadAssetTextureDesc );

    return texture;
}

IBufferResource *BatchResourceCopy::CreateUniformBuffer( const InteropArray<Byte> &data, const uint32_t numBytes )
{
    BufferDesc bufferDesc{ };
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    bufferDesc.InitialUsage = ResourceUsage::CopyDst;
    bufferDesc.NumBytes     = numBytes;
    bufferDesc.DebugName    = NextId( "Uniform" ).c_str( );

    const auto buffer = m_device->CreateBufferResource( bufferDesc );

    CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer = buffer;
    copyDesc.Data      = data;
    CopyToGPUBuffer( copyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = buffer, .OldState = ResourceUsage::CopyDst, .NewState = ResourceUsage::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }

    return buffer;
}

[[nodiscard]] IBufferResource *BatchResourceCopy::CreateGeometryVertexBuffer( const GeometryData &geometryData )
{
    const size_t numBytes = geometryData.Vertices.NumElements( ) * sizeof( GeometryVertexData );

    BufferDesc vBufferDesc{ };
    vBufferDesc.HeapType     = HeapType::GPU;
    vBufferDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vBufferDesc.InitialUsage = ResourceUsage::CopyDst;
    vBufferDesc.NumBytes     = numBytes;
    vBufferDesc.DebugName    = NextId( "Vertex" ).c_str( );

    const auto vertexBuffer = m_device->CreateBufferResource( vBufferDesc );

    CopyToGpuBufferDesc vbCopyDesc{ };
    vbCopyDesc.DstBuffer = vertexBuffer;
    // Todo not efficient at all, fix later
    vbCopyDesc.Data.MemCpy( geometryData.Vertices.Data( ), numBytes );
    CopyToGPUBuffer( vbCopyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = vertexBuffer, .OldState = ResourceUsage::CopyDst, .NewState = ResourceUsage::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }

    return vertexBuffer;
}

[[nodiscard]] IBufferResource *BatchResourceCopy::CreateGeometryIndexBuffer( const GeometryData &geometryData )
{
    const size_t numBytes = geometryData.Indices.NumElements( ) * sizeof( uint32_t );

    BufferDesc iBufferDesc{ };
    iBufferDesc.HeapType     = HeapType::GPU;
    iBufferDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
    iBufferDesc.InitialUsage = ResourceUsage::CopyDst;
    iBufferDesc.NumBytes     = numBytes;
    iBufferDesc.DebugName    = NextId( "IndexBuffer" ).c_str( );

    const auto indexBuffer = m_device->CreateBufferResource( iBufferDesc );

    CopyToGpuBufferDesc ibCopyDesc{ };
    ibCopyDesc.DstBuffer = indexBuffer;
    // Todo not efficient at all, fix later
    ibCopyDesc.Data.MemCpy( geometryData.Indices.Data( ), numBytes );
    CopyToGPUBuffer( ibCopyDesc );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.BufferBarrier( { .Resource = indexBuffer, .OldState = ResourceUsage::CopyDst, .NewState = ResourceUsage::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }
    return indexBuffer;
}

void BatchResourceCopy::LoadTexture( const LoadTextureDesc &loadDesc )
{
    const Texture texture( loadDesc.File.Get( ) );
    LoadTextureInternal( texture, loadDesc.DstTexture );
}

void BatchResourceCopy::LoadAssetTexture( const LoadAssetTextureDesc &loadDesc )
{
    if ( !loadDesc.Reader || !loadDesc.DstTexture )
    {
        LOG( ERROR ) << "TextureAssetReader and DstTexture cannot be null";
        return;
    }

    BufferDesc stagingBufferDesc   = { };
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingBufferDesc.DebugName    = "LoadAssetTexture_StagingBuffer";
    stagingBufferDesc.NumBytes     = loadDesc.Reader->AlignedTotalNumBytes( m_device->DeviceInfo( ).Constants );

    const auto             stagingBuffer = m_device->CreateBufferResource( stagingBufferDesc );
    LoadIntoGpuTextureDesc readerLoadDesc{ };
    readerLoadDesc.Texture       = loadDesc.DstTexture;
    readerLoadDesc.CommandList   = m_copyCommandList;
    readerLoadDesc.StagingBuffer = stagingBuffer;

    loadDesc.Reader->LoadIntoGpuTexture( readerLoadDesc );

    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( std::unique_ptr<IBufferResource>( stagingBuffer ) );

    if ( m_issueBarriers )
    {
        const PipelineBarrierDesc barrierDesc =
            PipelineBarrierDesc{ }.TextureBarrier( { .Resource = loadDesc.DstTexture, .OldState = ResourceUsage::Common, .NewState = ResourceUsage::ShaderResource } );
        m_syncCommandList->PipelineBarrier( barrierDesc );
    }
}

void BatchResourceCopy::LoadAssetStreamToBuffer( const LoadAssetStreamToBufferDesc &loadDesc )
{
    DZ_NOT_NULL( loadDesc.DstBuffer );
    DZ_NOT_NULL( loadDesc.Reader );
    if ( loadDesc.Stream.NumBytes == 0 )
    {
        LOG( WARNING ) << "LoadAssetStreamToBuffer: Stream has no data to load.";
        return;
    }

    const auto reader   = loadDesc.Reader;
    const auto position = reader->Position( ); // Todo is rollback necessary?
    reader->Seek( loadDesc.Stream.Offset );
    InteropArray<Byte> fullData( loadDesc.Stream.NumBytes );
    uint64_t           memBytesCopied = 0;
    while ( memBytesCopied < loadDesc.Stream.NumBytes )
    {
        constexpr size_t chunkSize            = 65536;
        const uint64_t   bytesToReadMem       = std::min<uint32_t>( chunkSize, loadDesc.Stream.NumBytes - memBytesCopied );
        const int        bytesActuallyReadMem = reader->Read( fullData, static_cast<uint32_t>( memBytesCopied ), static_cast<uint32_t>( bytesToReadMem ) );
        if ( bytesActuallyReadMem != static_cast<int>( bytesToReadMem ) )
        {
            LOG( FATAL ) << "Failed to read expected chunk size from mesh asset stream into memory.";
        }
        memBytesCopied += bytesActuallyReadMem;
    }
    CopyToGpuBufferDesc copyDesc;
    copyDesc.DstBuffer = loadDesc.DstBuffer;
    copyDesc.Data      = std::move( fullData );
    if ( loadDesc.DstBufferOffset != 0 )
    {
        LOG( WARNING ) << "LoadStreamToBuffer: DstBufferOffset ignored by CopyToGPUBuffer.";
    }
    CopyToGPUBuffer( copyDesc );
    reader->Seek( position );
}

void BatchResourceCopy::Submit( ISemaphore *notify )
{
    m_copyCommandList->End( );

    ExecuteCommandListsDesc desc{ };
    m_executeFence->Reset( );
    desc.Signal = m_executeFence.get( );
    desc.SignalSemaphores.AddElement( m_batchCopyWait.get( ) );
    if ( notify )
    {
        desc.SignalSemaphores.AddElement( notify );
    }
    desc.CommandLists.AddElement( m_copyCommandList );
    m_copyQueue->ExecuteCommandLists( desc );
    m_cleanResourcesFuture = std::async( std::launch::async, [ this ] { CleanResources( ); } );

    if ( m_issueBarriers )
    {
        m_syncWait->Reset( );
        ExecuteCommandListsDesc syncDesc{ };
        syncDesc.Signal = m_syncWait.get( );
        syncDesc.WaitSemaphores.AddElement( m_batchCopyWait.get( ) );
        syncDesc.CommandLists.AddElement( m_syncCommandList );
        m_syncCommandList->End( );
        m_syncQueue->ExecuteCommandLists( syncDesc );
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
    stagingBufferDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingBufferDesc.DebugName    = "LoadTexture_StagingBuffer";

    for ( uint32_t i = 0; i < texture.MipLevels; ++i )
    {
        const uint32_t mipRowPitch   = Utilities::Align( std::max( 1u, texture.RowPitch >> i ), m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
        const uint32_t mipNumRows    = std::max( 1u, texture.NumRows >> i );
        const uint32_t mipSlicePitch = Utilities::Align( texture.Depth * mipRowPitch * mipNumRows, m_device->DeviceInfo( ).Constants.BufferTextureAlignment );
        stagingBufferDesc.NumBytes += mipSlicePitch;
    }

    const auto stagingBuffer       = m_device->CreateBufferResource( stagingBufferDesc );
    const auto stagingMappedMemory = static_cast<Byte *>( stagingBuffer->MapMemory( ) );

    texture.StreamMipData(
        [ & ]( const TextureMip &mipData )
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

void BatchResourceCopy::CopyTextureToMemoryAligned( const Texture &texture, const TextureMip &mipData, Byte *dst ) const
{
    const uint32_t alignedRowPitch   = Utilities::Align( mipData.RowPitch, m_device->DeviceInfo( ).Constants.BufferTextureRowAlignment );
    const uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * mipData.NumRows, GetSubresourceAlignment( texture.BitsPerPixel ) );

    const Byte *pSrcData = texture.Data.Data( ) + mipData.DataOffset;
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

std::string BatchResourceCopy::NextId( const std::string &prefix )
{
#ifndef NDEBUG
    static std::atomic<unsigned int> idCounter( 0 );
    const int                        next = idCounter.fetch_add( 1, std::memory_order_relaxed );
    return std::string( prefix ) + "_BatchResourceCopyResource#" + std::to_string( next );
#else
    return "BatchResourceCopyResource";
#endif
}
