/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
    @autoreleasepool
    {
        [m_buffer setLabel:[NSString stringWithUTF8String:name.c_str( )]];
    }
}

void DescriptorTable::Reset( size_t newNumEntries )
{
    if ( newNumEntries != m_numEntries )
    {
        m_numEntries  = newNumEntries;
        size_t length = sizeof( IRDescriptorTableEntry ) * m_numEntries;
        m_buffer      = [m_context->Device newBufferWithLength:length options:MTLResourceStorageModeShared];
        m_contents    = (IRDescriptorTableEntry *)m_buffer.contents;
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
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeTexture: index out of bounds" );
    IRDescriptorTableSetTexture( &m_contents[ index ], texture, minLodClamp, 0 );
}

void DescriptorTable::EncodeSampler( id<MTLSamplerState> sampler, float lodBias, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeSampler: index out of bounds" );
    IRDescriptorTableSetSampler( &m_contents[ index ], sampler, lodBias );
}

void DescriptorTable::EncodeAccelerationStructure( id<MTLBuffer> asHeader, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeAccelerationStructure: index out of bounds" );
    IRDescriptorTableSetAccelerationStructure( &m_contents[ index ], asHeader.gpuAddress );
}

MetalArgumentBuffer::MetalArgumentBuffer( MetalContext *context, size_t capacity ) : m_context( context ), m_capacity( capacity )
{
    m_nextOffset = 0;
    m_contents   = nullptr;
    if ( AddSegment( ) )
    {
        m_contents = (Byte *)m_segments[ 0 ].contents;
    }
    else
    {
        spdlog::error( "Failed to allocate Metal argument buffer" );
        m_segments.push_back( nil );
    }
}

bool MetalArgumentBuffer::AddSegment( )
{
    id<MTLBuffer> buffer = [m_context->Device newBufferWithLength:m_capacity options:MTLResourceStorageModeShared];
    if ( !buffer )
    {
        return false;
    }
    [buffer setLabel:@"MetalArgumentBuffer"];
    m_segments.push_back( buffer );
    return true;
}

bool MetalArgumentBuffer::NextSegment( )
{
    if ( m_segments.empty( ) )
    {
        return false;
    }
    if ( m_segment + 1 >= m_segments.size( ) && !AddSegment( ) )
    {
        return false;
    }
    m_segment++;
    m_contents      = (Byte *)m_segments[ m_segment ].contents;
    m_nextOffset    = 0;
    m_currentOffset = 0;
    return true;
}

std::pair<Byte *, uint64_t> MetalArgumentBuffer::Reserve( size_t numBytes )
{
    std::lock_guard<std::mutex> lock( m_reserveMutex );

    const uint64_t alignedNumBytes = Utilities::Align( static_cast<uint32_t>( numBytes ), 8 );
    if ( m_contents == nullptr || alignedNumBytes > m_capacity )
    {
        spdlog::error( "MetalArgumentBuffer::Reserve: {} bytes do not fit into an argument buffer segment of {} bytes", alignedNumBytes, m_capacity );
        return std::pair<Byte *, uint64_t>( nullptr, 0 );
    }
    if ( m_nextOffset + alignedNumBytes > m_capacity && !NextSegment( ) )
    {
        spdlog::error( "MetalArgumentBuffer::Reserve: out of memory" );
        return std::pair<Byte *, uint64_t>( nullptr, 0 );
    }

    m_currentOffset = m_nextOffset;
    Byte *ptr       = m_contents + m_currentOffset;
    m_nextOffset += alignedNumBytes;
    return std::pair<Byte *, uint64_t>( ptr, m_currentOffset );
}

std::pair<Byte *, uint64_t> MetalArgumentBuffer::Duplicate( size_t numBytes )
{
    if ( m_contents == nullptr )
    {
        return std::pair<Byte *, uint64_t>( nullptr, 0 );
    }
    Byte *prevPtr = &m_contents[ m_currentOffset ];
    auto  result  = Reserve( numBytes );
    if ( result.first != nullptr && numBytes > 0 )
    {
        std::memcpy( result.first, prevPtr, numBytes );
    }
    return result;
}

void MetalArgumentBuffer::EncodeRootConstant( uint64_t offset, uint32_t numRootConstantBytes, const Byte *data ) const
{
    if ( numRootConstantBytes == 0 )
    {
        spdlog::error( "MetalArgumentBuffer::EncodeRootConstant: No bytes reserved for root constants" );
        return;
    }

    if ( numRootConstantBytes + offset > m_capacity )
    {
        spdlog::error( "MetalArgumentBuffer::EncodeRootConstant: Index or offset out of bounds" );
        return;
    }

    std::memcpy( &m_contents[ offset ], data, numRootConstantBytes );
}

void MetalArgumentBuffer::EncodeAddress( uint64_t offset, uint64_t address ) const
{
    if ( offset + sizeof( uint64_t ) > m_capacity )
    {
        spdlog::error( "MetalArgumentBuffer::EncodeAddress: Index or offset out of bounds" );
        return;
    }
    std::memcpy( &m_contents[ offset ], &address, sizeof( uint64_t ) );
}

void MetalArgumentBuffer::Reset( )
{
    m_segment       = 0;
    m_contents      = m_segments.empty( ) ? nullptr : (Byte *)m_segments[ 0 ].contents;
    m_nextOffset    = 0;
    m_currentOffset = 0;
}

uint64_t MetalArgumentBuffer::Offset( ) const
{
    return m_nextOffset;
}

const id<MTLBuffer> &MetalArgumentBuffer::Buffer( ) const
{
    return m_segments[ m_segment ];
}

size_t MetalArgumentBuffer::Segment( ) const
{
    return m_segment;
}
