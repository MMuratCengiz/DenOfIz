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

#include <DenOfIzGraphics/Backends/DirectX12/DX12TextureResource.h>
#include "DenOfIzGraphics/Backends/DirectX12/DX12EnumConverter.h"

using namespace DenOfIz;

DX12TextureResource::DX12TextureResource(DX12Context *context, const TextureDesc &desc) : m_context(context), m_desc(desc)
{
    D3D12_RESOURCE_DESC resourceDesc = {};

    if ( m_desc.Depth > 1 )
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    }
    else if ( m_desc.Height > 1 )
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }
    else
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    }

    resourceDesc.Alignment = 0;
    resourceDesc.Width = m_desc.Width;
    resourceDesc.Height = m_desc.Height;
    resourceDesc.DepthOrArraySize = m_desc.ArrayLayers;
    resourceDesc.MipLevels = m_desc.MipLevels;
    resourceDesc.Format = DX12EnumConverter::ConvertFormat(m_desc.Format);
    // Todo
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

    // Decide UAV flags
    if ( m_desc.Descriptor.Texture && m_desc.Descriptor.ReadWrite )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12_RESOURCE_STATES initialState = DX12EnumConverter::ConvertResourceState(m_desc.InitialState);
    if ( m_desc.InitialState.RenderTarget )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        initialState = DX12EnumConverter::ConvertResourceState(ResourceState{ .RenderTarget = 1 });
    }
    else if ( m_desc.InitialState.DepthWrite )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        initialState = DX12EnumConverter::ConvertResourceState(ResourceState{ .DepthWrite = 1 });
    }

    HRESULT hr = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, initialState, NULL, &allocation, IID_PPV_ARGS(&m_resource));
    THROW_IF_FAILED(hr);

    CreateTextureSrv();
    CreateTextureUav();
}

void DX12TextureResource::CreateTextureSrv()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DX12EnumConverter::ConvertFormat(m_desc.Format);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( m_desc.Depth > 1 )
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = m_desc.MipLevels;
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
    }
    else if ( m_desc.Height > 1 )
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = m_desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    }
    else
    {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MipLevels = m_desc.MipLevels;
        srvDesc.Texture1D.MostDetailedMip = 0;
        srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
    }

    m_cpuHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetCPUStartHandle();
    m_context->D3DDevice->CreateShaderResourceView(m_resource, &srvDesc, m_cpuHandle);
}

void DX12TextureResource::CreateTextureUav()
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    if ( m_desc.Depth > 1 )
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = m_desc.Depth;
    }
    else if ( m_desc.Height > 1 )
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
    }
    else
    {
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        uavDesc.Texture1D.MipSlice = m_desc.MipLevels;
    }

    uavDesc.Format = DX12EnumConverter::ConvertFormat(m_desc.Format);
    for ( uint32_t i = 0; i < m_desc.MipLevels; ++i )
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = D3D12_CPU_DESCRIPTOR_HANDLE(m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetCPUStartHandle().ptr + m_cpuHandle.ptr + i);
        uavDesc.Texture1DArray.MipSlice = i;

        {
        }
        if ( m_desc.Depth > 1 )
        {
            uavDesc.Texture3D.WSize = m_desc.ArrayLayers / (UINT)pow(2.0, int(i));
            uavDesc.Texture3D.MipSlice = i;
        }
        else if ( m_height > 1 )
        {
            uavDesc.Texture2D.MipSlice = i;
        }

        m_context->D3DDevice->CreateUnorderedAccessView(m_resource, nullptr, &uavDesc, handle);
    }
}

DX12TextureResource::DX12TextureResource(ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle) : m_resource(resource), m_cpuHandle(cpuHandle)
{
    isExternalResource = true;
}

void DX12TextureResource::Allocate(const void *data)
{
    if ( isExternalResource )
    {
        LOG(WARNING) << "Allocating an externally managed resource(i.e. a swapchain render target).";
        return;
    }
}

void DX12TextureResource::AttachSampler(SamplerDesc &samplerDesc)
{
    D3D12_SAMPLER_DESC desc = {};
    desc.Filter = CalculateFilter(samplerDesc.MinFilter, samplerDesc.MagFilter, samplerDesc.MipmapMode, samplerDesc.CompareOp, samplerDesc.MaxAnisotropy);
    desc.AddressU = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeU);
    desc.AddressV = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeV);
    desc.AddressW = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeW);
    desc.MipLODBias = samplerDesc.MipLodBias;
    desc.MaxAnisotropy = samplerDesc.MaxAnisotropy;
    desc.ComparisonFunc = DX12EnumConverter::ConvertCompareOp(samplerDesc.CompareOp);
    desc.BorderColor[ 0 ] = 0.0f;
    desc.BorderColor[ 1 ] = 0.0f;
    desc.BorderColor[ 2 ] = 0.0f;
    desc.BorderColor[ 3 ] = 0.0f;
    desc.MinLOD = samplerDesc.MinLod;
    desc.MaxLOD = samplerDesc.MaxLod;

    m_context->D3DDevice->CreateSampler(&desc, m_cpuHandle);
}
void DX12TextureResource::Deallocate() {}
D3D12_FILTER DX12TextureResource::CalculateFilter(Filter min, Filter mag, MipmapMode mode, CompareOp compareOp, float maxAnisotropy) const
{
    int filter = (static_cast<int>(min) << 4) | (static_cast<int>(mag) << 2) | static_cast<int>(mode);
    int baseFilter = compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    if ( maxAnisotropy > 0.0f )
    {
        baseFilter = compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }
    return static_cast<D3D12_FILTER>(baseFilter + filter);
}
