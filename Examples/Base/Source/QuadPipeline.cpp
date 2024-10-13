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

    m_program              = std::unique_ptr<ShaderProgram>( graphicsApi->CreateShaderProgram( shaders ) );
    auto programReflection = m_program->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature( programReflection.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( programReflection.InputLayout ) );
    PipelineDesc pipelineDesc{ };
    pipelineDesc.Rendering.RenderTargets.push_back( { .Format = Format::B8G8R8A8Unorm } );
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.ShaderProgram = m_program.get( );
    pipelineDesc.CullMode      = CullMode::BackFace;

    m_pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );

    for ( const auto &resourceBinding : programReflection.RootSignature.ResourceBindings )
    {
        bindGroupDesc.RegisterSpace = resourceBinding.RegisterSpace;
        for ( uint32_t i = 0; i < 3; ++i )
        {
            m_bindGroups.push_back( std::unique_ptr<IResourceBindGroup>( logicalDevice->CreateResourceBindGroup( bindGroupDesc ) ) );
        }
    }

    m_sampler = std::unique_ptr<ISampler>( logicalDevice->CreateSampler( SamplerDesc{ } ) );
}

IPipeline *QuadPipeline::Pipeline( ) const
{
    return m_pipeline.get( );
}

IRootSignature *QuadPipeline::RootSignature( ) const
{
    return m_rootSignature.get( );
}

IResourceBindGroup *QuadPipeline::BindGroup( const uint32_t frame, const uint32_t registerSpace ) const
{
    return m_bindGroups[ registerSpace * 3 + frame ].get( );
}

void QuadPipeline::Render( ICommandList *commandList, const uint32_t frame ) const
{
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_bindGroups[ frame ].get( ) );
    commandList->Draw( 3, 1, 0, 0 );
}
