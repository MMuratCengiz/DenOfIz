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

#include <DenOfIzGraphics/UI/FullscreenQuadPipeline.h>
#include <DenOfIzGraphics/UI/FullscreenQuadShaders.h>
#include <DenOfIzGraphics/Utilities/Common.h>

using namespace DenOfIz;

FullscreenQuadPipeline::FullscreenQuadPipeline( const FullscreenQuadPipelineDesc &desc ) : m_logicalDevice( desc.LogicalDevice )
{
    if ( m_logicalDevice == nullptr )
    {
        LOG( ERROR ) << "FullscreenQuadPipeline: LogicalDevice cannot be null";
        return;
    }

    CreateShaderProgram( );
    CreatePipeline( desc );
    CreateSampler( );
}

void FullscreenQuadPipeline::CreateShaderProgram( )
{
    ShaderProgramDesc programDesc{ };

    ShaderStageDesc &vsDesc = programDesc.ShaderStages.EmplaceElement( );
    vsDesc.Stage            = ShaderStage::Vertex;
    vsDesc.EntryPoint       = InteropString( "main" );
    vsDesc.Data             = EmbeddedFullscreenQuadShaders::GetFullscreenQuadVertexShaderBytes( );

    ShaderStageDesc &psDesc = programDesc.ShaderStages.EmplaceElement( );
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.EntryPoint       = InteropString( "main" );
    psDesc.Data             = EmbeddedFullscreenQuadShaders::GetFullscreenQuadPixelShaderBytes( );

    m_shaderProgram = std::make_unique<ShaderProgram>( programDesc );
}

void FullscreenQuadPipeline::CreatePipeline( const FullscreenQuadPipelineDesc &desc )
{
    const ShaderReflectDesc reflectDesc = m_shaderProgram->Reflect( );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.InputLayout   = nullptr;
    pipelineDesc.ShaderProgram = m_shaderProgram.get( );
    pipelineDesc.BindPoint     = BindPoint::Graphics;

    RenderTargetDesc &renderTarget = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Format            = desc.OutputFormat;
    renderTarget.Blend.Enable      = false;

    m_pipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    m_resourceBindGroup         = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
}

void FullscreenQuadPipeline::CreateSampler( )
{
    SamplerDesc samplerDesc{ };
    samplerDesc.MagFilter    = Filter::Linear;
    samplerDesc.MinFilter    = Filter::Linear;
    samplerDesc.MipmapMode   = MipmapMode::Linear;
    samplerDesc.AddressModeU = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeV = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeW = SamplerAddressMode::ClampToEdge;
    m_linearSampler          = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );
}

void FullscreenQuadPipeline::DrawTextureToScreen( ICommandList *commandList, ITextureResource *sourceTexture ) const
{
    if ( sourceTexture == nullptr )
    {
        LOG( ERROR ) << "FullscreenQuadPipeline::DrawTextureToScreen: sourceTexture cannot be null";
        return;
    }

    // Todo move this to a separate UpdateTarget() call, we should also accept separate target for each frame(and require frameIndex to this call)
    m_resourceBindGroup->BeginUpdate( )->Srv( 0, sourceTexture )->Sampler( 0, m_linearSampler.get( ) )->EndUpdate( );
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->Draw( 3, 1, 0, 0 );
}
