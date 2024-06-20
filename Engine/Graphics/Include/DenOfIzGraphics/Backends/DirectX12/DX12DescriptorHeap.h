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

#include <directx/d3d12.h>
#include <mutex>

using namespace Microsoft::WRL;

namespace DenOfIz
{

    class DX12DescriptorHeap
    {
    private:
        std::mutex m_mutex;

        wil::com_ptr<ID3D12DescriptorHeap> m_heap;
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuStartHandle;
        CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuStartHandle;
        uint32_t m_descriptorSize;

    public:
        DX12DescriptorHeap(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
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

            desc.Type = type;
            desc.NodeMask = 0;

            device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.addressof()));
            m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
            m_cpuStartHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
            //		m_gpuStartHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
        }

        uint32_t GetDescriptorSize() const { return m_descriptorSize; }

        ID3D12DescriptorHeap *GetHeap() { return m_heap.get(); }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() { return m_heap->GetCPUDescriptorHandleForHeapStart(); }

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() { return m_heap->GetGPUDescriptorHandleForHeapStart(); }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetNextCPUHandleOffset(uint32_t count)
        {
            m_mutex.lock();
            CD3DX12_CPU_DESCRIPTOR_HANDLE handle = m_cpuStartHandle;
            m_cpuStartHandle.Offset(count, m_descriptorSize);
            m_mutex.unlock();
            return handle;
        }

        static const uint32_t RoundUp(uint32_t size, uint32_t alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) { return (size + (alignment - 1)) & ~(alignment - 1); }

        ~DX12DescriptorHeap() { /*m_heap.reset();*/ }
    };

} // namespace DenOfIz
