#pragma once

#include <mutex>
#include "Common.h"

namespace DenOfIz
{
    class IdGen
    {
        std::mutex m_mutex;
        uint32_t   m_id = 0;

    public:
        IdGen( ) = default;

        uint32_t NextId( )
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            return m_id++;
        }
    };
} // namespace DenOfIz

#define DZ_CLASS_UNIQUE_ID_PROVIDER( fieldName )                                                                                                                                   \
    uint32_t        fieldName = UID( );                                                                                                                                            \
    static uint32_t UID( )                                                                                                                                                         \
    {                                                                                                                                                                              \
        static DenOfIz::IdGen idGen;                                                                                                                                               \
        return idGen.NextId( );                                                                                                                                                    \
    }
