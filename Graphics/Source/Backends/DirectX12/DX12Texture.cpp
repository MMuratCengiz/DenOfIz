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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Texture.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

using namespace DenOfIz;

DX12Texture::DX12Texture( DX12Context *context, const DenOfIz_TextureDesc &desc ) : m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) ), m_context( context )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DENOFIZ_STRING( m_debugName.c_str( ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_width  = desc.Width;
    m_height = desc.Height;
    m_depth  = desc.Depth;
    m_format = desc.Format;

    bool isDepthFormat        = DenOfIz_Format_IsDepth( m_desc.Format ) || DenOfIz_Format_IsDepthStencil( m_desc.Format );
    bool isRenderAttachment   = ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT ) != 0;
    bool hasTextureBinding    = ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT ) != 0;
    bool needsDepthSrvSupport = isDepthFormat && isRenderAttachment && hasTextureBinding;

    m_useTypelessDepthFormat = needsDepthSrvSupport && !m_context->DX12Capabilities.CastingFullyTypedFormatSupported;
    m_useDepthSrvCasting     = needsDepthSrvSupport && m_context->DX12Capabilities.CastingFullyTypedFormatSupported;

    D3D12_RESOURCE_DESC resourceDesc = { };

    if ( m_desc.Depth > 1 )
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    }
    else if ( m_desc.Height == 0 )
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    }
    else
    {
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }

    resourceDesc.Alignment        = 0;
    resourceDesc.Width            = std::max( 1u, m_desc.Width );
    resourceDesc.Height           = std::max( 1u, m_desc.Height );
    resourceDesc.DepthOrArraySize = std::max( 1u, std::max( m_desc.ArraySize, m_desc.Depth ) );
    resourceDesc.MipLevels        = m_desc.MipLevels;
    resourceDesc.Format =
        m_useTypelessDepthFormat ? DenOfIz_DX12EnumConverter_GetTypelessFormatForDepth( m_desc.Format ) : DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );
    resourceDesc.SampleDesc.Count   = DenOfIz_DX12EnumConverter_ConvertSampleCount( m_desc.MSAASampleCount );
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags              = DenOfIz_DX12EnumConverter_ConvertTextureUsageToResourceFlags( m_desc.Usage, m_desc.Format );

    D3D12MA::ALLOCATION_DESC allocationDesc = { };
    allocationDesc.HeapType                 = D3D12_HEAP_TYPE_DEFAULT;

    m_resourceDesc               = resourceDesc;
    D3D12_CLEAR_VALUE clearValue = { };
    clearValue.Format            = m_useTypelessDepthFormat ? DenOfIz_DX12EnumConverter_GetDsvFormat( m_desc.Format ) : resourceDesc.Format;

    if ( isRenderAttachment )
    {
        if ( isDepthFormat )
        {
            clearValue.DepthStencil.Depth   = m_desc.ClearDepthStencilHint.X;
            clearValue.DepthStencil.Stencil = static_cast<UINT8>( m_desc.ClearDepthStencilHint.Y );
        }
        else
        {
            clearValue.Color[ 0 ] = m_desc.ClearColorHint.X;
            clearValue.Color[ 1 ] = m_desc.ClearColorHint.Y;
            clearValue.Color[ 2 ] = m_desc.ClearColorHint.Z;
            clearValue.Color[ 3 ] = m_desc.ClearColorHint.W;
        }
    }

    auto initialState = D3D12_RESOURCE_STATE_COMMON;
    if ( isRenderAttachment )
    {
        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, initialState, &clearValue, &m_allocation, IID_PPV_ARGS( &m_resource ) );
        DX_CHECK_RESULT( hr );
    }
    else
    {
        HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, initialState, nullptr, &m_allocation, IID_PPV_ARGS( &m_resource ) );
        DX_CHECK_RESULT( hr );
    }

    if ( !m_debugName.empty( ) )
    {
        const std::wstring name( m_debugName.begin( ), m_debugName.end( ) );
        DX_CHECK_RESULT( m_resource->SetName( name.c_str( ) ) );
        m_allocation->SetName( name.c_str( ) );
    }
}

DX12Texture::DX12Texture( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle ) : m_resource( resource ), m_rtvHandle( cpuHandle )
{
    isExternalResource = true;
}

DX12Texture::~DX12Texture( )
{
    if ( !isExternalResource )
    {
        // DX12Texture is a unique use case, since it can also be initialized by SwapChain which owns its own resource
        // Therefore manual memory management is required in this specific use case.
        m_allocation->Release( );
        m_resource->Release( );
    }
}

void DX12Texture::CreateView( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const uint32_t mipLevel ) const
{
    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT )
    {
        CreateTextureSrv( cpuHandle, mipLevel );
    }
    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        CreateTextureUav( cpuHandle, mipLevel );
    }
}

const D3D12_CPU_DESCRIPTOR_HANDLE &DX12Texture::GetOrCreateRtvHandle( )
{
    if ( m_rtvHandle.ptr != 0 )
    {
        return m_rtvHandle;
    }

    m_rtvHandle                           = m_context->RtvDescriptorHeap->AllocateDescriptors( 1 ).Cpu;
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
    rtvDesc.Format                        = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );

    if ( m_desc.Depth > 1 )
    {
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
    }
    else if ( m_desc.Height == 0 )
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
    else
    {
        if ( m_desc.MSAASampleCount != DENOFIZ_MSAA_SAMPLE_COUNT_0 )
        {
            if ( m_desc.ArraySize > 1 )
            {
                rtvDesc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtvDesc.Texture2DMSArray.FirstArraySlice = 0;
                rtvDesc.Texture2DMSArray.ArraySize       = m_desc.ArraySize;
            }
            else
            {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
            }
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

    m_context->D3DDevice->CreateRenderTargetView( m_resource, &rtvDesc, m_rtvHandle );
    return m_rtvHandle;
}

const D3D12_CPU_DESCRIPTOR_HANDLE &DX12Texture::GetOrCreateDsvHandle( )
{
    if ( m_dsvHandle.ptr != 0 )
    {
        return m_dsvHandle;
    }

    m_dsvHandle                           = m_context->DsvDescriptorHeap->AllocateDescriptors( 1 ).Cpu;
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
    dsvDesc.Format = m_useTypelessDepthFormat ? DenOfIz_DX12EnumConverter_GetDsvFormat( m_desc.Format ) : DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );

    if ( m_desc.ArraySize > 1 )
    {
        if ( m_desc.MSAASampleCount > DENOFIZ_MSAA_SAMPLE_COUNT_1 )
        {
            dsvDesc.ViewDimension                    = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
            dsvDesc.Texture2DMSArray.FirstArraySlice = 0;
            dsvDesc.Texture2DMSArray.ArraySize       = m_desc.ArraySize;
        }
        else
        {
            dsvDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice        = 0;
            dsvDesc.Texture2DArray.FirstArraySlice = 0;
            dsvDesc.Texture2DArray.ArraySize       = m_desc.ArraySize;
        }
    }
    else
    {
        if ( m_desc.MSAASampleCount > DENOFIZ_MSAA_SAMPLE_COUNT_1 )
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }
    }

    m_context->D3DDevice->CreateDepthStencilView( m_resource, &dsvDesc, m_dsvHandle );
    return m_dsvHandle;
}

void DX12Texture::CreateTextureSrv( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const uint32_t mipLevel ) const
{
    const bool                      singleMip = mipLevel != UINT32_MAX;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc   = { };
    srvDesc.Format                            = ( m_useTypelessDepthFormat || m_useDepthSrvCasting ) ? DenOfIz_DX12EnumConverter_GetSrvFormatForDepth( m_desc.Format )
                                                                                                     : DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );
    srvDesc.Shader4ComponentMapping           = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( m_desc.Depth > 1 )
    {
        srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels       = singleMip ? 1 : m_desc.MipLevels;
        srvDesc.Texture3D.MostDetailedMip = singleMip ? mipLevel : 0;
    }
    else if ( m_desc.Height == 0 )
    {
        if ( m_desc.ArraySize > 1 )
        {
            srvDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            srvDesc.Texture1DArray.ArraySize       = m_desc.ArraySize;
            srvDesc.Texture1DArray.FirstArraySlice = 0;
            srvDesc.Texture1DArray.MipLevels       = singleMip ? 1 : m_desc.MipLevels;
            srvDesc.Texture1DArray.MostDetailedMip = singleMip ? mipLevel : 0;
        }
        else
        {
            srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE1D;
            srvDesc.Texture1D.MipLevels       = singleMip ? 1 : m_desc.MipLevels;
            srvDesc.Texture1D.MostDetailedMip = singleMip ? mipLevel : 0;
        }
    }
    else
    {
        if ( m_desc.MSAASampleCount != DENOFIZ_MSAA_SAMPLE_COUNT_0 )
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
                srvDesc.Texture2DArray.MipLevels       = singleMip ? 1 : m_desc.MipLevels;
                srvDesc.Texture2DArray.MostDetailedMip = singleMip ? mipLevel : 0;
            }
            else
            {
                srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels       = singleMip ? 1 : m_desc.MipLevels;
                srvDesc.Texture2D.MostDetailedMip = singleMip ? mipLevel : 0;
            }
        }
    }

    m_context->D3DDevice->CreateShaderResourceView( m_resource, &srvDesc, cpuHandle );
}

void DX12Texture::CreateTextureUav( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const uint32_t mipLevel ) const
{
    const bool singleMip = mipLevel != UINT32_MAX;

    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
    if ( m_desc.Depth > 1 )
    {
        desc.ViewDimension         = D3D12_UAV_DIMENSION_TEXTURE3D;
        desc.Texture3D.MipSlice    = singleMip ? mipLevel : 0;
        desc.Texture3D.FirstWSlice = 0;
        desc.Texture3D.WSize       = singleMip ? std::max( 1u, m_desc.Depth >> mipLevel ) : m_desc.Depth;
    }
    else if ( m_desc.Height == 0 )
    {
        if ( m_desc.ArraySize > 1 )
        {
            desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            desc.Texture1DArray.ArraySize       = m_desc.ArraySize;
            desc.Texture1DArray.FirstArraySlice = 0;
            desc.Texture1DArray.MipSlice        = singleMip ? mipLevel : 0;
        }
        else
        {
            desc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
            desc.Texture1D.MipSlice = singleMip ? mipLevel : 0;
        }
    }
    else
    {
        if ( m_desc.ArraySize > 1 )
        {
            desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.ArraySize       = m_desc.ArraySize;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.MipSlice        = singleMip ? mipLevel : 0;
            desc.Texture2DArray.PlaneSlice      = 0;
        }
        else
        {
            desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice   = singleMip ? mipLevel : 0;
            desc.Texture2D.PlaneSlice = 0;
        }
    }

    desc.Format = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );

    if ( singleMip )
    {
        m_context->D3DDevice->CreateUnorderedAccessView( m_resource, nullptr, &desc, cpuHandle );
    }
    else
    {
        std::unique_ptr<DX12DescriptorHeap> &heap = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

        for ( uint32_t i = 0; i < m_desc.MipLevels; ++i )
        {
            auto handle = D3D12_CPU_DESCRIPTOR_HANDLE( cpuHandle.ptr + i * heap->GetDescriptorSize( ) );

            if ( m_desc.Depth > 1 )
            {
                desc.Texture3D.WSize    = m_desc.Depth / pow( 2, static_cast<int>( i ) );
                desc.Texture3D.MipSlice = i;
            }
            else if ( m_desc.Height == 0 )
            {
                desc.Texture1D.MipSlice       = i;
                desc.Texture1DArray.MipSlice  = i;
                desc.Texture1DArray.ArraySize = m_desc.ArraySize;
            }
            else
            {
                desc.Texture2D.MipSlice       = i;
                desc.Texture2DArray.MipSlice  = i;
                desc.Texture2DArray.ArraySize = m_desc.ArraySize;
            }
            m_context->D3DDevice->CreateUnorderedAccessView( m_resource, nullptr, &desc, handle );
        }
    }
}

const DenOfIz_TextureDesc &DX12Texture::GetDesc( ) const
{
    return m_desc;
}

const D3D12_RESOURCE_DESC &DX12Texture::GetResourceDesc( ) const
{
    return m_resourceDesc;
}

ID3D12Resource *DX12Texture::Resource( ) const
{
    return m_resource;
}

uint32_t DX12Texture::GetWidth( ) const
{
    return m_width;
}

uint32_t DX12Texture::GetHeight( ) const
{
    return m_height;
}

uint32_t DX12Texture::GetDepth( ) const
{
    return m_depth;
}

DenOfIz_Format DX12Texture::GetFormat( ) const
{
    return m_format;
}

uint32_t DX12Texture::GetMipLevels( ) const
{
    return m_desc.MipLevels;
}

DX12Sampler::DX12Sampler( DX12Context *context, const DenOfIz_SamplerDesc &desc ) : m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    m_samplerDesc.Filter           = CalculateFilter( desc.MinFilter, desc.MagFilter, desc.MipmapMode, desc.CompareOp, desc.MaxAnisotropy );
    m_samplerDesc.AddressU         = DenOfIz_DX12EnumConverter_ConvertSamplerAddressMode( desc.AddressModeU );
    m_samplerDesc.AddressV         = DenOfIz_DX12EnumConverter_ConvertSamplerAddressMode( desc.AddressModeV );
    m_samplerDesc.AddressW         = DenOfIz_DX12EnumConverter_ConvertSamplerAddressMode( desc.AddressModeW );
    m_samplerDesc.MipLODBias       = desc.MipLodBias;
    m_samplerDesc.MaxAnisotropy    = desc.MaxAnisotropy;
    m_samplerDesc.ComparisonFunc   = DenOfIz_DX12EnumConverter_ConvertCompareOp( desc.CompareOp );
    m_samplerDesc.BorderColor[ 0 ] = 1.0f;
    m_samplerDesc.BorderColor[ 1 ] = 0.0f;
    m_samplerDesc.BorderColor[ 2 ] = 0.0f;
    m_samplerDesc.BorderColor[ 3 ] = 1.0f;
    m_samplerDesc.MinLOD           = desc.MinLod;
    m_samplerDesc.MaxLOD           = desc.MaxLod;
}

void DX12Sampler::CreateView( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle )
{
    if ( m_descriptorAllocated && m_cpuHandle.ptr == cpuHandle.ptr )
    {
        return;
    }
    m_cpuHandle = cpuHandle;
    m_context->D3DDevice->CreateSampler( &m_samplerDesc, m_cpuHandle );
    m_descriptorAllocated = true;
}

D3D12_FILTER DX12Sampler::CalculateFilter( DenOfIz_Filter min, DenOfIz_Filter mag, DenOfIz_MipmapMode mode, const DenOfIz_CompareOp compareOp, const float maxAnisotropy ) const
{
    if ( maxAnisotropy > 0.0f )
    {
        return compareOp != DENOFIZ_COMPARE_OP_NEVER ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }

    const int filter     = static_cast<int>( min ) << 4 | static_cast<int>( mag ) << 2 | static_cast<int>( mode );
    const int baseFilter = compareOp != DENOFIZ_COMPARE_OP_NEVER ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    return static_cast<D3D12_FILTER>( baseFilter + filter );
}
