#pragma once

#include "MetalContext.h"

namespace DenOfIz
{
    struct MetalHeapDesc
    {
    };

    class MetalHeap
    {
        MetalContext *m_context;
        id<MTLHeap>   m_heap;
        uint64_t      m_gpuHandle = 0;

    public:
        MetalHeap( MetalContext *context, const MetalHeapDesc &desc );
        ~MetalHeap( ) = default;

        [[nodiscard]] uint64_t    GetNextHandle( uint32_t count );
        [[nodiscard]] id<MTLHeap> GetHeap( ) const
        {
            return m_heap;
        }
    };
} // namespace DenOfIz
