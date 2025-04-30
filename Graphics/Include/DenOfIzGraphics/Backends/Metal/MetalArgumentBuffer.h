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

        void                        EncodeBuffer( id<MTLBuffer> buffer, uint32_t index, uint32_t offset = 0 );
        void                        EncodeTexture( id<MTLTexture> texture, float minLodClamp, uint32_t index );
        void                        EncodeSampler( id<MTLSamplerState> sampler, float lodBias, uint32_t index );
        void                        EncodeAccelerationStructure( id<MTLBuffer> asHeader, uint32_t index );
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
        uint64_t      m_nextOffset    = 0;
        uint64_t      m_capacity;
        Byte         *m_contents;
        std::mutex    m_reserveMutex;

    public:
        MetalArgumentBuffer( MetalContext *context, size_t capacity );
        void                        EncodeRootConstant( uint64_t offset, uint32_t numRootConstantBytes, const Byte *data ) const;
        void                        EncodeAddress( uint64_t offset, uint32_t index, uint64_t address ) const;
        std::pair<Byte *, uint64_t> Reserve( size_t numAddresses, uint32_t numRootConstantBytes = 0 );
        std::pair<Byte *, uint64_t> Duplicate( size_t numAddresses, uint32_t numRootConstantBytes = 0 );
        void                        Reset( );
        [[nodiscard]] uint64_t      Offset( ) const;
        [[nodiscard]] id<MTLBuffer> Buffer( ) const;
    };
} // namespace DenOfIz
