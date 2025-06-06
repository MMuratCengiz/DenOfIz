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
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;
using namespace Microsoft::WRL;

DX12DescriptorHeap::DX12DescriptorHeap( ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible ) : m_shaderVisible( shaderVisible )
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = { };
    if ( shaderVisible )
    {
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV )
        {
            desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
        }
        else if ( type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER )
        {
            desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        }
    }
    else
    {
        switch ( type )
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            desc.NumDescriptors = 1024 * 256;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            desc.NumDescriptors = 2048;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            desc.NumDescriptors = 512;
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
            break;
        }
    }

    desc.Type     = type;
    desc.NodeMask = 0;

    if( FAILED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_heap.addressof( ) ) ) ) )
    {
        spdlog::error("Failed to create descriptor heap.");
    }

    m_descriptorSize  = device->GetDescriptorHandleIncrementSize( type );
    m_startHandle.Cpu = m_heap->GetCPUDescriptorHandleForHeapStart( );
    if ( shaderVisible )
    {
        m_startHandle.GpuVisible = true;
        m_startHandle.Gpu        = m_heap->GetGPUDescriptorHandleForHeapStart( );
    }
    m_nextHandle = m_startHandle;
}

DescriptorHandle DX12DescriptorHeap::GetNextHandle( const uint32_t count )
{
    std::lock_guard        lock( m_mutex );
    const DescriptorHandle handle = m_nextHandle;
    m_nextHandle.Cpu.Offset( count, m_descriptorSize );
    if ( m_shaderVisible )
    {
        m_nextHandle.GpuVisible = true;
        m_nextHandle.Gpu.Offset( count, m_descriptorSize );
    }
    return handle;
}

uint32_t DX12DescriptorHeap::GetDescriptorSize( ) const
{
    return m_descriptorSize;
}

ID3D12DescriptorHeap *DX12DescriptorHeap::GetHeap( ) const
{
    return m_heap.get( );
}

DescriptorHandle DX12DescriptorHeap::GetStartHandle( ) const
{
    return m_startHandle;
}

uint32_t DX12DescriptorHeap::RoundUp( const uint32_t size, const uint32_t alignment )
{
    return ( size + ( alignment - 1 ) ) & ~( alignment - 1 );
}
