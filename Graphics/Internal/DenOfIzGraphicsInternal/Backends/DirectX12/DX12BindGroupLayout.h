/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include "DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"

namespace DenOfIz
{

    struct DX12RootLevelDescriptorRange
    {
        CD3DX12_DESCRIPTOR_RANGE Range;
        D3D12_SHADER_VISIBILITY  Visibility;
    };

    class DX12BindGroupLayout final : public IBindGroupLayout
    {
    private:
        DX12Context                *m_context;
        DenOfIz_BindGroupLayoutDesc m_desc;

        std::vector<DX12RootLevelDescriptorRange> m_rootLevelRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE>     m_cbvSrvUavRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE>     m_samplerRanges;

        std::unordered_map<uint32_t, uint32_t> m_resourceOffsetMap;
        std::unordered_map<uint32_t, uint32_t> m_numBytesMap;
        uint32_t                               m_rootLevelBufferCount = 0;
        uint32_t                               m_cbvSrvUavCount       = 0;
        uint32_t                               m_samplerCount         = 0;

        std::unordered_set<D3D12_SHADER_VISIBILITY> m_cbvSrvUavVisibilities;
        std::unordered_set<D3D12_SHADER_VISIBILITY> m_samplerVisibilities;
        uint32_t                                    m_usedStages = 0;

    public:
        DX12BindGroupLayout( DX12Context *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~DX12BindGroupLayout( ) override;

        [[nodiscard]] const DenOfIz_BindGroupLayoutDesc &Desc( ) const;

        [[nodiscard]] uint32_t GetResourceOffset( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t GetNumBytes( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t RegisterSpace( ) const;
        [[nodiscard]] uint32_t CbvSrvUavCount( ) const;
        [[nodiscard]] uint32_t SamplerCount( ) const;
        [[nodiscard]] uint32_t RootLevelBufferCount( ) const;
        [[nodiscard]] uint32_t UsedStages( ) const;

        [[nodiscard]] const std::vector<DX12RootLevelDescriptorRange> &RootLevelRanges( ) const;
        [[nodiscard]] const std::vector<CD3DX12_DESCRIPTOR_RANGE>     &CbvSrvUavRanges( ) const;
        [[nodiscard]] const std::vector<CD3DX12_DESCRIPTOR_RANGE>     &SamplerRanges( ) const;

        [[nodiscard]] D3D12_SHADER_VISIBILITY CbvSrvUavVisibility( ) const;
        [[nodiscard]] D3D12_SHADER_VISIBILITY SamplerVisibility( ) const;

    private:
        void AddResourceBinding( const DenOfIz_BindingDesc &binding );
    };

} // namespace DenOfIz
