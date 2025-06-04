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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12LocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"

using namespace DenOfIz;

DX12LocalRootSignature::DX12LocalRootSignature( DX12Context *context, const LocalRootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters( desc.ResourceBindings.NumElements( ) );

    std::vector<D3D12_DESCRIPTOR_RANGE1> samplerRanges;
    for ( uint32_t i = 0; i < desc.ResourceBindings.NumElements( ); ++i )
    {
        const auto &binding = desc.ResourceBindings.GetElement( i );

        D3D12_ROOT_PARAMETER1 &rootParameter = rootParameters[ i ];
        bool                   isDescriptor  = false;
        switch ( binding.BindingType )
        {
        case ResourceBindingType::ConstantBuffer:
            rootParameter.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParameter.Constants.RegisterSpace  = binding.RegisterSpace;
            rootParameter.Constants.ShaderRegister = binding.Binding;
            rootParameter.Constants.Num32BitValues = binding.Reflection.NumBytes / 4;
            m_shaderRecordDataNumBytes += binding.Reflection.NumBytes;
            ContainerUtilities::SafeSet( m_bindingIndices[ CBV_INDEX ], binding.Binding, i );
            ContainerUtilities::SafeSet( m_cbvNumBytes, binding.Binding, binding.Reflection.NumBytes );
            break;
        case ResourceBindingType::ShaderResource:
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            isDescriptor                = true;
            ContainerUtilities::SafeSet( m_bindingIndices[ SRV_INDEX ], binding.Binding, i );
            break;
        case ResourceBindingType::UnorderedAccess:
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            isDescriptor                = true;
            ContainerUtilities::SafeSet( m_bindingIndices[ UAV_INDEX ], binding.Binding, i );
            break;
        case ResourceBindingType::Sampler:
            m_samplerTableIndex = i;
            samplerRanges.push_back( { .RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                                       .NumDescriptors                    = 1,
                                       .BaseShaderRegister                = binding.Binding,
                                       .RegisterSpace                     = binding.RegisterSpace,
                                       .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND } );
            break;
        }

        if ( isDescriptor )
        {
            rootParameter.Descriptor.RegisterSpace  = binding.RegisterSpace;
            rootParameter.Descriptor.ShaderRegister = binding.Binding;
            m_shaderRecordDataNumBytes += sizeof( D3D12_GPU_DESCRIPTOR_HANDLE );
        }
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC localSigDesc = { };
    localSigDesc.Version                             = D3D_ROOT_SIGNATURE_VERSION_1_1;
    localSigDesc.Desc_1_1.Flags                      = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    localSigDesc.Desc_1_1.NumParameters              = rootParameters.size( );
    localSigDesc.Desc_1_1.pParameters                = rootParameters.data( );

    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> errorBlob;
    DX_CHECK_RESULT( D3D12SerializeVersionedRootSignature( &localSigDesc, &serializedRootSig, &errorBlob ) );
    if ( errorBlob )
    {
        LOG( ERROR ) << "Failed to serialize local root signature: " << errorBlob->GetBufferPointer( );
        return;
    }
    DX_CHECK_RESULT(
        m_context->D3DDevice->CreateRootSignature( 0, serializedRootSig->GetBufferPointer( ), serializedRootSig->GetBufferSize( ), IID_PPV_ARGS( &m_rootSignature ) ) );
}

uint32_t DX12LocalRootSignature::CbvIndex( const uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ CBV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for CBV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ CBV_INDEX ][ bindingIndex ];
}

uint32_t DX12LocalRootSignature::CbvOffset( uint32_t bindingIndex ) const
{
    uint32_t offset = 0;
    for ( uint32_t i = 0; i < bindingIndex; i++ )
    {
        offset += m_cbvNumBytes[ i ];
    }
    return offset;
}

size_t DX12LocalRootSignature::CbvNumBytes( const uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_cbvNumBytes.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for CBV(" << bindingIndex << ")";
    }
    return m_cbvNumBytes[ bindingIndex ];
}

uint32_t DX12LocalRootSignature::SrvIndex( const uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ SRV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for SRV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ SRV_INDEX ][ bindingIndex ];
}

uint32_t DX12LocalRootSignature::UavIndex( const uint32_t bindingIndex ) const
{
    if ( bindingIndex >= m_bindingIndices[ UAV_INDEX ].size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for UAV(" << bindingIndex << ")";
    }
    return m_bindingIndices[ UAV_INDEX ][ bindingIndex ];
}

bool DX12LocalRootSignature::HasBinding( const ResourceBindingType type, const uint32_t bindingIndex ) const
{
    for ( int i = 0; i < m_desc.ResourceBindings.NumElements( ); ++i )
    {
        if ( const auto &binding = m_desc.ResourceBindings.GetElement( i ); binding.BindingType == type && binding.Binding == bindingIndex )
        {
            return true;
        }
    }
    return false;
}

ID3D12RootSignature *DX12LocalRootSignature::RootSignature( ) const
{
    return m_rootSignature.get( );
}

uint32_t DX12LocalRootSignature::SamplerIndex( ) const
{
    return m_samplerTableIndex;
}

uint32_t DX12LocalRootSignature::LocalDataNumBytes( ) const
{
    return m_shaderRecordDataNumBytes;
}
