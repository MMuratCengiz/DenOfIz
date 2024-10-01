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

#include <utility>

using namespace DenOfIz;

DX12Pipeline::DX12Pipeline( DX12Context *context, PipelineDesc desc ) : m_context( context ), m_desc( std::move( desc ) )
{
    DZ_NOT_NULL( context );

    DZ_ASSERTM( m_desc.RootSignature != nullptr, "Root signature is not set for the pipeline" );
    DZ_ASSERTM( m_desc.InputLayout != nullptr, "Input layout is not set for the pipeline" );

    m_rootSignature = reinterpret_cast<DX12RootSignature *>( m_desc.RootSignature );

    switch ( m_desc.BindPoint )
    {
    case BindPoint::Graphics:
        CreateGraphicsPipeline( );
        break;
    case BindPoint::Compute:
        CreateComputePipeline( );
        break;
    case BindPoint::RayTracing:
        break;
    }
}

void DX12Pipeline::CreateGraphicsPipeline( )
{
    m_topology             = DX12EnumConverter::ConvertPrimitiveTopology( m_desc.PrimitiveTopology );
    const auto inputLayout = reinterpret_cast<DX12InputLayout *>( m_desc.InputLayout );

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.InputLayout                        = inputLayout->GetInputLayout( );
    psoDesc.pRootSignature                     = m_rootSignature->Instance( );
    SetGraphicsShaders( psoDesc );

    psoDesc.RasterizerState          = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.RasterizerState.CullMode = DX12EnumConverter::ConvertCullMode( m_desc.CullMode );

    InitDepthStencil( psoDesc );

    psoDesc.BlendState.AlphaToCoverageEnable  = m_desc.Rendering.AlphaToCoverageEnable;
    psoDesc.BlendState.IndependentBlendEnable = m_desc.Rendering.IndependentBlendEnable;
    psoDesc.SampleMask                        = UINT_MAX;
    psoDesc.PrimitiveTopologyType             = DX12EnumConverter::ConvertPrimitiveTopologyToType( m_desc.PrimitiveTopology );
    psoDesc.NumRenderTargets                  = m_desc.Rendering.RenderTargets.size( );
    for ( uint32_t i = 0; i < m_desc.Rendering.RenderTargets.size( ); ++i )
    {
        BlendDesc &blendDesc                                       = m_desc.Rendering.RenderTargets[ i ].Blend;
        psoDesc.BlendState.RenderTarget[ i ].BlendEnable           = blendDesc.Enable;
        psoDesc.BlendState.RenderTarget[ i ].LogicOpEnable         = m_desc.Rendering.BlendLogicOpEnable;
        psoDesc.BlendState.RenderTarget[ i ].SrcBlend              = DX12EnumConverter::ConvertBlend( blendDesc.SrcBlend );
        psoDesc.BlendState.RenderTarget[ i ].DestBlend             = DX12EnumConverter::ConvertBlend( blendDesc.DestBlend );
        psoDesc.BlendState.RenderTarget[ i ].BlendOp               = DX12EnumConverter::ConvertBlendOp( blendDesc.BlendOp );
        psoDesc.BlendState.RenderTarget[ i ].SrcBlendAlpha         = DX12EnumConverter::ConvertBlend( blendDesc.SrcBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].DestBlendAlpha        = DX12EnumConverter::ConvertBlend( blendDesc.DestBlendAlpha );
        psoDesc.BlendState.RenderTarget[ i ].BlendOpAlpha          = DX12EnumConverter::ConvertBlendOp( blendDesc.BlendOpAlpha );
        psoDesc.BlendState.RenderTarget[ i ].LogicOp               = DX12EnumConverter::ConvertLogicOp( m_desc.Rendering.BlendLogicOp );
        psoDesc.BlendState.RenderTarget[ i ].RenderTargetWriteMask = blendDesc.RenderTargetWriteMask;

        psoDesc.RTVFormats[ i ] = DX12EnumConverter::ConvertFormat( m_desc.Rendering.RenderTargets[ i ].Format );
    }

    if ( m_desc.Rendering.DepthStencilAttachmentFormat != Format::Undefined )
    {
        psoDesc.DSVFormat = DX12EnumConverter::ConvertFormat( m_desc.Rendering.DepthStencilAttachmentFormat );
    }

    SetMSAASampleCount( m_desc, psoDesc );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( m_graphicsPipeline.put( ) ) ) );
}

void DX12Pipeline::CreateComputePipeline( )
{
    const auto &compiledShaders = m_desc.ShaderProgram->GetCompiledShaders( );
    DZ_ASSERTM( compiledShaders.size( ) == 1, "Compute pipeline must have at least/only one shader" );

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.pRootSignature                    = m_rootSignature->Instance( );
    psoDesc.CS                                = GetShaderByteCode( compiledShaders[ 0 ] );

    DX_CHECK_RESULT( m_context->D3DDevice->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( m_graphicsPipeline.put( ) ) ) );
}

void DX12Pipeline::InitDepthStencil( D3D12_GRAPHICS_PIPELINE_STATE_DESC &psoDesc ) const
{
    psoDesc.DepthStencilState.DepthEnable    = m_desc.DepthTest.Enable;
    psoDesc.DepthStencilState.DepthFunc      = DX12EnumConverter::ConvertCompareOp( m_desc.DepthTest.CompareOp );
    psoDesc.DepthStencilState.DepthWriteMask = m_desc.DepthTest.Write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

    psoDesc.DepthStencilState.StencilEnable    = m_desc.StencilTest.Enable;
    psoDesc.DepthStencilState.StencilReadMask  = m_desc.StencilTest.ReadMask;
    psoDesc.DepthStencilState.StencilWriteMask = m_desc.StencilTest.WriteMask;

    InitStencilFace( psoDesc.DepthStencilState.FrontFace, m_desc.StencilTest.FrontFace );
    InitStencilFace( psoDesc.DepthStencilState.BackFace, m_desc.StencilTest.BackFace );
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
    switch ( desc.MSAASampleCount )
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
    for ( const auto compiledShader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        switch ( compiledShader->Stage )
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

D3D12_SHADER_BYTECODE DX12Pipeline::GetShaderByteCode( const CompiledShader *compiledShader ) const
{
    return D3D12_SHADER_BYTECODE( compiledShader->Blob->GetBufferPointer( ), compiledShader->Blob->GetBufferSize( ) );
}

DX12Pipeline::~DX12Pipeline( )
{
    m_graphicsPipeline.reset( );
}
