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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12RootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define BIND_GROUP_LAYOUT_IMPL( handle ) static_cast<DX12BindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, handle ) )

DX12RootSignature::DX12RootSignature( DX12Context *context, const DenOfIz_RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData          = { };
    D3D_ROOT_SIGNATURE_VERSION        rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if ( FAILED( context->D3DDevice->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
    {
        rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    uint32_t maxRegisterSpace = 0;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        DX12BindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        maxRegisterSpace            = std::max( maxRegisterSpace, layout->RegisterSpace( ) );
    }

    m_bindGroupLayouts.resize( maxRegisterSpace + 1, nullptr );
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        DX12BindGroupLayout *layout                    = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        m_bindGroupLayouts[ layout->RegisterSpace( ) ] = layout;
    }

    for ( uint32_t i = 0; i < desc.RootConstants.NumElements; ++i )
    {
        AddRootConstant( desc.RootConstants.Elements[ i ] );
    }

    std::ranges::copy( m_rootConstants, std::back_inserter( m_rootParameters ) );

    for ( uint32_t space = 0; space <= maxRegisterSpace; ++space )
    {
        DX12BindGroupLayout *layout = m_bindGroupLayouts[ space ];
        if ( layout != nullptr )
        {
            ContainerUtilities::SafeSet( m_registerSpaceOffsets, space, static_cast<uint32_t>( m_rootParameters.size( ) ) );
            ProcessBindGroupLayout( layout );
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc( static_cast<uint32_t>( m_rootParameters.size( ) ), m_rootParameters.data( ) );
    wil::com_ptr<ID3DBlob>    signature;
    wil::com_ptr<ID3DBlob>    error;
    rootSignatureDesc.Flags             = ComputeShaderVisibility( );
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers   = nullptr;
    DX_CHECK_RESULT( D3D12SerializeRootSignature( &rootSignatureDesc, rootSignatureVersion, &signature, &error ) );
    if ( signature == nullptr )
    {
        spdlog::error( "Failed to serialize root signature: {}", std::string( static_cast<char *>( error->GetBufferPointer( ) ), error->GetBufferSize( ) ) );
        return;
    }
    DX_CHECK_RESULT( m_context->D3DDevice->CreateRootSignature( 0, signature->GetBufferPointer( ), signature->GetBufferSize( ), IID_PPV_ARGS( m_rootSignature.put( ) ) ) );
}

void DX12RootSignature::ProcessBindGroupLayout( DX12BindGroupLayout *layout )
{
    m_usedStages |= layout->UsedStages( );

    if ( !layout->CbvSrvUavRanges( ).empty( ) )
    {
        CD3DX12_ROOT_PARAMETER &rootParameter = m_rootParameters.emplace_back( );
        rootParameter.InitAsDescriptorTable( static_cast<uint32_t>( layout->CbvSrvUavRanges( ).size( ) ), layout->CbvSrvUavRanges( ).data( ), layout->CbvSrvUavVisibility( ) );
    }
    if ( !layout->SamplerRanges( ).empty( ) )
    {
        CD3DX12_ROOT_PARAMETER &samplerRootParameter = m_rootParameters.emplace_back( );
        samplerRootParameter.InitAsDescriptorTable( static_cast<uint32_t>( layout->SamplerRanges( ).size( ) ), layout->SamplerRanges( ).data( ), layout->SamplerVisibility( ) );
    }

    for ( const auto &rootRange : layout->RootLevelRanges( ) )
    {
        CD3DX12_ROOT_PARAMETER &rootParameter = m_rootParameters.emplace_back( );
        switch ( rootRange.Range.RangeType )
        {
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            rootParameter.InitAsShaderResourceView( rootRange.Range.BaseShaderRegister, rootRange.Range.RegisterSpace, rootRange.Visibility );
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            rootParameter.InitAsUnorderedAccessView( rootRange.Range.BaseShaderRegister, rootRange.Range.RegisterSpace, rootRange.Visibility );
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            rootParameter.InitAsConstantBufferView( rootRange.Range.BaseShaderRegister, rootRange.Range.RegisterSpace, rootRange.Visibility );
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
            break;
        }
    }
}

D3D12_ROOT_SIGNATURE_FLAGS DX12RootSignature::ComputeShaderVisibility( ) const
{
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if ( !( m_usedStages & DENOFIZ_SHADER_STAGE_VERTEX_BIT ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & DENOFIZ_SHADER_STAGE_HULL_BIT ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & DENOFIZ_SHADER_STAGE_DOMAIN_BIT ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & DENOFIZ_SHADER_STAGE_GEOMETRY_BIT ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & DENOFIZ_SHADER_STAGE_PIXEL_BIT ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    }
    return flags;
}

void DX12RootSignature::AddRootConstant( const DenOfIz_RootConstantBindingDesc &rootConstant )
{
    CD3DX12_ROOT_PARAMETER &dxRootConstant  = m_rootConstants.emplace_back( );
    dxRootConstant.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    dxRootConstant.ShaderVisibility         = DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( rootConstant.Stages );
    dxRootConstant.Constants.Num32BitValues = rootConstant.NumBytes / sizeof( uint32_t );
    dxRootConstant.Constants.ShaderRegister = rootConstant.Binding;
    dxRootConstant.Constants.RegisterSpace  = DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE;
    m_usedStages |= rootConstant.Stages;
}

uint32_t DX12RootSignature::RegisterSpaceOffset( const uint32_t registerSpace ) const
{
    if ( registerSpace >= m_registerSpaceOffsets.size( ) )
    {
        spdlog::error( "Register space {} does not exists in any binding.", registerSpace );
    }

    return m_registerSpaceOffsets[ registerSpace ];
}

ID3D12RootSignature *DX12RootSignature::Instance( ) const
{
    return m_rootSignature.get( );
}

const std::vector<CD3DX12_ROOT_PARAMETER> &DX12RootSignature::RootParameters( ) const
{
    return m_rootParameters;
}

const std::vector<CD3DX12_ROOT_PARAMETER> &DX12RootSignature::RootConstants( ) const
{
    return m_rootConstants;
}

const std::vector<DX12BindGroupLayout *> &DX12RootSignature::BindGroupLayouts( ) const
{
    return m_bindGroupLayouts;
}

DX12BindGroupLayout *DX12RootSignature::GetBindGroupLayout( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_bindGroupLayouts.size( ) )
    {
        spdlog::error( "Register space {} does not exist.", registerSpace );
        return nullptr;
    }
    return m_bindGroupLayouts[ registerSpace ];
}

DX12RootSignature::~DX12RootSignature( ) = default;
