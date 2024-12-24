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

#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12TextureResource final : public ITextureResource
    {
        Format        m_format       = Format::Undefined;
        uint32_t      m_width        = 1;
        uint32_t      m_height       = 1;
        uint32_t      m_depth        = 1;
        ResourceUsage m_currentUsage = ResourceUsage::Undefined;

        TextureDesc                 m_desc{ };
        DX12Context                *m_context    = nullptr;
        D3D12MA::Allocation        *m_allocation = nullptr;
        ID3D12Resource2            *m_resource   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle{ };
        D3D12_RESOURCE_DESC         m_resourceDesc{ };
        bool                        isExternalResource = false; // Used for swap chain render targets, might need a better way

    public:
        DX12TextureResource( DX12Context *context, const TextureDesc &desc );
        DX12TextureResource( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle );
        ~DX12TextureResource( ) override;
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );

        [[nodiscard]] const TextureDesc               &GetDesc( ) const;
        [[nodiscard]] const D3D12_RESOURCE_DESC       &GetResourceDesc( ) const;
        [[nodiscard]] ID3D12Resource                  *Resource( ) const;

        [[nodiscard]] BitSet<ResourceUsage> InitialState( ) const override;
        [[nodiscard]] uint32_t              GetWidth( ) const;
        [[nodiscard]] uint32_t              GetHeight( ) const;
        [[nodiscard]] uint32_t              GetDepth( ) const;
        [[nodiscard]] Format                GetFormat( ) const override;

        [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE &GetOrCreateRtvHandle( );

    private:
        void CreateTextureSrv( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle ) const;
        void CreateTextureUav( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle ) const;
    };

    class DX12Sampler final : public ISampler
    {
        DX12Context                *m_context;
        SamplerDesc                 m_desc;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ };
        D3D12_SAMPLER_DESC          m_samplerDesc{ };

    public:
        DX12Sampler( DX12Context *context, const SamplerDesc &desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );
        ~DX12Sampler( ) override = default;
        [[nodiscard]] D3D12_FILTER CalculateFilter( Filter min, Filter mag, MipmapMode mode, CompareOp compareOp, float maxAnisotropy ) const;
    };
} // namespace DenOfIz
