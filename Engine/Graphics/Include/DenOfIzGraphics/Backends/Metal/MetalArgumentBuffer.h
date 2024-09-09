#pragma once

#include <DenOfIzGraphics/Backends/Interface/ReflectionData.h>
#include "MetalContext.h"

namespace DenOfIz
{
    class DescriptorTable
    {
        MetalContext           *m_context;
        id<MTLBuffer>           m_buffer;
        IRDescriptorTableEntry *m_contents;
        size_t                  m_numEntries;

    public:
        DescriptorTable( MetalContext *context, size_t numEntries );
        void SetDebugName( const std::string &name );

        void                        EncodeBuffer( id<MTLBuffer> buffer, uint32_t index );
        void                        EncodeTexture( id<MTLTexture> texture, float minLodClamp, uint32_t index );
        void                        EncodeSampler( id<MTLSamplerState> sampler, float lodBias, uint32_t index );
        [[nodiscard]] id<MTLBuffer> Buffer( ) const
        {
            return m_buffer;
        }
    };

    class MetalArgumentBuffer
    {
        MetalContext *m_context;
        id<MTLBuffer> m_buffer;
        uint64_t      m_currentOffset = 0;
        uint64_t      m_nextOffset = 0;
        uint64_t      m_capacity;
        uint64_t     *m_contents;

    public:
        MetalArgumentBuffer( MetalContext *context, size_t capacity );
        void                            EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const;
        std::pair<uint64_t *, uint64_t> Reserve( size_t numAddresses );
        std::pair<uint64_t *, uint64_t> Duplicate( size_t numAddresses  );
        void                            Reset( );
        [[nodiscard]] uint64_t          Offset( ) const
        {
            return m_nextOffset;
        }
        [[nodiscard]] id<MTLBuffer> Buffer( ) const
        {
            return m_buffer;
        }
    };
} // namespace DenOfIz
