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

#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <algorithm>
#include "DX12BufferResource.h"
#include "DX12Context.h"
#include "DX12RootSignature.h"
#include "DX12TextureResource.h"

namespace DenOfIz
{
    // For DirectX12 this is kind of a dummy class as resources are bound to heaps. At a given point we only use 2 heaps one for CBV/SRV/UAV and one for Sampler.
    class DX12ResourceBindGroup final : public IResourceBindGroup
    {
    private:
        DX12Context       *m_context;
        uint32_t           m_samplerCount   = 0;
        uint32_t           m_cbvSrvUavCount = 0;
        uint32_t           m_offset         = 0;
        DescriptorHandle   m_cbvSrvUavHandle;
        DescriptorHandle   m_samplerHandle;
        DX12RootSignature *m_dx12RootSignature;

    public:
        DX12ResourceBindGroup( DX12Context *context, const ResourceBindGroupDesc &desc );

        [[nodiscard]] DescriptorHandle GetCbvSrvUavHandle( ) const
        {
            return m_cbvSrvUavHandle;
        }

        [[nodiscard]] DescriptorHandle GetSamplerHandle( ) const
        {
            return m_samplerHandle;
        }

        [[nodiscard]] uint32_t GetCbvSrvUavCount( ) const
        {
            return m_cbvSrvUavCount;
        }

        [[nodiscard]] uint32_t GetSamplerCount( ) const
        {
            return m_samplerCount;
        }

        [[nodiscard]] DX12RootSignature *RootSignature( ) const
        {
            return m_dx12RootSignature;
        }

        void Update( const UpdateDesc &desc ) override;

    protected:
        void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) override;
        void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource ) override;
        void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler ) override;

    private:
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleCbvSrvUav( uint32_t binding ) const;
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleSampler( uint32_t binding ) const;
    };

} // namespace DenOfIz
