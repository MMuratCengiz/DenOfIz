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

#include <unordered_set>
#include "DX12Context.h"
#include "DenOfIzGraphics/Backends/Interface/IRootSignature.h"

namespace DenOfIz
{

    struct RootLevelDescriptorRange
    {
        CD3DX12_DESCRIPTOR_RANGE Range;
        D3D12_SHADER_VISIBILITY  Visibility;
    };

    /*
     * Given a register space, this structure holds the root level descriptor ranges, cbv/srv/uav ranges and sampler ranges.
     */
    struct RegisterSpaceRangesDesc
    {
        // -1 is here to denote that the space is not set, i.e. nothing is bound to this space.
        int                                   Space = -1;
        std::vector<RootLevelDescriptorRange> RootLevelRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> CbvSrvUavRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> SamplerRanges;
    };

    class DX12RootSignature final : public IRootSignature
    {
        struct RegisterSpaceOrder
        {
            uint32_t                               Space{ };
            uint32_t                               RootLevelBufferCount{ };
            uint32_t                               ResourceCount{ };
            uint32_t                               SamplerCount{ };
            std::unordered_map<uint32_t, uint32_t> ResourceOffsetMap;
        };

        DX12Context                      *m_context;
        RootSignatureDesc                 m_desc;
        wil::com_ptr<ID3D12RootSignature> m_rootSignature;

        std::vector<RegisterSpaceOrder>        m_registerSpaceOrder;
        std::vector<CD3DX12_ROOT_PARAMETER>    m_rootParameters;
        std::vector<CD3DX12_ROOT_PARAMETER>    m_rootConstants;
        std::vector<RegisterSpaceRangesDesc>   m_registerSpaceRanges;
        std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplerDescriptorRanges;

        std::unordered_set<D3D12_SHADER_VISIBILITY> m_descriptorRangesShaderVisibilities;
        std::unordered_set<D3D12_SHADER_VISIBILITY> m_samplerRangesShaderVisibilities;
        uint32_t                                    m_usedStages = 0;
        std::vector<uint32_t>                       m_registerSpaceOffsets;

        D3D12_SHADER_VISIBILITY m_cbvSrvUavVisibility = D3D12_SHADER_VISIBILITY_ALL;
        D3D12_SHADER_VISIBILITY m_samplerVisibility   = D3D12_SHADER_VISIBILITY_ALL;

    public:
        DX12RootSignature( DX12Context *context, const RootSignatureDesc &desc );
        // Properties: --
        [[nodiscard]] uint32_t                                   GetResourceOffset( const ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t                                   RegisterSpaceOffset( uint32_t registerSpace ) const;
        [[nodiscard]] ID3D12RootSignature                       *Instance( ) const;
        [[nodiscard]] const std::vector<CD3DX12_ROOT_PARAMETER> &RootParameters( ) const;
        [[nodiscard]] const std::vector<CD3DX12_ROOT_PARAMETER> &RootConstants( ) const;
        // --
        ~DX12RootSignature( ) override;

    private:
        void                                     ProcessRegisterSpaceRange( const RegisterSpaceRangesDesc &range );
        void                                     AddStaticSampler( const StaticSamplerDesc &desc );
        void                                     AddResourceBinding( const ResourceBindingDesc &binding );
        void                                     AddRootConstant( const RootConstantResourceBindingDesc &rootConstant );
        void                                     AddBindlessResource( const BindlessResourceDesc &bindlessResource );
        [[nodiscard]] D3D12_ROOT_SIGNATURE_FLAGS ComputeShaderVisibility( ) const;
    };

} // namespace DenOfIz
