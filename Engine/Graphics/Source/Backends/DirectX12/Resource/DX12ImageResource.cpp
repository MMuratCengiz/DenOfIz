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

#include <DenOfIzGraphics/Backends/DirectX12/Resource/DX12ImageResource.h>

using namespace DenOfIz;

DX12ImageResource::DX12ImageResource(DX12Context *context, const ImageCreateInfo &createInfo) : m_context(context), m_createInfo(createInfo) {}

DX12ImageResource::DX12ImageResource(ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle) : m_resource(resource), m_cpuHandle(cpuHandle)
{
    isExternalResource = true;
}

void DX12ImageResource::Allocate(const void *data)
{
    if ( isExternalResource )
    {
        LOG(WARNING) << "Allocating an externally managed resource(i.e. a swapchain render target).";
        return;
    }

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = 1024;
    resourceDesc.Height = 1024;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    // Remove the following line to once dependency to The Forge is removed !TF!
    allocationDesc.CreationNodeMask = 1;
    allocationDesc.VisibleNodeMask = 1;
    // --
    D3D12MA::Allocation *allocation;
    HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, &allocation, IID_PPV_ARGS(&m_resource));

    THROW_IF_FAILED(hr);
}

void DX12ImageResource::AttachSampler(SamplerCreateInfo &info) {}

void DX12ImageResource::Deallocate()
{
//    DX_SAFE_RELEASE(m_resource);
}
