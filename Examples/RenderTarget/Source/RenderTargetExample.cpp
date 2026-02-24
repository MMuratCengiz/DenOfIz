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
#include <string>
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

using namespace DenOfIz;

RenderTargetExample::~RenderTargetExample( )
{
    for ( auto &renderTarget : m_deferredRenderTargets )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( renderTarget ) )
        {
            DenOfIz_TextureResource_Destroy( renderTarget );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_defaultSampler ) )
    {
        DenOfIz_Sampler_Destroy( m_defaultSampler );
    }

    for ( auto &semaphore : m_deferredSemaphores )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( semaphore ) )
        {
            DenOfIz_Semaphore_Destroy( semaphore );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_deferredCommandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_deferredCommandListPool );
    }
}

void RenderTargetExample::Init( )
{
    {
        DenOfIz_BatchResourceCopy     batchResourceCopy;
        DenOfIz_BatchResourceCopyDesc batchDesc{ };
        batchDesc.Device        = m_logicalDevice;
        batchDesc.IssueBarriers = true;
        DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchResourceCopy );
        DenOfIz_BatchResourceCopy_Begin( batchResourceCopy );
        m_sphere = std::make_unique<SphereAsset>( m_logicalDevice, batchResourceCopy );
        DenOfIz_BatchResourceCopy_Submit( batchResourceCopy, DENOFIZ_NULL_HANDLE );
        DenOfIz_BatchResourceCopy_Destroy( batchResourceCopy );
    }

    m_quadPipeline   = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice, "Assets/Shaders/SampleBasic.ps.hlsl" );
    m_renderPipeline = std::make_unique<DefaultRenderPipeline>( m_graphicsApi, m_logicalDevice );

    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = m_windowDesc.Width;
    textureDesc.Height    = m_windowDesc.Height;
    textureDesc.Depth     = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format    = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        const std::string debugName  = "Deferred Render Target " + std::to_string( i );
        textureDesc.DebugName        = DENOFIZ_STRING( debugName.c_str( ) );
        DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &renderTarget );
        m_deferredRenderTargets.push_back( renderTarget );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, renderTarget, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }

    DenOfIz_SamplerDesc samplerDesc{ };
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_defaultSampler );

    DenOfIz_BindGroup bindGroup0 = m_quadPipeline->BindGroup( 0 );
    DenOfIz_BindGroup_BeginUpdate( bindGroup0 );
    DenOfIz_BindGroup_SrvTexture( bindGroup0, 0, m_deferredRenderTargets[ 0 ] );
    DenOfIz_BindGroup_Sampler( bindGroup0, 0, m_defaultSampler );
    DenOfIz_BindGroup_EndUpdate( bindGroup0 );

    DenOfIz_BindGroup bindGroup1 = m_quadPipeline->BindGroup( 1 );
    DenOfIz_BindGroup_BeginUpdate( bindGroup1 );
    DenOfIz_BindGroup_SrvTexture( bindGroup1, 0, m_deferredRenderTargets[ 1 ] );
    DenOfIz_BindGroup_Sampler( bindGroup1, 0, m_defaultSampler );
    DenOfIz_BindGroup_EndUpdate( bindGroup1 );

    DenOfIz_BindGroup bindGroup2 = m_quadPipeline->BindGroup( 2 );
    DenOfIz_BindGroup_BeginUpdate( bindGroup2 );
    DenOfIz_BindGroup_SrvTexture( bindGroup2, 0, m_deferredRenderTargets[ 2 ] );
    DenOfIz_BindGroup_Sampler( bindGroup2, 0, m_defaultSampler );
    DenOfIz_BindGroup_EndUpdate( bindGroup2 );

    auto &materialBatch    = m_worldData.Batch.MaterialBatches.emplace_back( m_renderPipeline->GetPerMaterialBinding( ), m_sphere->Data( )->Material( ) );
    auto &sphereRenderItem = materialBatch.RenderItems.emplace_back( );
    sphereRenderItem.Data  = m_sphere->Data( );
    sphereRenderItem.Model = m_sphere->ModelMatrix( );

    DenOfIz_CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue    = m_graphicsQueue;
    commandListPoolDesc.NumCommandLists = 3;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &commandListPoolDesc, &m_deferredCommandListPool );

    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( m_deferredCommandListPool, &commandLists );
    for ( size_t i = 0; i < commandLists.NumElements; ++i )
    {
        m_deferredCommandLists[ i ] = commandLists.Elements[ i ];
        DenOfIz_LogicalDevice_CreateSemaphore( m_logicalDevice, &m_deferredSemaphores[ i ] );
    }

    auto           eye    = XMVectorSet( 0.0f, 0.9f, -3.0f, 1.0f );
    const XMMATRIX rotate = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    eye                   = XMVector3Transform( eye, rotate );

    m_camera->SetPosition( eye );
    m_camera->SetFront( XMVECTOR{ 0.67f, -0.29f, 0.67f, 0.0f } );
}

void RenderTargetExample::RenderDeferredImage( const uint32_t frameIndex )
{
    DenOfIz_CommandList deferredCommandList = m_deferredCommandLists[ frameIndex ];
    DenOfIz_CommandList_Begin( deferredCommandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, deferredCommandList, m_deferredRenderTargets[ frameIndex ], DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT,
                                                DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = m_deferredRenderTargets[ frameIndex ];

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &renderingAttachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;

    DenOfIz_CommandList_BeginRendering( deferredCommandList, &renderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( deferredCommandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( deferredCommandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );

    m_renderPipeline->Render( deferredCommandList, m_worldData );
    DenOfIz_CommandList_EndRendering( deferredCommandList );
    DenOfIz_CommandList_End( deferredCommandList );

    DenOfIz_ExecuteCommandListsDesc executeCommandListsDesc{ };
    executeCommandListsDesc.CommandLists.Elements        = &deferredCommandList;
    executeCommandListsDesc.CommandLists.NumElements     = 1;
    DenOfIz_Semaphore semaphore                          = m_deferredSemaphores[ frameIndex ];
    executeCommandListsDesc.SignalSemaphores.Elements    = &semaphore;
    executeCommandListsDesc.SignalSemaphores.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( m_graphicsQueue, &executeCommandListsDesc );
}

void RenderTargetExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );
    {
        uint32_t imageIndex = 0;
        DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

        DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
        DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

        DenOfIz_TransitionTextureDesc textureTransitions[ 2 ] = { };
        textureTransitions[ 0 ].Texture                       = m_deferredRenderTargets[ frameIndex ];
        textureTransitions[ 0 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;
        textureTransitions[ 0 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;
        textureTransitions[ 1 ].Texture                       = renderTarget;
        textureTransitions[ 1 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT;
        textureTransitions[ 1 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;

        DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
        batchTransitionDesc.Textures.Elements    = textureTransitions;
        batchTransitionDesc.Textures.NumElements = 2;
        DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

        DenOfIz_RenderingAttachmentDesc quadAttachmentDesc{ };
        quadAttachmentDesc.Resource = renderTarget;

        DenOfIz_RenderingDesc quadRenderingDesc{ };
        quadRenderingDesc.RTAttachments.Elements    = &quadAttachmentDesc;
        quadRenderingDesc.RTAttachments.NumElements = 1;

        DenOfIz_CommandList_BeginRendering( commandList, &quadRenderingDesc );

        const DenOfIz_Viewport *viewport = nullptr;
        DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
        DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
        DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
        m_quadPipeline->Render( commandList, frameIndex );

        DenOfIz_CommandList_EndRendering( commandList );

        DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }
    DenOfIz_CommandList_End( commandList );
}

void RenderTargetExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
}

void RenderTargetExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    uint32_t frameIndex = 0;
    DenOfIz_FrameSync_NextFrame( m_frameSync, &frameIndex );
    RenderDeferredImage( frameIndex );

    DenOfIz_CommandList commandList = DENOFIZ_NULL_HANDLE;
    DenOfIz_FrameSync_GetCommandList( m_frameSync, frameIndex, &commandList );
    Render( frameIndex, commandList );

    DenOfIz_Semaphore      deferredSemaphore = m_deferredSemaphores[ frameIndex ];
    DenOfIz_SemaphoreArray additionalSemaphores{ };
    additionalSemaphores.Elements    = &deferredSemaphore;
    additionalSemaphores.NumElements = 1;
    DenOfIz_FrameSync_ExecuteCommandList( m_frameSync, frameIndex, &additionalSemaphores );
    Present( frameIndex );
}

void RenderTargetExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RenderTargetExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}
