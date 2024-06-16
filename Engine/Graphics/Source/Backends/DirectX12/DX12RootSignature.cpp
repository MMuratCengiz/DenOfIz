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

DX12RootSignature::DX12RootSignature(DX12Context *context, const RootSignatureCreateInfo &createInfo) : m_context(context), m_createInfo(createInfo)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    m_rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if ( FAILED(context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))) )
    {
        m_rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

void DX12RootSignature::AddResourceBindingInternal(const ResourceBinding &binding)
{
    CD3DX12_DESCRIPTOR_RANGE descriptorRange = {};
    descriptorRange.Init(DX12EnumConverter::ConvertBindingTypeToDescriptorRangeType(binding.Type), binding.ArraySize, binding.Binding, binding.RegisterSpace);

    m_descriptorRanges.push_back(descriptorRange);
    for ( const auto &stage : binding.Stages )
    {
        m_descriptorRangesShaderVisibilities.insert(DX12EnumConverter::ConvertShaderStageToShaderVisibility(stage));
    }
}

void DX12RootSignature::AddRootConstantInternal(const RootConstantBinding &rootConstant)
{
    CD3DX12_ROOT_PARAMETER dxRootConstant = {};
    dxRootConstant.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    if ( rootConstant.Stages.size() > 1 || rootConstant.Stages.empty() )
    {
        dxRootConstant.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }
    else
    {
        dxRootConstant.ShaderVisibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility(rootConstant.Stages[ 0 ]);
    }
    dxRootConstant.Constants.Num32BitValues = rootConstant.Size / sizeof(uint32_t);
    dxRootConstant.Constants.ShaderRegister = rootConstant.Binding;
    dxRootConstant.Constants.RegisterSpace = rootConstant.RegisterSpace;
    m_rootConstants.push_back(dxRootConstant);
}

void DX12RootSignature::CreateInternal()
{
    D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    if ( m_descriptorRangesShaderVisibilities.size() == 1 )
    {
        shaderVisibility = *m_descriptorRangesShaderVisibilities.begin();
    }

    std::copy(m_rootConstants.begin(), m_rootConstants.end(), std::back_inserter(m_rootParameters));

    CD3DX12_ROOT_PARAMETER descriptors = {};
    descriptors.InitAsDescriptorTable(m_descriptorRanges.size(), m_descriptorRanges.data(), shaderVisibility);
    m_rootParameters.push_back(descriptors);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc(static_cast<uint32_t>(m_rootParameters.size()), m_rootParameters.data());
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    DX_CHECK_RESULT(D3D12SerializeRootSignature(&rootSignatureDesc, m_rootSignatureVersion, &signature, &error));
    DX_CHECK_RESULT(m_context->D3DDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

DX12RootSignature::~DX12RootSignature() { m_rootSignature.Reset(); }
