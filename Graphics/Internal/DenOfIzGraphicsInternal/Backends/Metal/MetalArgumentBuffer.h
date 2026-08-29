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

#pragma once

#include <mutex>
#include <string>
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
        void                        SetDebugName( const std::string &name );
        void                        Reset( size_t newNumEntries );
        void                        EncodeBuffer( id<MTLBuffer> buffer, uint32_t index, uint32_t offset = 0 );
        void                        EncodeTexture( id<MTLTexture> texture, float minLodClamp, uint32_t index );
        void                        EncodeSampler( id<MTLSamplerState> sampler, float lodBias, uint32_t index );
        void                        EncodeAccelerationStructure( id<MTLBuffer> asHeader, uint32_t index );
        [[nodiscard]] size_t        NumEntries( ) const
        {
            return m_numEntries;
        }
        // Returned by reference on purpose: returning `id` by value makes ARC retain+autorelease the
        // buffer, which leaks when the calling thread has no autorelease pool (e.g. a .NET host).
        [[nodiscard]] const id<MTLBuffer> &Buffer( ) const
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
        void                        EncodeAddress( uint64_t offset, uint64_t address ) const;
        std::pair<Byte *, uint64_t> Reserve( size_t numBytes );
        std::pair<Byte *, uint64_t> Duplicate( size_t numBytes );
        void                        Reset( );
        [[nodiscard]] uint64_t             Offset( ) const;
        [[nodiscard]] const id<MTLBuffer> &Buffer( ) const;
    };
} // namespace DenOfIz
