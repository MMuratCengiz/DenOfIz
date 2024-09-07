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

void DescriptorTable::EncodeBuffer( id<MTLBuffer> buffer, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetBuffer( &m_contents[ index ], buffer.gpuAddress, 0 );
}

void DescriptorTable::EncodeTexture( id<MTLTexture> texture, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetTexture( &m_contents[ index ], texture, 0.0f, 0 );
}

void DescriptorTable::EncodeSampler( id<MTLSamplerState> sampler, uint32_t index )
{
    DZ_ASSERTM( index < m_numEntries, "DescriptorTable::EncodeBuffer: index out of bounds" );
    IRDescriptorTableSetSampler( &m_contents[ index ], sampler, 0 );
}

MetalArgumentBuffer::MetalArgumentBuffer( MetalContext *context, size_t numAddresses, size_t numSets ) : m_context( context ), m_numAddresses( numAddresses )
{
    m_offset   = 0;
    m_capacity = Utilities::Align( numSets * numAddresses * sizeof( uint64_t ), 8 );
    m_buffer   = [m_context->Device newBufferWithLength:m_capacity options:MTLResourceStorageModeShared];
    if ( !m_buffer )
    {
        LOG( ERROR ) << "Failed to allocate Metal argument buffer";
    }
    [m_buffer setLabel:@"MetalArgumentBuffer"];
    m_contents = (uint64_t *)m_buffer.contents;
}

std::pair<uint64_t *, uint64_t> MetalArgumentBuffer::Allocate( )
{
    auto size = Utilities::Align( sizeof( uint64_t ) * m_numAddresses, 8 );
    if ( m_offset + size > m_capacity )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::Allocate: out of memory";
        return std::pair<uint64_t *, uint64_t>( nullptr, 0 );
    }

    uint64_t  offsetBefore = m_offset;
    uint64_t *ptr          = m_contents + ( m_offset / sizeof( uint64_t ) );
    m_offset += size;
    return std::pair<uint64_t *, uint64_t>( ptr, offsetBefore );
}

void MetalArgumentBuffer::EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const
{
    if ( ( offset + ( index * sizeof( uint64_t ) ) ) > m_capacity )
    {
        LOG( ERROR ) << "MetalArgumentBuffer::EncodeAddress: Index or offset out of bounds";
        return;
    }

    m_contents[ ( offset / sizeof( uint64_t ) ) + index ] = address;
}

void MetalArgumentBuffer::Reset( )
{
    m_offset = 0;
}
