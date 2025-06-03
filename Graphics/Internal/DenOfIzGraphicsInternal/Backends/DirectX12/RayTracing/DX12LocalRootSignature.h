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
#include <DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h>

namespace DenOfIz
{
    class DX12LocalRootSignature final : public ILocalRootSignature
    {
        DX12Context           *m_context;
        LocalRootSignatureDesc m_desc;

        wil::com_ptr<ID3D12RootSignature> m_rootSignature;
        size_t                            m_shaderRecordDataNumBytes = 0;
        int                               m_samplerTableIndex        = -1;

        static constexpr uint32_t NUM_DESCRIPTOR_TYPES = 4;
        static constexpr uint32_t CBV_INDEX            = 0;
        static constexpr uint32_t SRV_INDEX            = 1;
        static constexpr uint32_t UAV_INDEX            = 2;
        static constexpr uint32_t SAMPLER_INDEX        = 3;

        std::array<std::vector<uint32_t>, NUM_DESCRIPTOR_TYPES> m_bindingIndices;
        std::vector<size_t>                                     m_cbvNumBytes;

    public:
        DX12LocalRootSignature( DX12Context *context, const LocalRootSignatureDesc &desc );
        [[nodiscard]] ID3D12RootSignature *RootSignature( ) const;
        [[nodiscard]] uint32_t             CbvIndex( uint32_t bindingIndex ) const;
        [[nodiscard]] uint32_t             CbvOffset( uint32_t bindingIndex ) const;
        [[nodiscard]] size_t               CbvNumBytes( uint32_t bindingIndex ) const;
        [[nodiscard]] uint32_t             SrvIndex( uint32_t bindingIndex ) const;
        [[nodiscard]] uint32_t             UavIndex( uint32_t bindingIndex ) const;
        [[nodiscard]] bool                 HasBinding( ResourceBindingType type, uint32_t bindingIndex ) const;
        [[nodiscard]] uint32_t             SamplerIndex( ) const;
        [[nodiscard]] uint32_t             LocalDataNumBytes( ) const;
        ~DX12LocalRootSignature( ) override = default;
    };
} // namespace DenOfIz
