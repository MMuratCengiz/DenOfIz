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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Pipeline.h"
#include <codecvt>
#include <directx/d3dx12.h>
#include <ranges>
#include <utility>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12InputLayout.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12PipelineCache.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12LocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Storage.h"

using namespace DenOfIz;

#define ROOT_SIGNATURE_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IRootSignature, handle )
#define INPUT_LAYOUT_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IInputLayout, handle )
#define PIPELINE_CACHE_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IPipelineCache, handle )
#define LOCAL_ROOT_SIGNATURE_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ILocalRootSignature, handle )

DX12Pipeline::DX12Pipeline( DX12Context *context, DenOfIz_PipelineDesc desc ) : m_context( context ), m_desc( std::move( desc ) )
{
    DZ_NOT_NULL( context );

    DZ_ASSERTM( DENOFIZ_HANDLE_IS_VALID( m_desc.RootSignature ), "Root signature is not set for the pipeline" );
    DZ_ASSERTM( DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ), "Shader program is not set for the pipeline" );
    if ( m_desc.BindPoint != DENOFIZ_BIND_POINT_RAYTRACING && DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ) )
    {
        DZ_ASSERTM( DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ), "Input layout is not set for the pipeline" );
    }

    m_rootSignature = dynamic_cast<DX12RootSignature *>( ROOT_SIGNATURE_PTR( m_desc.RootSignature ) );

    switch ( m_desc.BindPoint )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
        CreateGraphicsPipeline( );
        break;
    case DENOFIZ_BIND_POINT_COMPUTE:
        CreateComputePipeline( );
        break;
    case DENOFIZ_BIND_POINT_RAYTRACING:
        CreateRayTracingPipeline( );
        break;
    case DENOFIZ_BIND_POINT_MESH:
        CreateMeshPipeline( );
        break;
    }
}

void DX12Pipeline::CreateGraphicsPipeline( )
{
    m_topology                          = DenOfIz_DX12EnumConverter_ConvertPrimitiveTopology( m_desc.Graphics.PrimitiveTopology );
    const auto              inputLayout = DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ) ? dynamic_cast<DX12InputLayout *>( INPUT_LAYOUT_PTR( m_desc.InputLayout ) ) : nullptr;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{ };
    if ( inputLayout == nullptr )
    {
        m_iaStride = 0;
    }
    else
    {
        m_iaStride      = inputLayout->Stride( 0 ); // Use stride from first input group
        inputLayoutDesc = inputLayout->GetInputLayout( );
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.InputLayout                        = inputLayoutDesc;
    psoDesc.pRootSignature                     = m_rootSignature->Instance( );
    SetGraphicsShaders( psoDesc );

    psoDesc.RasterizerState                       = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.RasterizerState.CullMode              = DenOfIz_DX12EnumConverter_ConvertCullMode( m_desc.Graphics.CullMode );
    psoDesc.RasterizerState.FillMode              = DenOfIz_DX12EnumConverter_ConvertFillMode( m_desc.Graphics.FillMode );
    psoDesc.RasterizerState.DepthBias             = m_desc.Graphics.Rasterization.DepthBias;
    psoDesc.RasterizerState.DepthBiasClamp        = m_desc.Graphics.Rasterization.DepthBiasClamp;
    psoDesc.RasterizerState.SlopeScaledDepthBias  = m_desc.Graphics.Rasterization.SlopeScaledDepthBias;
    psoDesc.RasterizerState.FrontCounterClockwise = m_desc.Graphics.Rasterization.FrontCounterClockwise;

    InitDepthStencil( psoDesc );

    psoDesc.BlendState.AlphaToCoverageEnable  = m_desc.Graphics.AlphaToCoverageEnable;
    psoDesc.BlendState.IndependentBlendEnable = m_desc.Graphics.IndependentBlendEnable;
    psoDesc.SampleMask                        = UINT_MAX;
    psoDesc.PrimitiveTopologyType             = DenOfIz_DX12EnumConverter_ConvertPrimitiveTopologyToType( m_desc.Graphics.PrimitiveTopology );
    psoDesc.NumRenderTargets                  = m_desc.Graphics.RenderTargets.NumElements;
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        DenOfIz_BlendDesc &blendDesc                               = m_desc.Graphics.RenderTargets.Elements[ i ].Blend;
        psoDesc.BlendState.RenderTarget[ i ].BlendEnable           = blendDesc.Enable;
        psoDesc.BlendState.RenderTarget[ i ].LogicOpEnable         = m_desc.Graphics.BlendLogicOpEnable;
        psoDesc.BlendState.RenderTarget[ i ].SrcBlend              = DenOfIz_DX12EnumConverter_ConvertBlend( blendDesc.SrcBlend );
        psoDesc.BlendState.RenderTarget[ i ].DestBlend             = DenOfIz_DX12EnumConverter_ConvertBlend( blendDesc.DstBlend );
        psoDesc.BlendState.RenderTarget[ i ].BlendOp               = DenOfIz_DX12EnumConverter_ConvertBlendOp( blendDesc.BlendOp );
        psoDesc.BlendState.RenderTarget[ i ].SrcBlendAlpha         = DenOfIz_DX12EnumConverter_ConvertBlend( blendDesc.SrcBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].DestBlendAlpha        = DenOfIz_DX12EnumConverter_ConvertBlend( blendDesc.DstBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].BlendOpAlpha          = DenOfIz_DX12EnumConverter_ConvertBlendOp( blendDesc.BlendOpAlpha );
        psoDesc.BlendState.RenderTarget[ i ].LogicOp               = DenOfIz_DX12EnumConverter_ConvertLogicOp( m_desc.Graphics.BlendLogicOp );
        psoDesc.BlendState.RenderTarget[ i ].RenderTargetWriteMask = blendDesc.RenderTargetWriteMask;

        psoDesc.RTVFormats[ i ] = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Graphics.RenderTargets.Elements[ i ].Format );
    }

    if ( m_desc.Graphics.DepthStencilAttachmentFormat != DENOFIZ_FORMAT_UNDEFINED )
    {
        psoDesc.DSVFormat = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    }

    SetMSAASampleCount( m_desc, psoDesc );

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) )
    {
        auto dx12Cache = static_cast<DX12PipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( ID3D12PipelineLibrary *library = dx12Cache->GetLibrary( ) )
        {
            std::wstring pipelineName = L"GraphicsPipeline_" + std::to_wstring( reinterpret_cast<uintptr_t>( this ) );
            HRESULT      hr           = library->LoadGraphicsPipeline( pipelineName.c_str( ), &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) );
            if ( SUCCEEDED( hr ) )
            {
                return;
            }
        }
    }

    DX_CHECK_RESULT( m_context->D3DDevice->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) ) );

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) && m_pipeline )
    {
        auto dx12Cache = static_cast<DX12PipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( ID3D12PipelineLibrary *library = dx12Cache->GetLibrary( ) )
        {
            std::wstring pipelineName = L"GraphicsPipeline_" + std::to_wstring( reinterpret_cast<uintptr_t>( this ) );
            DX_CHECK_RESULT( library->StorePipeline( pipelineName.c_str( ), m_pipeline.get( ) ) );
        }
    }
}

void DX12Pipeline::CreateComputePipeline( )
{
    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    DZ_ASSERTM( compiledShaders.NumElements == 1, "Compute pipeline must have at least/only one shader" );

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.pRootSignature                    = m_rootSignature->Instance( );
    psoDesc.CS                                = GetShaderByteCode( compiledShaders.Elements[ 0 ] );

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) )
    {
        const auto dx12Cache = static_cast<DX12PipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( ID3D12PipelineLibrary *library = dx12Cache->GetLibrary( ) )
        {
            const std::wstring pipelineName = L"ComputePipeline_" + std::to_wstring( reinterpret_cast<uintptr_t>( this ) );
            const HRESULT      hr           = library->LoadComputePipeline( pipelineName.c_str( ), &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) );
            if ( SUCCEEDED( hr ) )
            {
                return;
            }
        }
    }

    DX_CHECK_RESULT( m_context->D3DDevice->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( m_pipeline.put( ) ) ) );

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) && m_pipeline )
    {
        const auto dx12Cache = static_cast<DX12PipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( ID3D12PipelineLibrary *library = dx12Cache->GetLibrary( ) )
        {
            const std::wstring pipelineName = L"ComputePipeline_" + std::to_wstring( reinterpret_cast<uintptr_t>( this ) );
            DX_CHECK_RESULT( library->StorePipeline( pipelineName.c_str( ), m_pipeline.get( ) ) );
        }
    }
}

void DX12Pipeline::CreateRayTracingPipeline( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ) )
    {
        spdlog::warn( "Input layout is provided to a ray tracing pipeline, this has no effect." );
    }

    auto *rootSignature = dynamic_cast<DX12RootSignature *>( ROOT_SIGNATURE_PTR( m_desc.RootSignature ) );
    if ( rootSignature->Instance( ) == nullptr )
    {
        spdlog::error( "Root signature is not initialized" );
    }

    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    DenOfIz_ShaderProgramDesc shaderProgramDesc;
    DenOfIz_ShaderProgram_Desc( m_desc.ShaderProgram, &shaderProgramDesc );
    Storage storage{ };
    storage.Reserve( compiledShaders.NumElements * 2 ); // 2, Local root signature Desc, SubObject to exports association Desc
    std::vector<D3D12_STATE_SUBOBJECT> subObjects;
    subObjects.reserve( 256 );
    m_hitGroups.reserve( m_desc.RayTracing.HitGroups.NumElements );
    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = { };
    shaderConfig.MaxAttributeSizeInBytes        = shaderProgramDesc.RayTracing.MaxNumAttributeBytes;
    shaderConfig.MaxPayloadSizeInBytes          = shaderProgramDesc.RayTracing.MaxNumPayloadBytes;
    subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig );

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature    = { rootSignature->Instance( ) };
    D3D12_STATE_SUBOBJECT      &rootSignatureSubObject = subObjects.emplace_back( );
    rootSignatureSubObject.Type                        = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    rootSignatureSubObject.pDesc                       = &globalRootSignature;

    // std::vector<D3D12_DXIL_LIBRARY_DESC> dxilLibs;
    std::vector<D3D12_DXIL_LIBRARY_DESC> dxilLibs;
    dxilLibs.reserve( compiledShaders.NumElements );
    // Lifetime Management:
    std::vector<std::wstring> entryPoints;
    entryPoints.reserve( compiledShaders.NumElements * 2 );
    // --
    for ( uint32_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        if ( const auto &compiledShader = compiledShaders.Elements[ i ] )
        {
            std::string entryPoint( compiledShader->EntryPoint.Chars, compiledShader->EntryPoint.NumChars );
            DZ_WS_STRING( wEntryPoint, entryPoint.c_str( ) );
            entryPoints.push_back( wEntryPoint );

            D3D12_DXIL_LIBRARY_DESC &libraryDesc = dxilLibs.emplace_back( );
            libraryDesc.DXILLibrary              = { .pShaderBytecode = compiledShader->DXIL.Elements, .BytecodeLength = compiledShader->DXIL.NumElements };
            subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &libraryDesc );
        }
    }

    std::unordered_map<ID3D12RootSignature *, std::vector<LPCWSTR>> rootSignatureExports;
    std::unordered_set<std::wstring>                                processedExports;
    // Handle local root signatures per shader
    auto localRootSignatures = m_desc.RayTracing.LocalRootSignatures;
    for ( uint32_t i = 0; i < localRootSignatures.NumElements; ++i )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( localRootSignatures.Elements[ i ] ) )
        {
            if ( auto *localRootSignature = dynamic_cast<DX12LocalRootSignature *>( LOCAL_ROOT_SIGNATURE_PTR( localRootSignatures.Elements[ i ] ) ) )
            {
                auto &exports = rootSignatureExports[ localRootSignature->RootSignature( ) ];
                exports.push_back( entryPoints[ i ].c_str( ) );
            }
        }
    }

    // Handle RootSignatures for hit groups, create said hit groups in the meantime
    for ( uint32_t i = 0; i < m_desc.RayTracing.HitGroups.NumElements; ++i )
    {
        const auto &hitGroup = m_desc.RayTracing.HitGroups.Elements[ i ];

        std::string hitGroupNameStr( hitGroup.Name.Chars, hitGroup.Name.NumChars );
        DZ_WS_STRING( hgExport, hitGroupNameStr.c_str( ) );
        m_exportNames.push_back( hgExport );

        D3D12_HIT_GROUP_DESC &hitGroupDesc = m_hitGroups[ hitGroupNameStr ];
        hitGroupDesc.HitGroupExport        = m_exportNames.back( ).c_str( );
        hitGroupDesc.Type                  = hitGroup.Type == DENOFIZ_HIT_GROUP_TYPE_TRIANGLES ? D3D12_HIT_GROUP_TYPE_TRIANGLES : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;

        if ( hitGroup.ClosestHitShaderIndex >= 0 )
        {
            hitGroupDesc.ClosestHitShaderImport = entryPoints[ hitGroup.ClosestHitShaderIndex ].c_str( );
        }
        if ( hitGroup.AnyHitShaderIndex >= 0 )
        {
            hitGroupDesc.AnyHitShaderImport = entryPoints[ hitGroup.AnyHitShaderIndex ].c_str( );
        }

        if ( hitGroup.IntersectionShaderIndex >= 0 )
        {
            hitGroupDesc.IntersectionShaderImport = entryPoints[ hitGroup.IntersectionShaderIndex ].c_str( );
        }

        if ( DENOFIZ_HANDLE_IS_VALID( hitGroup.LocalRootSignature ) )
        {
            if ( auto *localRootSignature = dynamic_cast<DX12LocalRootSignature *>( LOCAL_ROOT_SIGNATURE_PTR( hitGroup.LocalRootSignature ) ); localRootSignature != nullptr )
            {
                auto &exports = rootSignatureExports[ localRootSignature->RootSignature( ) ];
                exports.push_back( hitGroupDesc.HitGroupExport );
            }
        }
    }

    for ( auto &[ rootSig, exports ] : rootSignatureExports )
    {
        auto &localRootSignature               = storage.Store<D3D12_LOCAL_ROOT_SIGNATURE>( );
        localRootSignature.pLocalRootSignature = rootSig;

        auto &localRootSigSubObject = subObjects.emplace_back( );
        localRootSigSubObject.Type  = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        localRootSigSubObject.pDesc = &localRootSignature;

        auto &association                 = storage.Store<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>( );
        association.NumExports            = static_cast<UINT>( exports.size( ) );
        association.pExports              = exports.data( );
        association.pSubobjectToAssociate = &localRootSigSubObject;

        auto &associationSubObject = subObjects.emplace_back( );
        associationSubObject.Type  = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        associationSubObject.pDesc = &association;
    }

    for ( auto &hitGroup : m_hitGroups | std::views::values )
    {
        subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup );
    }

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = { };
    pipelineConfig.MaxTraceRecursionDepth           = shaderProgramDesc.RayTracing.MaxRecursionDepth;
    subObjects.emplace_back( D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig );

    D3D12_STATE_OBJECT_DESC pipelineStateDesc = { };
    pipelineStateDesc.Type                    = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    pipelineStateDesc.NumSubobjects           = static_cast<UINT>( subObjects.size( ) );
    pipelineStateDesc.pSubobjects             = subObjects.data( );

    PrintRayTracingPipelineDesc( &pipelineStateDesc );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateStateObject( &pipelineStateDesc, IID_PPV_ARGS( m_rayTracingSO.put( ) ) ) );
    DX_CHECK_RESULT( m_rayTracingSO->QueryInterface( IID_PPV_ARGS( m_soProperties.put( ) ) ) );
}

std::string WideToUtf8( const std::wstring &wide )
{
    std::string narrow;
    narrow.assign( wide.begin( ), wide.end( ) );
    return narrow;
}

void DX12Pipeline::PrintRayTracingPipelineDesc( const D3D12_STATE_OBJECT_DESC *desc ) const
{
    std::stringstream ss;
    ss << "\n";
    ss << "--------------------------------------------------------------------\n";
    ss << "| D3D12 State Object 0x" << std::hex << desc << ": Raytracing Pipeline\n";

    for ( UINT i = 0; i < desc->NumSubobjects; i++ )
    {
        const auto &subObject = desc->pSubobjects[ i ];
        ss << "| [" << std::dec << i << "]: ";

        switch ( subObject.Type )
        {
        case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
            {
                const auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC *>( subObject.pDesc );
                ss << "DXIL Library 0x" << std::hex << lib->DXILLibrary.pShaderBytecode << ", " << std::dec << lib->DXILLibrary.BytecodeLength << " bytes\n";
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
            {
                const auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC *>( subObject.pDesc );
                ss << "Hit Group (" << ( hitGroup->HitGroupExport ? WideToUtf8( hitGroup->HitGroupExport ) : "[none]" ) << ")\n";
                ss << "|  [0]: Any Hit Import: " << ( hitGroup->AnyHitShaderImport ? WideToUtf8( hitGroup->AnyHitShaderImport ) : "[none]" ) << "\n";
                ss << "|  [1]: Closest Hit Import: " << ( hitGroup->ClosestHitShaderImport ? WideToUtf8( hitGroup->ClosestHitShaderImport ) : "[none]" ) << "\n";
                ss << "|  [2]: Intersection Import: " << ( hitGroup->IntersectionShaderImport ? WideToUtf8( hitGroup->IntersectionShaderImport ) : "[none]" ) << "\n";
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
            {
                const auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG *>( subObject.pDesc );
                ss << "Raytracing Shader Config\n";
                ss << "|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << " bytes\n";
                ss << "|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << " bytes\n";
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
            {
                const auto rootSig = static_cast<const D3D12_LOCAL_ROOT_SIGNATURE *>( subObject.pDesc );
                ss << "Local Root Signature 0x" << std::hex << rootSig->pLocalRootSignature << "\n";
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
            {
                const auto rootSig = static_cast<const D3D12_GLOBAL_ROOT_SIGNATURE *>( subObject.pDesc );
                ss << "Global Root Signature 0x" << std::hex << rootSig->pGlobalRootSignature << "\n";
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
            {
                const auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION *>( subObject.pDesc );
                const UINT index       = static_cast<UINT>( association->pSubobjectToAssociate - desc->pSubobjects );
                ss << "Subobject to Exports Association (Subobject [" << index << "])\n";
                for ( UINT j = 0; j < association->NumExports; j++ )
                {
                    ss << "|  [" << j << "]: " << WideToUtf8( association->pExports[ j ] ) << "\n";
                }
                break;
            }

        case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
            {
                const auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG *>( subObject.pDesc );
                ss << "Raytracing Pipeline Config\n";
                ss << "|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << "\n";
                break;
            }
        default:
            break;
        }
        ss << "--------------------------------------------------------------------\n";
    }

    spdlog::info( "\n {}", ss.str( ) );
}

void DX12Pipeline::InitDepthStencil( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    psoDesc.DepthStencilState.DepthEnable    = m_desc.Graphics.DepthTest.Enable;
    psoDesc.DepthStencilState.DepthFunc      = DenOfIz_DX12EnumConverter_ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
    psoDesc.DepthStencilState.DepthWriteMask = m_desc.Graphics.DepthTest.Write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

    const DenOfIz_Format dsvFormat = m_desc.Graphics.DepthStencilAttachmentFormat;
    if ( dsvFormat != DENOFIZ_FORMAT_UNDEFINED )
    {
        const bool isDepthOnly = DenOfIz_Format_IsDepthOnly( dsvFormat );

        if ( m_desc.Graphics.StencilTest.Enable && isDepthOnly )
        {
            spdlog::warn( "DX12Pipeline: Stencil operations enabled with depth-only format ({}). Stencil operations will be ignored.", static_cast<int>( dsvFormat ) );
        }
    }

    psoDesc.DepthStencilState.StencilEnable    = m_desc.Graphics.StencilTest.Enable;
    psoDesc.DepthStencilState.StencilReadMask  = m_desc.Graphics.StencilTest.ReadMask;
    psoDesc.DepthStencilState.StencilWriteMask = m_desc.Graphics.StencilTest.WriteMask;

    InitStencilFace( psoDesc.DepthStencilState.FrontFace, m_desc.Graphics.StencilTest.FrontFace );
    InitStencilFace( psoDesc.DepthStencilState.BackFace, m_desc.Graphics.StencilTest.BackFace );
}

void DX12Pipeline::InitStencilFace( D3D12_DEPTH_STENCILOP_DESC &stencilFace, const DenOfIz_StencilFace &face ) const
{
    stencilFace.StencilDepthFailOp = DenOfIz_DX12EnumConverter_ConvertStencilOp( face.FailOp );
    stencilFace.StencilFunc        = DenOfIz_DX12EnumConverter_ConvertCompareOp( face.CompareOp );
    stencilFace.StencilFailOp      = DenOfIz_DX12EnumConverter_ConvertStencilOp( face.FailOp );
    stencilFace.StencilPassOp      = DenOfIz_DX12EnumConverter_ConvertStencilOp( face.PassOp );
}

void DX12Pipeline::SetMSAASampleCount( const DenOfIz_PipelineDesc &desc, D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    switch ( desc.Graphics.MSAASampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_0:
    case DENOFIZ_MSAA_SAMPLE_COUNT_1:
        psoDesc.SampleDesc.Count = 1;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_2:
        psoDesc.SampleDesc.Count = 2;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_4:
        psoDesc.SampleDesc.Count = 4;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_8:
        psoDesc.SampleDesc.Count = 8;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_16:
        psoDesc.SampleDesc.Count = 16;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_32:
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        psoDesc.SampleDesc.Count = 32;
        break;
    default:
        break;
    }
}

void DX12Pipeline::SetGraphicsShaders( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    for ( uint32_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        const auto &compiledShader = compiledShaders.Elements[ i ];
        if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
        {
            psoDesc.VS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_HULL_BIT )
        {
            psoDesc.HS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_DOMAIN_BIT )
        {
            psoDesc.DS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_GEOMETRY_BIT )
        {
            psoDesc.GS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
        {
            psoDesc.PS = GetShaderByteCode( compiledShader );
        }
    }
}

D3D12_SHADER_BYTECODE DX12Pipeline::GetShaderByteCode( const DenOfIz_CompiledShaderStage *const &compiledShader ) const
{
    return D3D12_SHADER_BYTECODE( compiledShader->DXIL.Elements, compiledShader->DXIL.NumElements );
}

DX12Pipeline::~DX12Pipeline( )
{
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

DX12RootSignature *DX12Pipeline::RootSignature( ) const
{
    return m_rootSignature;
}

D3D12_PRIMITIVE_TOPOLOGY DX12Pipeline::GetTopology( ) const
{
    return m_topology;
}

DenOfIz_BindPoint DX12Pipeline::GetBindPoint( ) const
{
    return m_desc.BindPoint;
}

uint32_t DX12Pipeline::GetIAStride( ) const
{
    return m_iaStride;
}

void DX12Pipeline::CreateMeshPipeline( )
{
    m_topology = DenOfIz_DX12EnumConverter_ConvertPrimitiveTopology( m_desc.Graphics.PrimitiveTopology );

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineDesc = { };
    struct MeshPipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        RootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_AS                    AS; // Task Shader (Amplification Shader)
        CD3DX12_PIPELINE_STATE_STREAM_MS                    MS; // Mesh Shader
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1        DepthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
    } meshPipelineStateStream;

    meshPipelineStateStream.RootSignature = m_rootSignature->Instance( );
    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );

    meshPipelineStateStream.PS = D3D12_SHADER_BYTECODE{ };
    meshPipelineStateStream.AS = D3D12_SHADER_BYTECODE{ };
    meshPipelineStateStream.MS = D3D12_SHADER_BYTECODE{ };

    for ( uint32_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        const auto &compiledShader = compiledShaders.Elements[ i ];
        if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_TASK_BIT )
        {
            meshPipelineStateStream.AS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_MESH_BIT )
        {
            meshPipelineStateStream.MS = GetShaderByteCode( compiledShader );
        }
        else if ( compiledShader->Stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
        {
            meshPipelineStateStream.PS = GetShaderByteCode( compiledShader );
        }
        else
        {
            spdlog::warn( "Unsupported shader stage for mesh pipeline: {}", static_cast<int>( compiledShader->Stage ) );
        }
    }

    CD3DX12_BLEND_DESC blendDesc     = { };
    blendDesc.AlphaToCoverageEnable  = m_desc.Graphics.AlphaToCoverageEnable;
    blendDesc.IndependentBlendEnable = m_desc.Graphics.IndependentBlendEnable;

    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        DenOfIz_BlendDesc &pipelineBlendDesc              = m_desc.Graphics.RenderTargets.Elements[ i ].Blend;
        blendDesc.RenderTarget[ i ].BlendEnable           = pipelineBlendDesc.Enable;
        blendDesc.RenderTarget[ i ].LogicOpEnable         = m_desc.Graphics.BlendLogicOpEnable;
        blendDesc.RenderTarget[ i ].SrcBlend              = DenOfIz_DX12EnumConverter_ConvertBlend( pipelineBlendDesc.SrcBlend );
        blendDesc.RenderTarget[ i ].DestBlend             = DenOfIz_DX12EnumConverter_ConvertBlend( pipelineBlendDesc.DstBlend );
        blendDesc.RenderTarget[ i ].BlendOp               = DenOfIz_DX12EnumConverter_ConvertBlendOp( pipelineBlendDesc.BlendOp );
        blendDesc.RenderTarget[ i ].SrcBlendAlpha         = DenOfIz_DX12EnumConverter_ConvertBlend( pipelineBlendDesc.SrcBlendAlpha );
        blendDesc.RenderTarget[ i ].DestBlendAlpha        = DenOfIz_DX12EnumConverter_ConvertBlend( pipelineBlendDesc.DstBlendAlpha );
        blendDesc.RenderTarget[ i ].BlendOpAlpha          = DenOfIz_DX12EnumConverter_ConvertBlendOp( pipelineBlendDesc.BlendOpAlpha );
        blendDesc.RenderTarget[ i ].LogicOp               = DenOfIz_DX12EnumConverter_ConvertLogicOp( m_desc.Graphics.BlendLogicOp );
        blendDesc.RenderTarget[ i ].RenderTargetWriteMask = pipelineBlendDesc.RenderTargetWriteMask;
    }
    meshPipelineStateStream.BlendState = blendDesc;
    meshPipelineStateStream.SampleMask = UINT_MAX;

    CD3DX12_RASTERIZER_DESC rasterizerDesc  = { };
    rasterizerDesc.FillMode                 = DenOfIz_DX12EnumConverter_ConvertFillMode( m_desc.Graphics.FillMode );
    rasterizerDesc.CullMode                 = DenOfIz_DX12EnumConverter_ConvertCullMode( m_desc.Graphics.CullMode );
    rasterizerDesc.FrontCounterClockwise    = m_desc.Graphics.Rasterization.FrontCounterClockwise;
    rasterizerDesc.DepthBias                = m_desc.Graphics.Rasterization.DepthBias;
    rasterizerDesc.DepthBiasClamp           = m_desc.Graphics.Rasterization.DepthBiasClamp;
    rasterizerDesc.SlopeScaledDepthBias     = m_desc.Graphics.Rasterization.SlopeScaledDepthBias;
    rasterizerDesc.DepthClipEnable          = TRUE;
    rasterizerDesc.MultisampleEnable        = FALSE;
    rasterizerDesc.AntialiasedLineEnable    = FALSE;
    rasterizerDesc.ForcedSampleCount        = 0;
    rasterizerDesc.ConservativeRaster       = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    meshPipelineStateStream.RasterizerState = rasterizerDesc;

    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc = { };
    depthStencilDesc.DepthEnable                 = m_desc.Graphics.DepthTest.Enable;
    depthStencilDesc.DepthWriteMask              = m_desc.Graphics.DepthTest.Write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc                   = DenOfIz_DX12EnumConverter_ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );

    const DenOfIz_Format dsvFormat = m_desc.Graphics.DepthStencilAttachmentFormat;
    if ( dsvFormat != DENOFIZ_FORMAT_UNDEFINED )
    {
        const bool isDepthOnly = DenOfIz_Format_IsDepthOnly( dsvFormat );

        if ( m_desc.Graphics.StencilTest.Enable && isDepthOnly )
        {
            spdlog::warn( "DX12Pipeline: Stencil operations enabled with depth-only format ({}) in mesh pipeline. Stencil operations will be ignored.",
                          static_cast<int>( dsvFormat ) );
        }
    }

    depthStencilDesc.StencilEnable    = m_desc.Graphics.StencilTest.Enable;
    depthStencilDesc.StencilReadMask  = m_desc.Graphics.StencilTest.ReadMask;
    depthStencilDesc.StencilWriteMask = m_desc.Graphics.StencilTest.WriteMask;
    InitStencilFace( depthStencilDesc.FrontFace, m_desc.Graphics.StencilTest.FrontFace );
    InitStencilFace( depthStencilDesc.BackFace, m_desc.Graphics.StencilTest.BackFace );
    meshPipelineStateStream.DepthStencilState = depthStencilDesc;

    D3D12_RT_FORMAT_ARRAY rtvFormats = { };
    rtvFormats.NumRenderTargets      = m_desc.Graphics.RenderTargets.NumElements;
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        rtvFormats.RTFormats[ i ] = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Graphics.RenderTargets.Elements[ i ].Format );
    }
    meshPipelineStateStream.RTVFormats = rtvFormats;
    meshPipelineStateStream.DSVFormat  = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );

    DXGI_SAMPLE_DESC sampleDesc = { };
    switch ( m_desc.Graphics.MSAASampleCount )
    {
    case DENOFIZ_MSAA_SAMPLE_COUNT_0:
    case DENOFIZ_MSAA_SAMPLE_COUNT_1:
        sampleDesc.Count = 1;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_2:
        sampleDesc.Count = 2;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_4:
        sampleDesc.Count = 4;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_8:
        sampleDesc.Count = 8;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_16:
        sampleDesc.Count = 16;
        break;
    case DENOFIZ_MSAA_SAMPLE_COUNT_32:
    case DENOFIZ_MSAA_SAMPLE_COUNT_64:
        sampleDesc.Count = 32;
        break;
    default:
        sampleDesc.Count = 1;
        break;
    }
    sampleDesc.Quality                            = 0;
    meshPipelineStateStream.SampleDesc            = sampleDesc;
    meshPipelineStateStream.PrimitiveTopologyType = DenOfIz_DX12EnumConverter_ConvertPrimitiveTopologyToType( m_desc.Graphics.PrimitiveTopology );

    pipelineDesc.pPipelineStateSubobjectStream = &meshPipelineStateStream;
    pipelineDesc.SizeInBytes                   = sizeof( meshPipelineStateStream );

    DX_CHECK_RESULT( m_context->D3DDevice->CreatePipelineState( &pipelineDesc, IID_PPV_ARGS( m_pipeline.put( ) ) ) );
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
