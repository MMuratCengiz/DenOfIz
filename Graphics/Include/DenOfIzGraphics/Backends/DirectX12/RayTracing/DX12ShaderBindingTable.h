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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Pipeline.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h>

namespace DenOfIz
{
    class DX12ShaderBindingTable final : public IShaderBindingTable
    {
        DX12Context           *m_context;
        ShaderBindingTableDesc m_desc;
        DX12Pipeline          *m_pipeline;

        uint32_t                            m_numBufferBytes = 0;
        std::unique_ptr<DX12BufferResource> m_buffer;
        std::unique_ptr<DX12BufferResource> m_stagingBuffer;
        void                               *m_mappedMemory = nullptr;

        D3D12_GPU_VIRTUAL_ADDRESS_RANGE            m_rayGenerationShaderRange{ };
        D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE m_hitGroupShaderRange{ };
        D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE m_missShaderRange{ };

        uint32_t m_missGroupOffset   = 0;
        uint32_t m_hitGroupOffset    = 0;
        uint32_t m_rayGenNumBytes    = 0;
        uint32_t m_missNumBytes = 0;
        uint32_t m_hitGroupNumBytes  = 0;

        ShaderBindingTableDebugData m_debugData;

    public:
        DX12ShaderBindingTable( DX12Context *context, const ShaderBindingTableDesc &desc );
        ~DX12ShaderBindingTable( ) override = default;
        void Resize( const SBTSizeDesc &desc ) override;
        void BindRayGenerationShader( const RayGenerationBindingDesc &desc ) override;
        void BindHitGroup( const HitGroupBindingDesc &desc ) override;
        void BindMissShader( const MissBindingDesc &desc ) override;
        void Build( ) override;

        [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS_RANGE            RayGenerationShaderRecord( ) const;
        [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupShaderRange( ) const;
        [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderRange( ) const;

    private:
        uint32_t AlignRecord( uint32_t size ) const;
    };
} // namespace DenOfIz
