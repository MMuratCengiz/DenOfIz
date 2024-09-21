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
#include <DenOfIzCore/Time.h>
#include <DenOfIzExamples/RenderTargetExample.h>

using namespace DenOfIz;

void RenderTargetExample::Init( )
{
    m_commandListRing = std::make_unique<CommandListRing>( m_logicalDevice );
    BatchResourceCopy::SyncOp( m_logicalDevice, [ & ]( BatchResourceCopy *batchResourceCopy ) { m_sphere = std::make_unique<SphereAsset>( m_logicalDevice, batchResourceCopy ); } );

    m_quadPipeline   = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice );
    m_renderPipeline = std::make_unique<DefaultRenderPipeline>( m_graphicsApi, m_logicalDevice );

    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::R32G32B32A32Float;
    textureDesc.Descriptor   = ResourceDescriptor::RWTexture;
    textureDesc.InitialState = ResourceState::RenderTarget;
    m_deferredRenderTarget   = m_logicalDevice->CreateTextureResource( textureDesc );

    auto &materialBatch    = m_worldData.RenderBatch.MaterialBatches.emplace_back( );
    materialBatch.Material = m_sphere->Data( )->MaterialData( );
    auto &sphereRenderItem = materialBatch.RenderItems.emplace_back( );
    sphereRenderItem.Data  = m_sphere->Data( );
    sphereRenderItem.Model = m_sphere->ModelMatrix( );
}

void RenderTargetExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
}

void RenderTargetExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime();
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_commandListRing->NextFrame( );
    const auto     currentCommandList = m_commandListRing->FrameCommandList( 0 );
    const uint32_t currentImageIndex  = m_commandListRing->CurrentImage( m_swapChain.get( ) );
    currentCommandList->Begin( );

    RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = m_swapChain->GetRenderTarget( currentImageIndex );

    RenderingDesc renderingInfo{ };
    renderingInfo.RTAttachments.push_back( renderingAttachmentDesc );

    currentCommandList->PipelineBarrier( PipelineBarrierDesc::UndefinedToRenderTarget( m_swapChain->GetRenderTarget( currentImageIndex ) ) );
    currentCommandList->BeginRendering( renderingInfo );

    const Viewport &viewport = m_swapChain->GetViewport( );
    currentCommandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    currentCommandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    m_renderPipeline->Render( currentCommandList, m_worldData );

    currentCommandList->EndRendering( );
    currentCommandList->PipelineBarrier( PipelineBarrierDesc::RenderTargetToPresent( m_swapChain->GetRenderTarget( currentImageIndex ) ) );
    m_commandListRing->ExecuteAndPresent( currentCommandList, m_swapChain.get( ), currentImageIndex );
}

void RenderTargetExample::HandleEvent( SDL_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RenderTargetExample::Quit( )
{
    m_commandListRing->WaitIdle( );
    IExample::Quit( );
}
