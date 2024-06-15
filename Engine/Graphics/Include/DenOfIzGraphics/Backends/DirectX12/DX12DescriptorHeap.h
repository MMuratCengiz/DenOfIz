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
#include <directx/d3d12.h>

using namespace Microsoft::WRL;

namespace DenOfIz
{

class DX12DescriptorHeap
{
private:
	std::mutex m_mutex;

	ComPtr<ID3D12DescriptorHeap> m_heap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuStartHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuStartHandle;
	uint32_t m_descriptorSize;
public:
	DX12DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		if (shaderVisible)
		{
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
			{
				desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
			}
			else if (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
			{
				desc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
			}
		}
		else
		{
			switch (type)
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

		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
		m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		m_cpuStartHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
//		m_gpuStartHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
	}

	uint32_t GetDescriptorSize() const
	{
		return m_descriptorSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle()
	{
		return m_heap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle()
	{
		return m_heap->GetGPUDescriptorHandleForHeapStart();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetNextCPUHandleOffset(uint32_t count)
	{
		m_mutex.lock();
		m_cpuStartHandle.Offset(m_descriptorSize * count);
		m_mutex.unlock();
		return m_cpuStartHandle;
	}

	~DX12DescriptorHeap()
	{
		m_heap.Reset();
	}
};

}