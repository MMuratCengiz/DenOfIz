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

#include <DenOfIzGraphics/Helpers/BatchResourceCopyHelper.h>
#include <atomic>
#include <utility>

using namespace DenOfIz;

BatchResourceCopyHelper::BatchResourceCopyHelper( ILogicalDevice *device, BatchResourceCopy *batchCopy ) : m_device( device ), m_batchCopy( batchCopy )
{
    CommandListPoolDesc poolDesc{ };
    poolDesc.QueueType        = QueueType::Graphics;
    poolDesc.CommandListCount = 1;

    m_syncCommandPool = m_device->CreateCommandListPool( poolDesc );
    m_syncCommandList = m_syncCommandPool->GetCommandLists( ).front( );
}

void BatchResourceCopyHelper::Begin( ) const
{
    m_batchCopy->Begin( );
    m_syncCommandList->Begin( );
}

UniformBufferHolder BatchResourceCopyHelper::CreateUniformBuffer( std::string name, const void *data, const uint32_t numBytes ) const
{
    BufferDesc bufferDesc{ };
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    bufferDesc.InitialState = ResourceState::CopyDst;
    bufferDesc.NumBytes     = numBytes;

    auto buffer = m_device->CreateBufferResource( std::move( name ), bufferDesc );

    CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer = buffer.get( );
    copyDesc.Data      = data;
    copyDesc.NumBytes  = numBytes;
    m_batchCopy->CopyToGPUBuffer( copyDesc );

    PipelineBarrierDesc barrierDesc =
        PipelineBarrierDesc{ }.BufferBarrier( { .Resource = buffer.get( ), .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );

    return UniformBufferHolder{ .Buffer = std::move( buffer ) };
}

VertexIndexBufferPairHolder BatchResourceCopyHelper::CreateGeometryBuffers( const GeometryData &geometryData ) const
{
    VertexIndexBufferPairHolder result{ };

    BufferDesc vBufferDesc{ };
    vBufferDesc.HeapType     = HeapType::GPU;
    vBufferDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vBufferDesc.InitialState = ResourceState::CopyDst;
    vBufferDesc.NumBytes     = geometryData.SizeOfVertices( );
    result.VertexBuffer      = m_device->CreateBufferResource( NextId( "Vertex" ), vBufferDesc );

    BufferDesc iBufferDesc{ };
    iBufferDesc.HeapType     = HeapType::GPU;
    iBufferDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
    iBufferDesc.InitialState = ResourceState::CopyDst;
    iBufferDesc.NumBytes     = geometryData.SizeOfIndices( );
    result.IndexBuffer       = m_device->CreateBufferResource( NextId( "Index" ), iBufferDesc );

    CopyToGpuBufferDesc vbCopyDesc{ };
    vbCopyDesc.DstBuffer = result.VertexBuffer.get( );
    vbCopyDesc.Data      = geometryData.Vertices.data( );
    vbCopyDesc.NumBytes  = geometryData.SizeOfVertices( );
    m_batchCopy->CopyToGPUBuffer( vbCopyDesc );

    CopyToGpuBufferDesc ibCopyDesc{ };
    ibCopyDesc.DstBuffer = result.IndexBuffer.get( );
    ibCopyDesc.Data      = geometryData.Indices.data( );
    ibCopyDesc.NumBytes  = geometryData.SizeOfIndices( );
    m_batchCopy->CopyToGPUBuffer( ibCopyDesc );

    const PipelineBarrierDesc barrierDesc =
        PipelineBarrierDesc{ }
            .BufferBarrier( { .Resource = result.VertexBuffer.get( ), .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } )
            .BufferBarrier( { .Resource = result.IndexBuffer.get( ), .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } );

    m_syncCommandList->PipelineBarrier( barrierDesc );
    return result;
}

SamplerHolder BatchResourceCopyHelper::CreateSampler( const std::string &name, const SamplerDesc &desc ) const
{
    return SamplerHolder{ .Sampler = m_device->CreateSampler( name, SamplerDesc{ } ) };
}

TextureHolder BatchResourceCopyHelper::CreateTexture( const std::string &name, const std::string &path ) const
{
    auto texture = m_batchCopy->CreateAndLoadTexture( name, path );
    m_syncCommandList->PipelineBarrier(
        PipelineBarrierDesc{ }.TextureBarrier( { .Resource = texture.get( ), .OldState = ResourceState::CopyDst, .NewState = ResourceState::ShaderResource } ) );
    return TextureHolder{ .Texture = std::move( texture ) };
}

std::string BatchResourceCopyHelper::NextId( const std::string &prefix )
{
    static std::atomic<unsigned int> idCounter( 0 );
    const int                        next = idCounter.fetch_add( 1, std::memory_order_relaxed );
    return prefix + "_BufferHelperResource#" + std::to_string( next );
}

void BatchResourceCopyHelper::Submit( ) const
{
    const std::unique_ptr<ISemaphore> batchCopyWait = m_device->CreateSemaphore( );
    m_batchCopy->End( batchCopyWait.get( ) );

    const std::unique_ptr<IFence> syncWait = m_device->CreateFence( );
    ExecuteDesc                   executeDesc{ };
    executeDesc.WaitOnSemaphores.push_back( batchCopyWait.get( ) );
    executeDesc.Notify = syncWait.get( );
    m_syncCommandList->Execute( executeDesc );

    syncWait->Wait( );
}
