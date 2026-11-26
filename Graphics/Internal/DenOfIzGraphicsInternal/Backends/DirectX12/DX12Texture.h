/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ITexture.h>
#include <string>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12Texture final : public ITexture
    {
        DenOfIz_Format m_format = DENOFIZ_FORMAT_UNDEFINED;
        uint32_t       m_width  = 1;
        uint32_t       m_height = 1;
        uint32_t       m_depth  = 1;

        DenOfIz_TextureDesc         m_desc{ };
        std::string                 m_debugName;
        DX12Context                *m_context    = nullptr;
        D3D12MA::Allocation        *m_allocation = nullptr;
        ID3D12Resource2            *m_resource   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle{ };
        D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle{ };
        D3D12_RESOURCE_DESC         m_resourceDesc{ };
        bool                        isExternalResource       = false; // Swap chain render targets, the swap chain owns the resource
        bool                        m_useTypelessDepthFormat = false;
        bool                        m_useDepthSrvCasting     = false;

    public:
        DX12Texture( DX12Context *context, const DenOfIz_TextureDesc &desc );
        DX12Texture( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle );
        ~DX12Texture( ) override;
        void UpdateExternal( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle, const DenOfIz_TextureDesc &desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, uint32_t mipLevel = UINT32_MAX ) const;

        [[nodiscard]] const DenOfIz_TextureDesc &GetDesc( ) const override;
        [[nodiscard]] const D3D12_RESOURCE_DESC &GetResourceDesc( ) const;
        [[nodiscard]] ID3D12Resource            *Resource( ) const;

        [[nodiscard]] uint32_t       GetWidth( ) const;
        [[nodiscard]] uint32_t       GetHeight( ) const;
        [[nodiscard]] uint32_t       GetDepth( ) const;
        [[nodiscard]] DenOfIz_Format GetFormat( ) const override;
        [[nodiscard]] uint32_t       GetMipLevels( ) const override;

        [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE &GetOrCreateRtvHandle( );
        [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE &GetOrCreateDsvHandle( );

    private:
        void CreateTextureSrv( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, uint32_t mipLevel = UINT32_MAX ) const;
        void CreateTextureUav( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, uint32_t mipLevel = UINT32_MAX ) const;
    };

    class DX12Sampler final : public ISampler
    {
        DX12Context                *m_context;
        DenOfIz_SamplerDesc         m_desc;
        std::string                 m_debugName;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ };
        D3D12_SAMPLER_DESC          m_samplerDesc{ };
        bool                        m_descriptorAllocated{ false };

    public:
        DX12Sampler( DX12Context *context, const DenOfIz_SamplerDesc &desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );
        ~DX12Sampler( ) override = default;
        [[nodiscard]] D3D12_FILTER CalculateFilter( DenOfIz_Filter min, DenOfIz_Filter mag, DenOfIz_MipmapMode mode, DenOfIz_CompareOp compareOp, float maxAnisotropy ) const;
    };
} // namespace DenOfIz
