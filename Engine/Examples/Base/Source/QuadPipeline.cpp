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

#include <DenOfIzExamples/QuadPipeline.h>

using namespace DenOfIz;

QuadPipeline::QuadPipeline( const GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice, const std::string &pixelShader )
{
    std::vector<ShaderDesc> shaders{ };
    ShaderDesc              vertexShaderDesc{ };
    vertexShaderDesc.Path  = "Assets/Shaders/FullscreenQuad.vs.hlsl";
    vertexShaderDesc.Stage = ShaderStage::Vertex;
    shaders.push_back( vertexShaderDesc );
    ShaderDesc pixelShaderDesc{ };
    pixelShaderDesc.Path  = pixelShader;
    pixelShaderDesc.Stage = ShaderStage::Pixel;
    shaders.push_back( pixelShaderDesc );

    m_program              = graphicsApi->CreateShaderProgram( shaders );
    auto programReflection = m_program->Reflect( );

    m_rootSignature = logicalDevice->CreateRootSignature( programReflection.RootSignature );
    m_inputLayout   = logicalDevice->CreateInputLayout( programReflection.InputLayout );
    PipelineDesc pipelineDesc{ };
    pipelineDesc.Rendering.RenderTargets.push_back( { .Format = Format::B8G8R8A8Unorm } );
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.ShaderProgram = m_program.get( );
    pipelineDesc.CullMode      = CullMode::None;

    m_pipeline = logicalDevice->CreatePipeline( pipelineDesc );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    for ( uint32_t i = 0; i < 3; ++i )
    {
        m_bindGroups.push_back( logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
    }
    m_sampler = logicalDevice->CreateSampler( SamplerDesc{ } );
}

IResourceBindGroup *QuadPipeline::BindGroup( const uint32_t frame ) const
{
    return m_bindGroups[ frame ].get( );
}

void QuadPipeline::Render( ICommandList *commandList, const uint32_t frame ) const
{
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_bindGroups[ frame ].get( ) );
    commandList->Draw( 3, 1, 0, 0 );
}
