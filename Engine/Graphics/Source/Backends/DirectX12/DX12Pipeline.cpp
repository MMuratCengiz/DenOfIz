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
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};

	assertm(m_createInfo.RootSignature != nullptr, "Root signature is not set for the pipeline");
	assertm(m_createInfo.InputLayout != nullptr, "Input layout is not set for the pipeline");

	CreateGraphicsPipeline();
}

void DX12Pipeline::CreateGraphicsPipeline()
{
	m_topology = {};
	m_topology = DX12EnumConverter::ConvertPrimitiveTopology(m_createInfo.PrimitiveTopology);
	m_rootSignature = reinterpret_cast<DX12RootSignature*>(m_createInfo.RootSignature);
	DX12InputLayout* inputLayout = reinterpret_cast<DX12InputLayout*>(m_createInfo.InputLayout);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayout->GetInputLayout();
	psoDesc.pRootSignature = m_rootSignature->GetRootSignature();
	SetGraphicsShaders(psoDesc);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = DX12EnumConverter::ConvertCullMode(m_createInfo.CullMode);

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	InitDepthStencil(psoDesc);

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = DX12EnumConverter::ConvertPrimitiveTopologyToType(m_createInfo.PrimitiveTopology);

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	SetMSAASampleCount(m_createInfo, psoDesc);
	DX_CHECK_RESULT(m_context->D3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_graphicsPipeline)));
}

void DX12Pipeline::InitDepthStencil(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const
{
	psoDesc.DepthStencilState.DepthEnable = m_createInfo.DepthTest.Enable;
	psoDesc.DepthStencilState.DepthFunc = DX12EnumConverter::ConvertCompareOp(m_createInfo.DepthTest.CompareOp);
	psoDesc.DepthStencilState.DepthWriteMask = m_createInfo.DepthTest.Write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

	psoDesc.DepthStencilState.StencilEnable = m_createInfo.StencilTest.Enable;
	psoDesc.DepthStencilState.StencilReadMask = m_createInfo.StencilTest.ReadMask;
	psoDesc.DepthStencilState.StencilWriteMask = m_createInfo.StencilTest.WriteMask;

	InitStencilFace(psoDesc.DepthStencilState.FrontFace, m_createInfo.StencilTest.FrontFace);
	InitStencilFace(psoDesc.DepthStencilState.BackFace, m_createInfo.StencilTest.BackFace);
}

void DX12Pipeline::InitStencilFace(D3D12_DEPTH_STENCILOP_DESC& stencilFace, const StencilFace& face) const
{
	stencilFace.StencilDepthFailOp = DX12EnumConverter::ConvertStencilOp(face.FailOp);
	stencilFace.StencilFunc = DX12EnumConverter::ConvertCompareOp(face.CompareOp);
	stencilFace.StencilFailOp = DX12EnumConverter::ConvertStencilOp(face.FailOp);
	stencilFace.StencilPassOp = DX12EnumConverter::ConvertStencilOp(face.PassOp);
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

void DX12Pipeline::SetGraphicsShaders(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	for (const CompiledShader& compiledShader: m_createInfo.ShaderProgram.GetCompiledShaders())
	{
		switch (compiledShader.Stage) {
		case ShaderStage::Vertex:
			psoDesc.VS = GetShaderByteCode(compiledShader);
			break;
		case ShaderStage::Hull:
			psoDesc.HS = GetShaderByteCode(compiledShader);
			break;
		case ShaderStage::Domain:
			psoDesc.DS = GetShaderByteCode(compiledShader);
			break;
		case ShaderStage::Geometry:
			psoDesc.GS = GetShaderByteCode(compiledShader);
			break;
		case ShaderStage::Fragment:
			psoDesc.PS = GetShaderByteCode(compiledShader);
			break;
		default:
			break;
		}
	}
}

D3D12_SHADER_BYTECODE DX12Pipeline::GetShaderByteCode(const CompiledShader& compiledShader) const
{
	return D3D12_SHADER_BYTECODE(compiledShader.Data.data(), compiledShader.Data.size());
}

DX12Pipeline::~DX12Pipeline()
{
	m_graphicsPipeline.Reset();
}
