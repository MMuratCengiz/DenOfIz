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
#include "DenOfIzExamples/RenderTargetExample.h"
#include "DenOfIzGraphics/Utilities/Time.h"

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
    textureDesc.Width      = m_windowDesc.Width;
    textureDesc.Height     = m_windowDesc.Height;
    textureDesc.Format     = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor = BitSet( ResourceDescriptor::Texture ) | ResourceDescriptor::RenderTarget;
    // textureDesc.InitialUsage = ResourceUsage::ShaderResource;
    textureDesc.DebugName = "Deferred Render Target";
    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName.Append( "Deferred Render Target " ).Append( std::to_string( i ).c_str( ) );
        m_deferredRenderTargets.push_back( std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) ) );
        m_resourceTracking.TrackTexture( m_deferredRenderTargets[ i ].get( ), ResourceUsage::Common );
    }
    m_defaultSampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( SamplerDesc{ } ) );
    m_quadPipeline->BindGroup( 0 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 0 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_quadPipeline->BindGroup( 1 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 1 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_quadPipeline->BindGroup( 2 )->BeginUpdate( )->Srv( 0, m_deferredRenderTargets[ 2 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );

    auto &materialBatch    = m_worldData.RenderBatch.MaterialBatches.emplace_back( m_renderPipeline->PerMaterialBinding( ), m_sphere->Data( )->Material( ) );
    auto &sphereRenderItem = materialBatch.RenderItems.emplace_back( );
    sphereRenderItem.Data  = m_sphere->Data( );
    sphereRenderItem.Model = m_sphere->ModelMatrix( );

    m_time.OnEachSecond = []( const double fps ) { spdlog::warn("FPS: {}", fps); };

    CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue    = m_graphicsQueue.get( );
    commandListPoolDesc.NumCommandLists = m_deferredCommandLists.size( );
    m_deferredCommandListPool           = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( commandListPoolDesc ) );

    auto commandLists = m_deferredCommandListPool->GetCommandLists( );
    for ( int i = 0; i < commandLists.NumElements( ); ++i )
    {
        m_deferredCommandLists[ i ] = commandLists.GetElement( i );
        m_deferredSemaphores[ i ]   = std::unique_ptr<ISemaphore>( m_logicalDevice->CreateSemaphore( ) );
    }

    auto           eye    = XMVectorSet( 0.0f, 0.9f, -3.0f, 1.0f );
    const XMMATRIX rotate = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    eye                   = XMVector3Transform( eye, rotate );

    m_camera->SetPosition( eye );

    // Lin
    m_camera->SetFront( XMVECTOR{ 0.67f, -0.29f, 0.67f, 0.0f } );
}

void RenderTargetExample::RenderDeferredImage( const uint32_t frameIndex )
{
    const auto &m_deferredCommandList = m_deferredCommandLists[ frameIndex ];
    m_deferredCommandList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ m_deferredCommandList };
    batchTransitionDesc.TransitionTexture( m_deferredRenderTargets[ frameIndex ].get( ), ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = m_deferredRenderTargets[ frameIndex ].get( );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( renderingAttachmentDesc );

    m_deferredCommandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    m_deferredCommandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_deferredCommandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_renderPipeline->Render( m_deferredCommandList, m_worldData );
    m_deferredCommandList->EndRendering( );
    m_deferredCommandList->End( );

    ExecuteCommandListsDesc executeCommandListsDesc{ };
    executeCommandListsDesc.CommandLists.AddElement( m_deferredCommandList );
    executeCommandListsDesc.SignalSemaphores.AddElement( m_deferredSemaphores[ frameIndex ].get( ) );
    m_graphicsQueue->ExecuteCommandLists( executeCommandListsDesc );
}

void RenderTargetExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{

    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_deferredRenderTargets[ frameIndex ].get( ), ResourceUsage::ShaderResource );
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc quadAttachmentDesc{ };
    quadAttachmentDesc.Resource = renderTarget;

    RenderingDesc quadRenderingDesc{ };
    quadRenderingDesc.RTAttachments.AddElement( quadAttachmentDesc );

    commandList->BeginRendering( quadRenderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_quadPipeline->Render( commandList, frameIndex );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void RenderTargetExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void RenderTargetExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    // Custom RenderAndPresentFrame for additional semaphores
    const uint64_t frameIndex = m_frameSync->NextFrame( );
    RenderDeferredImage( frameIndex );
    Render( frameIndex, m_frameSync->GetCommandList( frameIndex ) );
    InteropArray<ISemaphore *> additionalSemaphores{ };
    additionalSemaphores.AddElement( m_deferredSemaphores[ frameIndex ].get( ) );
    m_frameSync->ExecuteCommandList( frameIndex, additionalSemaphores );
    Present( frameIndex );
}

void RenderTargetExample::HandleEvent( Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RenderTargetExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}
