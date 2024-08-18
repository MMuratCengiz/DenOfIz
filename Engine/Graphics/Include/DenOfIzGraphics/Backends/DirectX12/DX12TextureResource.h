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
        TextureDesc                 m_desc{ };
        DX12Context                *m_context    = nullptr;
        D3D12MA::Allocation        *m_allocation = nullptr;
        ID3D12Resource2            *m_resource   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ };
        D3D12_RESOURCE_DESC         m_resourceDesc{ };
        D3D12_ROOT_PARAMETER_TYPE   m_rootParameterType{ };
        bool                        isExternalResource = false; // Used for swap chain render targets, might need a better way

    public:
             DX12TextureResource( DX12Context *context, const TextureDesc &desc );
             DX12TextureResource( ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle );
        ~    DX12TextureResource( ) override = default;
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );

        [[nodiscard]] const TextureDesc &GetDesc( ) const
        {
            return m_desc;
        }

        [[nodiscard]] const D3D12_RESOURCE_DESC &GetResourceDesc( ) const
        {
            return m_resourceDesc;
        }

        [[nodiscard]] const D3D12_ROOT_PARAMETER_TYPE &GetRootParameterType( ) const
        {
            return m_rootParameterType;
        }

        [[nodiscard]] ID3D12Resource *GetResource( ) const
        {
            return m_resource;
        }

        [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE &GetCpuHandle( ) const
        {
            return m_cpuHandle;
        }

    private:
        void CreateTextureSrv( ) const;
        void CreateTextureUav( ) const;
    };

    class DX12Sampler final : public ISampler
    {
    private:
        DX12Context                *m_context;
        SamplerDesc                 m_desc;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ };
        D3D12_SAMPLER_DESC          m_samplerDesc{ };

    public:
             DX12Sampler( DX12Context *context, const SamplerDesc &desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle );
        ~    DX12Sampler( ) override = default;

        [[nodiscard]] D3D12_FILTER CalculateFilter( Filter min, Filter mag, MipmapMode mode, CompareOp compareOp, float maxAnisotropy ) const;

        [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE &GetCpuHandle( ) const
        {
            return m_cpuHandle;
        }

        [[nodiscard]] const D3D12_SAMPLER_DESC &GetSamplerDesc( ) const
        {
            return m_samplerDesc;
        }
    };
} // namespace DenOfIz
