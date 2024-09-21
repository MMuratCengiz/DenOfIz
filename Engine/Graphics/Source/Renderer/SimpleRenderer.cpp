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

#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>

namespace DenOfIz
{

    SimpleRenderer::SimpleRenderer( GraphicsApi *gApi, ILogicalDevice *logicalDevice ) : m_gApi( gApi ), m_logicalDevice( logicalDevice )
    {
    }

    void SimpleRenderer::Init( GraphicsWindowHandle *window )
    {
        m_window = window;

        m_program = m_gApi->CreateShaderProgram(
            { ShaderDesc{ .Stage = ShaderStage::Vertex, .Path = "Assets/Shaders/vs.hlsl" }, ShaderDesc{ .Stage = ShaderStage::Pixel, .Path = "Assets/Shaders/fs.hlsl" } } );

        ShaderReflectDesc reflection = m_program->Reflect( );
        m_rootSignature              = m_logicalDevice->CreateRootSignature( reflection.RootSignature );
        m_inputLayout                = m_logicalDevice->CreateInputLayout( VertexPositionNormalTexture::InputLayout );

        const GraphicsWindowSurface &surface = m_window->GetSurface( );
        m_swapChain                          = m_logicalDevice->CreateSwapChain( SwapChainDesc{ .WindowHandle = m_window, .Width = surface.Width, .Height = surface.Height } );

        PipelineDesc pipelineDesc{ .ShaderProgram = m_program.get( ) };
        pipelineDesc.BlendModes    = { BlendMode::None };
        pipelineDesc.RootSignature = m_rootSignature.get( );
        pipelineDesc.InputLayout   = m_inputLayout.get( );
        pipelineDesc.Rendering.ColorAttachmentFormats.push_back( m_swapChain->GetPreferredFormat( ) );

        m_pipeline        = m_logicalDevice->CreatePipeline( pipelineDesc );
        m_commandListRing = std::make_unique<CommandListRing>( m_logicalDevice );

        UpdateMVPMatrix( );

        BufferDesc deltaTimeBufferDesc{ };
        deltaTimeBufferDesc.HeapType   = HeapType::CPU_GPU;
        deltaTimeBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
        deltaTimeBufferDesc.NumBytes   = sizeof( float );
        deltaTimeBufferDesc.DebugName  = "TimePassedBuffer";

        float timePassed         = 1.0f;
        m_timePassedBuffer       = m_logicalDevice->CreateBufferResource( deltaTimeBufferDesc );
        m_mappedTimePassedBuffer = m_timePassedBuffer->MapMemory( );
        memcpy( m_mappedTimePassedBuffer, &timePassed, sizeof( float ) );

        XMStoreFloat4x4( &m_identityMatrix, XMMatrixIdentity( ) );
        XMStoreFloat4x4( &m_planeModelMatrix, XMMatrixTranslation( 0.0f, -5.0f, 0.0f ) );

        BatchResourceCopy       batchResourceCopy( m_logicalDevice );
        batchResourceCopy.Begin( );
        batchResourceCopy.CreateAndStoreUniformBuffer( &m_identityMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_sphereModelMatrixBuffer );
        batchResourceCopy.CreateAndStoreUniformBuffer( &m_planeModelMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_planeModelMatrixBuffer );
        batchResourceCopy.CreateAndStoreUniformBuffer( &m_mvpMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_viewProjectionMatrixBuffer );
        batchResourceCopy.CreateAndStoreGeometryBuffers( m_sphere ).Into( m_sphereVb, m_sphereIb );
        batchResourceCopy.CreateAndStoreGeometryBuffers( m_plane ).Into( m_planeVb, m_planeIb );
        batchResourceCopy.CreateAndStoreSampler( SamplerDesc{ } ).Into( m_sphereSampler );
        batchResourceCopy.CreateAndStoreSampler( SamplerDesc{ } ).Into( m_planeSampler );
        batchResourceCopy.CreateAndStoreTexture( "Assets/Textures/Dracolich.png" ).Into( m_sphereTexture );
        batchResourceCopy.CreateAndStoreTexture( "Assets/Textures/test-dxt5.dds" ).Into( m_planeTexture );
        batchResourceCopy.Submit( );

        ResourceBindGroupDesc bindGroupDesc = { };
        bindGroupDesc.RootSignature         = m_rootSignature.get( );
        bindGroupDesc.RegisterSpace         = 0;
        bindGroupDesc.NumBuffers            = 2;
        m_perCameraBindGroup                = m_logicalDevice->CreateResourceBindGroup( bindGroupDesc );
        bindGroupDesc.RegisterSpace         = 1;
        bindGroupDesc.NumBuffers            = 1;
        bindGroupDesc.NumTextures           = 1;
        bindGroupDesc.NumSamplers           = 1;
        m_sphereModelBindGroup              = m_logicalDevice->CreateResourceBindGroup( bindGroupDesc );
        m_planeModelBindGroup               = m_logicalDevice->CreateResourceBindGroup( bindGroupDesc );

        { // Update the bind groups, TODO, can the model bindings be merged somehow?
            UpdateDesc updateDesc = UpdateDesc( 1 ).Cbv( 0, m_sphereModelMatrixBuffer.get( ) ).Srv( 0, m_sphereTexture.get( ) ).Sampler( 0, m_sphereSampler.get( ) );
            m_sphereModelBindGroup->Update( updateDesc );
            updateDesc = UpdateDesc( 1 ).Cbv( 0, m_planeModelMatrixBuffer.get( ) ).Srv( 0, m_planeTexture.get( ) ).Sampler( 0, m_planeSampler.get( ) );
            m_planeModelBindGroup->Update( updateDesc );
            updateDesc = UpdateDesc( 0 ).Cbv( 0, m_viewProjectionMatrixBuffer.get( ) ).Cbv( 1, m_timePassedBuffer.get( ) );
            m_perCameraBindGroup->Update( updateDesc );
        }

        m_time->OnEachSecond = []( const double fps ) { DLOG( INFO ) << std::format( "FPS: {}", fps ); };

        LOG( INFO ) << "Initialization Complete.";
    }

    void SimpleRenderer::UpdateMVPMatrix( )
    {
        constexpr XMFLOAT3 eyePosition = XMFLOAT3( 0.0f, -1.0f, -2.0f );
        constexpr XMFLOAT3 focusPoint  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
        constexpr XMFLOAT3 upDirection = XMFLOAT3( 0.0f, 1.0f, 0.0f );
        float              aspectRatio = m_window->GetSurface( ).Width / m_window->GetSurface( ).Height;
        constexpr float    nearZ       = 0.1f;
        constexpr float    farZ        = 100.0f;

        // Set up the matrices
        const XMMATRIX modelMatrix      = XMMatrixIdentity( );
        const XMMATRIX viewMatrix       = XMMatrixLookAtLH( XMLoadFloat3( &eyePosition ), XMLoadFloat3( &focusPoint ), XMLoadFloat3( &upDirection ) );
        const XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV4, aspectRatio, nearZ, farZ );

        // Compute the MVP matrix
        const XMMATRIX mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;
        XMStoreFloat4x4( &m_mvpMatrix, XMMatrixTranspose( mvpMatrix ) );
    }

    void SimpleRenderer::Render( ) const
    {
        const float timePassed = std::fmax( 1.0f, ( Time::DoubleEpochNow( ) - m_time->GetFirstTickTime( ) ) / 1000000.0f );
        memcpy( m_mappedTimePassedBuffer, &timePassed, sizeof( float ) );
        m_time->Tick( );

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
        currentCommandList->BindPipeline( m_pipeline.get( ) );
        currentCommandList->BindResourceGroup( m_perCameraBindGroup.get( ) );

        { // Draw the sphere
            currentCommandList->BindResourceGroup( m_sphereModelBindGroup.get( ) );
            currentCommandList->BindVertexBuffer( m_sphereVb.get( ) );
            currentCommandList->BindIndexBuffer( m_sphereIb.get( ), IndexType::Uint32 );
            currentCommandList->DrawIndexed( m_sphere.Indices.size( ), 1 );
        }

        { // Draw the plane
            currentCommandList->BindResourceGroup( m_planeModelBindGroup.get( ) );
            currentCommandList->BindVertexBuffer( m_planeVb.get( ) );
            currentCommandList->BindIndexBuffer( m_planeIb.get( ), IndexType::Uint32 );
            currentCommandList->DrawIndexed( m_plane.Indices.size( ), 1 );
        }

        currentCommandList->EndRendering( );
        currentCommandList->PipelineBarrier( PipelineBarrierDesc::RenderTargetToPresent( m_swapChain->GetRenderTarget( currentImageIndex ) ) );
        m_commandListRing->ExecuteAndPresent( currentCommandList, m_swapChain.get( ), currentImageIndex );
    }

    void SimpleRenderer::Quit( ) const
    {
        m_timePassedBuffer->UnmapMemory( );
        m_commandListRing->WaitIdle( );
        m_logicalDevice->WaitIdle( );
    }

} // namespace DenOfIz
