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
#include "DenOfIzExamples/TransparencyExample.h"
#include <DirectXMath.h>
#include "DenOfIzExamples/ColoredSphereAsset.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

using namespace DenOfIz;
using namespace DirectX;

TransparencyExample::~TransparencyExample( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    for ( auto *sphere : m_spheres )
    {
        delete sphere;
    }
    m_spheres.clear( );

    delete m_opaquePipeline;
    delete m_transparentPipeline;

    if ( DENOFIZ_HANDLE_IS_VALID( m_depthBuffer ) )
    {
        DenOfIz_TextureResource_Destroy( m_depthBuffer );
    }
}

void TransparencyExample::Init( )
{
    {
        DenOfIz_BatchResourceCopy     batchResourceCopy;
        DenOfIz_BatchResourceCopyDesc batchDesc{ };
        batchDesc.Device        = m_logicalDevice;
        batchDesc.IssueBarriers = true;
        DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchResourceCopy );
        DenOfIz_BatchResourceCopy_Begin( batchResourceCopy );

        m_spheres.push_back( new ColoredSphereAsset( m_logicalDevice, batchResourceCopy, XMFLOAT4( 0.9f, 0.2f, 0.2f, 1.0f ) ) ); // Red
        m_spheres.push_back( new ColoredSphereAsset( m_logicalDevice, batchResourceCopy, XMFLOAT4( 0.2f, 0.2f, 0.9f, 1.0f ) ) ); // Blue
        m_spheres.push_back( new ColoredSphereAsset( m_logicalDevice, batchResourceCopy, XMFLOAT4( 0.6f, 0.8f, 1.0f, 0.5f ) ) ); // Light blue glass

        DenOfIz_BatchResourceCopy_Submit( batchResourceCopy, DENOFIZ_NULL_HANDLE );
        DenOfIz_BatchResourceCopy_Destroy( batchResourceCopy );
    }

    DenOfIz_TextureDesc depthDesc{ };
    depthDesc.Width     = m_windowDesc.Width;
    depthDesc.Height    = m_windowDesc.Height;
    depthDesc.Depth     = 1;
    depthDesc.ArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format    = DENOFIZ_FORMAT_D32_FLOAT;
    depthDesc.Usage     = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    depthDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    depthDesc.DebugName = DENOFIZ_STRING( "DepthBuffer" );
    DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &depthDesc, &m_depthBuffer );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_depthBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    m_opaquePipeline      = new ColoredSpherePipeline( m_graphicsApi, m_logicalDevice, false, 2 );
    m_transparentPipeline = new ColoredSpherePipeline( m_graphicsApi, m_logicalDevice, true, 1 );

    m_opaquePipeline->UpdateMaterialColor( 0, m_spheres[ 0 ]->GetColor( ) );
    m_opaquePipeline->UpdateMaterialColor( 1, m_spheres[ 1 ]->GetColor( ) );
    m_transparentPipeline->UpdateMaterialColor( 0, m_spheres[ 2 ]->GetColor( ) );

    const XMMATRIX sphere1Transform = XMMatrixTranslation( -2.0f, 0.0f, 0.0f );
    XMFLOAT4X4     sphere1Matrix;
    XMStoreFloat4x4( &sphere1Matrix, sphere1Transform );
    m_sphereTransforms.push_back( sphere1Matrix );

    const XMMATRIX sphere2Transform = XMMatrixTranslation( 0.0f, 0.0f, -2.0f );
    XMFLOAT4X4     sphere2Matrix;
    XMStoreFloat4x4( &sphere2Matrix, sphere2Transform );
    m_sphereTransforms.push_back( sphere2Matrix );

    const XMMATRIX sphere3Transform = XMMatrixTranslation( 1.5f, 0.0f, 0.0f );
    XMFLOAT4X4     sphere3Matrix;
    XMStoreFloat4x4( &sphere3Matrix, sphere3Transform );
    m_sphereTransforms.push_back( sphere3Matrix );

    auto eye = XMVectorSet( 0.0f, 0.5f, -5.0f, 1.0f );
    m_camera->SetPosition( eye );
    m_camera->SetFront( XMVECTOR{ 0.0f, 0.0f, 1.0f, 0.0f } );

    m_alphaValue     = 0.5f;
    m_alphaDirection = 1;
}

void TransparencyExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );
    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_TransitionTextureDesc textureTransitions[ 2 ]{ };
    textureTransitions[ 0 ].Texture   = renderTarget;
    textureTransitions[ 0 ].NewUsage  = DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT;
    textureTransitions[ 0 ].QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    textureTransitions[ 1 ].Texture   = m_depthBuffer;
    textureTransitions[ 1 ].NewUsage  = DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT;
    textureTransitions[ 1 ].QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Textures.Elements    = textureTransitions;
    batchTransitionDesc.Textures.NumElements = 2;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    DenOfIz_RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource   = renderTarget;
    renderingAttachmentDesc.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    renderingAttachmentDesc.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    DenOfIz_RenderingAttachmentDesc depthAttachmentDesc{ };
    depthAttachmentDesc.Resource          = m_depthBuffer;
    depthAttachmentDesc.LoadOp            = DENOFIZ_LOAD_OP_CLEAR;
    depthAttachmentDesc.ClearDepthStencil = { 1.0f, 0.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &renderingAttachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    renderingDesc.DepthAttachment           = depthAttachmentDesc;

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );

    m_opaquePipeline->UpdateViewProjection( m_camera.get( ) );
    m_transparentPipeline->UpdateViewProjection( m_camera.get( ) );

    m_opaquePipeline->UpdateMaterialColor( 0, m_spheres[ 0 ]->GetColor( ) );
    m_opaquePipeline->UpdateModel( 0, m_sphereTransforms[ 0 ] );
    m_opaquePipeline->Render( 0, commandList, m_spheres[ 0 ]->Data( ) );

    m_opaquePipeline->UpdateMaterialColor( 1, m_spheres[ 1 ]->GetColor( ) );
    m_opaquePipeline->UpdateModel( 1, m_sphereTransforms[ 1 ] );
    m_opaquePipeline->Render( 1, commandList, m_spheres[ 1 ]->Data( ) );

    m_transparentPipeline->UpdateMaterialColor( 0, m_spheres[ 2 ]->GetColor( ) );
    m_transparentPipeline->UpdateModel( 0, m_sphereTransforms[ 2 ] );
    m_transparentPipeline->UpdateAlphaValue( 0, m_alphaValue );
    m_transparentPipeline->Render( 0, commandList, m_spheres[ 2 ]->Data( ) );

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void TransparencyExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
    defaultApiPreference.OSX = DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE;
}

void TransparencyExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.Camera    = m_camera.get( );
    m_camera->Update( m_worldData.DeltaTime );

    const float  rotationSpeed = 0.7f;
    static float totalRotation = 0.0f;
    totalRotation += m_worldData.DeltaTime * rotationSpeed;

    const XMMATRIX rotationMatrix    = XMMatrixRotationY( totalRotation );
    const XMMATRIX translationMatrix = XMMatrixTranslation( 1.5f, 0.0f, 0.0f );
    const XMMATRIX combinedMatrix    = rotationMatrix * translationMatrix;

    XMStoreFloat4x4( &m_sphereTransforms[ 2 ], combinedMatrix );

    constexpr float alphaPulseSpeed = 0.3f;
    m_alphaValue += m_alphaDirection * alphaPulseSpeed * m_worldData.DeltaTime;
    if ( m_alphaValue >= 0.8f )
    {
        m_alphaValue     = 0.8f;
        m_alphaDirection = -1;
    }
    else if ( m_alphaValue <= 0.2f )
    {
        m_alphaValue     = 0.2f;
        m_alphaDirection = 1;
    }

    RenderAndPresentFrame( );
}

void TransparencyExample::HandleEvent( DenOfIz_Event &event )
{
    IExample::HandleEvent( event );
    m_camera->HandleEvent( event );
}

void TransparencyExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}
