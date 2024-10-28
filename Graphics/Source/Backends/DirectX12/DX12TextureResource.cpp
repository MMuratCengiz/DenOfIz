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

DX12TextureResource::DX12TextureResource( DX12Context *context, const TextureDesc &desc ) : m_desc( desc ), m_context( context )
{
    m_width        = desc.Width;
    m_height       = desc.Height;
    m_depth        = desc.Depth;
    m_format       = desc.Format;
    m_initialState = desc.InitialUsage;

    ValidateTextureDesc( m_desc );

    D3D12_RESOURCE_DESC resourceDesc = { };

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
    resourceDesc.Width              = std::max( 1u, m_desc.Width );
    resourceDesc.Height             = std::max( 1u, m_desc.Height );
    resourceDesc.DepthOrArraySize   = std::max( 1u, std::max( m_desc.ArraySize, m_desc.Depth ) );
    resourceDesc.MipLevels          = m_desc.MipLevels;
    resourceDesc.Format             = DX12EnumConverter::ConvertFormat( m_desc.Format );
    resourceDesc.SampleDesc.Count   = DX12EnumConverter::ConvertSampleCount( m_desc.MSAASampleCount );
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocationDesc = { };
    allocationDesc.HeapType                 = D3D12_HEAP_TYPE_DEFAULT;

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12_RESOURCE_STATES initialState = DX12EnumConverter::ConvertResourceUsage( m_desc.InitialUsage );
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RenderTarget ) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    else if ( m_desc.Descriptor.IsSet( ResourceDescriptor::DepthStencil ) )
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }

    // Used for certain commands
    m_resourceDesc               = resourceDesc;
    D3D12_CLEAR_VALUE clearValue = { };
    clearValue.Format            = resourceDesc.Format;

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RenderTarget ) )
    {
        clearValue.Color[ 0 ] = 0.0f;
        clearValue.Color[ 1 ] = 0.0f;
        clearValue.Color[ 2 ] = 0.0f;
        clearValue.Color[ 3 ] = 1.0f;
    }
    else if ( m_desc.Descriptor.IsSet( ResourceDescriptor::DepthStencil ) )
    {
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0.0f;
    }

    if ( m_desc.Descriptor.Any( { ResourceDescriptor::DepthStencil, ResourceDescriptor::RenderTarget } ) )
    {
        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, initialState, &clearValue, &m_allocation, IID_PPV_ARGS( &m_resource ) );
        DX_CHECK_RESULT( hr );
    }
    else
    {
        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, initialState, nullptr, &m_allocation, IID_PPV_ARGS( &m_resource ) );
        DX_CHECK_RESULT( hr );
    }

    const std::string debugName = m_desc.DebugName.Get( );
    const std::wstring name = std::wstring( debugName.begin(), debugName.end() );
    DX_CHECK_RESULT( m_resource->SetName( name.c_str( ) ) );
    m_allocation->SetName( name.c_str( ) );
}

DX12TextureResource::DX12TextureResource( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle ) : m_resource( resource ), m_rtvHandle( cpuHandle )
{
    isExternalResource = true;
}

DX12TextureResource::~DX12TextureResource( )
{
    if ( !isExternalResource )
    {
        // DX12TextureResource is a unique use case, since it can also be initialized by SwapChain which owns its own resource
        // Therefore manual memory management is required in this specific use case.
        m_allocation->Release( );
        m_resource->Release( );
    }
}

void DX12TextureResource::CreateView( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle )
{
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::Texture ) )
    {
        CreateTextureSrv( cpuHandle );
    }
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) )
    {
        CreateTextureUav( cpuHandle );
    }
}

const D3D12_CPU_DESCRIPTOR_HANDLE &DX12TextureResource::GetOrCreateRtvHandle( )
{
    if ( m_rtvHandle.ptr != 0 )
    {
        return m_rtvHandle;
    }

    m_rtvHandle                           = m_context->RtvDescriptorHeap->GetNextHandle( 1 ).Cpu;
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
    rtvDesc.Format                        = DX12EnumConverter::ConvertFormat( m_desc.Format );

    if ( m_desc.Depth > 1 )
    {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
    }
    else if ( m_desc.Height > 1 )
    {
        if ( m_desc.MSAASampleCount != MSAASampleCount::_0 )
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
        }
        else
        {
            if ( m_desc.ArraySize > 1 )
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            }
        }
    }
    else
    {
        if ( m_desc.ArraySize > 1 )
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
        }
        else
        {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
        }
    }

    m_context->D3DDevice->CreateRenderTargetView( m_resource, &rtvDesc, m_rtvHandle );
    return m_rtvHandle;
}

void DX12TextureResource::CreateTextureSrv( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle ) const
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
    srvDesc.Format                          = DX12EnumConverter::ConvertFormat( m_desc.Format );
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

    m_context->D3DDevice->CreateShaderResourceView( m_resource, &srvDesc, cpuHandle );
}

void DX12TextureResource::CreateTextureUav( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle ) const
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
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

    desc.Format = DX12EnumConverter::ConvertFormat( m_desc.Format );

    std::unique_ptr<DX12DescriptorHeap> &heap = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

    for ( uint32_t i = 0; i < m_desc.MipLevels; ++i )
    {
        auto handle                   = D3D12_CPU_DESCRIPTOR_HANDLE( cpuHandle.ptr + i * heap->GetDescriptorSize( ) );
        desc.Texture1DArray.MipSlice  = i;
        desc.Texture1DArray.ArraySize = m_desc.ArraySize;
        desc.Texture1D.MipSlice       = i;

        if ( m_desc.Depth > 1 )
        {
            desc.Texture3D.WSize    = m_desc.ArraySize / pow( 2, static_cast<int>( i ) );
            desc.Texture3D.MipSlice = i;
        }
        else if ( m_height > 1 )
        {
            desc.Texture2D.MipSlice       = i;
            desc.Texture2DArray.MipSlice  = i;
            desc.Texture2DArray.ArraySize = m_desc.ArraySize;
        }
        m_context->D3DDevice->CreateUnorderedAccessView( m_resource, nullptr, &desc, handle );
    }
}

const TextureDesc &DX12TextureResource::GetDesc( ) const
{
    return m_desc;
}

const D3D12_RESOURCE_DESC &DX12TextureResource::GetResourceDesc( ) const
{
    return m_resourceDesc;
}

ID3D12Resource *DX12TextureResource::GetResource( ) const
{
    return m_resource;
}

BitSet<ResourceUsage> DX12TextureResource::InitialState( ) const
{
    return m_initialState;
}

uint32_t DX12TextureResource::GetWidth( ) const
{
    return m_width;
}

uint32_t DX12TextureResource::GetHeight( ) const
{
    return m_height;
}

uint32_t DX12TextureResource::GetDepth( ) const
{
    return m_depth;
}

Format DX12TextureResource::GetFormat( ) const
{
    return m_format;
}

DX12Sampler::DX12Sampler( DX12Context *context, const SamplerDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_samplerDesc.Filter           = CalculateFilter( desc.MinFilter, desc.MagFilter, desc.MipmapMode, desc.CompareOp, desc.MaxAnisotropy );
    m_samplerDesc.AddressU         = DX12EnumConverter::ConvertSamplerAddressMode( desc.AddressModeU );
    m_samplerDesc.AddressV         = DX12EnumConverter::ConvertSamplerAddressMode( desc.AddressModeV );
    m_samplerDesc.AddressW         = DX12EnumConverter::ConvertSamplerAddressMode( desc.AddressModeW );
    m_samplerDesc.MipLODBias       = desc.MipLodBias;
    m_samplerDesc.MaxAnisotropy    = desc.MaxAnisotropy;
    m_samplerDesc.ComparisonFunc   = DX12EnumConverter::ConvertCompareOp( desc.CompareOp );
    m_samplerDesc.BorderColor[ 0 ] = 1.0f;
    m_samplerDesc.BorderColor[ 1 ] = 0.0f;
    m_samplerDesc.BorderColor[ 2 ] = 0.0f;
    m_samplerDesc.BorderColor[ 3 ] = 1.0f;
    m_samplerDesc.MinLOD           = desc.MinLod;
    m_samplerDesc.MaxLOD           = desc.MaxLod;
}

void DX12Sampler::CreateView( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle )
{
    DZ_RETURN_IF( m_cpuHandle.ptr != 0 && m_cpuHandle.ptr == cpuHandle.ptr );
    m_cpuHandle = cpuHandle;
    m_context->D3DDevice->CreateSampler( &m_samplerDesc, m_cpuHandle );
}

D3D12_FILTER DX12Sampler::CalculateFilter( Filter min, Filter mag, MipmapMode mode, const CompareOp compareOp, const float maxAnisotropy ) const
{
    if ( maxAnisotropy > 0.0f )
    {
        return compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }

    const int filter     = static_cast<int>( min ) << 4 | static_cast<int>( mag ) << 2 | static_cast<int>( mode );
    const int baseFilter = compareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    return static_cast<D3D12_FILTER>( baseFilter + filter );
}
