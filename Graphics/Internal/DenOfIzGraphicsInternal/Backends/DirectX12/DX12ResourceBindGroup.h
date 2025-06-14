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
    struct DX12RootConstant
    {
        uint32_t Binding{ };
        void    *Data{ };
        uint32_t NumBytes{ };
    };

    struct DX12RootDescriptor
    {
        uint32_t                  RootParameterIndex;
        D3D12_ROOT_PARAMETER_TYPE ParameterType;
        D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
    };

    // For DirectX12 this is kind of a dummy class as resources are bound to heaps. At a given point we only use 2 heaps one for CBV/SRV/UAV and one for Sampler.
    class DX12ResourceBindGroup final : public IResourceBindGroup
    {
        DX12Context                    *m_context;
        ResourceBindGroupDesc           m_desc;
        uint32_t                        m_samplerCount   = 0;
        uint32_t                        m_cbvSrvUavCount = 0;
        DescriptorHandle                m_cbvSrvUavHandle;
        DescriptorHandle                m_samplerHandle;
        DX12RootSignature              *m_dx12RootSignature;
        std::vector<DX12RootConstant>   m_rootConstants;
        std::vector<DX12RootDescriptor> m_rootDescriptors;

    public:
        DX12ResourceBindGroup( DX12Context *context, const ResourceBindGroupDesc &desc );
        // Properties: --
        [[nodiscard]] DescriptorHandle                       CbvSrvUavHandle( ) const;
        [[nodiscard]] DescriptorHandle                       SamplerHandle( ) const;
        [[nodiscard]] uint32_t                               CbvSrvUavCount( ) const;
        [[nodiscard]] uint32_t                               SamplerCount( ) const;
        [[nodiscard]] DX12RootSignature                     *RootSignature( ) const;
        [[nodiscard]] const std::vector<DX12RootDescriptor> &RootDescriptors( ) const;
        [[nodiscard]] const std::vector<DX12RootConstant>   &RootConstants( ) const;
        [[nodiscard]] uint32_t                               RegisterSpace( ) const;
        // --
        void                SetRootConstantsData( uint32_t binding, const ByteArrayView &data ) override;
        void                SetRootConstants( uint32_t binding, void *data ) override;
        IResourceBindGroup *BeginUpdate( ) override;
        IResourceBindGroup *Cbv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Cbv( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Srv( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *SrvArray( const uint32_t binding, const TextureResourceArray &resources ) override;
        IResourceBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITextureResource *resource ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IResourceBindGroup *Uav( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Uav( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Uav( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void                EndUpdate( ) override;

    private:
        void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource );
        void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource );
        void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler );

        bool                                      UpdateRootDescriptor( const ResourceBindingSlot &slot, const D3D12_GPU_VIRTUAL_ADDRESS &gpuAddress );
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleCbvSrvUav( uint32_t binding ) const;
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleSampler( uint32_t binding ) const;
        ResourceBindingSlot                       GetSlot( uint32_t binding, const ResourceBindingType &type ) const;
    };

} // namespace DenOfIz
