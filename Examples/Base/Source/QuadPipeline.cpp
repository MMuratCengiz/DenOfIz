/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzExamples/QuadPipeline.h"

using namespace DenOfIz;

QuadPipeline::QuadPipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice logicalDevice, const char *pixelShader )
{
    std::vector<DenOfIz_ShaderStageDesc> shaderStages;
    DenOfIz_ShaderStageDesc             &vertexShaderDesc = shaderStages.emplace_back( );
    vertexShaderDesc.Path                                 = DENOFIZ_STRING( "Assets/Shaders/FullscreenQuad.vs.hlsl" );
    vertexShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vertexShaderDesc.EntryPoint                           = DENOFIZ_STRING( "main" );
    DenOfIz_ShaderStageDesc &pixelShaderDesc              = shaderStages.emplace_back( );
    pixelShaderDesc.Path                                  = DENOFIZ_STRING( pixelShader );
    pixelShaderDesc.Stage                                 = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    pixelShaderDesc.EntryPoint                            = DENOFIZ_STRING( "main" );

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = static_cast<uint32_t>( shaderStages.size( ) );

    m_program = DenOfIz_ShaderProgram_Create( &programDesc );
    DenOfIz_ShaderReflectDesc programReflection{ };
    DenOfIz_ShaderProgram_Reflect( m_program, &programReflection );

    m_bindGroupLayouts.resize( programReflection.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < programReflection.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( logicalDevice, &programReflection.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = programReflection.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( logicalDevice, &rootSigDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( logicalDevice, &programReflection.InputLayout, &m_inputLayout );

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                        = m_inputLayout;
    pipelineDesc.RootSignature                      = m_rootSignature;
    pipelineDesc.ShaderProgram                      = m_program;
    DenOfIz_RenderTargetDesc renderTarget           = { .Blend = { .RenderTargetWriteMask = 0x0F }, .Format = DENOFIZ_FORMAT_B8G8R8A8_UNORM };
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.CullMode                  = DENOFIZ_CULL_MODE_BACK_FACE;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;

    DenOfIz_LogicalDevice_CreatePipeline( logicalDevice, &pipelineDesc, &m_pipeline );

    for ( uint32_t layoutIndex = 0; layoutIndex < m_bindGroupLayouts.size( ); ++layoutIndex )
    {
        DenOfIz_BindGroupDesc bindGroupDesc{ };
        bindGroupDesc.Layout = m_bindGroupLayouts[ layoutIndex ];
        for ( uint32_t i = 0; i < 3; ++i )
        {
            DenOfIz_LogicalDevice_CreateBindGroup( logicalDevice, &bindGroupDesc, &m_bindGroups[ layoutIndex * 3 + i ] );
        }
    }
}

QuadPipeline::~QuadPipeline( )
{
    for ( auto &bindGroup : m_bindGroups )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( bindGroup );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_program ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_program );
    }
}

DenOfIz_Pipeline QuadPipeline::Pipeline( ) const
{
    return m_pipeline;
}

DenOfIz_RootSignature QuadPipeline::RootSignature( ) const
{
    return m_rootSignature;
}

DenOfIz_BindGroup QuadPipeline::BindGroup( const uint32_t frame, const uint32_t registerSpace ) const
{
    return m_bindGroups[ registerSpace * 3 + frame ];
}

void QuadPipeline::Render( DenOfIz_CommandList commandList, const uint32_t frame ) const
{
    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_bindGroups[ frame ] );
    DenOfIz_CommandList_Draw( commandList, 3, 1, 0, 0 );
}
