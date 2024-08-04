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

    void SimpleRenderer::Init( GraphicsWindowHandle *window )
    {
        m_window = window;
        GraphicsAPI::SetAPIPreference( APIPreference{
            .Windows = APIPreferenceWindows::Vulkan,
        } );

        m_logicalDevice     = GraphicsAPI::CreateAndLoadOptimalLogicalDevice( );
        m_batchResourceCopy = std::make_unique<BatchResourceCopy>( m_logicalDevice.get( ) );

        m_program       = std::make_unique<ShaderProgram>( ShaderProgramDesc{ .Shaders = { ShaderDesc{ .Stage = ShaderStage::Vertex, .Path = "Assets/Shaders/vs.hlsl" },
                                                                                           ShaderDesc{ .Stage = ShaderStage::Pixel, .Path = "Assets/Shaders/fs.hlsl" } } } );
        m_rootSignature = m_logicalDevice->CreateRootSignature( RootSignatureDesc{
            .ResourceBindings = {
                ResourceBindingDesc{ .Name = "model", .Binding = 0, .RegisterSpace = 1, .Descriptor = ResourceDescriptor::UniformBuffer, .Stages = { ShaderStage::Vertex } },
                ResourceBindingDesc{ .Name = "viewProjection", .Binding = 0, .Descriptor = ResourceDescriptor::UniformBuffer, .Stages = { ShaderStage::Vertex } },
                ResourceBindingDesc{ .Name = "time", .Binding = 1, .Descriptor = ResourceDescriptor::UniformBuffer, .Stages = { ShaderStage::Vertex } },
                ResourceBindingDesc{ .Name = "texture1", .Binding = 0, .RegisterSpace = 1, .Descriptor = ResourceDescriptor::Texture, .Stages = { ShaderStage::Pixel } },
                ResourceBindingDesc{ .Name = "sampler1", .Binding = 0, .RegisterSpace = 1, .Descriptor = ResourceDescriptor::Sampler, .Stages = { ShaderStage::Pixel } },
            } } );

        m_inputLayout = m_logicalDevice->CreateInputLayout( VertexPositionNormalTexture::InputLayout );

        const GraphicsWindowSurface &surface = m_window->GetSurface( );
        m_swapChain =
            m_logicalDevice->CreateSwapChain( SwapChainDesc{ .WindowHandle = m_window, .Width = surface.Width, .Height = surface.Height, .BufferCount = mc_framesInFlight } );

        PipelineDesc pipelineDesc{ .ShaderProgram = m_program.get( ) };
        pipelineDesc.BlendModes    = { BlendMode::None };
        pipelineDesc.RootSignature = m_rootSignature.get( );
        pipelineDesc.InputLayout   = m_inputLayout.get( );
        pipelineDesc.Rendering.ColorAttachmentFormats.push_back( m_swapChain->GetPreferredFormat( ) );

        m_pipeline        = m_logicalDevice->CreatePipeline( pipelineDesc );
        m_commandListRing = std::make_unique<CommandListRing>( m_logicalDevice.get( ) );

        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences.push_back( m_logicalDevice->CreateFence( ) );
            m_imageReadySemaphores.push_back( m_logicalDevice->CreateSemaphore( ) );
            m_imageRenderedSemaphores.push_back( m_logicalDevice->CreateSemaphore( ) );
        }

        UpdateMVPMatrix( );

        BufferDesc deltaTimeBufferDesc{ };
        deltaTimeBufferDesc.HeapType   = HeapType::CPU_GPU;
        deltaTimeBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
        deltaTimeBufferDesc.NumBytes   = sizeof( float );

        float timePassed         = 1.0f;
        m_timePassedBuffer       = m_logicalDevice->CreateBufferResource( "time", deltaTimeBufferDesc );
        m_mappedTimePassedBuffer = m_timePassedBuffer->MapMemory( );
        memcpy( m_mappedTimePassedBuffer, &timePassed, sizeof( float ) );

        XMStoreFloat4x4( &m_identityMatrix, XMMatrixIdentity( ) );
        XMStoreFloat4x4( &m_planeModelMatrix, XMMatrixTranslation( 0.0f, -5.0f, 0.0f ) );

        BatchResourceCopyHelper copyHelper( m_logicalDevice.get( ), m_batchResourceCopy.get( ) );
        copyHelper.Begin( );
        copyHelper.CreateUniformBuffer( "model", &m_identityMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_sphereModelMatrixBuffer );
        copyHelper.CreateUniformBuffer( "model", &m_planeModelMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_planeModelMatrixBuffer );
        copyHelper.CreateUniformBuffer( "viewProjection", &m_mvpMatrix, sizeof( XMFLOAT4X4 ) ).Into( m_viewProjectionMatrixBuffer );
        copyHelper.CreateGeometryBuffers( m_sphere ).Into( m_sphereVb, m_sphereIb );
        copyHelper.CreateGeometryBuffers( m_plane ).Into( m_planeVb, m_planeIb );
        copyHelper.CreateSampler( "sampler1", SamplerDesc{ } ).Into( m_sphereSampler );
        copyHelper.CreateSampler( "sampler1", SamplerDesc{ } ).Into( m_planeSampler );
        copyHelper.CreateTexture( "texture1", "Assets/Textures/Dracolich.png" ).Into( m_sphereTexture );
        copyHelper.CreateTexture( "texture1", "Assets/Textures/test-dxt5.dds" ).Into( m_planeTexture );
        copyHelper.Submit( );

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
            UpdateDesc updateDesc{ };
            updateDesc.Buffer( "model", m_sphereModelMatrixBuffer.get( ) ).Texture( "texture1", m_sphereTexture.get( ) ).Sampler( "sampler1", m_sphereSampler.get( ) );
            m_sphereModelBindGroup->Update( updateDesc );
            updateDesc = { };
            updateDesc.Buffer( "model", m_planeModelMatrixBuffer.get( ) ).Texture( "texture1", m_planeTexture.get( ) ).Sampler( "sampler1", m_planeSampler.get( ) );
            m_planeModelBindGroup->Update( updateDesc );
            updateDesc = { };
            updateDesc.Buffer( "viewProjection", m_viewProjectionMatrixBuffer.get( ) ).Buffer( "time", m_timePassedBuffer.get( ) );
            m_perCameraBindGroup->Update( updateDesc );
        }

        m_time->ListenFps = []( const double fps ) { DLOG( INFO ) << std::format( "FPS: {}", fps ); };

        LOG( INFO ) << "Initialization Complete.";
    }

    void SimpleRenderer::UpdateMVPMatrix( )
    {
        constexpr XMFLOAT3 eyePosition = XMFLOAT3( 0.0f, -1.0f, -2.0f );
        constexpr XMFLOAT3 focusPoint  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
        constexpr XMFLOAT3 upDirection = XMFLOAT3( 0.0f, 1.0f, 0.0f );
        constexpr float    aspectRatio = 800.0f / 600.0f;
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
        const float timePassed = std::fmax( 1.0f, ( m_time->DoubleEpochNow( ) - m_time->GetFirstTickTime( ) ) / 1000000.0f );
        memcpy( m_mappedTimePassedBuffer, &timePassed, sizeof( float ) );
        m_time->Tick( );

        const auto     nextCommandList = m_commandListRing->GetNext( );
        const uint32_t currentFrame    = m_commandListRing->GetCurrentFrame( );
        m_fences[ currentFrame ]->Wait( );

        const uint32_t nextImage = m_swapChain->AcquireNextImage( m_imageReadySemaphores[ currentFrame ].get( ) );
        nextCommandList->Begin( );

        RenderingAttachmentDesc renderingAttachmentDesc{ };
        renderingAttachmentDesc.Resource = m_swapChain->GetRenderTarget( nextImage );

        RenderingDesc renderingInfo{ };
        renderingInfo.RTAttachments.push_back( renderingAttachmentDesc );

        nextCommandList->PipelineBarrier( PipelineBarrierDesc::UndefinedToRenderTarget( m_swapChain->GetRenderTarget( nextImage ) ) );
        nextCommandList->BeginRendering( renderingInfo );

        const Viewport &viewport = m_swapChain->GetViewport( );
        nextCommandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
        nextCommandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
        nextCommandList->BindPipeline( m_pipeline.get( ) );
        nextCommandList->BindResourceGroup( m_perCameraBindGroup.get( ) );

        { // Draw the sphere
            nextCommandList->BindResourceGroup( m_sphereModelBindGroup.get( ) );
            nextCommandList->BindVertexBuffer( m_sphereVb.get( ) );
            nextCommandList->BindIndexBuffer( m_sphereIb.get( ), IndexType::Uint32 );
            nextCommandList->DrawIndexed( m_sphere.Indices.size( ), 1 );
        }

        { // Draw the plane
            nextCommandList->BindResourceGroup( m_planeModelBindGroup.get( ) );
            nextCommandList->BindVertexBuffer( m_planeVb.get( ) );
            nextCommandList->BindIndexBuffer( m_planeIb.get( ), IndexType::Uint32 );
            nextCommandList->DrawIndexed( m_plane.Indices.size( ), 1 );
        }

        nextCommandList->EndRendering( );
        nextCommandList->PipelineBarrier( PipelineBarrierDesc::RenderTargetToPresent( m_swapChain->GetRenderTarget( nextImage ) ) );

        ExecuteDesc submitInfo{ };
        submitInfo.Notify = m_fences[ currentFrame ].get( );
        submitInfo.WaitOnSemaphores.push_back( m_imageReadySemaphores[ currentFrame ].get( ) );
        submitInfo.NotifySemaphores.push_back( m_imageRenderedSemaphores[ currentFrame ].get( ) );
        nextCommandList->Execute( submitInfo );
        nextCommandList->Present( m_swapChain.get( ), nextImage, { m_imageRenderedSemaphores[ currentFrame ].get( ) } );
    }

    void SimpleRenderer::Quit( ) const
    {
        m_timePassedBuffer->UnmapMemory( );
        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences[ i ]->Wait( );
        }
        m_logicalDevice->WaitIdle( );
        GfxGlobal::Destroy( );
    }

} // namespace DenOfIz
