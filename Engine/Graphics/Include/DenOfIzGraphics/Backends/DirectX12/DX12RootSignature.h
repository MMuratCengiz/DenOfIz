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

#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <unordered_set>
#include "DX12Context.h"
#include "DX12EnumConverter.h"

namespace DenOfIz
{
    struct MultiRangeDesc
    {
        CD3DX12_DESCRIPTOR_RANGE Resource;
        // Samplers must be bound to separate tables
        CD3DX12_DESCRIPTOR_RANGE Sampler;
    };

    struct RegisterSpaceRangesDesc
    {
        int                                   Space;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> CbvSrvUavRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> SamplerRanges;
    };

    class DX12RootSignature : public IRootSignature
    {
    private:
        D3D_ROOT_SIGNATURE_VERSION        m_rootSignatureVersion;
        DX12Context                      *m_context;
        RootSignatureDesc                 m_desc;
        wil::com_ptr<ID3D12RootSignature> m_rootSignature;

        std::vector<CD3DX12_ROOT_PARAMETER>                m_rootParameters;
        std::vector<CD3DX12_ROOT_PARAMETER>                m_rootConstants;
        std::vector<RegisterSpaceRangesDesc>               m_registerSpaceRanges;
        std::vector<D3D12_STATIC_SAMPLER_DESC>             m_staticSamplerDescriptorRanges;

        std::unordered_set<D3D12_SHADER_VISIBILITY> m_descriptorRangesShaderVisibilities;
        std::unordered_set<D3D12_SHADER_VISIBILITY> m_samplerRangesShaderVisibilities;
        uint32_t                                    m_usedStages = 0;
        std::vector<uint32_t>                       m_registerSpaceOffsets;

    public:
        DX12RootSignature(DX12Context *context, const RootSignatureDesc &desc);

        inline uint32_t RegisterSpaceOffset(uint32_t registerSpace) const
        {
            if ( registerSpace >= m_registerSpaceOffsets.size() )
            {
                LOG(ERROR) << "Register space " << registerSpace << " is not bound to any bind group.";
            }

            return m_registerSpaceOffsets[ registerSpace ];
        }

        inline ID3D12RootSignature *Instance() const
        {
            return m_rootSignature.get();
        }

        inline size_t RootParameterCount() const
        {
            return m_rootParameters.size();
        }

        ~DX12RootSignature() override;

    protected:
        void                       AddStaticSampler(const StaticSamplerDesc &desc);
        void                       AddResourceBindingInternal(const ResourceBindingDesc &binding) override;
        void                       AddRootConstantInternal(const RootConstantResourceBinding &rootConstant) override;
        D3D12_ROOT_SIGNATURE_FLAGS ComputeShaderVisibility() const;
    };

} // namespace DenOfIz
