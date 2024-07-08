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

    class DX12Pipeline : public IPipeline
    {
    private:
        DX12Context                      *m_context;
        wil::com_ptr<ID3D12PipelineState> m_graphicsPipeline;
        DX12RootSignature                *m_rootSignature;
        PipelineDesc                      m_desc;
        D3D12_PRIMITIVE_TOPOLOGY          m_topology;

    public:
        DX12Pipeline(DX12Context *context, const PipelineDesc &desc);
        ID3D12PipelineState *GetPipeline() const
        {
            return m_graphicsPipeline.get();
        }
        ID3D12RootSignature *GetRootSignature() const
        {
            return m_rootSignature->GetRootSignature();
        }
        D3D12_PRIMITIVE_TOPOLOGY GetTopology() const
        {
            return m_topology;
        }
        ~DX12Pipeline() override;

    private:
        void                  SetMSAASampleCount(const PipelineDesc &desc, D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc) const;
        void                  SetGraphicsShaders(D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc);
        D3D12_SHADER_BYTECODE GetShaderByteCode(const CompiledShader &compiledShader) const;
        void                  CreateGraphicsPipeline();
        void                  CreateComputePipeline();
        void                  InitStencilFace(D3D12_DEPTH_STENCILOP_DESC &stencilFace, const StencilFace &face) const;
        void                  InitDepthStencil(D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc) const;
    };

} // namespace DenOfIz
