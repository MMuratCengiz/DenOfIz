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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include "DX12Context.h"
#include "DX12InputLayout.h"
#include "DX12RootSignature.h"

namespace DenOfIz
{

    class DX12Pipeline final : public IPipeline
    {
        DX12Context                                          *m_context      = nullptr;
        wil::com_ptr<ID3D12PipelineState>                     m_pipeline     = nullptr;
        wil::com_ptr<ID3D12StateObject>                       m_rayTracingSO = nullptr;
        wil::com_ptr<ID3D12StateObjectProperties>             m_soProperties = nullptr;
        std::unordered_map<std::string, void *>               m_shaderIdentifiers;
        DX12RootSignature                                    *m_rootSignature = nullptr;
        PipelineDesc                                          m_desc{ };
        D3D12_PRIMITIVE_TOPOLOGY                              m_topology{ };
        std::vector<std::wstring>                             m_exportNames; // Only for lifetime management
        std::unordered_map<std::string, D3D12_HIT_GROUP_DESC> m_hitGroups;

    public:
        DX12Pipeline( DX12Context *context, PipelineDesc desc );
        [[nodiscard]] ID3D12PipelineState     *GetPipeline( ) const;
        [[nodiscard]] ID3D12StateObject       *GetRayTracingSO( ) const;
        [[nodiscard]] void                    *GetShaderIdentifier( const std::string &exportName );
        [[nodiscard]] ID3D12RootSignature     *GetRootSignature( ) const;
        [[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY GetTopology( ) const;
        ~DX12Pipeline( ) override;

    private:
        void                  SetMSAASampleCount( const PipelineDesc &desc, D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const;
        void                  SetGraphicsShaders( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const;
        D3D12_SHADER_BYTECODE GetShaderByteCode( const CompiledShader *const &compiledShader ) const;
        void                  CreateGraphicsPipeline( );
        void                  CreateComputePipeline( );
        void                  CreateRayTracingPipeline( );
        void                  InitStencilFace( D3D12_DEPTH_STENCILOP_DESC &stencilFace, const StencilFace &face ) const;
        void                  InitDepthStencil( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const;
    };

} // namespace DenOfIz
