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

MetalBufferResource::MetalBufferResource( MetalContext *context, const BufferDesc &desc ) : IBufferResource( desc ), m_context( context ), m_desc( desc )
{
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

    NSString *nsName = [NSString stringWithUTF8String:desc.DebugName.c_str( )];
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
        LOG( WARNING ) << "Memory already mapped, buffer: " << m_desc.DebugName.c_str( );
        return m_mappedMemory;
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( WARNING ) << "Buffer is not shared, buffer: " << m_desc.DebugName.c_str( );
        return nullptr;
    }

    m_mappedMemory = [m_buffer contents];
    return m_mappedMemory;
}

void MetalBufferResource::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        LOG( WARNING ) << "Memory not mapped, buffer: " << m_desc.DebugName.c_str( );
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        LOG( ERROR ) << "Buffer is not shared, buffer: " << m_desc.DebugName.c_str( );
    }

    m_mappedMemory = nullptr;
}
