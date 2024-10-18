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

#import <DenOfIzGraphics/Backends/Metal/MetalBufferResource.h>
#import <DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h>

using namespace DenOfIz;

MetalBufferResource::MetalBufferResource( MetalContext *context, const BufferDesc &desc ) : m_context( context ), m_desc( desc )
{
    NSUInteger numBytes        = m_desc.NumBytes;
    m_numBytes                 = numBytes;
    MTLResourceOptions options = MTLResourceStorageModeShared;

    if ( m_desc.HeapType == HeapType::GPU )
    {
        options = MTLResourceStorageModePrivate;
    }

    m_buffer = [m_context->Device newBufferWithLength:numBytes options:options];
    if ( !m_buffer )
    {
        LOG( ERROR ) << "Failed to create Metal buffer";
    }

    NSString *nsName = [NSString stringWithUTF8String:desc.DebugName.Get( )];
    m_buffer.label   = nsName;

    if ( m_desc.Descriptor.Any( { ResourceDescriptor::RWBuffer, ResourceDescriptor::AccelerationStructure } ) ||
         m_desc.InitialState.Any( { ResourceState::UnorderedAccess, ResourceState::DepthWrite, ResourceState::AccelerationStructureWrite, ResourceState::CopyDst } ) )
    {
        m_usage = MTLResourceUsageRead | MTLResourceUsageWrite;
    }
    else
    {
        m_usage = MTLResourceUsageRead;
    }

    m_dataType = MetalEnumConverter::ConvertFormatToDataType( m_desc.Format );
}

MetalBufferResource::~MetalBufferResource( )
{
}

void *MetalBufferResource::MapMemory( )
{
    if ( m_mappedMemory != nullptr )
    {
        LOG( WARNING ) << "Memory already mapped, buffer: " << m_desc.DebugName.Get( );
        return m_mappedMemory;
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( WARNING ) << "Buffer is not shared, buffer: " << m_desc.DebugName.Get( );
        return nullptr;
    }

    m_mappedMemory = [m_buffer contents];
    return m_mappedMemory;
}

void MetalBufferResource::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        LOG( WARNING ) << "Memory not mapped, buffer: " << m_desc.DebugName.Get( );
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( ERROR ) << "Buffer is not shared, buffer: " << m_desc.DebugName.Get( );
    }

    m_mappedMemory = nullptr;
}

std::vector<Byte> MetalBufferResource::GetData( ) const
{
    std::vector<Byte> data( m_numBytes );
    std::memcpy( data.data( ), m_mappedMemory, m_numBytes );
    return std::move( data );
}

void MetalBufferResource::SetData( const std::vector<Byte> &data, bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.data( ), data.size( ) );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

[[nodiscard]] size_t MetalBufferResource::NumBytes( ) const
{
    return m_numBytes;
}

[[nodiscard]] const void *MetalBufferResource::Data( ) const
{
    return m_mappedMemory;
}

[[nodiscard]] DenOfIz::BitSet<ResourceState> MetalBufferResource::InitialState( ) const
{
    // Doesn't matter much for Metal
    return m_desc.InitialState;
}

const id<MTLBuffer> &MetalBufferResource::Instance( ) const
{
    return m_buffer;
}

const MTLResourceUsage &MetalBufferResource::Usage( ) const
{
    return m_usage;
}

const MTLDataType &MetalBufferResource::Type( ) const
{
    return m_dataType;
}
