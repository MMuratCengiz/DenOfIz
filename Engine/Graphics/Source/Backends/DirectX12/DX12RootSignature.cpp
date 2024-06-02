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

#include <DenOfIzGraphics/Backends/DirectX12/DX12RootSignature.h>

using namespace DenOfIz;

DX12RootSignature::DX12RootSignature(DX12Context* context, const RootSignatureCreateInfo& createInfo)
		:m_context(context), m_createInfo(createInfo)
{
}

void DX12RootSignature::AddResourceBindingInternal(const ResourceBinding& binding)
{
	D3D12_DESCRIPTOR_RANGE descriptorRange = {};

	descriptorRange.NumDescriptors = binding.ArraySize;
	descriptorRange.BaseShaderRegister = binding.Binding;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter.DescriptorTable.NumDescriptorRanges = 1;
	rootParameter.DescriptorTable.pDescriptorRanges = nullptr;
	m_rootParameters.push_back(rootParameter);
}

void DX12RootSignature::AddRootConstantInternal(const RootConstantBinding& rootConstant)
{
	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter.Constants.Num32BitValues = rootConstant.Num32BitValues;
	rootParameter.Constants.RegisterSpace = rootConstant.RegisterSpace;
	rootParameter.Constants.ShaderRegister = rootConstant.ShaderRegister;
	m_rootParameters.push_back(rootParameter);
}

void DX12RootSignature::CreateInternal()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	DX_CHECK_RESULT(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	DX_CHECK_RESULT(m_context->D3DDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

DX12RootSignature::~DX12RootSignature()
{
	m_rootSignature.Reset();
}
