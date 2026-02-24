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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

using namespace DenOfIz;

MetalBuffer::MetalBuffer( MetalContext *context, const DenOfIz_BufferDesc &desc ) :
    m_context( context ),
    m_desc( desc ),
    m_debugName( StringViewToStdString( desc.DebugName ) )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    uint32_t alignment = 16;
    if ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_UNIFORM_BIT )
    {
        alignment = m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment;
    }
    if ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT )
    {
        alignment = m_context->SelectedDeviceInfo.Constants.StorageBufferAlignment;
    }
    alignment  = std::max<uint32_t>( alignment, m_desc.StructureDesc.Stride );
    m_numBytes = Utilities::Align( m_desc.NumBytes, alignment );

    MTLResourceOptions options = MTLResourceStorageModeShared;
    if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_GPU )
    {
        options = MTLResourceStorageModePrivate;
    }

    m_buffer = [m_context->Device newBufferWithLength:m_numBytes options:options];
    if ( !m_buffer )
    {
        spdlog::error("Failed to create Metal buffer");
    }

    NSString *nsName = [NSString stringWithUTF8String:m_debugName.empty( ) ? "" : m_debugName.c_str( )];
    m_buffer.label   = nsName;

    if ( m_desc.Usage & ( DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT | DENOFIZ_BUFFER_USAGE_INDIRECT_BIT ) )
    {
        m_usage = MTLResourceUsageRead | MTLResourceUsageWrite;
    }
    else
    {
        m_usage = MTLResourceUsageRead;
    }

    m_dataType = DenOfIz_MetalEnumConverter_ConvertFormatToDataType( m_desc.Format );
}

MetalBuffer::~MetalBuffer( )
{
}

void *MetalBuffer::MapMemory( )
{
    if ( m_mappedMemory != nullptr )
    {
        spdlog::warn("Memory already mapped, buffer: {}", m_debugName );
        return m_mappedMemory;
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        spdlog::warn("Buffer is not shared, buffer: {}", m_debugName );
        return nullptr;
    }

    m_mappedMemory = [m_buffer contents];
    return m_mappedMemory;
}

void MetalBuffer::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        spdlog::warn("Memory not mapped, buffer: {}", m_debugName );
    }
    if ( m_buffer.storageMode != MTLStorageModeShared )
    {
        spdlog::error("Buffer is not shared, buffer: {}", m_debugName );
    }

    m_mappedMemory = nullptr;
}

DenOfIz_ByteArray MetalBuffer::GetData( ) const
{
    return { static_cast<Byte *>( m_mappedMemory ), m_numBytes };
}

void MetalBuffer::SetData( const DenOfIz_ByteArrayView &data, bool keepMapped )
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

void MetalBuffer::WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( static_cast<Byte *>( m_mappedMemory ) + bufferOffset, data.Elements, data.NumElements );
}


[[nodiscard]] size_t MetalBuffer::NumBytes( ) const
{
    return m_numBytes;
}

[[nodiscard]] const void *MetalBuffer::Data( ) const
{
    return m_mappedMemory;
}

const id<MTLBuffer> &MetalBuffer::Instance( ) const
{
    return m_buffer;
}

const MTLResourceUsage &MetalBuffer::Usage( ) const
{
    return m_usage;
}

const MTLDataType &MetalBuffer::Type( ) const
{
    return m_dataType;
}
