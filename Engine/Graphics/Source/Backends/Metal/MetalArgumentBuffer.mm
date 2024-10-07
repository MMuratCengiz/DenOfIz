#include <DenOfIzGraphics/Backends/Metal/MetalArgumentBuffer.h>
#import "DenOfIzCore/ContainerUtilities.h"
#import "DenOfIzCore/Utilities.h"

using namespace DenOfIz;

DescriptorTable::DescriptorTable( MetalContext *context, size_t numEntries ) : m_context( context ), m_numEntries( numEntries )
{
    size_t length = sizeof( IRDescriptorTableEntry ) * numEntries;
    m_buffer      = [m_context->Device newBufferWithLength:length options:MTLResourceStorageModeShared];
    m_contents    = (IRDescriptorTableEntry *)m_buffer.contents;
    [m_buffer setLabel:@"DescriptorTable"];
}

void DescriptorTable::SetDebugName( const std::string &name )
{
    [m_buffer setLabel:[NSString stringWithUTF8String:name.c_str( )]];
}

void DescriptorTable::EncodeBuffer( id<MTLBuffer> buffer, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetBuffer( &m_contents[ index ], [buffer gpuAddress], 0 );
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

MetalArgumentBuffer::MetalArgumentBuffer( MetalContext *context, size_t capacity ) : m_context( context ), m_capacity( capacity )
{
    m_nextOffset = 0;
    m_buffer     = [m_context->Device newBufferWithLength:m_capacity options:MTLResourceStorageModeShared];
    if ( !m_buffer )
    {
        LOG( ERROR ) << "Failed to allocate Metal argument buffer";
    }
    [m_buffer setLabel:@"MetalArgumentBuffer"];
    m_contents = (Byte *)m_buffer.contents;
}

std::pair<Byte *, uint64_t> MetalArgumentBuffer::Reserve( size_t numAddresses, uint32_t numRootConstantBytes )
{
    m_numRootConstantBytes = numRootConstantBytes;
    auto numBytes          = Utilities::Align( sizeof( uint64_t ) * numAddresses + numRootConstantBytes, 8 );
    if ( m_nextOffset + numBytes > m_capacity )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::Allocate: out of memory";
        return std::pair<Byte *, uint64_t>( nullptr, 0 );
    }

    m_currentOffset = m_nextOffset;
    Byte *ptr       = m_contents + m_nextOffset;
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

void MetalArgumentBuffer::EncodeRootConstant( const Byte *data ) const
{
    if ( m_numRootConstantBytes == 0 )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::EncodeRootConstant: No bytes reserved for root constants";
        return;
    }

    if ( m_numRootConstantBytes > m_capacity )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::EncodeRootConstant: Index or offset out of bounds";
        return;
    }

    std::memcpy( &m_contents[ 0 ], data, m_numRootConstantBytes );
}

void MetalArgumentBuffer::EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const
{
    uint64_t addressLocation = offset + m_numRootConstantBytes + ( index * sizeof( uint64_t ) );
    uint64_t abOffset        = offset / sizeof( uint64_t );

    if ( addressLocation > m_capacity )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::EncodeAddress: Index or offset out of bounds";
        return;
    }

    std::memcpy( &m_contents[ addressLocation ], &address, sizeof( uint64_t ) );
}

void MetalArgumentBuffer::Reset( )
{
    m_nextOffset = 0;
}
