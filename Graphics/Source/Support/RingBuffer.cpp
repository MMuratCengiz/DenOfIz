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

#include "DenOfIzGraphics/Support/RingBuffer.h"
#include "DenOfIzGraphics/Support/GPUBufferView.h"

#include <algorithm>
#include <vector>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define RING_BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::RingBuffer, handle )

namespace DenOfIz
{
    class RingBuffer
    {
        static constexpr size_t ALIGNMENT = 256;

        struct ChunkInfo
        {
            DenOfIz_Buffer Buffer;
            uint8_t       *MappedMemory = nullptr;
            size_t         StartIndex;
            size_t         EndIndex;
            size_t         NumBytes;
        };

        std::vector<ChunkInfo> m_chunks;
        size_t                 m_dataNumBytes;
        size_t                 m_numEntries;
        size_t                 m_alignedNumBytes;
        size_t                 m_totalNumBytes;
        size_t                 m_maxChunkNumBytes;
        bool                   m_isStructuredBuffer{ };
        bool                   m_memoryMapped{ };

    public:
        explicit RingBuffer( const DenOfIz_RingBufferDesc *desc );
        ~RingBuffer( );

        DenOfIz_GPUBufferView GetBufferView( size_t index ) const;
        Byte                 *GetMappedMemory( size_t index );
        size_t                GetAlignedNumBytes( ) const;
        size_t                GetTotalNumBytes( ) const;

    private:
        void          MapMemory( );
        static size_t AlignUp( size_t value, size_t alignment );
        size_t        GetChunkIndexForEntry( size_t entryIndex ) const;
    };

    RingBuffer::RingBuffer( const DenOfIz_RingBufferDesc *desc ) :
        m_dataNumBytes( desc->DataNumBytes ), m_numEntries( desc->NumEntries ), m_maxChunkNumBytes( desc->MaxChunkNumBytes )
    {
        if ( desc->LogicalDevice == DENOFIZ_NULL_HANDLE )
        {
            spdlog::error( "RingBuffer: LogicalDevice is null" );
            return;
        }
        if ( desc->DataNumBytes <= 0 )
        {
            spdlog::error( "RingBuffer: DataNumBytes must be greater than 0" );
            return;
        }
        if ( desc->NumEntries <= 0 )
        {
            spdlog::error( "RingBuffer: NumEntries must be greater than 0" );
            return;
        }
        if ( desc->MaxChunkNumBytes <= 0 )
        {
            spdlog::error( "RingBuffer: MaxChunkNumBytes must be greater than 0" );
            return;
        }

        m_alignedNumBytes    = AlignUp( m_dataNumBytes, ALIGNMENT );
        m_totalNumBytes      = m_alignedNumBytes * m_numEntries;
        m_isStructuredBuffer = desc->IsStructuredBuffer;

        const size_t entriesPerChunk = desc->MaxChunkNumBytes / m_alignedNumBytes;
        if ( entriesPerChunk == 0 )
        {
            spdlog::critical( "RingBuffer: Single entry size {} exceeds MaxChunkNumBytes {}", m_alignedNumBytes, desc->MaxChunkNumBytes );
            return;
        }
        size_t remainingEntries = m_numEntries;
        size_t currentIndex     = 0;
        while ( remainingEntries > 0 )
        {
            const size_t entriesInThisChunk = std::min( entriesPerChunk, remainingEntries );
            const size_t chunkNumBytes      = entriesInThisChunk * m_alignedNumBytes;

            ChunkInfo chunk;
            chunk.StartIndex = currentIndex;
            chunk.EndIndex   = currentIndex + entriesInThisChunk;
            chunk.NumBytes   = chunkNumBytes;

            DenOfIz_BufferDesc bufferDesc{ };
            bufferDesc.NumBytes  = chunkNumBytes;
            bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
            bufferDesc.DebugName = DENOFIZ_STRING( "RingBuffer_Chunk" );
            if ( desc->IsStructuredBuffer )
            {
                bufferDesc.StructureDesc.NumElements = entriesInThisChunk;
                bufferDesc.StructureDesc.Stride      = m_dataNumBytes;
            }
            else
            {
                bufferDesc.Usage = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
            }

            DenOfIz_LogicalDevice_CreateBuffer( desc->LogicalDevice, &bufferDesc, &chunk.Buffer );
            m_chunks.push_back( std::move( chunk ) );

            currentIndex += entriesInThisChunk;
            remainingEntries -= entriesInThisChunk;

            if ( desc->IsStructuredBuffer )
            {
                break;
            }
        }
    }

    RingBuffer::~RingBuffer( )
    {
        if ( m_memoryMapped )
        {
            for ( const auto &chunk : m_chunks )
            {
                if ( chunk.MappedMemory )
                {
                    DenOfIz_Buffer_UnmapMemory( chunk.Buffer );
                }
            }
        }
        for ( const auto &chunk : m_chunks )
        {
            DenOfIz_Buffer_Destroy( chunk.Buffer );
        }
    }

    DenOfIz_GPUBufferView RingBuffer::GetBufferView( const size_t index ) const
    {
        if ( index >= m_numEntries )
        {
            spdlog::warn( "RingBuffer::GetBufferView invalid index" );
            return DenOfIz_GPUBufferView{ };
        }

        const size_t chunkIndex = GetChunkIndexForEntry( index );
        if ( chunkIndex >= m_chunks.size( ) )
        {
            spdlog::warn( "RingBuffer::GetBufferView invalid chunk index" );
            return DenOfIz_GPUBufferView{ };
        }

        const auto  &chunk             = m_chunks[ chunkIndex ];
        const size_t indexWithinChunk  = index - chunk.StartIndex;
        const size_t offsetWithinChunk = indexWithinChunk * m_alignedNumBytes;

        DenOfIz_GPUBufferView view{ };
        view.Buffer   = chunk.Buffer;
        view.NumBytes = m_dataNumBytes;
        view.Offset   = offsetWithinChunk;

        return view;
    }

    void RingBuffer::MapMemory( )
    {
        if ( m_memoryMapped )
        {
            return;
        }

        m_memoryMapped = true;
        for ( auto &chunk : m_chunks )
        {
            DenOfIz_Buffer_MapMemory( chunk.Buffer, (void **)&chunk.MappedMemory );
        }
    }

    Byte *RingBuffer::GetMappedMemory( const size_t index )
    {
        if ( index >= m_numEntries )
        {
            spdlog::warn( "RingBuffer::GetMappedMemoryForEntry invalid index" );
        }

        MapMemory( );

        const size_t chunkIndex = GetChunkIndexForEntry( index );

        const auto  &chunk             = m_chunks[ chunkIndex ];
        const size_t indexWithinChunk  = index - chunk.StartIndex;
        const size_t offsetWithinChunk = indexWithinChunk * m_alignedNumBytes;

        return chunk.MappedMemory + offsetWithinChunk;
    }

    size_t RingBuffer::GetAlignedNumBytes( ) const
    {
        return m_alignedNumBytes;
    }

    size_t RingBuffer::GetTotalNumBytes( ) const
    {
        return m_totalNumBytes;
    }

    size_t RingBuffer::AlignUp( const size_t value, const size_t alignment )
    {
        return value + alignment - 1 & ~( alignment - 1 );
    }

    size_t RingBuffer::GetChunkIndexForEntry( const size_t entryIndex ) const
    {
        if ( m_isStructuredBuffer )
        {
            return 0;
        }

        size_t left  = 0;
        size_t right = m_chunks.size( ) - 1;
        while ( left <= right )
        {
            const size_t mid = left + ( right - left ) / 2;
            if ( const auto &chunk = m_chunks[ mid ]; entryIndex >= chunk.StartIndex && entryIndex < chunk.EndIndex )
            {
                return mid;
            }
            else if ( entryIndex < chunk.StartIndex )
            {
                right = mid - 1;
            }
            else
            {
                left = mid + 1;
            }
        }
        return 0;
    }
} // namespace DenOfIz

extern "C"
{
    DenOfIz_RingBuffer DenOfIz_RingBuffer_Create( const DenOfIz_RingBufferDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        auto *ringBuffer = new DenOfIz::RingBuffer( desc );
        return DENOFIZ_TO_HANDLE( ringBuffer );
    }

    void DenOfIz_RingBuffer_Destroy( DenOfIz_RingBuffer ringBuffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( ringBuffer ) )
        {
            return;
        }
        delete RING_BUFFER_IMPL( ringBuffer );
    }

    DenOfIz_GPUBufferView DenOfIz_RingBuffer_GetBufferView( DenOfIz_RingBuffer ringBuffer, size_t index )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( ringBuffer ) )
        {
            return DenOfIz_GPUBufferView{ };
        }
        return RING_BUFFER_IMPL( ringBuffer )->GetBufferView( index );
    }

    Byte *DenOfIz_RingBuffer_GetMappedMemory( DenOfIz_RingBuffer ringBuffer, size_t index )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( ringBuffer ) )
        {
            return NULL;
        }
        return RING_BUFFER_IMPL( ringBuffer )->GetMappedMemory( index );
    }

    size_t DenOfIz_RingBuffer_GetAlignedNumBytes( DenOfIz_RingBuffer ringBuffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( ringBuffer ) )
        {
            return 0;
        }
        return RING_BUFFER_IMPL( ringBuffer )->GetAlignedNumBytes( );
    }

    size_t DenOfIz_RingBuffer_GetTotalNumBytes( DenOfIz_RingBuffer ringBuffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( ringBuffer ) )
        {
            return 0;
        }
        return RING_BUFFER_IMPL( ringBuffer )->GetTotalNumBytes( );
    }
}
