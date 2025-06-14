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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalBufferResource.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalBufferResource::MetalBufferResource( MetalContext *context, const BufferDesc &desc ) : m_context( context ), m_desc( desc )
{
    NSUInteger alignment = 16;
    if ( m_desc.Descriptor & ResourceDescriptor::UniformBuffer )
    {
        alignment = (NSUInteger)m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment;
    }
    alignment  = std::max( alignment, (NSUInteger)m_desc.Alignment );
    m_numBytes = Utilities::Align( m_desc.NumBytes, alignment );
    if ( m_desc.Descriptor & ResourceDescriptor::StructuredBuffer )
    {
        size_t elementSize  = m_desc.StructureDesc.Stride;
        size_t elementCount = m_desc.StructureDesc.NumElements;
        m_numBytes          = Utilities::Align( elementSize * elementCount, alignment );
    }

    MTLResourceOptions options = MTLResourceStorageModeShared;
    if ( m_desc.HeapType == HeapType::GPU )
    {
        options = MTLResourceStorageModePrivate;
    }

    m_buffer = [m_context->Device newBufferWithLength:m_numBytes options:options];
    if ( !m_buffer )
    {
        spdlog::error("Failed to create Metal buffer");
    }

    NSString *nsName = [NSString stringWithUTF8String:desc.DebugName.Get( )];
    m_buffer.label   = nsName;

    if ( m_desc.Descriptor & ( ResourceDescriptor::RWBuffer | ResourceDescriptor::AccelerationStructure ) || m_desc.InitialUsage == ResourceUsage::UnorderedAccess ||
         m_desc.InitialUsage == ResourceUsage::DepthWrite || m_desc.InitialUsage == ResourceUsage::AccelerationStructureWrite || m_desc.InitialUsage == ResourceUsage::CopyDst )
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
        spdlog::warn("Memory already mapped, buffer: {}", m_desc.DebugName.Get( ));
        return m_mappedMemory;
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        spdlog::warn("Buffer is not shared, buffer: {}", m_desc.DebugName.Get( ));
        return nullptr;
    }

    m_mappedMemory = [m_buffer contents];
    return m_mappedMemory;
}

void MetalBufferResource::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        spdlog::warn("Memory not mapped, buffer: {}", m_desc.DebugName.Get( ));
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        spdlog::error("Buffer is not shared, buffer: {}", m_desc.DebugName.Get( ));
    }

    m_mappedMemory = nullptr;
}

ByteArray MetalBufferResource::GetData( ) const
{
    return { static_cast<Byte *>( m_mappedMemory ), m_numBytes };
}

void MetalBufferResource::SetData( const ByteArrayView &data, bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.Elements, data.NumElements );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

void MetalBufferResource::WriteData( const ByteArrayView &data, uint32_t bufferOffset )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( static_cast<Byte *>( m_mappedMemory ) + bufferOffset, data.Elements, data.NumElements );
}


[[nodiscard]] size_t MetalBufferResource::NumBytes( ) const
{
    return m_numBytes;
}

[[nodiscard]] const void *MetalBufferResource::Data( ) const
{
    return m_mappedMemory;
}

[[nodiscard]] uint32_t MetalBufferResource::InitialState( ) const
{
    // Doesn't matter much for Metal
    return m_desc.InitialUsage;
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
