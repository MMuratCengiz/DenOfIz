/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
            DenOfIz_Texture_Destroy( renderTarget );
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

    DenOfIz_SamplerDesc samplerDesc{ };
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_defaultSampler );

    m_deferredRenderTargets.assign( 3, DENOFIZ_NULL_HANDLE );
    CreateDeferredRenderTargets( );

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
        DenOfIz_Texture renderTarget = m_frame.BackBuffer;

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

void RenderTargetExample::CreateDeferredRenderTargets( )
{
    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Depth     = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format    = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;

    for ( uint32_t i = 0; i < m_deferredRenderTargets.size( ); ++i )
    {
        const std::string debugName = "Deferred Render Target " + std::to_string( i );
        textureDesc.DebugName       = DENOFIZ_STRING( debugName.c_str( ) );
        RecreateScreenSizedTexture( m_deferredRenderTargets[ i ], textureDesc );

        DenOfIz_BindGroup bindGroup = m_quadPipeline->BindGroup( i );
        DenOfIz_BindGroup_BeginUpdate( bindGroup );
        DenOfIz_BindGroup_SrvTexture( bindGroup, 0, m_deferredRenderTargets[ i ] );
        DenOfIz_BindGroup_Sampler( bindGroup, 0, m_defaultSampler );
        DenOfIz_BindGroup_EndUpdate( bindGroup );
    }
}

void RenderTargetExample::OnResize( const uint32_t width, const uint32_t height )
{
    IExample::OnResize( width, height );
    CreateDeferredRenderTargets( );
}

void RenderTargetExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
}

void RenderTargetExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    uint32_t frameIndex = 0;
    if ( !BeginFrame( frameIndex ) )
    {
        return;
    }
    RenderDeferredImage( frameIndex );

    DenOfIz_CommandList commandList = DENOFIZ_NULL_HANDLE;
    DenOfIz_FrameSync_GetCommandList( m_frameSync, frameIndex, &commandList );
    Render( frameIndex, commandList );

    DenOfIz_Semaphore      deferredSemaphore = m_deferredSemaphores[ frameIndex ];
    DenOfIz_SemaphoreArray additionalSemaphores{ };
    additionalSemaphores.Elements    = &deferredSemaphore;
    additionalSemaphores.NumElements = 1;
    EndFrame( &additionalSemaphores );
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
