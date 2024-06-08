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

#include <DenOfIzGraphics/Backends/DirectX12/Resource/DX12BufferResource.h>

using namespace DenOfIz;

DX12BufferResource::DX12BufferResource(DX12Context* context, const BufferCreateInfo& createInfo) : m_context(context), m_createInfo(createInfo)
{

}

void DX12BufferResource::Allocate(const void* data)
{
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = m_size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12MA::ALLOCATION_DESC allocationDesc = {};
	allocationDesc.HeapType = DX12EnumConverter::ConvertHeapType(m_createInfo.HeapType);

	HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(
			&allocationDesc,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			NULL,
			&m_allocation,
			IID_PPV_ARGS(&m_resource));

	DX_CHECK_RESULT(hr);
}

void DX12BufferResource::Deallocate()
{
	m_allocation->Release();
	m_resource->Release();
}
