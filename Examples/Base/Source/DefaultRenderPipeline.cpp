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

#include "DenOfIzExamples/DefaultRenderPipeline.h"

using namespace DenOfIz;

DefaultRenderPipeline::DefaultRenderPipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice logicalDevice )
{
    std::vector<DenOfIz_ShaderStageDesc> shaderStages{ };
    DenOfIz_ShaderStageDesc             &vertexShaderDesc = shaderStages.emplace_back( );
    vertexShaderDesc.Path                                 = DENOFIZ_STRING( "Assets/Shaders/DefaultRenderPipeline.vs.hlsl" );
    vertexShaderDesc.EntryPoint                           = DENOFIZ_STRING( "main" );
    vertexShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    DenOfIz_ShaderStageDesc &pixelShaderDesc              = shaderStages.emplace_back( );
    pixelShaderDesc.Path                                  = DENOFIZ_STRING( "Assets/Shaders/DefaultRenderPipeline.ps.hlsl" );
    pixelShaderDesc.EntryPoint                            = DENOFIZ_STRING( "main" );
    pixelShaderDesc.Stage                                 = DENOFIZ_SHADER_STAGE_PIXEL_BIT;

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
        m_spaceToLayoutIndex[ programReflection.BindGroupLayouts.Elements[ i ].RegisterSpace ] = i;
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
    pipelineDesc.Graphics.CullMode                  = DENOFIZ_CULL_MODE_BACK_FACE;
    DenOfIz_RenderTargetDesc renderTarget           = { .Blend = { .RenderTargetWriteMask = 0x0F }, .Format = DENOFIZ_FORMAT_B8G8R8A8_UNORM };
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;

    DenOfIz_LogicalDevice_CreatePipeline( logicalDevice, &pipelineDesc, &m_pipeline );

    m_perFrameBinding    = std::make_unique<PerFrameBinding>( logicalDevice, m_bindGroupLayouts[ m_spaceToLayoutIndex[ 0 ] ] );
    m_perMaterialBinding = std::make_unique<class PerMaterialBinding>( logicalDevice, m_bindGroupLayouts[ m_spaceToLayoutIndex[ 1 ] ] );
    m_perDrawBinding     = std::make_unique<PerDrawBinding>( logicalDevice, m_bindGroupLayouts[ m_spaceToLayoutIndex[ DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE ] ] );
}

DefaultRenderPipeline::~DefaultRenderPipeline( )
{
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

void DefaultRenderPipeline::Render( DenOfIz_CommandList commandList, const WorldData &worldData ) const
{
    m_perFrameBinding->Update( worldData.Camera, worldData.DeltaTime );
    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_perFrameBinding->BindGroup( ) );

    for ( auto &materialBatch : worldData.Batch.MaterialBatches )
    {
        DenOfIz_CommandList_BindGroup( commandList, materialBatch.MaterialBinding->BindGroup( ) );
        for ( const auto &renderItem : materialBatch.RenderItems )
        {
            m_perDrawBinding->Update( renderItem.Model );
            DenOfIz_CommandList_BindGroup( commandList, m_perDrawBinding->BindGroup( ) );
            DenOfIz_CommandList_BindVertexBuffer( commandList, renderItem.Data->VertexBuffer( ), 0, 0, 0 );
            DenOfIz_CommandList_BindIndexBuffer( commandList, renderItem.Data->IndexBuffer( ), DENOFIZ_INDEX_TYPE_UINT32, 0 );
            DenOfIz_CommandList_DrawIndexed( commandList, renderItem.Data->NumIndices( ), 1, 0, 0, 0 );
        }
    }
}
PerMaterialBinding *DefaultRenderPipeline::GetPerMaterialBinding( ) const
{
    return m_perMaterialBinding.get( );
}
