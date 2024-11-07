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

#include <DenOfIzGraphics/Backends/DirectX12/DX12EnumConverter.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderRecordLayout.h>
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>

using namespace DenOfIz;

DX12ShaderRecordLayout::DX12ShaderRecordLayout( DX12Context *context, const ShaderRecordLayoutDesc &desc ) : m_context( context ), m_desc( desc )
{
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters( desc.Bindings.NumElements( ) );

    uint32_t registerSpace = 0;
    switch ( desc.Stage )
    {
    case ShaderStage::Raygen:
        registerSpace = DZConfiguration::Instance( ).RaygenDataRegisterSpace;
        break;
    case ShaderStage::Miss:
        registerSpace = DZConfiguration::Instance( ).MissDataRegisterSpace;
        break;
    case ShaderStage::AnyHit:
    case ShaderStage::ClosestHit:
    case ShaderStage::Intersection:
        registerSpace = DZConfiguration::Instance( ).HitGroupDataRegisterSpace;
    default:
        LOG( ERROR ) << "Invalid shader stage, IShaderRecordLayout can only be created for ray tracing shaders";
        break;
    }

    std::vector<D3D12_DESCRIPTOR_RANGE1> samplerRanges;
    for ( uint32_t i = 0; i < desc.Bindings.NumElements( ); ++i )
    {
        const auto &binding = desc.Bindings.GetElement( i );

        D3D12_ROOT_PARAMETER1 &rootParameter = rootParameters[ i ];
        bool                   isDescriptor  = false;
        switch ( binding.Type )
        {
        case DescriptorBufferBindingType::ConstantBuffer:
            rootParameter.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParameter.Constants.RegisterSpace  = registerSpace;
            rootParameter.Constants.ShaderRegister = binding.Binding;
            rootParameter.Constants.Num32BitValues = binding.NumBytes / 4;
            m_shaderRecordNumBytes += binding.NumBytes;
            ContainerUtilities::SafeSet( m_bindingIndices[ CBV_INDEX ], binding.Binding, i );
            ContainerUtilities::SafeSet( m_cbvNumBytes, binding.Binding, binding.NumBytes );
            break;
        case DescriptorBufferBindingType::ShaderResource:
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            isDescriptor                = true;
            ContainerUtilities::SafeSet( m_bindingIndices[ SRV_INDEX ], binding.Binding, i );
            break;
        case DescriptorBufferBindingType::UnorderedAccess:
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            isDescriptor                = true;
            ContainerUtilities::SafeSet( m_bindingIndices[ UAV_INDEX ], binding.Binding, i );
            break;
        case DescriptorBufferBindingType::Sampler:
            m_samplerTableIndex = i;
            samplerRanges.push_back( { .RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                                       .NumDescriptors                    = 1,
                                       .BaseShaderRegister                = binding.Binding,
                                       .RegisterSpace                     = registerSpace,
                                       .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } );
            break;
        }

        if ( isDescriptor )
        {
            rootParameter.Descriptor.RegisterSpace  = registerSpace;
            rootParameter.Descriptor.ShaderRegister = binding.Binding;
            m_shaderRecordNumBytes += sizeof( D3D12_GPU_DESCRIPTOR_HANDLE );
        }

        rootParameter.ShaderVisibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility( desc.Stage );
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC localSigDesc = { };
    localSigDesc.Version                             = D3D_ROOT_SIGNATURE_VERSION_1_1;
    localSigDesc.Desc_1_1.Flags                      = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    localSigDesc.Desc_1_1.NumParameters              = rootParameters.size( );
    localSigDesc.Desc_1_1.pParameters                = rootParameters.data( );

    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> errorBlob;
    D3D12SerializeVersionedRootSignature( &localSigDesc, &serializedRootSig, &errorBlob );

    ComPtr<ID3D12RootSignature> hitGroupRootSignature;
    m_context->D3DDevice->CreateRootSignature( 0, serializedRootSig->GetBufferPointer( ), serializedRootSig->GetBufferSize( ), IID_PPV_ARGS( &hitGroupRootSignature ) );
}

uint32_t DX12ShaderRecordLayout::CbvIndex( uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ CBV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for CBV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ CBV_INDEX ][ bindingIndex ];
}

size_t DX12ShaderRecordLayout::CbvNumBytes( uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_cbvNumBytes.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for CBV(" << bindingIndex << ")";
    }
    return m_cbvNumBytes[ bindingIndex ];
}

uint32_t DX12ShaderRecordLayout::SrvIndex( uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ SRV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for SRV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ SRV_INDEX ][ bindingIndex ];
}

uint32_t DX12ShaderRecordLayout::UavIndex( uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ UAV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for UAV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ UAV_INDEX ][ bindingIndex ];
}

uint32_t DX12ShaderRecordLayout::SamplerIndex( ) const
{
    return m_samplerTableIndex;
}
