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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Pipeline.h>

using namespace DenOfIz;

DX12Pipeline::DX12Pipeline(DX12Context* context, const PipelineCreateInfo& createInfo)
		:m_context(context), m_createInfo(createInfo)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	SetMSAASampleCount(createInfo, psoDesc);

	DX_CHECK_RESULT(m_context->D3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void DX12Pipeline::SetMSAASampleCount(const PipelineCreateInfo& createInfo, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const
{
	switch (createInfo.MSAASampleCount) {
		case MSAASampleCount::_1:
			psoDesc.SampleDesc.Count = 1;
			break;
		case MSAASampleCount::_2:
			psoDesc.SampleDesc.Count = 2;
			break;
		case MSAASampleCount::_4:
			psoDesc.SampleDesc.Count = 4;
			break;
		case MSAASampleCount::_8:
			psoDesc.SampleDesc.Count = 8;
			break;
		case MSAASampleCount::_16:
			psoDesc.SampleDesc.Count = 16;
			break;
		case MSAASampleCount::_32:
			psoDesc.SampleDesc.Count = 32;
			break;
		case MSAASampleCount::_64:
			psoDesc.SampleDesc.Count = 32;
			break;
		default:
			break;
	}
}

DX12Pipeline::~DX12Pipeline()
{

}