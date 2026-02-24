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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12DescriptorHeap.h"
#include <algorithm>
#include <cstring>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;
using namespace Microsoft::WRL;

DX12DescriptorHeap::DX12DescriptorHeap( ID3D12Device *device, const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool shaderVisible ) :
    m_device( device ), m_type( type ), m_shaderVisible( shaderVisible ), m_usedDescriptors( 0 ), m_flags( nullptr )
{
    m_descriptorSize = device->GetDescriptorHandleIncrementSize( type );

    if ( shaderVisible )
    {
        switch ( type )
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            m_maxDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            m_maxDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
            break;
        default:
            spdlog::error( "Invalid shader-visible heap type: {}", static_cast<int>( type ) );
            m_maxDescriptors = 1024;
            break;
        }
    }
    else
    {
        switch ( type )
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            m_maxDescriptors = 1024 * 256;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            m_maxDescriptors = 2048;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            m_maxDescriptors = 512;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            m_maxDescriptors = 512;
            break;
        default:
            m_maxDescriptors = 1024;
            break;
        }
    }

    CreateHeap( );
}

DX12DescriptorHeap::~DX12DescriptorHeap( )
{
    if ( m_flags )
    {
        delete[] m_flags;
        m_flags = nullptr;
    }
}

void DX12DescriptorHeap::CreateHeap( )
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = { };
    desc.Type                       = m_type;
    desc.NumDescriptors             = m_maxDescriptors;
    desc.NodeMask                   = 0;
    desc.Flags                      = m_shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = m_device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_heap.addressof( ) ) );
    if ( FAILED( hr ) )
    {
        spdlog::error( "Failed to create descriptor heap: type={}, size={}, shaderVisible={}, HRESULT=0x{:08X}", static_cast<int>( m_type ), m_maxDescriptors, m_shaderVisible,
                       hr );
        m_maxDescriptors = 0;
        return;
    }

    m_startHandle.Cpu = m_heap->GetCPUDescriptorHandleForHeapStart( );
    if ( m_shaderVisible )
    {
        m_startHandle.GpuVisible = true;
        m_startHandle.Gpu        = m_heap->GetGPUDescriptorHandleForHeapStart( );
    }
    else
    {
        m_startHandle.GpuVisible = false;
    }

    const uint32_t numBlocks = ( m_maxDescriptors + DESCRIPTOR_HEAP_BLOCK_SIZE - 1 ) / DESCRIPTOR_HEAP_BLOCK_SIZE;
    m_flags                  = new uint32_t[ numBlocks ];
    memset( m_flags, 0, numBlocks * sizeof( uint32_t ) );

    spdlog::info( "Created descriptor heap: type={}, size={}, shaderVisible={}, blocks={}", static_cast<int>( m_type ), m_maxDescriptors, m_shaderVisible, numBlocks );
}

DescriptorHandle DX12DescriptorHeap::AllocateDescriptors( const uint32_t count )
{
    std::lock_guard lock( m_mutex );

    if ( !m_heap || m_maxDescriptors == 0 )
    {
        spdlog::error( "Descriptor heap not initialized" );
        return { };
    }

    if ( count == 0 )
    {
        return { };
    }

    const uint32_t startIndex = ConsumeDescriptorHandles( count );
    if ( startIndex == UINT32_MAX )
    {
        spdlog::error( "Failed to allocate {} descriptors from heap (type={}, used={}, max={})", count, static_cast<int>( m_type ), m_usedDescriptors, m_maxDescriptors );
        return { };
    }

    DescriptorHandle result = m_startHandle;
    result.Cpu.Offset( startIndex, m_descriptorSize );
    if ( m_shaderVisible )
    {
        result.Gpu.Offset( startIndex, m_descriptorSize );
    }

    m_usedDescriptors += count;
    return result;
}

void DX12DescriptorHeap::FreeDescriptors( const DescriptorHandle &handle, const uint32_t count )
{
    std::lock_guard lock( m_mutex );
    if ( !m_heap || count == 0 )
    {
        return;
    }

    const SIZE_T heapStart = m_startHandle.Cpu.ptr;
    const SIZE_T heapEnd   = heapStart + m_maxDescriptors * m_descriptorSize;

    if ( handle.Cpu.ptr >= heapStart && handle.Cpu.ptr < heapEnd )
    {
        const uint32_t descriptorIndex = static_cast<uint32_t>( ( handle.Cpu.ptr - heapStart ) / m_descriptorSize );
        ReturnDescriptorHandles( descriptorIndex, count );
        m_usedDescriptors -= count;
    }
    else
    {
        spdlog::warn( "Attempted to free descriptor handle not from this heap" );
    }
}

uint32_t DX12DescriptorHeap::ConsumeDescriptorHandles( uint32_t descriptorCount ) const
{
    if ( descriptorCount == 0 )
    {
        return UINT32_MAX;
    }

    const uint32_t numBlocks = ( m_maxDescriptors + DESCRIPTOR_HEAP_BLOCK_SIZE - 1 ) / DESCRIPTOR_HEAP_BLOCK_SIZE;

    for ( uint32_t i = 0; i < numBlocks; ++i )
    {
        const uint32_t &flag       = m_flags[ i ];
        const uint32_t  blockStart = i * DESCRIPTOR_HEAP_BLOCK_SIZE;
        const uint32_t  blockEnd   = std::min( blockStart + DESCRIPTOR_HEAP_BLOCK_SIZE, m_maxDescriptors );

        for ( uint32_t j = 0; j < blockEnd - blockStart; ++j )
        {
            const uint32_t mask = 1u << j;
            if ( ( flag & mask ) == 0 )
            {
                uint32_t contiguousCount = 0;
                uint32_t checkIndex      = blockStart + j;

                while ( contiguousCount < descriptorCount && checkIndex < m_maxDescriptors )
                {
                    const uint32_t checkBlock = checkIndex / DESCRIPTOR_HEAP_BLOCK_SIZE;
                    const uint32_t checkBit   = checkIndex % DESCRIPTOR_HEAP_BLOCK_SIZE;
                    const uint32_t checkMask  = 1u << checkBit;

                    if ( m_flags[ checkBlock ] & checkMask )
                    {
                        break;
                    }

                    ++contiguousCount;
                    ++checkIndex;
                }

                if ( contiguousCount >= descriptorCount )
                {
                    for ( uint32_t k = 0; k < descriptorCount; ++k )
                    {
                        const uint32_t index      = blockStart + j + k;
                        const uint32_t blockIndex = index / DESCRIPTOR_HEAP_BLOCK_SIZE;
                        const uint32_t bitIndex   = index % DESCRIPTOR_HEAP_BLOCK_SIZE;
                        m_flags[ blockIndex ] |= 1u << bitIndex;
                    }
                    return blockStart + j;
                }

                j += contiguousCount;
            }
        }
    }

    return UINT32_MAX;
}

void DX12DescriptorHeap::ReturnDescriptorHandles( uint32_t startIndex, uint32_t descriptorCount ) const
{
    for ( uint32_t i = 0; i < descriptorCount; ++i )
    {
        const uint32_t index      = startIndex + i;
        const uint32_t blockIndex = index / DESCRIPTOR_HEAP_BLOCK_SIZE;
        const uint32_t bitIndex   = index % DESCRIPTOR_HEAP_BLOCK_SIZE;
        m_flags[ blockIndex ] &= ~( 1u << bitIndex );
    }
}

uint32_t DX12DescriptorHeap::GetDescriptorSize( ) const
{
    return m_descriptorSize;
}

ID3D12DescriptorHeap *DX12DescriptorHeap::GetActiveHeap( ) const
{
    return m_heap.get( );
}

const std::vector<ID3D12DescriptorHeap *> &DX12DescriptorHeap::GetActiveHeaps( ) const
{
    static std::vector<ID3D12DescriptorHeap *> heaps;
    heaps.clear( );
    if ( m_heap )
    {
        heaps.push_back( m_heap.get( ) );
    }
    return heaps;
}

uint32_t DX12DescriptorHeap::RoundUp( const uint32_t size, const uint32_t alignment )
{
    return size + alignment - 1 & ~( alignment - 1 );
}
