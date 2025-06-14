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

#include "DenOfIzGraphicsInternal/UI/FullscreenQuadPipeline.h"
#include "DenOfIzGraphicsInternal/UI/FullscreenQuadShaders.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

FullscreenQuadPipeline::FullscreenQuadPipeline( const FullscreenQuadPipelineDesc &desc ) : m_logicalDevice( desc.LogicalDevice )
{
    if ( m_logicalDevice == nullptr )
    {
        spdlog::error( "FullscreenQuadPipeline: LogicalDevice cannot be null" );
        return;
    }

    CreateShaderProgram( );
    CreatePipeline( desc );
    CreateSampler( );
}

void FullscreenQuadPipeline::CreateShaderProgram( )
{
    auto vertexShader = EmbeddedFullscreenQuadShaders::GetFullscreenQuadVertexShaderBytes( );
    auto pixelShader  = EmbeddedFullscreenQuadShaders::GetFullscreenQuadPixelShaderBytes( );

    std::array<ShaderStageDesc, 2> shaderStages( { } );
    ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                          = ShaderStage::Vertex;
    vsDesc.EntryPoint                     = InteropString( "main" );
    vsDesc.Data.Elements                  = vertexShader.data( );
    vsDesc.Data.NumElements               = vertexShader.size( );

    ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.EntryPoint       = InteropString( "main" );
    psDesc.Data.Elements    = pixelShader.data( );
    psDesc.Data.NumElements = pixelShader.size( );

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    m_shaderProgram                      = std::make_unique<ShaderProgram>( programDesc );
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

    RenderTargetDesc renderTarget;
    renderTarget.Format       = desc.OutputFormat;
    renderTarget.Blend.Enable = false;

    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    m_pipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    m_resourceBindGroups.resize( desc.NumFrames );
    for ( uint32_t i = 0; i < desc.NumFrames; ++i )
    {
        ResourceBindGroupDesc bindGroupDesc{ };
        bindGroupDesc.RootSignature = m_rootSignature.get( );
        m_resourceBindGroups[ i ]   = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
    }
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

void FullscreenQuadPipeline::UpdateTarget( const uint32_t frameIndex, ITextureResource *sourceTexture ) const
{
    if ( frameIndex >= m_resourceBindGroups.size( ) )
    {
        spdlog::error( "FullscreenQuadPipeline::UpdateTarget: Invalid frame index {}", frameIndex );
        return;
    }

    if ( sourceTexture == nullptr )
    {
        spdlog::error( "FullscreenQuadPipeline::UpdateTarget: sourceTexture cannot be null" );
        return;
    }

    m_resourceBindGroups[ frameIndex ]->BeginUpdate( )->Srv( 0, sourceTexture )->Sampler( 0, m_linearSampler.get( ) )->EndUpdate( );
}

void FullscreenQuadPipeline::DrawTextureToScreen( ICommandList *commandList, const uint32_t frameIndex ) const
{
    if ( frameIndex >= m_resourceBindGroups.size( ) )
    {
        spdlog::error( "FullscreenQuadPipeline::DrawTextureToScreen: Invalid frame index {}", frameIndex );
        return;
    }

    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroups[ frameIndex ].get( ) );
    commandList->Draw( 3, 1, 0, 0 );
}
