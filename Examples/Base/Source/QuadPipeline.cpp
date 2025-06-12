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

#include "DenOfIzExamples/QuadPipeline.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"

using namespace DenOfIz;

QuadPipeline::QuadPipeline( const GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice, const char *pixelShader )
{
    std::vector<ShaderStageDesc> shaderStages;
    ShaderStageDesc             &vertexShaderDesc = shaderStages.emplace_back( );
    vertexShaderDesc.Path                         = "Assets/Shaders/FullscreenQuad.vs.hlsl";
    vertexShaderDesc.Stage                        = ShaderStage::Vertex;
    ShaderStageDesc &pixelShaderDesc              = shaderStages.emplace_back( );
    pixelShaderDesc.Path                          = pixelShader;
    pixelShaderDesc.Stage                         = ShaderStage::Pixel;

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );

    auto program           = std::make_unique<ShaderProgram>( programDesc );
    auto programReflection = program->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature( programReflection.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( programReflection.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.ShaderProgram = program.get( );
    pipelineDesc.Graphics.RenderTargets.AddElement( { .Format = Format::B8G8R8A8Unorm } );
    pipelineDesc.Graphics.CullMode = CullMode::BackFace;

    m_pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );

    for ( int bindingIndex = 0; bindingIndex < programReflection.RootSignature.ResourceBindings.NumElements; ++bindingIndex )
    {
        const auto &resourceBinding = programReflection.RootSignature.ResourceBindings.Elements[ bindingIndex ];
        bindGroupDesc.RegisterSpace = resourceBinding.RegisterSpace;
        for ( uint32_t i = 0; i < 3; ++i )
        {
            m_bindGroups[ i ] = std::unique_ptr<IResourceBindGroup>( logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
        }
    }
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
