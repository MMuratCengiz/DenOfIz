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

#include <DenOfIzGraphicsInternal/Backends/Interface/IBindGroup.h>
#include <algorithm>
#include "DX12BindGroupLayout.h"
#include "DX12Buffer.h"
#include "DX12Context.h"
#include "DX12Texture.h"

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

    class DX12BindGroup final : public IBindGroup
    {
        DX12Context                    *m_context;
        DenOfIz_BindGroupDesc           m_desc;
        uint32_t                        m_samplerCount   = 0;
        uint32_t                        m_cbvSrvUavCount = 0;
        DescriptorHandle                m_cbvSrvUavHandle;
        DescriptorHandle                m_samplerHandle;
        DX12BindGroupLayout            *m_bindGroupLayout;
        std::vector<DX12RootDescriptor> m_rootDescriptors;

    public:
        DX12BindGroup( DX12Context *context, const DenOfIz_BindGroupDesc &desc );
        ~DX12BindGroup( ) override;
        // Properties: --
        [[nodiscard]] DescriptorHandle                       CbvSrvUavHandle( ) const;
        [[nodiscard]] DescriptorHandle                       SamplerHandle( ) const;
        [[nodiscard]] uint32_t                               CbvSrvUavCount( ) const;
        [[nodiscard]] uint32_t                               SamplerCount( ) const;
        [[nodiscard]] DX12BindGroupLayout                   *BindGroupLayout( ) const;
        [[nodiscard]] const std::vector<DX12RootDescriptor> &RootDescriptors( ) const;
        [[nodiscard]] uint32_t                               RegisterSpace( ) const;
        // --
        IBindGroup *BeginUpdate( ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources ) override;
        IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) override;
        IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Uav( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void        EndUpdate( ) override;

    private:
        void BindTexture( const DenOfIz_ResourceBindingSlot &slot, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS );
        void BindBuffer( const DenOfIz_ResourceBindingSlot &slot, IBuffer *resource );
        void BindSampler( const DenOfIz_ResourceBindingSlot &slot, ISampler *sampler );

        bool                                      UpdateRootDescriptor( const DenOfIz_ResourceBindingSlot &slot, const D3D12_GPU_VIRTUAL_ADDRESS &gpuAddress );
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleCbvSrvUav( uint32_t binding ) const;
        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CpuHandleSampler( uint32_t binding ) const;
        DenOfIz_ResourceBindingSlot               GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const;
    };

} // namespace DenOfIz
