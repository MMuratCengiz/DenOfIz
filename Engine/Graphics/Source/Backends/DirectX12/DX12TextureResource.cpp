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
    Validate();
    InitDimensions(desc);

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

    resourceDesc.Alignment          = 0;
    resourceDesc.Width              = std::max(1u, m_desc.Width);
    resourceDesc.Height             = std::max(1u, m_desc.Height);
    resourceDesc.DepthOrArraySize   = std::max(1u, std::max(m_desc.ArraySize, m_desc.Depth));
    resourceDesc.MipLevels          = m_desc.MipLevels;
    resourceDesc.Format             = DX12EnumConverter::ConvertFormat(m_desc.Format);
    resourceDesc.SampleDesc.Count   = DX12EnumConverter::ConvertSampleCount(m_desc.MSAASampleCount);
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType                 = D3D12_HEAP_TYPE_DEFAULT;
    // Remove the following line to once dependency to The Forge is removed !TF!
    allocationDesc.CreationNodeMask = 1;
    allocationDesc.VisibleNodeMask  = 1;
    // --

    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::RWTexture) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12_RESOURCE_STATES initialState = DX12EnumConverter::ConvertResourceState(m_desc.InitialState);
    if ( m_desc.InitialState.IsSet(ResourceState::RenderTarget) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        initialState = DX12EnumConverter::ConvertResourceState(ResourceState::RenderTarget);
    }
    else if ( m_desc.InitialState.IsSet(ResourceState::DepthWrite) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        initialState = DX12EnumConverter::ConvertResourceState(ResourceState::DepthWrite);
    }

    // Used for certain commands
    m_resourceDesc = resourceDesc;
    HRESULT hr     = m_context->DX12MemoryAllocator->CreateResource(&allocationDesc, &resourceDesc, initialState, NULL, &m_allocation, IID_PPV_ARGS(&m_resource));
    THROW_IF_FAILED(hr);

    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::Texture) )
    {
        CreateTextureSrv();
        m_rootParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    }
    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::RWTexture) )
    {
        CreateTextureUav();
        m_rootParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    }
}

void DX12TextureResource::Validate()
{
    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::RWTexture) && m_desc.MSAASampleCount != MSAASampleCount::_0 )
    {
        LOG(WARNING) << "MSAA textures cannot be used as UAVs. Resetting MSAASampleCount to 0.";
        m_desc.MSAASampleCount = MSAASampleCount::_0;
    }

    if ( m_desc.MSAASampleCount != MSAASampleCount::_0 && m_desc.MipLevels > 1 )
    {
        LOG(WARNING) << "Mip mapped textures cannot be sampled. Resetting MSAASampleCount to 0.";
        m_desc.MSAASampleCount = MSAASampleCount::_0;
    }

    if ( m_desc.ArraySize > 1 && m_desc.Depth > 1 )
    {
        LOG(WARNING) << "Array textures cannot have depth. Resetting depth to 1.";
        m_desc.Depth = 1;
    }

    if ( !this->m_desc.Descriptor.IsSet(ResourceDescriptor::Texture) || !m_desc.Descriptor.IsSet(ResourceDescriptor::TextureCube) )
    {
        LOG(WARNING) << "Descriptor for texture contains neither Texture nor TextureCube.";
    }

    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::TextureCube) && m_desc.ArraySize != 6 )
    {
        LOG(WARNING) << "TextureCube does not have an array size of 6. ";
    }

    if ( m_desc.Descriptor.IsSet(ResourceDescriptor::TextureCube) && m_desc.Height != m_desc.Width )
    {
        LOG(WARNING) << "TextureCube does not have equal width and height.";
    }
}

void DX12TextureResource::CreateTextureSrv()
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = DX12EnumConverter::ConvertFormat(m_desc.Format);
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( m_desc.Depth > 1 )
    {
        srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels       = m_desc.MipLevels;
        srvDesc.Texture3D.MostDetailedMip = 0;
    }
    else if ( m_desc.Height > 1 )
    {
        if ( m_desc.MSAASampleCount != MSAASampleCount::_0 )
        {
            srvDesc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            srvDesc.Texture2DMSArray.ArraySize       = m_desc.ArraySize;
            srvDesc.Texture2DMSArray.FirstArraySlice = 0;
        }
        else
        {
            if ( m_desc.ArraySize > 1 )
            {
                srvDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.ArraySize       = m_desc.ArraySize;
                srvDesc.Texture2DArray.FirstArraySlice = 0;
                srvDesc.Texture2DArray.MipLevels       = m_desc.MipLevels;
                srvDesc.Texture2DArray.MostDetailedMip = 0;
            }
            else
            {
                srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels       = m_desc.MipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
            }
        }
    }
    else
    {
        if ( m_desc.ArraySize > 1 )
        {
            srvDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.ArraySize       = m_desc.ArraySize;
            srvDesc.Texture1DArray.FirstArraySlice = 0;
            srvDesc.Texture1DArray.MipLevels       = m_desc.MipLevels;
            srvDesc.Texture1DArray.MostDetailedMip = 0;
        }
        else
        {
            srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MipLevels       = m_desc.MipLevels;
            srvDesc.Texture1D.MostDetailedMip = 0;
        }
    }

    m_cpuHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetCPUStartHandle();
    m_context->D3DDevice->CreateShaderResourceView(m_resource, &srvDesc, m_cpuHandle);
}

void DX12TextureResource::CreateTextureUav()
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
    if ( m_desc.Depth > 1 )
    {
        if ( m_desc.ArraySize > 1 )
        {
            desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            desc.Texture1DArray.ArraySize       = m_desc.ArraySize;
            desc.Texture1DArray.FirstArraySlice = 0;
            desc.Texture1DArray.MipSlice        = 0;
        }
        else
        {
            desc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
            desc.Texture1D.MipSlice = 0;
        }
    }
    else if ( m_desc.Height > 1 )
    {
        if ( m_desc.ArraySize > 1 )
        {
            desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.ArraySize       = m_desc.ArraySize;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.MipSlice        = 0;
            desc.Texture2DArray.PlaneSlice      = 0;
        }
        else
        {
            desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice   = 0;
            desc.Texture2D.PlaneSlice = 0;
        }
    }
    else
    {
        desc.ViewDimension         = D3D12_UAV_DIMENSION_TEXTURE3D;
        desc.Texture3D.MipSlice    = 0;
        desc.Texture3D.FirstWSlice = 0;
        desc.Texture3D.WSize       = m_desc.ArraySize;
    }

    desc.Format = DX12EnumConverter::ConvertFormat(m_desc.Format);

    std::unique_ptr<DX12DescriptorHeap> &heap = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];
    m_cpuHandle                               = heap->GetNextCPUHandleOffset(m_desc.MipLevels);

    for ( uint32_t i = 0; i < m_desc.MipLevels; ++i )
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = D3D12_CPU_DESCRIPTOR_HANDLE(m_cpuHandle.ptr + i * heap->GetDescriptorSize());
        desc.Texture1DArray.MipSlice       = i;
        desc.Texture1DArray.ArraySize      = m_desc.ArraySize;
        desc.Texture1D.MipSlice            = i;

        if ( m_desc.Depth > 1 )
        {
            desc.Texture3D.WSize    = m_desc.ArraySize / pow(2, int(i));
            desc.Texture3D.MipSlice = i;
        }
        else if ( m_height > 1 )
        {
            desc.Texture2D.MipSlice       = i;
            desc.Texture2DArray.MipSlice  = i;
            desc.Texture2DArray.ArraySize = m_desc.ArraySize;
        }
        m_context->D3DDevice->CreateUnorderedAccessView(m_resource, nullptr, &desc, handle);
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

void DX12TextureResource::Deallocate()
{
    if ( !isExternalResource )
    {
        m_allocation->Release();
        m_resource->Release();
    }
}

DX12Sampler::DX12Sampler(DX12Context *context, const SamplerDesc &desc) : m_context(context), m_desc(desc)
{
    m_samplerDesc.Filter             = CalculateFilter(desc.MinFilter, desc.MagFilter, desc.MipmapMode, desc.CompareOp, desc.MaxAnisotropy);
    m_samplerDesc.AddressU           = DX12EnumConverter::ConvertSamplerAddressMode(desc.AddressModeU);
    m_samplerDesc.AddressV           = DX12EnumConverter::ConvertSamplerAddressMode(desc.AddressModeV);
    m_samplerDesc.AddressW           = DX12EnumConverter::ConvertSamplerAddressMode(desc.AddressModeW);
    m_samplerDesc.MipLODBias         = desc.MipLodBias;
    m_samplerDesc.MaxAnisotropy      = desc.MaxAnisotropy;
    m_samplerDesc.ComparisonFunc     = DX12EnumConverter::ConvertCompareOp(desc.CompareOp);
    m_samplerDesc.BorderColor[ 0 ]   = 0.0f;
    m_samplerDesc.BorderColor[ 1 ]   = 0.0f;
    m_samplerDesc.BorderColor[ 2 ]   = 0.0f;
    m_samplerDesc.BorderColor[ 3 ]   = 0.0f;
    m_samplerDesc.MinLOD             = desc.MinLod;
    m_samplerDesc.MaxLOD             = desc.MaxLod;

    m_cpuHandle = context->ShaderVisibleSamplerDescriptorHeap->GetNextCPUHandleOffset(1);
    m_context->D3DDevice->CreateSampler(&m_samplerDesc, m_cpuHandle);
}

D3D12_FILTER DX12Sampler::CalculateFilter(Filter min, Filter mag, MipmapMode mode, CompareOp compareOp, float maxAnisotropy) const
{
    int filter     = (static_cast<int>(min) << 4) | (static_cast<int>(mag) << 2) | static_cast<int>(mode);
    int baseFilter = compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    if ( maxAnisotropy > 0.0f )
    {
        baseFilter = compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }
    return static_cast<D3D12_FILTER>(baseFilter + filter);
}
