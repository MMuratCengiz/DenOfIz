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

DX12RootSignature::DX12RootSignature( DX12Context *context, const RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData          = { };
    D3D_ROOT_SIGNATURE_VERSION        rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if ( FAILED( context->D3DDevice->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
    {
        rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    for ( int i = 0; i < desc.ResourceBindings.NumElements( ); ++i )
    {
        AddResourceBinding( desc.ResourceBindings.GetElement( i ) );
    }

    for ( int i = 0; i < m_desc.StaticSamplers.NumElements( ); ++i )
    {
        const StaticSamplerDesc &staticSamplerDesc = m_desc.StaticSamplers.GetElement( i );
        AddStaticSampler( staticSamplerDesc );
    }

    for ( int i = 0; i < desc.RootConstants.NumElements( ); ++i )
    {
        AddRootConstant( desc.RootConstants.GetElement( i ) );
    }

    if ( m_descriptorRangesShaderVisibilities.size( ) == 1 )
    {
        m_cbvSrvUavVisibility = *m_descriptorRangesShaderVisibilities.begin( );
    }

    if ( m_samplerRangesShaderVisibilities.size( ) == 1 )
    {
        m_samplerVisibility = *m_samplerRangesShaderVisibilities.begin( );
    }

    std::copy( m_rootConstants.begin( ), m_rootConstants.end( ), std::back_inserter( m_rootParameters ) );
    for ( const auto &range : m_registerSpaceRanges )
    {
        if ( range.Space != -1 )
        {
            ContainerUtilities::SafeSet( m_registerSpaceOffsets, range.Space, static_cast<uint32_t>( m_rootParameters.size( ) ) );
            ProcessRegisterSpaceRange( range );
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc( static_cast<uint32_t>( m_rootParameters.size( ) ), m_rootParameters.data( ) );
    wil::com_ptr<ID3DBlob>    signature;
    wil::com_ptr<ID3DBlob>    error;
    rootSignatureDesc.Flags             = ComputeShaderVisibility( );
    rootSignatureDesc.NumStaticSamplers = m_staticSamplerDescriptorRanges.size( );
    rootSignatureDesc.pStaticSamplers   = m_staticSamplerDescriptorRanges.data( );
    DX_CHECK_RESULT( D3D12SerializeRootSignature( &rootSignatureDesc, rootSignatureVersion, &signature, &error ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateRootSignature( 0, signature->GetBufferPointer( ), signature->GetBufferSize( ), IID_PPV_ARGS( m_rootSignature.put( ) ) ) );
}

uint32_t DX12RootSignature::GetResourceOffset( const ResourceBindingSlot &slot ) const
{
    if ( slot.RegisterSpace >= m_registerSpaceOrder.size( ) )
    {
        LOG( ERROR ) << "Register space " << slot.RegisterSpace << " is not bound to any bind group.";
    }
    return ContainerUtilities::SafeGetMapValue( m_registerSpaceOrder[ slot.RegisterSpace ].ResourceOffsetMap, slot.Key( ),
                                                "Binding slot does not exist in root signature: " + std::string( slot.ToString( ).Get( ) ) );
}

void DX12RootSignature::ProcessRegisterSpaceRange( const RegisterSpaceRangesDesc &range )
{
    if ( !range.CbvSrvUavRanges.empty( ) )
    {
        CD3DX12_ROOT_PARAMETER &rootParameter = m_rootParameters.emplace_back( );
        rootParameter.InitAsDescriptorTable( static_cast<uint32_t>( range.CbvSrvUavRanges.size( ) ), range.CbvSrvUavRanges.data( ), m_cbvSrvUavVisibility );
    }
    if ( !range.SamplerRanges.empty( ) )
    {
        CD3DX12_ROOT_PARAMETER &samplerRootParameter = m_rootParameters.emplace_back( );
        samplerRootParameter.InitAsDescriptorTable( static_cast<uint32_t>( range.SamplerRanges.size( ) ), range.SamplerRanges.data( ), m_samplerVisibility );
    }

    for ( const auto &rootRange : range.RootLevelRanges )
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
    if ( !( m_usedStages & D3D12_SHADER_VISIBILITY_VERTEX ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & D3D12_SHADER_VISIBILITY_HULL ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & D3D12_SHADER_VISIBILITY_DOMAIN ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & D3D12_SHADER_VISIBILITY_GEOMETRY ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    }
    if ( !( m_usedStages & D3D12_SHADER_VISIBILITY_PIXEL ) )
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    }
    return flags;
}

void DX12RootSignature::AddStaticSampler( const StaticSamplerDesc &staticSamplerDesc )
{
    const SamplerDesc        &samplerDesc = staticSamplerDesc.Sampler;
    D3D12_STATIC_SAMPLER_DESC desc        = { };

    const int filter     = static_cast<int>( samplerDesc.MinFilter ) << 4 | static_cast<int>( samplerDesc.MagFilter ) << 2 | static_cast<int>( samplerDesc.MipmapMode );
    int       baseFilter = samplerDesc.CompareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
    if ( samplerDesc.MaxAnisotropy > 0.0f )
    {
        baseFilter = samplerDesc.CompareOp != CompareOp::Never ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
    }

    desc.Filter         = static_cast<D3D12_FILTER>( baseFilter + filter );
    desc.AddressU       = DX12EnumConverter::ConvertSamplerAddressMode( samplerDesc.AddressModeU );
    desc.AddressV       = DX12EnumConverter::ConvertSamplerAddressMode( samplerDesc.AddressModeV );
    desc.AddressW       = DX12EnumConverter::ConvertSamplerAddressMode( samplerDesc.AddressModeW );
    desc.MipLODBias     = samplerDesc.MipLodBias;
    desc.MaxAnisotropy  = samplerDesc.MaxAnisotropy;
    desc.ComparisonFunc = DX12EnumConverter::ConvertCompareOp( samplerDesc.CompareOp );
    desc.MinLOD         = samplerDesc.MinLod;
    desc.MaxLOD         = samplerDesc.MaxLod;

    desc.ShaderRegister   = staticSamplerDesc.Binding.Binding;
    desc.RegisterSpace    = staticSamplerDesc.Binding.RegisterSpace;
    desc.ShaderVisibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility( staticSamplerDesc.Binding.Stages.GetElement( 0 ) );

    m_staticSamplerDescriptorRanges.push_back( desc );
}

void DX12RootSignature::AddResourceBinding( const ResourceBindingDesc &binding )
{
    RegisterSpaceOrder       &spaceOrder = ContainerUtilities::SafeAt( m_registerSpaceOrder, binding.RegisterSpace );
    const ResourceBindingSlot slot{
        .Binding       = binding.Binding,
        .RegisterSpace = binding.RegisterSpace,
        .Type          = binding.BindingType,
    };

    CD3DX12_DESCRIPTOR_RANGE descriptorRange = { };
    descriptorRange.Init( DX12EnumConverter::ConvertResourceDescriptorToDescriptorRangeType( binding.Descriptor ), binding.ArraySize, binding.Binding, binding.RegisterSpace );

    RegisterSpaceRangesDesc &spaceDesc = ContainerUtilities::SafeAt( m_registerSpaceRanges, binding.RegisterSpace );
    spaceDesc.Space                    = binding.RegisterSpace;
    /**
     * In the code below a few tracking mechanisms are implemented:
     * - ResourceOffsetMap is used to track the offset of the resource in the descriptor table. This is then used to figure out the RootParameterIndex.
     * - Shader visibilities are tracked in a hash set to determine the visibility of the descriptor table. Note since all bindings could use the same shader visibility, we cannot
     * simply check for size.
     */
    if ( binding.Descriptor.IsSet( ResourceDescriptor::Sampler ) )
    {
        spaceDesc.SamplerRanges.push_back( descriptorRange );
        spaceOrder.ResourceOffsetMap[ slot.Key( ) ] = spaceOrder.SamplerCount++;

        for ( int i = 0; i < binding.Stages.NumElements( ); ++i )
        {
            const auto             &stage     = binding.Stages.GetElement( i );
            D3D12_SHADER_VISIBILITY usedStage = DX12EnumConverter::ConvertShaderStageToShaderVisibility( stage );
            m_samplerRangesShaderVisibilities.insert( usedStage );
            m_usedStages |= usedStage;
        }
    }
    /**
     * OptimizedRegisterSpace means Root level descriptor binding.
     */
    else if ( spaceDesc.Space == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace &&
              ( binding.Reflection.Type == ReflectionBindingType::Struct || binding.Reflection.Type == ReflectionBindingType::Pointer ) )
    {
        RootLevelDescriptorRange &rootLevelRange = spaceDesc.RootLevelRanges.emplace_back( );
        rootLevelRange.Range                     = descriptorRange;

        // Shader visibilities are skipped as they are not part of the same descriptor table. No need to track via a hash set.
        if ( binding.Stages.NumElements( ) == 0 || binding.Stages.NumElements( ) > 1 )
        {
            rootLevelRange.Visibility = D3D12_SHADER_VISIBILITY_ALL;
            m_usedStages |= D3D12_SHADER_VISIBILITY_ALL;
        }
        else
        {
            rootLevelRange.Visibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility( binding.Stages.GetElement( 0 ) );
            m_usedStages |= rootLevelRange.Visibility;
        }
        spaceOrder.ResourceOffsetMap[ slot.Key( ) ] = spaceOrder.RootLevelBufferCount++;
    }
    else
    {
        spaceDesc.CbvSrvUavRanges.push_back( descriptorRange );
        spaceOrder.ResourceOffsetMap[ slot.Key( ) ] = spaceOrder.ResourceCount++;

        for ( int i = 0; i < binding.Stages.NumElements( ); ++i )
        {
            const auto             &stage     = binding.Stages.GetElement( i );
            D3D12_SHADER_VISIBILITY usedStage = DX12EnumConverter::ConvertShaderStageToShaderVisibility( stage );
            m_descriptorRangesShaderVisibilities.insert( usedStage );
            m_usedStages |= usedStage;
        }
    }
}

void DX12RootSignature::AddRootConstant( const RootConstantResourceBindingDesc &rootConstant )
{
    CD3DX12_ROOT_PARAMETER &dxRootConstant = m_rootConstants.emplace_back( );
    dxRootConstant.ParameterType           = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    dxRootConstant.ShaderVisibility        = D3D12_SHADER_VISIBILITY_ALL;
    if ( rootConstant.Stages.NumElements( ) == 1 )
    {
        dxRootConstant.ShaderVisibility = DX12EnumConverter::ConvertShaderStageToShaderVisibility( rootConstant.Stages.GetElement( 0 ) );
    }
    dxRootConstant.Constants.Num32BitValues = rootConstant.NumBytes / sizeof( uint32_t );
    dxRootConstant.Constants.ShaderRegister = rootConstant.Binding;
    dxRootConstant.Constants.RegisterSpace  = DZConfiguration::Instance( ).RootConstantRegisterSpace;
    m_usedStages |= dxRootConstant.ShaderVisibility;
}

uint32_t DX12RootSignature::RegisterSpaceOffset( const uint32_t registerSpace ) const
{
    if ( registerSpace >= m_registerSpaceOffsets.size( ) )
    {
        LOG( ERROR ) << "Register space " << registerSpace << " does not exists in any binding.";
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

DX12RootSignature::~DX12RootSignature( ) = default;
