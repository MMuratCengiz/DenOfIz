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

DX12RootSignature::DX12RootSignature(DX12Context *context, const RootSignatureDesc &desc) : m_context(context), m_desc(desc)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    m_rootSignatureVersion                        = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if ( FAILED(context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))) )
    {
        m_rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    uint32_t index = 0;
    for ( const ResourceBindingDesc &binding : desc.ResourceBindings )
    {
        AddResourceBinding(binding);
    }

    for ( const StaticSamplerDesc &staticSamplerDesc : desc.StaticSamplers )
    {
        AddStaticSampler(staticSamplerDesc);
    }

    D3D12_SHADER_VISIBILITY descShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    if ( m_descriptorRangesShaderVisibilities.size() == 1 )
    {
        descShaderVisibility = *m_descriptorRangesShaderVisibilities.begin();
    }

    D3D12_SHADER_VISIBILITY samplerShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    if ( m_samplerRangesShaderVisibilities.size() == 1 )
    {
        samplerShaderVisibility = *m_samplerRangesShaderVisibilities.begin();
    }

    std::copy(m_rootConstants.begin(), m_rootConstants.end(), std::back_inserter(m_rootParameters));

    for ( const auto &range : m_descriptorRanges )
    {
        CD3DX12_ROOT_PARAMETER descriptors = {};
        descriptors.InitAsDescriptorTable(static_cast<uint32_t>(range.size()), range.data(), descShaderVisibility);
        m_rootParameters.push_back(descriptors);
    }

    for ( const auto &range : m_samplerDescriptorRanges )
    {
        CD3DX12_ROOT_PARAMETER samplers = {};
        samplers.InitAsDescriptorTable(static_cast<uint32_t>(range.size()), range.data(), samplerShaderVisibility);
        m_rootParameters.push_back(samplers);
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc(static_cast<uint32_t>(m_rootParameters.size()), m_rootParameters.data());
    wil::com_ptr<ID3DBlob>    signature;
    wil::com_ptr<ID3DBlob>    error;
    rootSignatureDesc.Flags             = ComputeShaderVisibility();
    rootSignatureDesc.NumStaticSamplers = m_staticSamplerDescriptorRanges.size();
    rootSignatureDesc.pStaticSamplers   = m_staticSamplerDescriptorRanges.data();
    THROW_IF_FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, m_rootSignatureVersion, &signature, &error));
    THROW_IF_FAILED(m_context->D3DDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.put())));
}

D3D12_ROOT_SIGNATURE_FLAGS DX12RootSignature::ComputeShaderVisibility() const
{
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if ( !(m_usedStages & D3D12_SHADER_VISIBILITY_VERTEX) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    }
    if ( !(m_usedStages & D3D12_SHADER_VISIBILITY_HULL) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    }
    if ( !(m_usedStages & D3D12_SHADER_VISIBILITY_DOMAIN) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    }
    if ( !(m_usedStages & D3D12_SHADER_VISIBILITY_GEOMETRY) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    }
    if ( !(m_usedStages & D3D12_SHADER_VISIBILITY_PIXEL) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    }
    return flags;
}

void DX12RootSignature::AddStaticSampler(const StaticSamplerDesc &staticSamplerDesc)
{
    const SamplerDesc        &samplerDesc = staticSamplerDesc.Sampler;
    D3D12_STATIC_SAMPLER_DESC desc        = {};

    int filter     = (static_cast<int>(samplerDesc.MinFilter) << 4) | (static_cast<int>(samplerDesc.MagFilter) << 2) | static_cast<int>(samplerDesc.MipmapMode);
    int baseFilter = samplerDesc.CompareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    if ( samplerDesc.MaxAnisotropy > 0.0f )
    {
        baseFilter = samplerDesc.CompareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }

    desc.Filter         = static_cast<D3D12_FILTER>(baseFilter + filter);
    desc.AddressU       = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeU);
    desc.AddressV       = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeV);
    desc.AddressW       = DX12EnumConverter::ConvertSamplerAddressMode(samplerDesc.AddressModeW);
    desc.MipLODBias     = samplerDesc.MipLodBias;
    desc.MaxAnisotropy  = samplerDesc.MaxAnisotropy;
    desc.ComparisonFunc = DX12EnumConverter::ConvertCompareOp(samplerDesc.CompareOp);
    desc.MinLOD         = samplerDesc.MinLod;
    desc.MaxLOD         = samplerDesc.MaxLod;

    desc.ShaderRegister   = staticSamplerDesc.Binding.Binding;
    desc.RegisterSpace    = staticSamplerDesc.Binding.RegisterSpace;
    desc.ShaderVisibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility(staticSamplerDesc.Binding.Stages[ 0 ]);

    m_staticSamplerDescriptorRanges.push_back(desc);
}

void DX12RootSignature::AddResourceBindingInternal(const ResourceBindingDesc &binding)
{
    CD3DX12_DESCRIPTOR_RANGE descriptorRange = {};
    descriptorRange.Init(DX12EnumConverter::ConvertResourceDescriptorToDescriptorRangeType(binding.Descriptor), binding.ArraySize, binding.Binding, binding.RegisterSpace);

    if ( binding.Descriptor.IsSet(ResourceDescriptor::Sampler) )
    {
        Utilities::SafeGetInnerVec(m_samplerDescriptorRanges, binding.RegisterSpace).push_back(descriptorRange);
    }
    else
    {
        Utilities::SafeGetInnerVec(m_descriptorRanges, binding.RegisterSpace).push_back(descriptorRange);
    }

    for ( const auto &stage : binding.Stages )
    {
        D3D12_SHADER_VISIBILITY usedStage = DX12EnumConverter::ConvertShaderStageToShaderVisibility(stage);
        if ( binding.Descriptor == ResourceDescriptor::Sampler )
        {
            m_samplerRangesShaderVisibilities.insert(usedStage);
        }
        else
        {
            m_descriptorRangesShaderVisibilities.insert(usedStage);
        }
        m_usedStages |= usedStage;
    }
}

void DX12RootSignature::AddRootConstantInternal(const RootConstantResourceBinding &rootConstant)
{
    CD3DX12_ROOT_PARAMETER dxRootConstant = {};
    dxRootConstant.ParameterType          = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
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
    dxRootConstant.Constants.RegisterSpace  = rootConstant.RegisterSpace;
    m_rootConstants.push_back(dxRootConstant);
}

DX12RootSignature::~DX12RootSignature()
{
}
