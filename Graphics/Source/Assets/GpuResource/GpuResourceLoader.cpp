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

#include <DenOfIzGraphics/Assets/GpuResource/GpuResourceLoader.h>

using namespace DenOfIz;

StagingBuffer::StagingBuffer( ILogicalDevice *device, uint64_t numBytes )
{
    BufferDesc desc{ };
    desc.Usages.Set( ResourceUsage::CopySrc );
    desc.NumBytes   = numBytes;
    desc.HeapType   = HeapType::CPU_GPU;
    m_buffer        = std::unique_ptr<IBufferResource>( device->CreateBufferResource( desc ) );
    m_currentOffset = 0;
    m_totalNumBytes = numBytes;
    m_lastHandle    = 0;
}

bool StagingBuffer::CanFit( uint64_t size ) const
{
    return ( m_currentOffset + size ) <= m_totalNumBytes;
}

void *StagingBuffer::Map( )
{
    return m_buffer->MapMemory( );
}

void StagingBuffer::Unmap( )
{
    m_buffer->UnmapMemory( );
}

void StagingBuffer::Reset( )
{
    m_currentOffset = 0;
    m_lastHandle    = 0;
}

IBufferResource *StagingBuffer::GetBuffer( ) const
{
    return m_buffer.get( );
}

uint64_t StagingBuffer::GetOffset( ) const
{
    return m_currentOffset;
}

UpdateHandle StagingBuffer::GetLastHandle( ) const
{
    return m_lastHandle;
}

void StagingBuffer::Advance( uint64_t size, UpdateHandle handle )
{
    m_currentOffset += size;
    m_lastHandle = handle;
}

