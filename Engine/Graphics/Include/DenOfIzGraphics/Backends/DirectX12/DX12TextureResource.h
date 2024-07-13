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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>

namespace DenOfIz
{

    class DX12TextureResource : public ITextureResource
    {
    private:
        TextureDesc                 m_desc;
        DX12Context                *m_context;
        D3D12MA::Allocation        *m_allocation;
        ID3D12Resource2            *m_resource;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
        D3D12_RESOURCE_DESC         m_resourceDesc;
        D3D12_ROOT_PARAMETER_TYPE   m_rootParameterType;
        bool                        isExternalResource = false; // Used for swap chain render targets, might need a better way

    public:
        DX12TextureResource(DX12Context *context, const TextureDesc &desc);
        DX12TextureResource(ID3D12Resource2 *resource, const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle);
        ~DX12TextureResource() override = default;

        void Deallocate() override;

        const TextureDesc &GetDesc() const
        {
            return m_desc;
        }

        const D3D12_RESOURCE_DESC &GetResourceDesc() const
        {
            return m_resourceDesc;
        }

        const D3D12_ROOT_PARAMETER_TYPE &GetRootParameterType() const
        {
            return m_rootParameterType;
        }

        ID3D12Resource *GetResource() const
        {
            return m_resource;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE &GetCpuHandle() const
        {
            return m_cpuHandle;
        };

    protected:
        void Allocate(const void *data) override;

    private:
        void         Validate();
        void         CreateTextureSrv();
        void         CreateTextureUav();
    };

    class DX12Sampler : public ISampler
    {
    private:
        DX12Context                *m_context;
        SamplerDesc                 m_desc;
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
        D3D12_SAMPLER_DESC          m_samplerDesc;
    public:
        DX12Sampler(DX12Context *context, const SamplerDesc &desc);
        ~DX12Sampler() override = default;

        D3D12_FILTER CalculateFilter(Filter min, Filter mag, MipmapMode mode, CompareOp compareOp, float maxAnisotropy) const;


        const D3D12_CPU_DESCRIPTOR_HANDLE &GetCpuHandle() const
        {
            return m_cpuHandle;
        }

        const D3D12_SAMPLER_DESC &GetSamplerDesc() const
        {
            return m_samplerDesc;
        }
    };
} // namespace DenOfIz
