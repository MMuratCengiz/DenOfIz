/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include "DX12BindGroupLayout.h"
#include "DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"

namespace DenOfIz
{

    class DX12RootSignature final : public IRootSignature
    {
        DX12Context                      *m_context;
        DenOfIz_RootSignatureDesc         m_desc;
        wil::com_ptr<ID3D12RootSignature> m_rootSignature;

        std::vector<DX12BindGroupLayout *>  m_bindGroupLayouts;
        std::vector<CD3DX12_ROOT_PARAMETER> m_rootParameters;
        std::vector<CD3DX12_ROOT_PARAMETER> m_rootConstants;
        std::vector<uint32_t>               m_registerSpaceOffsets;
        uint32_t                            m_usedStages = 0;

    public:
        DX12RootSignature( DX12Context *context, const DenOfIz_RootSignatureDesc &desc );
        [[nodiscard]] uint32_t                                   RegisterSpaceOffset( uint32_t registerSpace ) const;
        [[nodiscard]] ID3D12RootSignature                       *Instance( ) const;
        [[nodiscard]] const std::vector<CD3DX12_ROOT_PARAMETER> &RootParameters( ) const;
        [[nodiscard]] const std::vector<CD3DX12_ROOT_PARAMETER> &RootConstants( ) const;
        [[nodiscard]] const std::vector<DX12BindGroupLayout *>  &BindGroupLayouts( ) const;
        [[nodiscard]] DX12BindGroupLayout                       *GetBindGroupLayout( uint32_t registerSpace ) const;
        ~DX12RootSignature( ) override;

    private:
        void                                     ProcessBindGroupLayout( DX12BindGroupLayout *layout );
        void                                     AddRootConstant( const DenOfIz_RootConstantBindingDesc &rootConstant );
        [[nodiscard]] D3D12_ROOT_SIGNATURE_FLAGS ComputeShaderVisibility( ) const;
    };

} // namespace DenOfIz
