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

#include <future>

#import "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#import "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalArgumentBuffer.h"

using namespace DenOfIz;

DescriptorTable::DescriptorTable( MetalContext *context, size_t numEntries ) : m_context( context ), m_numEntries( numEntries )
{
    size_t length = sizeof( IRDescriptorTableEntry ) * numEntries;
    m_buffer      = [m_context->Device newBufferWithLength:length options:MTLResourceStorageModeShared];
    m_contents    = (IRDescriptorTableEntry *)m_buffer.contents;
    [m_buffer setLabel:@"DescriptorTable"];
    
    memset( m_contents, 0, length );
}

void DescriptorTable::SetDebugName( const std::string &name )
{
    [m_buffer setLabel:[NSString stringWithUTF8String:name.c_str( )]];
}

void DescriptorTable::Reset( size_t newNumEntries )
{
    if ( newNumEntries != m_numEntries )
    {
        m_numEntries = newNumEntries;
        size_t length = sizeof( IRDescriptorTableEntry ) * m_numEntries;
        m_buffer = [m_context->Device newBufferWithLength:length options:MTLResourceStorageModeShared];
        m_contents = (IRDescriptorTableEntry *)m_buffer.contents;
    }
    
    memset( m_contents, 0, sizeof( IRDescriptorTableEntry ) * m_numEntries );
}

void DescriptorTable::EncodeBuffer( id<MTLBuffer> buffer, uint32_t index, uint32_t offset )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetBuffer( &m_contents[ index ], [buffer gpuAddress] + offset, 0 );
}

void DescriptorTable::EncodeTexture( id<MTLTexture> texture, float minLodClamp, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetTexture( &m_contents[ index ], texture, minLodClamp, 0 );
}

void DescriptorTable::EncodeSampler( id<MTLSamplerState> sampler, float lodBias, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetSampler( &m_contents[ index ], sampler, lodBias );
}

void DescriptorTable::EncodeAccelerationStructure( id<MTLBuffer> asHeader, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetAccelerationStructure( &m_contents[ index ], asHeader.gpuAddress );
}

MetalArgumentBuffer::MetalArgumentBuffer( MetalContext *context, size_t capacity ) : m_context( context ), m_capacity( capacity )
{
    m_nextOffset = 0;
    m_buffer     = [m_context->Device newBufferWithLength:m_capacity options:MTLResourceStorageModeShared];
    if ( !m_buffer )
    {
        spdlog::error("Failed to allocate Metal argument buffer");
    }
    [m_buffer setLabel:@"MetalArgumentBuffer"];
    m_contents = (Byte *)m_buffer.contents;
}

std::pair<Byte *, uint64_t> MetalArgumentBuffer::Reserve( size_t numAddresses, uint32_t numRootConstantBytes )
{
    std::lock_guard<std::mutex> lock( m_reserveMutex );

    auto numBytes = Utilities::Align( sizeof( uint64_t ) * numAddresses + numRootConstantBytes, 8 );
    if ( m_nextOffset + numBytes > m_capacity )
    {
        spdlog::error("MetalArgumentBuffer::Allocate: out of memory");
        return std::pair<Byte *, uint64_t>( nullptr, 0 );
    }

    m_currentOffset = m_nextOffset;
    Byte *ptr       = m_contents + m_currentOffset;
    m_nextOffset += numBytes;
    return std::pair<Byte *, uint64_t>( ptr, m_currentOffset );
}

std::pair<Byte *, uint64_t> MetalArgumentBuffer::Duplicate( size_t numAddresses, uint32_t numRootConstantBytes )
{
    Byte *prevPtr = &m_contents[ m_currentOffset ];
    auto  result  = Reserve( numAddresses, numRootConstantBytes );
    std::memcpy( result.first, prevPtr, numRootConstantBytes + numAddresses * sizeof( uint64_t ) );
    return result;
}

void MetalArgumentBuffer::EncodeRootConstant( uint64_t offset, uint32_t numRootConstantBytes, const Byte *data ) const
{
    //    spdlog::info("Encoding root constant at offset: {} numRootConstantBytes: {} data: {}", offset, numRootConstantBytes, data);
    if ( numRootConstantBytes == 0 )
    {
        spdlog::error("MetalArgumentBuffer::EncodeRootConstant: No bytes reserved for root constants");
        return;
    }

    if ( numRootConstantBytes + offset > m_capacity )
    {
        spdlog::error("MetalArgumentBuffer::EncodeRootConstant: Index or offset out of bounds");
        return;
    }

    std::memcpy( &m_contents[ offset ], data, numRootConstantBytes );
}

void MetalArgumentBuffer::EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const
{
    uint64_t addressLocation = Utilities::Align( offset + ( index * sizeof( uint64_t ) ), 8 );
    if ( addressLocation > m_capacity )
    {
        spdlog::error("MetalArgumentBuffer::EncodeAddress: Index or offset out of bounds");
        return;
    }
    std::memcpy( &m_contents[ addressLocation ], &address, sizeof( uint64_t ) );
}

void MetalArgumentBuffer::Reset( )
{
    m_nextOffset    = 0;
    m_currentOffset = 0;
}

uint64_t MetalArgumentBuffer::Offset( ) const
{
    return m_nextOffset;
}

id<MTLBuffer> MetalArgumentBuffer::Buffer( ) const
{
    return m_buffer;
}
