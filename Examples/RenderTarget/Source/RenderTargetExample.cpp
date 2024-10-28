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
#include <DenOfIzExamples/RenderTargetExample.h>
#include <DenOfIzGraphics/Utilities/Time.h>

using namespace DenOfIz;

void RenderTargetExample::Init( )
{
    {
        BatchResourceCopy batchResourceCopy( m_logicalDevice );
        batchResourceCopy.Begin( );
        m_sphere = std::make_unique<SphereAsset>( m_logicalDevice, &batchResourceCopy );
        batchResourceCopy.Submit( );
    }

    m_quadPipeline   = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice, "Assets/Shaders/SampleBasic.ps.hlsl" );
    m_renderPipeline = std::make_unique<DefaultRenderPipeline>( m_graphicsApi, m_logicalDevice );

    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture ) | ResourceDescriptor::RenderTarget;
    textureDesc.InitialUsage = ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "Deferred Render Target";
    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName.Append( "Deferred Render Target " ).Append( std::to_string( i ).c_str( ) );
        m_deferredRenderTargets.push_back( std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) ) );
    }
    m_defaultSampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( SamplerDesc{ } ) );
    m_quadPipeline->BindGroup( 0 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 0 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_quadPipeline->BindGroup( 1 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 1 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_quadPipeline->BindGroup( 2 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 2 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );

    auto &materialBatch    = m_worldData.RenderBatch.MaterialBatches.emplace_back( m_renderPipeline->PerMaterialBinding( ), m_sphere->Data( )->Material( ) );
    auto &sphereRenderItem = materialBatch.RenderItems.emplace_back( );
    sphereRenderItem.Data  = m_sphere->Data( );
    sphereRenderItem.Model = m_sphere->ModelMatrix( );

    RenderGraphDesc renderGraphDesc{ };
    renderGraphDesc.GraphicsApi   = m_graphicsApi;
    renderGraphDesc.LogicalDevice = m_logicalDevice;
    renderGraphDesc.SwapChain     = m_swapChain.get( );

    m_renderGraph = std::make_unique<RenderGraph>( renderGraphDesc );

    NodeDesc deferredNode{ };
    deferredNode.Name = "Deferred";
    deferredNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_deferredRenderTargets[ 0 ].get( ), ResourceUsage::RenderTarget ) );
    deferredNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_deferredRenderTargets[ 1 ].get( ), ResourceUsage::RenderTarget ) );
    deferredNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_deferredRenderTargets[ 2 ].get( ), ResourceUsage::RenderTarget ) );
    deferredNode.Execute = this;

    PresentNodeDesc presentNode{ };
    presentNode.SwapChain = m_swapChain.get( );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_deferredRenderTargets[ 0 ].get( ), ResourceUsage::ShaderResource ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_deferredRenderTargets[ 1 ].get( ), ResourceUsage::ShaderResource ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_deferredRenderTargets[ 2 ].get( ), ResourceUsage::ShaderResource ) );
    presentNode.Dependencies.AddElement( "Deferred" );
    presentNode.Execute = this;

    m_renderGraph->AddNode( deferredNode );
    m_renderGraph->SetPresentNode( presentNode );
    m_renderGraph->BuildGraph( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

// Node execution
void RenderTargetExample::Execute( uint32_t frameIndex, ICommandList *commandList )
{
    RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = m_deferredRenderTargets[ frameIndex ].get( );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( renderingAttachmentDesc );

    commandList->BeginRendering( renderingDesc );

    const Viewport &viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_renderPipeline->Render( commandList, m_worldData );

    commandList->EndRendering( );
}

void RenderTargetExample::Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget )
{
    RenderingAttachmentDesc quadAttachmentDesc{ };
    quadAttachmentDesc.Resource = renderTarget;

    RenderingDesc quadRenderingDesc{ };
    quadRenderingDesc.RTAttachments.AddElement( quadAttachmentDesc );

    commandList->BeginRendering( quadRenderingDesc );

    const Viewport &viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_quadPipeline->Render( commandList, frameIndex );
    commandList->EndRendering( );
}

void RenderTargetExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::DirectX12;
}

void RenderTargetExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_renderGraph->Update( );
}

void RenderTargetExample::HandleEvent( SDL_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RenderTargetExample::Quit( )
{
    m_renderGraph->WaitIdle( );
    IExample::Quit( );
}
