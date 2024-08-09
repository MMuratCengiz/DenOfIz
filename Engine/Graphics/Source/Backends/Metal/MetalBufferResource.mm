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

using namespace DenOfIz;

MetalBufferResource::MetalBufferResource( MetalContext *context, const BufferDesc &desc, std::string name ) : m_context( context ), m_desc( desc )
{
    m_name                      = name;
    NSUInteger         numBytes = m_desc.NumBytes;
    MTLResourceOptions options  = MTLResourceStorageModeShared;

    if ( m_desc.HeapType == HeapType::GPU )
    {
        options = MTLResourceStorageModePrivate;
    }

    m_buffer = [m_context->Device newBufferWithLength:numBytes options:options];
    if ( !m_buffer )
    {
        LOG( ERROR ) << "Failed to create Metal buffer";
    }

    NSString *nsName = [NSString stringWithUTF8String:m_name.c_str( )];
    m_buffer.label = nsName;

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
    }
    else if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWBuffer ) )
    {
        m_usage = MTLResourceUsageRead | MTLResourceUsageWrite;
    }
    else if ( m_desc.Descriptor.IsSet( ResourceDescriptor::VertexBuffer ) || m_desc.Descriptor.IsSet( ResourceDescriptor::IndexBuffer ) )
    {
        m_usage = MTLResourceUsageRead;
    }
}

MetalBufferResource::~MetalBufferResource( )
{
    [m_buffer release];
}

void *MetalBufferResource::MapMemory( )
{
    if ( m_mappedMemory != nullptr )
    {
        LOG( WARNING ) << "Memory already mapped, buffer: " << m_name.c_str( );
        return m_mappedMemory;
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( WARNING ) << "Buffer is not shared, buffer: " << m_name.c_str( );
        return nullptr;
    }

    m_mappedMemory = [m_buffer contents];
    return m_mappedMemory;
}

void MetalBufferResource::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        LOG( WARNING ) << "Memory not mapped, buffer: " << m_name.c_str( );
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( ERROR ) << "Buffer is not shared, buffer: " << m_name.c_str( );
    }

    m_mappedMemory = nullptr;
}
