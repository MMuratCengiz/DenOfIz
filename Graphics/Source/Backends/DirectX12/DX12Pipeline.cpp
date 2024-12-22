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
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderLocalDataLayout.h>
#include <DenOfIzGraphics/Utilities/Storage.h>
#include <utility>

using namespace DenOfIz;

DX12Pipeline::DX12Pipeline( DX12Context *context, PipelineDesc desc ) : m_context( context ), m_desc( std::move( desc ) )
{
    DZ_NOT_NULL( context );

    DZ_ASSERTM( m_desc.RootSignature != nullptr, "Root signature is not set for the pipeline" );
    DZ_ASSERTM( m_desc.ShaderProgram != nullptr, "Shader program is not set for the pipeline" );
    if ( m_desc.BindPoint != BindPoint::RayTracing )
    {
        DZ_ASSERTM( m_desc.InputLayout != nullptr, "Input layout is not set for the pipeline" );
    }

    m_rootSignature = dynamic_cast<DX12RootSignature *>( m_desc.RootSignature );

    switch ( m_desc.BindPoint )
    {
    case BindPoint::Graphics:
        CreateGraphicsPipeline( );
        break;
    case BindPoint::Compute:
        CreateComputePipeline( );
        break;
    case BindPoint::RayTracing:
        CreateRayTracingPipeline( );
        break;
    }
}

void DX12Pipeline::CreateGraphicsPipeline( )
{
    m_topology             = DX12EnumConverter::ConvertPrimitiveTopology( m_desc.Graphics.PrimitiveTopology );
    const auto inputLayout = dynamic_cast<DX12InputLayout *>( m_desc.InputLayout );

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.InputLayout                        = inputLayout->GetInputLayout( );
    psoDesc.pRootSignature                     = m_rootSignature->Instance( );
    SetGraphicsShaders( psoDesc );

    psoDesc.RasterizerState          = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.RasterizerState.CullMode = DX12EnumConverter::ConvertCullMode( m_desc.Graphics.CullMode );

    InitDepthStencil( psoDesc );

    psoDesc.BlendState.AlphaToCoverageEnable  = m_desc.Graphics.AlphaToCoverageEnable;
    psoDesc.BlendState.IndependentBlendEnable = m_desc.Graphics.IndependentBlendEnable;
    psoDesc.SampleMask                        = UINT_MAX;
    psoDesc.PrimitiveTopologyType             = DX12EnumConverter::ConvertPrimitiveTopologyToType( m_desc.Graphics.PrimitiveTopology );
    psoDesc.NumRenderTargets                  = m_desc.Graphics.RenderTargets.NumElements( );
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements( ); ++i )
    {
        BlendDesc &blendDesc                                       = m_desc.Graphics.RenderTargets.GetElement( i ).Blend;
        psoDesc.BlendState.RenderTarget[ i ].BlendEnable           = blendDesc.Enable;
        psoDesc.BlendState.RenderTarget[ i ].LogicOpEnable         = m_desc.Graphics.BlendLogicOpEnable;
        psoDesc.BlendState.RenderTarget[ i ].SrcBlend              = DX12EnumConverter::ConvertBlend( blendDesc.SrcBlend );
        psoDesc.BlendState.RenderTarget[ i ].DestBlend             = DX12EnumConverter::ConvertBlend( blendDesc.DstBlend );
        psoDesc.BlendState.RenderTarget[ i ].BlendOp               = DX12EnumConverter::ConvertBlendOp( blendDesc.BlendOp );
        psoDesc.BlendState.RenderTarget[ i ].SrcBlendAlpha         = DX12EnumConverter::ConvertBlend( blendDesc.SrcBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].DestBlendAlpha        = DX12EnumConverter::ConvertBlend( blendDesc.DstBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].BlendOpAlpha          = DX12EnumConverter::ConvertBlendOp( blendDesc.BlendOpAlpha );
        psoDesc.BlendState.RenderTarget[ i ].LogicOp               = DX12EnumConverter::ConvertLogicOp( m_desc.Graphics.BlendLogicOp );
        psoDesc.BlendState.RenderTarget[ i ].RenderTargetWriteMask = blendDesc.RenderTargetWriteMask;

        psoDesc.RTVFormats[ i ] = DX12EnumConverter::ConvertFormat( m_desc.Graphics.RenderTargets.GetElement( i ).Format );
    }

    if ( m_desc.Graphics.DepthStencilAttachmentFormat != Format::Undefined )
    {
        psoDesc.DSVFormat = DX12EnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    }

    SetMSAASampleCount( m_desc, psoDesc );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) ) );
}

void DX12Pipeline::CreateComputePipeline( )
{
    const auto &compiledShaders = m_desc.ShaderProgram->CompiledShaders( );
    DZ_ASSERTM( compiledShaders.NumElements( ) == 1, "Compute pipeline must have at least/only one shader" );

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.pRootSignature                    = m_rootSignature->Instance( );
    psoDesc.CS                                = GetShaderByteCode( compiledShaders.GetElement( 0 ) );

    DX_CHECK_RESULT( m_context->D3DDevice->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) ) );
}

void DX12Pipeline::CreateRayTracingPipeline( )
{
    if ( m_desc.InputLayout )
    {
        LOG( WARNING ) << "Input layout is provided to a ray tracing pipeline, this has no effect.";
    }

    auto *rootSignature = dynamic_cast<DX12RootSignature *>( m_desc.RootSignature );
    if ( rootSignature->Instance( ) == nullptr )
    {
        LOG( ERROR ) << "Root signature is not initialized";
    }

    auto    compiledShaders = m_desc.ShaderProgram->CompiledShaders( );
    Storage storage{ };
    storage.Reserve( compiledShaders.NumElements( ) * 2 ); // 2, Local root signature Desc, SubObject to exports association Desc
    std::vector<D3D12_STATE_SUBOBJECT> subObjects;
    subObjects.reserve( 256 );
    m_hitGroups.reserve( m_desc.RayTracing.HitGroups.NumElements( ) );
    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = { };
    shaderConfig.MaxAttributeSizeInBytes        = m_desc.RayTracing.MaxNumAttributeBytes;
    shaderConfig.MaxPayloadSizeInBytes          = m_desc.RayTracing.MaxNumPayloadBytes;
    subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig );

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature    = { rootSignature->Instance( ) };
    D3D12_STATE_SUBOBJECT      &rootSignatureSubObject = subObjects.emplace_back( );
    rootSignatureSubObject.Type                        = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    rootSignatureSubObject.pDesc                       = &globalRootSignature;

    std::vector<D3D12_DXIL_LIBRARY_DESC> dxilLibs;
    dxilLibs.reserve( compiledShaders.NumElements( ) );
    std::unordered_set<std::string> visitedShaders;
    // Lifetime Management:
    std::vector<std::wstring> entryPoints;
    entryPoints.reserve( compiledShaders.NumElements( ) );
    std::vector<LPCWSTR> exportNames_cStr;
    exportNames_cStr.reserve( compiledShaders.NumElements( ) );
    // --
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        if ( const auto &compiledShader = compiledShaders.GetElement( i ); !visitedShaders.contains( compiledShader->Path.Get( ) ) )
        {
            dxilLibs.push_back( {
                .DXILLibrary = { .pShaderBytecode = compiledShader->Blob->GetBufferPointer( ), .BytecodeLength = compiledShader->Blob->GetBufferSize( ) },
            } );
            subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &dxilLibs.back( ) );
            visitedShaders.insert( compiledShader->Path.Get( ) );
        }
    }

    std::unordered_map<int32_t, std::wstring> hitGroupExports;
    for ( int i = 0; i < m_desc.RayTracing.HitGroups.NumElements( ); ++i )
    {
        const auto &hitGroup = m_desc.RayTracing.HitGroups.GetElement( i );

        DZ_WS_STRING( hgExport, hitGroup.Name.Get( ) );
        m_exportNames.push_back( hgExport );

        D3D12_HIT_GROUP_DESC &hitGroupDesc = m_hitGroups[ hitGroup.Name.Get( ) ];
        hitGroupDesc.HitGroupExport        = m_exportNames.back( ).c_str( );
        hitGroupDesc.Type                  = ( hitGroup.Type == HitGroupType::Triangles ) ? D3D12_HIT_GROUP_TYPE_TRIANGLES : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;

        if ( hitGroup.ClosestHitShaderIndex >= 0 )
        {
            const auto &shader = compiledShaders.GetElement( hitGroup.ClosestHitShaderIndex );
            DZ_WS_STRING( wEntryPoint, shader->EntryPoint.Get( ) );
            entryPoints.push_back( wEntryPoint );
            hitGroupDesc.ClosestHitShaderImport               = entryPoints.back( ).c_str( );
            hitGroupExports[ hitGroup.ClosestHitShaderIndex ] = hgExport;
        }

        if ( hitGroup.AnyHitShaderIndex >= 0 )
        {
            const auto &shader = compiledShaders.GetElement( hitGroup.AnyHitShaderIndex );
            DZ_WS_STRING( wEntryPoint, shader->EntryPoint.Get( ) );
            entryPoints.push_back( wEntryPoint );
            hitGroupDesc.AnyHitShaderImport               = entryPoints.back( ).c_str( );
            hitGroupExports[ hitGroup.AnyHitShaderIndex ] = hgExport;
        }

        if ( hitGroup.IntersectionShaderIndex >= 0 )
        {
            const auto &shader = compiledShaders.GetElement( hitGroup.IntersectionShaderIndex );
            DZ_WS_STRING( wEntryPoint, shader->EntryPoint.Get( ) );
            entryPoints.push_back( wEntryPoint );
            hitGroupDesc.IntersectionShaderImport               = entryPoints.back( ).c_str( );
            hitGroupExports[ hitGroup.IntersectionShaderIndex ] = hgExport;
        }
    }

    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto &compiledShader = compiledShaders.GetElement( i );

        if ( compiledShader->RayTracing.LocalBindings.NumElements( ) == 0 )
        {
            continue;
        }

        std::string   entryPoint   = compiledShader->EntryPoint.Get( );
        std::wstring &wsEntryPoint = entryPoints.emplace_back( entryPoint.begin( ), entryPoint.end( ) );
        std::wstring *exportName   = nullptr;
        if ( hitGroupExports.contains( i ) )
        {
            exportName = &hitGroupExports.at( i );
        }

        if ( exportName == nullptr )
        {
            exportName = &m_exportNames.emplace_back( wsEntryPoint );
        }

        // Create a local signature for the shader
        IShaderLocalDataLayout *layoutDesc = m_desc.RayTracing.ShaderLocalDataLayouts.GetElement( i );
        auto                   *layout     = dynamic_cast<DX12ShaderLocalDataLayout *>( layoutDesc );

        if ( layout == nullptr )
        {
            LOG( ERROR ) << "Local data layout is not initialized";
            continue;
        }

        auto &localRootSignature               = storage.Store<D3D12_LOCAL_ROOT_SIGNATURE>( );
        localRootSignature.pLocalRootSignature = layout->RootSignature( );

        size_t                 lrsSubObjectRef             = subObjects.size( );
        D3D12_STATE_SUBOBJECT &localRootSignatureSubObject = subObjects.emplace_back( );
        localRootSignatureSubObject.Type                   = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        localRootSignatureSubObject.pDesc                  = &localRootSignature;

        auto &newExportName = exportNames_cStr.emplace_back( exportName->c_str( ) );

        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION &localSignatureAssociation = storage.Store<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>( );
        localSignatureAssociation.NumExports                              = 1;
        localSignatureAssociation.pExports                                = &newExportName;
        localSignatureAssociation.pSubobjectToAssociate                   = &subObjects[ lrsSubObjectRef ];

        D3D12_STATE_SUBOBJECT &localSignatureAssociationSubObject = subObjects.emplace_back( );
        localSignatureAssociationSubObject.Type                   = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        localSignatureAssociationSubObject.pDesc                  = &localSignatureAssociation;
    }

    for ( auto &hitGroup : m_hitGroups | std::views::values )
    {
        subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup );
    }

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = { };
    pipelineConfig.MaxTraceRecursionDepth           = m_desc.RayTracing.MaxRecursionDepth;
    subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig );

    D3D12_STATE_OBJECT_DESC pipelineStateDesc = { };
    pipelineStateDesc.Type                    = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    pipelineStateDesc.NumSubobjects           = static_cast<UINT>( subObjects.size( ) );
    pipelineStateDesc.pSubobjects             = subObjects.data( );

    DX_CHECK_RESULT( m_context->D3DDevice->CreateStateObject( &pipelineStateDesc, IID_PPV_ARGS( m_rayTracingSO.put( ) ) ) );
    DX_CHECK_RESULT( m_rayTracingSO->QueryInterface( IID_PPV_ARGS( m_soProperties.put( ) ) ) );
}

void DX12Pipeline::InitDepthStencil( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    psoDesc.DepthStencilState.DepthEnable    = m_desc.Graphics.DepthTest.Enable;
    psoDesc.DepthStencilState.DepthFunc      = DX12EnumConverter::ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
    psoDesc.DepthStencilState.DepthWriteMask = m_desc.Graphics.DepthTest.Write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

    psoDesc.DepthStencilState.StencilEnable    = m_desc.Graphics.StencilTest.Enable;
    psoDesc.DepthStencilState.StencilReadMask  = m_desc.Graphics.StencilTest.ReadMask;
    psoDesc.DepthStencilState.StencilWriteMask = m_desc.Graphics.StencilTest.WriteMask;

    InitStencilFace( psoDesc.DepthStencilState.FrontFace, m_desc.Graphics.StencilTest.FrontFace );
    InitStencilFace( psoDesc.DepthStencilState.BackFace, m_desc.Graphics.StencilTest.BackFace );
}

void DX12Pipeline::InitStencilFace( D3D12_DEPTH_STENCILOP_DESC &stencilFace, const StencilFace &face ) const
{
    stencilFace.StencilDepthFailOp = DX12EnumConverter::ConvertStencilOp( face.FailOp );
    stencilFace.StencilFunc        = DX12EnumConverter::ConvertCompareOp( face.CompareOp );
    stencilFace.StencilFailOp      = DX12EnumConverter::ConvertStencilOp( face.FailOp );
    stencilFace.StencilPassOp      = DX12EnumConverter::ConvertStencilOp( face.PassOp );
}

void DX12Pipeline::SetMSAASampleCount( const PipelineDesc &desc, D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    switch ( desc.Graphics.MSAASampleCount )
    {
    case MSAASampleCount::_0:
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
    case MSAASampleCount::_64:
        psoDesc.SampleDesc.Count = 32;
        break;
    default:
        break;
    }
}

void DX12Pipeline::SetGraphicsShaders( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    const auto &compiledShaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        switch ( const auto &compiledShader = compiledShaders.GetElement( i ); compiledShader->Stage )
        {
        case ShaderStage::Vertex:
            psoDesc.VS = GetShaderByteCode( compiledShader );
            break;
        case ShaderStage::Hull:
            psoDesc.HS = GetShaderByteCode( compiledShader );
            break;
        case ShaderStage::Domain:
            psoDesc.DS = GetShaderByteCode( compiledShader );
            break;
        case ShaderStage::Geometry:
            psoDesc.GS = GetShaderByteCode( compiledShader );
            break;
        case ShaderStage::Pixel:
            psoDesc.PS = GetShaderByteCode( compiledShader );
            break;
        default:
            break;
        }
    }
}

D3D12_SHADER_BYTECODE DX12Pipeline::GetShaderByteCode( const CompiledShader *const &compiledShader ) const
{
    return D3D12_SHADER_BYTECODE( compiledShader->Blob->GetBufferPointer( ), compiledShader->Blob->GetBufferSize( ) );
}

DX12Pipeline::~DX12Pipeline( )
{
    m_pipeline.reset( );
}

ID3D12PipelineState *DX12Pipeline::GetPipeline( ) const
{
    return m_pipeline.get( );
}

ID3D12StateObject *DX12Pipeline::GetRayTracingSO( ) const
{
    return m_rayTracingSO.get( );
}

ID3D12RootSignature *DX12Pipeline::GetRootSignature( ) const
{
    return m_rootSignature->Instance( );
}

D3D12_PRIMITIVE_TOPOLOGY DX12Pipeline::GetTopology( ) const
{
    return m_topology;
}

void *DX12Pipeline::GetShaderIdentifier( const std::string &exportName )
{
    if ( m_soProperties )
    {
        if ( const auto existingIdentifier = m_shaderIdentifiers.find( exportName ); existingIdentifier != m_shaderIdentifiers.end( ) )
        {
            return existingIdentifier->second;
        }

        const std::wstring wExportName( exportName.begin( ), exportName.end( ) );
        void              *identifier     = m_soProperties->GetShaderIdentifier( wExportName.c_str( ) );
        m_shaderIdentifiers[ exportName ] = identifier;
        return identifier;
    }
    return nullptr;
}
