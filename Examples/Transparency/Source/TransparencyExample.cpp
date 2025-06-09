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

using namespace DenOfIz;
using namespace DirectX;

void TransparencyExample::Init( )
{
    {
        BatchResourceCopy batchResourceCopy( m_logicalDevice );
        batchResourceCopy.Begin( );

        m_spheres.push_back( std::make_unique<ColoredSphereAsset>( m_logicalDevice, &batchResourceCopy, XMFLOAT4( 0.9f, 0.2f, 0.2f, 1.0f ) ) ); // Red
        m_spheres.push_back( std::make_unique<ColoredSphereAsset>( m_logicalDevice, &batchResourceCopy, XMFLOAT4( 0.2f, 0.2f, 0.9f, 1.0f ) ) ); // Blue
        m_spheres.push_back( std::make_unique<ColoredSphereAsset>( m_logicalDevice, &batchResourceCopy, XMFLOAT4( 0.6f, 0.8f, 1.0f, 0.5f ) ) ); // Light blue glass

        batchResourceCopy.Submit( );
    }

    TextureDesc depthDesc;
    depthDesc.Width        = m_windowDesc.Width;
    depthDesc.Height       = m_windowDesc.Height;
    depthDesc.InitialUsage = ResourceUsage::DepthWrite;
    depthDesc.Format       = Format::D32Float;
    depthDesc.Descriptor   = ResourceDescriptor::DepthStencil;
    depthDesc.HeapType     = HeapType::GPU;
    depthDesc.DebugName    = "DepthBuffer";
    m_depthBuffer          = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( depthDesc ) );
    m_resourceTracking.TrackTexture( m_depthBuffer.get( ), ResourceUsage::Common );

    m_opaquePipeline      = std::make_unique<ColoredSpherePipeline>( m_graphicsApi, m_logicalDevice, false, 2 );
    m_transparentPipeline = std::make_unique<ColoredSpherePipeline>( m_graphicsApi, m_logicalDevice, true, 1 );

    m_opaquePipeline->UpdateMaterialColor( 0, m_spheres[ 0 ]->GetColor( ) );
    m_opaquePipeline->UpdateMaterialColor( 1, m_spheres[ 1 ]->GetColor( ) );
    m_transparentPipeline->UpdateMaterialColor( 0, m_spheres[ 2 ]->GetColor( ) );

    // Red sphere on the left
    const XMMATRIX sphere1Transform = XMMatrixTranslation( -2.0f, 0.0f, 0.0f );
    XMFLOAT4X4     sphere1Matrix;
    XMStoreFloat4x4( &sphere1Matrix, sphere1Transform );
    m_sphereTransforms.push_back( sphere1Matrix );

    // Blue sphere in the back-center
    const XMMATRIX sphere2Transform = XMMatrixTranslation( 0.0f, 0.0f, -2.0f );
    XMFLOAT4X4     sphere2Matrix;
    XMStoreFloat4x4( &sphere2Matrix, sphere2Transform );
    m_sphereTransforms.push_back( sphere2Matrix );

    // Glass sphere on the right
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

void TransparencyExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );

    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    batchTransitionDesc.TransitionTexture( m_depthBuffer.get( ), ResourceUsage::DepthWrite );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = renderTarget;
    renderingAttachmentDesc.LoadOp   = LoadOp::Clear;
    renderingAttachmentDesc.SetClearColor( 0.0f, 0.0f, 0.0f, 1.0f ); // Black background

    RenderingAttachmentDesc depthAttachmentDesc{ };
    depthAttachmentDesc.Resource = m_depthBuffer.get( );
    depthAttachmentDesc.LoadOp   = LoadOp::Clear;
    depthAttachmentDesc.SetClearDepthStencil( 1.0f, 0.0f );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &renderingAttachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.DepthAttachment           = depthAttachmentDesc;

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

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

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void TransparencyExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void TransparencyExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera    = m_camera.get( );
    m_camera->Update( m_worldData.DeltaTime );

    // Animate the transparent sphere - rotate around Y axis
    const float  rotationSpeed = 0.7f;
    static float totalRotation = 0.0f;
    totalRotation += m_worldData.DeltaTime * rotationSpeed;

    const XMMATRIX rotationMatrix    = XMMatrixRotationY( totalRotation );
    const XMMATRIX translationMatrix = XMMatrixTranslation( 1.5f, 0.0f, 0.0f );
    const XMMATRIX combinedMatrix    = rotationMatrix * translationMatrix;

    XMStoreFloat4x4( &m_sphereTransforms[ 2 ], combinedMatrix );

    constexpr float alphaPulseSpeed = 0.3f; // Units per second
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

void TransparencyExample::HandleEvent( Event &event )
{
    IExample::HandleEvent( event );
    m_camera->HandleEvent( event );
}

void TransparencyExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}
