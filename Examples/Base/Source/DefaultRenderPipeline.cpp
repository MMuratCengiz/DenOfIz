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
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"

using namespace DenOfIz;

DefaultRenderPipeline::DefaultRenderPipeline( const GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice )
{
    std::vector<ShaderStageDesc> shaderStages{ };
    ShaderStageDesc             &vertexShaderDesc = shaderStages.emplace_back( );
    vertexShaderDesc.Path                         = "Assets/Shaders/DefaultRenderPipeline.vs.hlsl";
    vertexShaderDesc.Stage                        = ShaderStage::Vertex;
    ShaderStageDesc &pixelShaderDesc              = shaderStages.emplace_back( );
    pixelShaderDesc.Path                          = "Assets/Shaders/DefaultRenderPipeline.ps.hlsl";
    pixelShaderDesc.Stage                         = ShaderStage::Pixel;

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );

    m_program              = std::make_unique<ShaderProgram>( programDesc );
    auto programReflection = m_program->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature( programReflection.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( programReflection.InputLayout ) );
    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                        = m_inputLayout.get( );
    pipelineDesc.RootSignature                      = m_rootSignature.get( );
    pipelineDesc.ShaderProgram                      = m_program.get( );
    pipelineDesc.Graphics.CullMode                  = CullMode::BackFace;
    RenderTargetDesc renderTarget                   = { .Format = Format::B8G8R8A8Unorm };
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    m_pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );

    m_perDrawBinding     = std::make_unique<PerDrawBinding>( logicalDevice, m_rootSignature.get( ) );
    m_perFrameBinding    = std::make_unique<PerFrameBinding>( logicalDevice, m_rootSignature.get( ) );
    m_perMaterialBinding = std::make_unique<class PerMaterialBinding>( logicalDevice, m_rootSignature.get( ) );
}

void DefaultRenderPipeline::Render( ICommandList *commandList, const WorldData &worldData ) const
{
    m_perFrameBinding->Update( worldData.Camera, worldData.DeltaTime );
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_perFrameBinding->BindGroup( ) );

    for ( auto &materialBatch : worldData.RenderBatch.MaterialBatches )
    {
        commandList->BindResourceGroup( materialBatch.MaterialBinding->BindGroup( ) );
        for ( const auto &renderItem : materialBatch.RenderItems )
        {
            m_perDrawBinding->Update( renderItem.Model );
            commandList->BindResourceGroup( m_perDrawBinding->BindGroup( ) );
            commandList->BindVertexBuffer( renderItem.Data->VertexBuffer( ) );
            commandList->BindIndexBuffer( renderItem.Data->IndexBuffer( ), IndexType::Uint32 );
            commandList->DrawIndexed( renderItem.Data->NumIndices( ), 1, 0, 0, 0 );
        }
    }
}
PerMaterialBinding *DefaultRenderPipeline::PerMaterialBinding( ) const
{
    return m_perMaterialBinding.get( );
}
