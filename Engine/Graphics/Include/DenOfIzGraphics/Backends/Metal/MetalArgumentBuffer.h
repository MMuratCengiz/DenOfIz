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

        void                        EncodeBuffer( id<MTLBuffer> buffer, uint32_t index );
        void                        EncodeTexture( id<MTLTexture> texture, uint32_t index );
        void                        EncodeSampler( id<MTLSamplerState> sampler, uint32_t index );
        [[nodiscard]] id<MTLBuffer> Buffer( ) const
        {
            return m_buffer;
        }
    };

    class MetalArgumentBuffer
    {
        MetalContext *m_context;
        id<MTLBuffer> m_buffer;
        uint64_t      m_offset = 0;
        uint64_t      m_capacity;
        uint64_t      m_numAddresses;
        uint64_t     *m_contents;

    public:
        MetalArgumentBuffer( MetalContext *context, size_t numAddresses, size_t numSets );
        void                            EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const;
        std::pair<uint64_t *, uint64_t> Allocate( );
        void                            Reset( );
        [[nodiscard]] uint64_t          Offset( ) const
        {
            return m_offset;
        }
        [[nodiscard]] id<MTLBuffer> Buffer( ) const
        {
            return m_buffer;
        }
    };
} // namespace DenOfIz
