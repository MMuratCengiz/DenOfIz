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

#include <array>
#include <atomic>
#include <future>
#include <mutex>
#include <string>
#include <vector>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

namespace DenOfIz
{
    class BatchResourceCopy
    {
        DenOfIz_LogicalDevice m_device;

        DenOfIz_CommandListPool m_commandListPool;
        DenOfIz_CommandList     m_copyCommandList;

        DenOfIz_Fence               m_executeFence;
        std::mutex                  m_resourceCleanLock;
        std::vector<DenOfIz_Buffer> m_resourcesToClean;
        std::vector<Byte *>         m_freeTextures;
        std::future<void>           m_cleanResourcesFuture;

        DenOfIz_CommandListPool m_syncCommandPool;
        DenOfIz_CommandList     m_syncCommandList;
        DenOfIz_Semaphore       m_batchCopyWait;
        DenOfIz_Fence           m_syncWait;
        bool                    m_issueBarriers;

        DenOfIz_CommandQueue m_copyQueue;
        DenOfIz_CommandQueue m_syncQueue;

    public:
        explicit BatchResourceCopy( const DenOfIz_BatchResourceCopyDesc &desc );
        ~BatchResourceCopy( );

        void            Begin( ) const;
        void            CopyToGPUBuffer( const DenOfIz_CopyToGpuBufferDesc &copyDesc );
        void            CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyDesc ) const;
        void            CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyDesc ) const;
        void            CopyDataToTexture( const DenOfIz_CopyDataToTextureDesc &copyDesc );
        DenOfIz_Texture CreateAndLoadTexture( DenOfIz_StringView file );
        void            LoadTexture( const DenOfIz_LoadTextureDesc &loadDesc );
        void            LoadTextureFromData( const DenOfIz_LoadTextureFromDataDesc &loadDesc );
        DenOfIz_Buffer  CreateUniformBuffer( const DenOfIz_ByteArrayView &data, uint32_t numBytes );
        DenOfIz_Buffer  CreateGeometryVertexBuffer( const DenOfIz_GeometryData &geometryData );
        DenOfIz_Buffer  CreateGeometryIndexBuffer( const DenOfIz_GeometryData &geometryData );
        void            Submit( DenOfIz_Semaphore notify );

    private:
        void               CleanResources( );
        void               LoadTextureInternal( DenOfIz_TextureData texture, DenOfIz_Texture dstTexture );
        void               AlignDataForTexture( const Byte *src, uint32_t width, uint32_t height, uint32_t bitsize, Byte *dst ) const;
        void               CopyTextureToMemoryAligned( DenOfIz_TextureData texture, const DenOfIz_TextureMip &mipData, Byte *dst ) const;
        uint32_t           GetSubresourceAlignment( uint32_t bitSize ) const;
        static std::string NextId( const std::string &prefix );
    };
} // namespace DenOfIz

using namespace DenOfIz;

BatchResourceCopy::BatchResourceCopy( const DenOfIz_BatchResourceCopyDesc &desc ) : m_device( desc.Device ), m_issueBarriers( desc.IssueBarriers )
{
    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_COPY;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &commandQueueDesc, &m_copyQueue );

    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = m_copyQueue;
    poolDesc.NumCommandLists = 1;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &poolDesc, &m_commandListPool );
    DenOfIz_CommandListArray commandLists;
    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &commandLists );
    DZ_ASSERTM( commandLists.NumElements != 0, "Command list pool did not produce any command lists." );

    m_copyCommandList = commandLists.Elements[ 0 ];
    DenOfIz_LogicalDevice_CreateFence( m_device, &m_executeFence );
    DenOfIz_LogicalDevice_CreateSemaphore( m_device, &m_batchCopyWait );

    if ( m_issueBarriers )
    {
        DenOfIz_CommandQueueDesc syncQueueDesc{ };
        syncQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
        DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &syncQueueDesc, &m_syncQueue );

        DenOfIz_CommandListPoolDesc syncPoolDesc{ };
        syncPoolDesc.CommandQueue    = m_syncQueue;
        syncPoolDesc.NumCommandLists = 1;
        DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &syncPoolDesc, &m_syncCommandPool );

        DenOfIz_CommandListArray syncCommandLists;
        DenOfIz_CommandListPool_GetCommandLists( m_syncCommandPool, &syncCommandLists );
        m_syncCommandList = syncCommandLists.Elements[ 0 ];
        DenOfIz_LogicalDevice_CreateFence( m_device, &m_syncWait );
    }
}

BatchResourceCopy::~BatchResourceCopy( )
{
#ifndef EMSCRIPTEN
    if ( m_cleanResourcesFuture.valid( ) )
    {
        m_cleanResourcesFuture.wait( );
    }
#endif

    for ( auto &resource : m_resourcesToClean )
    {
        DenOfIz_Buffer_Destroy( resource );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_batchCopyWait ) )
    {
        DenOfIz_Semaphore_Destroy( m_batchCopyWait );
    }
    if ( m_issueBarriers )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_syncWait ) )
        {
            DenOfIz_Fence_Destroy( m_syncWait );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_syncCommandPool ) )
        {
            DenOfIz_CommandListPool_Destroy( m_syncCommandPool );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_syncQueue ) )
        {
            DenOfIz_CommandQueue_Destroy( m_syncQueue );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_executeFence ) )
    {
        DenOfIz_Fence_Destroy( m_executeFence );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_commandListPool );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_copyQueue ) )
    {
        DenOfIz_CommandQueue_Destroy( m_copyQueue );
    }
}

void BatchResourceCopy::Begin( ) const
{
    DenOfIz_CommandList_Begin( m_copyCommandList );
    if ( m_issueBarriers )
    {
        DenOfIz_CommandList_Begin( m_syncCommandList );
    }
}

void BatchResourceCopy::CopyToGPUBuffer( const DenOfIz_CopyToGpuBufferDesc &copyDesc )
{
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    DenOfIz_BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    stagingBufferDesc.NumBytes  = Utilities::Align( copyDesc.Data.NumElements, deviceInfo.Constants.ConstantBufferAlignment );
    stagingBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
    stagingBufferDesc.DebugName = DENOFIZ_STRING( "CopyToGPUBuffer_StagingBuffer" );

    DenOfIz_Buffer stagingBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &stagingBufferDesc, &stagingBuffer );
    void *mappedMemory = NULL;
    DenOfIz_Buffer_MapMemory( stagingBuffer, &mappedMemory );
    memcpy( mappedMemory, copyDesc.Data.Elements, copyDesc.Data.NumElements );
    DenOfIz_Buffer_UnmapMemory( stagingBuffer );

    DenOfIz_CopyBufferRegionDesc copyBufferRegionDesc{ };
    copyBufferRegionDesc.DstBuffer = copyDesc.DstBuffer;
    copyBufferRegionDesc.SrcBuffer = stagingBuffer;
    copyBufferRegionDesc.DstOffset = copyDesc.DstBufferOffset;
    copyBufferRegionDesc.NumBytes  = copyDesc.Data.NumElements;

    CopyBufferRegion( copyBufferRegionDesc );

    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( stagingBuffer );
}

void BatchResourceCopy::CopyBufferRegion( const DenOfIz_CopyBufferRegionDesc &copyDesc ) const
{
    DenOfIz_CommandList_CopyBufferRegion( m_copyCommandList, &copyDesc );
}

void BatchResourceCopy::CopyTextureRegion( const DenOfIz_CopyTextureRegionDesc &copyDesc ) const
{
    DenOfIz_CommandList_CopyTextureRegion( m_copyCommandList, &copyDesc );
}

void BatchResourceCopy::CopyDataToTexture( const DenOfIz_CopyDataToTextureDesc &copyDesc )
{
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    DenOfIz_Format textureFormat;
    DenOfIz_TextureResource_GetFormat( copyDesc.DstTexture, &textureFormat );

    DenOfIz_BufferDesc stagingBufferDesc{ };
    stagingBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    stagingBufferDesc.NumBytes  = copyDesc.Data.NumElements;
    stagingBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
    stagingBufferDesc.DebugName = DENOFIZ_STRING( "CopyDataToTexture_StagingBuffer" );

    const uint32_t bitSize = DenOfIz_Format_NumBytes( textureFormat );
    if ( copyDesc.AutoAlign )
    {
        if ( copyDesc.Width == 0 || copyDesc.Height == 0 )
        {
            spdlog::error( "Width and Height cannot be 0 when AutoAlign is true." );
            return;
        }

        const uint32_t alignedRowPitch   = Utilities::Align( copyDesc.Width * bitSize, deviceInfo.Constants.BufferTextureRowAlignment );
        const uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * copyDesc.Height, GetSubresourceAlignment( bitSize ) );
        stagingBufferDesc.NumBytes       = alignedSlicePitch;
    }

    DenOfIz_Buffer stagingBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &stagingBufferDesc, &stagingBuffer );
    void *dst = NULL;
    DenOfIz_Buffer_MapMemory( stagingBuffer, &dst );
    if ( copyDesc.AutoAlign )
    {
        AlignDataForTexture( copyDesc.Data.Elements, copyDesc.Width, copyDesc.Height, bitSize, static_cast<Byte *>( dst ) );
    }
    else
    {
        memcpy( dst, copyDesc.Data.Elements, copyDesc.Data.NumElements );
    }

    DenOfIz_Buffer_UnmapMemory( stagingBuffer );

    DenOfIz_CopyBufferToTextureDesc copyBufferToTextureDesc{ };
    copyBufferToTextureDesc.DstTexture = copyDesc.DstTexture;
    copyBufferToTextureDesc.SrcBuffer  = stagingBuffer;
    copyBufferToTextureDesc.Format     = DenOfIz_Format_ToTypeless( textureFormat );
    copyBufferToTextureDesc.MipLevel   = copyDesc.MipLevel;
    copyBufferToTextureDesc.ArrayLayer = copyDesc.ArrayLayer;
    DenOfIz_CommandList_CopyBufferToTexture( m_copyCommandList, &copyBufferToTextureDesc );

    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( stagingBuffer );
}

DenOfIz_Texture BatchResourceCopy::CreateAndLoadTexture( DenOfIz_StringView file )
{
    std::string filePath;
    if ( file.Chars && file.NumChars > 0 )
    {
        filePath.assign( file.Chars, file.Chars + file.NumChars );
    }

    DenOfIz_TextureCreateFromPathDesc createDesc{ };
    createDesc.Path             = file;
    DenOfIz_TextureData texture = DenOfIz_TextureData_CreateFromPath( &createDesc );

    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.HeapType        = DENOFIZ_HEAP_TYPE_GPU;
    textureDesc.Usage           = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
    textureDesc.Width           = DenOfIz_TextureData_GetWidth( texture );
    textureDesc.Height          = DenOfIz_TextureData_GetHeight( texture );
    textureDesc.Format          = DenOfIz_TextureData_GetFormat( texture );
    textureDesc.Depth           = DenOfIz_TextureData_GetDepth( texture );
    textureDesc.ArraySize       = DenOfIz_TextureData_GetArraySize( texture );
    textureDesc.MipLevels       = DenOfIz_TextureData_GetMipLevels( texture );
    const std::string debugName = "CreateAndLoadTexture(" + filePath + ")";
    textureDesc.DebugName       = DenOfIz_StringView( debugName.c_str( ), static_cast<uint32_t>( debugName.size( ) ) );

    DenOfIz_Texture outTex;
    DenOfIz_LogicalDevice_CreateTexture( m_device, &textureDesc, &outTex );
    LoadTextureInternal( texture, outTex );
    DenOfIz_TextureData_Destroy( texture );

    if ( m_issueBarriers )
    {
        DenOfIz_TextureBarrierDesc textureBarrier{ };
        textureBarrier.Resource = outTex;
        textureBarrier.OldState = DENOFIZ_RESOURCE_USAGE_COMMON_BIT;
        textureBarrier.NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;

        DenOfIz_PipelineBarrierDesc barrierDesc{ };
        barrierDesc.TextureBarriers.Elements    = &textureBarrier;
        barrierDesc.TextureBarriers.NumElements = 1;
        DenOfIz_CommandList_PipelineBarrier( m_syncCommandList, &barrierDesc );
    }
    return outTex;
}

DenOfIz_Buffer BatchResourceCopy::CreateUniformBuffer( const DenOfIz_ByteArrayView &data, const uint32_t numBytes )
{
    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    bufferDesc.NumBytes  = numBytes;
    bufferDesc.DebugName = DENOFIZ_STRING( NextId( "Uniform" ).c_str( ) );

    DenOfIz_Buffer buffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &bufferDesc, &buffer );

    DenOfIz_CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer = buffer;
    copyDesc.Data      = data;
    CopyToGPUBuffer( copyDesc );

    if ( m_issueBarriers )
    {
        DenOfIz_BufferBarrierDesc bufferBarrier{ };
        bufferBarrier.Resource = buffer;
        bufferBarrier.OldState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
        bufferBarrier.NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;

        DenOfIz_PipelineBarrierDesc barrierDesc{ };
        barrierDesc.BufferBarriers.Elements    = &bufferBarrier;
        barrierDesc.BufferBarriers.NumElements = 1;
        DenOfIz_CommandList_PipelineBarrier( m_syncCommandList, &barrierDesc );
    }

    return buffer;
}

DenOfIz_Buffer BatchResourceCopy::CreateGeometryVertexBuffer( const DenOfIz_GeometryData &geometryData )
{
    uint32_t vertexCount = 0;
    DenOfIz_GeometryData_GetVertexCount( geometryData, &vertexCount );

    const size_t numBytes = vertexCount * sizeof( DenOfIz_GeometryVertexData );

    std::vector<DenOfIz_GeometryVertexData> vertices( vertexCount );
    DenOfIz_GeometryData_GetVertexData( geometryData, vertices.data( ) );

    DenOfIz_BufferDesc vBufferDesc{ };
    vBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    vBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    vBufferDesc.NumBytes  = numBytes;
    vBufferDesc.DebugName = DENOFIZ_STRING( NextId( "Vertex" ).c_str( ) );

    DenOfIz_Buffer vertexBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &vBufferDesc, &vertexBuffer );

    DenOfIz_CopyToGpuBufferDesc vbCopyDesc{ };
    vbCopyDesc.DstBuffer        = vertexBuffer;
    vbCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( vertices.data( ) );
    vbCopyDesc.Data.NumElements = numBytes;
    CopyToGPUBuffer( vbCopyDesc );

    if ( m_issueBarriers )
    {
        DenOfIz_BufferBarrierDesc bufferBarrier{ };
        bufferBarrier.Resource = vertexBuffer;
        bufferBarrier.OldState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
        bufferBarrier.NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;

        DenOfIz_PipelineBarrierDesc barrierDesc{ };
        barrierDesc.BufferBarriers.Elements    = &bufferBarrier;
        barrierDesc.BufferBarriers.NumElements = 1;
        DenOfIz_CommandList_PipelineBarrier( m_syncCommandList, &barrierDesc );
    }

    return vertexBuffer;
}

DenOfIz_Buffer BatchResourceCopy::CreateGeometryIndexBuffer( const DenOfIz_GeometryData &geometryData )
{
    uint32_t indexCount = 0;
    DenOfIz_GeometryData_GetIndexCount( geometryData, &indexCount );

    const size_t numBytes = indexCount * sizeof( uint32_t );

    std::vector<uint32_t> indices( indexCount );
    DenOfIz_GeometryData_GetIndexData( geometryData, indices.data( ) );

    DenOfIz_BufferDesc iBufferDesc{ };
    iBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    iBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    iBufferDesc.NumBytes  = numBytes;
    iBufferDesc.DebugName = DENOFIZ_STRING( NextId( "IndexBuffer" ).c_str( ) );

    DenOfIz_Buffer indexBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &iBufferDesc, &indexBuffer );

    DenOfIz_CopyToGpuBufferDesc ibCopyDesc{ };
    ibCopyDesc.DstBuffer        = indexBuffer;
    ibCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( indices.data( ) );
    ibCopyDesc.Data.NumElements = numBytes;
    CopyToGPUBuffer( ibCopyDesc );

    if ( m_issueBarriers )
    {
        DenOfIz_BufferBarrierDesc bufferBarrier{ };
        bufferBarrier.Resource = indexBuffer;
        bufferBarrier.OldState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
        bufferBarrier.NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;

        DenOfIz_PipelineBarrierDesc barrierDesc{ };
        barrierDesc.BufferBarriers.Elements    = &bufferBarrier;
        barrierDesc.BufferBarriers.NumElements = 1;
        DenOfIz_CommandList_PipelineBarrier( m_syncCommandList, &barrierDesc );
    }
    return indexBuffer;
}

void BatchResourceCopy::LoadTexture( const DenOfIz_LoadTextureDesc &loadDesc )
{
    DenOfIz_TextureCreateFromPathDesc createDesc{ };
    createDesc.Path                   = loadDesc.File;
    const DenOfIz_TextureData texture = DenOfIz_TextureData_CreateFromPath( &createDesc );
    LoadTextureInternal( texture, loadDesc.DstTexture );
    DenOfIz_TextureData_Destroy( texture );
}

void BatchResourceCopy::LoadTextureFromData( const DenOfIz_LoadTextureFromDataDesc &loadDesc )
{
    LoadTextureInternal( loadDesc.TextureData, loadDesc.DstTexture );

    if ( m_issueBarriers )
    {
        DenOfIz_TextureBarrierDesc textureBarrier{ };
        textureBarrier.Resource = loadDesc.DstTexture;
        textureBarrier.OldState = DENOFIZ_RESOURCE_USAGE_COMMON_BIT;
        textureBarrier.NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;

        DenOfIz_PipelineBarrierDesc barrierDesc{ };
        barrierDesc.TextureBarriers.Elements    = &textureBarrier;
        barrierDesc.TextureBarriers.NumElements = 1;
        DenOfIz_CommandList_PipelineBarrier( m_syncCommandList, &barrierDesc );
    }
}

void BatchResourceCopy::Submit( DenOfIz_Semaphore notify )
{
    DenOfIz_CommandList_End( m_copyCommandList );

    uint32_t                         numSignalSemaphores = 1;
    std::array<DenOfIz_Semaphore, 2> signalSemaphores    = { };
    signalSemaphores[ 0 ]                                = m_batchCopyWait;
    if ( DENOFIZ_HANDLE_IS_VALID( notify ) )
    {
        signalSemaphores[ 1 ] = notify;
        ++numSignalSemaphores;
    }

    DenOfIz_ExecuteCommandListsDesc desc{ };
    DenOfIz_Fence_Reset( m_executeFence );
    desc.Signal                       = m_executeFence;
    desc.SignalSemaphores.Elements    = signalSemaphores.data( );
    desc.SignalSemaphores.NumElements = numSignalSemaphores;
    desc.CommandLists.Elements        = &m_copyCommandList;
    desc.CommandLists.NumElements     = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( m_copyQueue, &desc );
#ifdef EMSCRIPTEN
    emscripten_async_call(
        []( void *userData )
        {
            auto *batch = static_cast<BatchResourceCopy *>( userData );
            batch->CleanResources( );
        },
        this, 100 );
#else
    m_cleanResourcesFuture = std::async( std::launch::async, [ this ] { CleanResources( ); } );
#endif

    if ( m_issueBarriers )
    {
        std::array<DenOfIz_Semaphore, 1> waitSemaphores = { };
        waitSemaphores[ 0 ]                             = m_batchCopyWait;
        DenOfIz_Fence_Reset( m_syncWait );
        DenOfIz_ExecuteCommandListsDesc syncDesc{ };
        syncDesc.Signal                     = m_syncWait;
        syncDesc.WaitSemaphores.Elements    = waitSemaphores.data( );
        syncDesc.WaitSemaphores.NumElements = waitSemaphores.size( );
        syncDesc.CommandLists.Elements      = &m_syncCommandList;
        syncDesc.CommandLists.NumElements   = 1;
        DenOfIz_CommandList_End( m_syncCommandList );
        DenOfIz_CommandQueue_ExecuteCommandLists( m_syncQueue, &syncDesc );
        DenOfIz_Fence_Wait( m_syncWait );
    }
}

void BatchResourceCopy::CleanResources( )
{
    std::lock_guard lock( m_resourceCleanLock );
    DenOfIz_Fence_Wait( m_executeFence );
    for ( const auto &resource : m_resourcesToClean )
    {
        DenOfIz_Buffer_Destroy( resource );
    }
    m_resourcesToClean.clear( );

    for ( const auto &texture : m_freeTextures )
    {
        free( texture );
    }
}

void BatchResourceCopy::LoadTextureInternal( DenOfIz_TextureData texture, DenOfIz_Texture dstTexture )
{
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    DenOfIz_Format textureFormat;
    DenOfIz_TextureResource_GetFormat( dstTexture, &textureFormat );

    DenOfIz_BufferDesc stagingBufferDesc = { };
    stagingBufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_CPU_GPU;
    stagingBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
    stagingBufferDesc.DebugName          = DENOFIZ_STRING( "LoadTexture_StagingBuffer" );

    const DenOfIz_TextureMipArray mipDataArray = DenOfIz_TextureData_ReadMipData( texture );
    const uint32_t                textureDepth = DenOfIz_TextureData_GetDepth( texture );
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        const DenOfIz_TextureMip &mipData       = mipDataArray.Elements[ i ];
        const uint32_t            mipRowPitch   = Utilities::Align( mipData.RowPitch, deviceInfo.Constants.BufferTextureRowAlignment );
        const uint32_t            mipSlicePitch = Utilities::Align( textureDepth * mipRowPitch * mipData.NumRows, deviceInfo.Constants.BufferTextureAlignment );
        stagingBufferDesc.NumBytes += mipSlicePitch;
    }

    DenOfIz_Buffer stagingBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &stagingBufferDesc, &stagingBuffer );
    void *stagingMappedMemory = NULL;
    DenOfIz_Buffer_MapMemory( stagingBuffer, &stagingMappedMemory );

    uint64_t stagingBufferOffset = 0;
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        const DenOfIz_TextureMip &mipData = mipDataArray.Elements[ i ];
        CopyTextureToMemoryAligned( texture, mipData, static_cast<Byte *>( stagingMappedMemory ) + stagingBufferOffset );

        DenOfIz_CopyBufferToTextureDesc copyBufferToTextureDesc{ };
        copyBufferToTextureDesc.DstTexture = dstTexture;
        copyBufferToTextureDesc.SrcBuffer  = stagingBuffer;
        copyBufferToTextureDesc.SrcOffset  = stagingBufferOffset;
        copyBufferToTextureDesc.Format     = textureFormat;
        copyBufferToTextureDesc.MipLevel   = mipData.MipIndex;
        copyBufferToTextureDesc.ArrayLayer = mipData.ArrayIndex;
        copyBufferToTextureDesc.RowPitch   = mipData.RowPitch;
        copyBufferToTextureDesc.NumRows    = mipData.NumRows;
        DenOfIz_CommandList_CopyBufferToTexture( m_copyCommandList, &copyBufferToTextureDesc );

        const uint32_t alignedRowPitch   = Utilities::Align( mipData.RowPitch, deviceInfo.Constants.BufferTextureRowAlignment );
        const uint32_t alignedSlicePitch = Utilities::Align( textureDepth * alignedRowPitch * mipData.NumRows, deviceInfo.Constants.BufferTextureAlignment );
        stagingBufferOffset += alignedSlicePitch;
    }

    DenOfIz_Buffer_UnmapMemory( stagingBuffer );
    std::lock_guard lock( m_resourceCleanLock );
    m_resourcesToClean.push_back( stagingBuffer );
}

void BatchResourceCopy::CopyTextureToMemoryAligned( DenOfIz_TextureData texture, const DenOfIz_TextureMip &mipData, Byte *dst ) const
{
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    const uint32_t alignedRowPitch   = Utilities::Align( mipData.RowPitch, deviceInfo.Constants.BufferTextureRowAlignment );
    const uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * mipData.NumRows, GetSubresourceAlignment( DenOfIz_TextureData_GetBitsPerPixel( texture ) ) );

    const DenOfIz_ByteArrayView textureData = DenOfIz_TextureData_GetData( texture );
    const Byte                 *pSrcData    = textureData.Elements + mipData.DataOffset;
    for ( uint32_t z = 0; z < DenOfIz_TextureData_GetArraySize( texture ); ++z )
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
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    const uint32_t blockSize = std::max( 1u, bitSize >> 3 );
    const uint32_t alignment = Utilities::Align( deviceInfo.Constants.BufferTextureAlignment, blockSize );
    return Utilities::Align( alignment, deviceInfo.Constants.BufferTextureRowAlignment );
}

void BatchResourceCopy::AlignDataForTexture( const Byte *src, const uint32_t width, const uint32_t height, const uint32_t bitsPerPixel, Byte *dst ) const
{
    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );

    const uint32_t alignedRowPitch   = Utilities::Align( width * bitsPerPixel, deviceInfo.Constants.BufferTextureRowAlignment );
    const uint32_t alignedSlicePitch = Utilities::Align( alignedRowPitch * height, GetSubresourceAlignment( bitsPerPixel ) );

    constexpr uint32_t arraySize = 1;
    for ( uint32_t z = 0; z < arraySize; ++z )
    {
        const auto dstSlice = dst + alignedSlicePitch * z;
        const auto srcSlice = src + width * height * z;
        for ( uint32_t y = 0; y < height; ++y )
        {
            memcpy( dstSlice + alignedRowPitch * y, srcSlice + width * y, width );
        }
    }
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

#define BATCH_RESOURCE_COPY_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BatchResourceCopy, handle )

extern "C"
{

    void DenOfIz_BatchResourceCopy_Create( const DenOfIz_BatchResourceCopyDesc *desc, DenOfIz_BatchResourceCopy *outBatchResourceCopy )
    {
        if ( desc == NULL || outBatchResourceCopy == NULL )
        {
            return;
        }

        BatchResourceCopy *batchResourceCopy = new BatchResourceCopy( *desc );
        *outBatchResourceCopy                = DENOFIZ_TO_HANDLE( batchResourceCopy );
    }

    void DenOfIz_BatchResourceCopy_Begin( DenOfIz_BatchResourceCopy batchResourceCopy )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->Begin( );
    }

    void DenOfIz_BatchResourceCopy_CopyToGPUBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyToGpuBufferDesc *copyDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || copyDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CopyToGPUBuffer( *copyDesc );
    }

    void DenOfIz_BatchResourceCopy_CopyBufferRegion( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyBufferRegionDesc *copyDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || copyDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CopyBufferRegion( *copyDesc );
    }

    void DenOfIz_BatchResourceCopy_CopyTextureRegion( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyTextureRegionDesc *copyDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || copyDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CopyTextureRegion( *copyDesc );
    }

    void DenOfIz_BatchResourceCopy_CopyDataToTexture( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_CopyDataToTextureDesc *copyDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || copyDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CopyDataToTexture( *copyDesc );
    }

    DenOfIz_Texture DenOfIz_BatchResourceCopy_CreateAndLoadTexture( DenOfIz_BatchResourceCopy batchResourceCopy, DenOfIz_StringView file )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        return BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CreateAndLoadTexture( file );
    }

    void DenOfIz_BatchResourceCopy_LoadTexture( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_LoadTextureDesc *loadDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || loadDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->LoadTexture( *loadDesc );
    }

    void DenOfIz_BatchResourceCopy_LoadTextureFromData( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_LoadTextureFromDataDesc *loadDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || loadDesc == NULL )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->LoadTextureFromData( *loadDesc );
    }

    DenOfIz_Buffer DenOfIz_BatchResourceCopy_CreateUniformBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_ByteArrayView *data, uint32_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || data == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        return BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CreateUniformBuffer( *data, numBytes );
    }

    DenOfIz_Buffer DenOfIz_BatchResourceCopy_CreateGeometryVertexBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_GeometryData *geometryData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || geometryData == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        return BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CreateGeometryVertexBuffer( *geometryData );
    }

    DenOfIz_Buffer DenOfIz_BatchResourceCopy_CreateGeometryIndexBuffer( DenOfIz_BatchResourceCopy batchResourceCopy, const DenOfIz_GeometryData *geometryData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) || geometryData == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        return BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->CreateGeometryIndexBuffer( *geometryData );
    }

    void DenOfIz_BatchResourceCopy_Submit( DenOfIz_BatchResourceCopy batchResourceCopy, DenOfIz_Semaphore notify )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) )
        {
            return;
        }

        BATCH_RESOURCE_COPY_IMPL( batchResourceCopy )->Submit( notify );
    }

    void DenOfIz_BatchResourceCopy_Destroy( DenOfIz_BatchResourceCopy batchResourceCopy )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( batchResourceCopy ) )
        {
            return;
        }

        delete BATCH_RESOURCE_COPY_IMPL( batchResourceCopy );
    }
}
