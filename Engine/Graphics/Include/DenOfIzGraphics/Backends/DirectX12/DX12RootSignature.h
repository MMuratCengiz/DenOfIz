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

#include <DenOfIzCore/ContainerUtilities.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <unordered_set>
#include "DX12Context.h"
#include "DX12EnumConverter.h"

namespace DenOfIz
{

    struct RegisterSpaceRangesDesc
    {
        int                                   Space;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> CbvSrvUavRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> SamplerRanges;
    };

    class DX12RootSignature final : public IRootSignature
    {
        struct RegisterSpaceOrder
        {
            uint32_t                               Space{ };
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

    public:
        DX12RootSignature( DX12Context *context, const RootSignatureDesc &desc );

        [[nodiscard]] uint32_t GetResourceOffset( const uint32_t registerSpace, const ResourceBindingSlot &slot ) const
        {
            if ( registerSpace >= m_registerSpaceOrder.size( ) )
            {
                LOG( ERROR ) << "Register space " << registerSpace << " is not bound to any bind group.";
            }
            return ContainerUtilities::SafeGetMapValue( m_registerSpaceOrder[ registerSpace ].ResourceOffsetMap, slot.Key( ),
                                                        "Binding slot does not exist in root signature: " + slot.ToString( ) );
        }

        [[nodiscard]] uint32_t RegisterSpaceOffset( const uint32_t registerSpace ) const
        {
            if ( registerSpace >= m_registerSpaceOffsets.size( ) )
            {
                LOG( ERROR ) << "Register space " << registerSpace << " is not bound to any bind group.";
            }

            return m_registerSpaceOffsets[ registerSpace ];
        }

        [[nodiscard]] ID3D12RootSignature *Instance( ) const
        {
            return m_rootSignature.get( );
        }

        [[nodiscard]] const std::vector<CD3DX12_ROOT_PARAMETER> &RootParameters( ) const
        {
            return m_rootParameters;
        }

        ~DX12RootSignature( ) override;

    private:
        void                                     AddStaticSampler( const StaticSamplerDesc &desc );
        void                                     AddResourceBinding( const ResourceBindingDesc &binding );
        void                                     AddRootConstant( const RootConstantResourceBinding &rootConstant );
        [[nodiscard]] D3D12_ROOT_SIGNATURE_FLAGS ComputeShaderVisibility( ) const;
    };

} // namespace DenOfIz
