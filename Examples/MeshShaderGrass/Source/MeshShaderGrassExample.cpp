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

#include "DenOfIzExamples/MeshShaderGrassExample.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

using namespace DirectX;
using namespace DenOfIz;

void MeshShaderGrassExample::Init( )
{
    m_camera->SetPosition( XMVECTOR{ 0.0f, 10.0f, -20.0f, 1.0f } ); // Higher and further back for better view
    m_camera->SetFront( XMVECTOR{ 0.0f, -0.3f, 1.0f, 0.0f } );      // Slightly steeper angle

    CreateConstantsBuffer( );
    LoadGrassTexture( );
    CreateTerrainGeometry( );
    LoadTerrainTexture( );
    CreateTerrainPipeline( );
    CreateMeshShaderPipeline( );
}

void MeshShaderGrassExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // Use DirectX12 for mesh shader support
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void MeshShaderGrassExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_camera->Update( m_worldData.DeltaTime );

    // Update time and animation parameters
    if ( m_animateWind )
    {
        m_elapsedTime += static_cast<float>( m_stepTimer.GetElapsedSeconds( ) );
    }

    UpdateConstants( );
    RenderAndPresentFrame( );
}

void MeshShaderGrassExample::Render( uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );
    ITextureResource *depthBuffer  = m_depthBuffer.get( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    batchTransitionDesc.TransitionTexture( depthBuffer, ResourceUsage::DepthWrite );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource        = renderTarget;
    attachmentDesc.LoadOp          = LoadOp::Clear;
    attachmentDesc.ClearColor[ 0 ] = 0.1f; // Sky blue
    attachmentDesc.ClearColor[ 1 ] = 0.4f;
    attachmentDesc.ClearColor[ 2 ] = 0.7f;
    attachmentDesc.ClearColor[ 3 ] = 1.0f;

    RenderingAttachmentDesc depthAttachmentDesc{ };
    depthAttachmentDesc.Resource               = depthBuffer;
    depthAttachmentDesc.LoadOp                 = LoadOp::Clear;
    depthAttachmentDesc.ClearDepthStencil[ 0 ] = 1.0f;
    depthAttachmentDesc.ClearDepthStencil[ 1 ] = 0.0f;

    RenderingDesc renderingDesc;
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.DepthAttachment           = depthAttachmentDesc;

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    // Then render the grass on top using mesh shader
    commandList->BindPipeline( m_meshPipeline.get( ) );
    commandList->BindResourceGroup( m_meshBindGroup.get( ) );

    // Dispatch mesh shader with the desired patch count
    // Parameters are grid dimensions X, Y, Z
    // Increased grid size for better coverage and overlapping patches
    commandList->DispatchMesh( 64, 64, 1 );
    // First render the terrain
    commandList->BindPipeline( m_terrainPipeline.get( ) );
    commandList->BindResourceGroup( m_terrainBindGroup.get( ) );

    // Bind vertex and index buffers for the terrain
    commandList->BindVertexBuffer( m_terrainVertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_terrainIndexBuffer.get( ), IndexType::Uint32 );

    commandList->DrawIndexed( m_terrainGeometry->Indices.NumElements, 1, 0 );
    // Draw the terrain

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void MeshShaderGrassExample::HandleEvent( Event &event )
{
    switch ( event.Type )
    {
    case EventType::KeyDown:
        {
            switch ( event.Key.Keycode )
            {
            case KeyCode::Return:
                m_animateWind = !m_animateWind;
                spdlog::info( "Wind animation {}", ( m_animateWind ? "enabled" : "disabled" ) );
                break;

            default:
                break;
            }
            break;
        }
    default:
        break;
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void MeshShaderGrassExample::Quit( )
{
    m_frameSync->WaitIdle( );

    if ( m_grassConstants )
    {
        m_grassConstantsBuffer->UnmapMemory( );
        m_grassConstants = nullptr;
    }

    IExample::Quit( );
}

void MeshShaderGrassExample::CreateMeshShaderPipeline( )
{
    std::array<ShaderStageDesc, 2> shaderStages( { } );

    ShaderStageDesc &meshShaderDesc = shaderStages[ 0 ];
    meshShaderDesc.Stage            = ShaderStage::Mesh;
    meshShaderDesc.Path             = "Assets/Shaders/GrassShader/GrassMS.hlsl";
    meshShaderDesc.EntryPoint       = "main";

    ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.Path             = "Assets/Shaders/GrassShader/GrassPS.hlsl";
    pixelShaderDesc.EntryPoint       = "main";

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_meshShaderProgram                  = std::make_unique<ShaderProgram>( programDesc );

    auto reflection = m_meshShaderProgram->Reflect( );

    m_meshRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint     = BindPoint::Mesh;
    pipelineDesc.RootSignature = m_meshRootSignature.get( );
    pipelineDesc.ShaderProgram = m_meshShaderProgram.get( );

    pipelineDesc.Graphics.PrimitiveTopology            = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode                     = CullMode::None;
    pipelineDesc.Graphics.FillMode                     = FillMode::Solid;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = Format::D32Float;
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = CompareOp::Less;

    RenderTargetDesc rtDesc{ };
    rtDesc.Format                                   = Format::B8G8R8A8Unorm;
    rtDesc.Blend.Enable                             = true;
    rtDesc.Blend.SrcBlend                           = Blend::SrcAlpha;
    rtDesc.Blend.DstBlend                           = Blend::InvSrcAlpha;
    rtDesc.Blend.SrcBlendAlpha                      = Blend::One;
    rtDesc.Blend.DstBlendAlpha                      = Blend::Zero;
    pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    m_meshPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_meshRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    m_meshBindGroup             = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    m_meshBindGroup->BeginUpdate( );
    m_meshBindGroup->Cbv( 0, m_grassConstantsBuffer.get( ) );
    m_meshBindGroup->Srv( 0, m_grassTexture.get( ) );
    m_meshBindGroup->Sampler( 0, m_grassSampler.get( ) );
    m_meshBindGroup->EndUpdate( );
}

void MeshShaderGrassExample::CreateConstantsBuffer( )
{
    // Create constants buffer
    BufferDesc constantsDesc{ };
    constantsDesc.HeapType   = HeapType::CPU_GPU;
    constantsDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    constantsDesc.NumBytes   = sizeof( GrassConstants );
    constantsDesc.Usages     = ResourceUsage::CopyDst | ResourceUsage::VertexAndConstantBuffer;
    constantsDesc.DebugName  = "GrassConstantsBuffer";
    m_grassConstantsBuffer   = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( constantsDesc ) );
    m_grassConstants         = static_cast<GrassConstants *>( m_grassConstantsBuffer->MapMemory( ) );

    m_grassConstants->WindDirection       = { 1.0f, 0.0f, 0.0f, 0.5f };
    m_grassConstants->GrassColor          = { 0.42f, 0.85f, 0.27f, 1.0f };
    m_grassConstants->GrassColorVariation = { 0.18f, 0.15f, 0.1f, 0.0f }; // Increased color variation for natural look
    m_grassConstants->Time                = 0.0f;
    m_grassConstants->DensityFactor       = 64.0f;
    m_grassConstants->HeightScale         = 1.0f;
    m_grassConstants->WidthScale          = 0.06f;
    m_grassConstants->MaxDistance         = 50.0f;
    m_grassConstants->TerrainScale        = 0.2f;
    m_grassConstants->TerrainHeight       = 3.0f;
    m_grassConstants->TerrainRoughness    = 0.7f;

    m_grassConstants->Model          = XMMatrixIdentity( );
    m_grassConstants->ViewProjection = XMMatrixIdentity( );

    TextureDesc depthDesc{ };
    depthDesc.Width        = m_windowDesc.Width;
    depthDesc.Height       = m_windowDesc.Height;
    depthDesc.Format       = Format::D32Float;
    depthDesc.Descriptor   = ResourceDescriptor::DepthStencil;
    depthDesc.InitialUsage = ResourceUsage::DepthWrite;
    depthDesc.Usages       = ResourceUsage::DepthWrite | ResourceUsage::DepthRead;
    depthDesc.DebugName    = "DepthBuffer";
    m_depthBuffer          = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( depthDesc ) );
    m_resourceTracking.TrackTexture( m_depthBuffer.get( ), ResourceUsage::DepthWrite );
}

void MeshShaderGrassExample::LoadGrassTexture( )
{
    SamplerDesc samplerDesc{ };
    samplerDesc.MinFilter    = Filter::Linear;
    samplerDesc.MagFilter    = Filter::Linear;
    samplerDesc.MipmapMode   = MipmapMode::Linear;
    samplerDesc.AddressModeU = SamplerAddressMode::ClampToEdge; // Or Repeat, etc.
    samplerDesc.AddressModeV = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeW = SamplerAddressMode::ClampToEdge;
    samplerDesc.DebugName    = "GrassSampler";
    m_grassSampler           = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );

    TextureDesc textureDesc{ };
    textureDesc.Width        = 128;
    textureDesc.Height       = 128;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = ResourceDescriptor::Texture;
    textureDesc.InitialUsage = ResourceUsage::CopyDst;
    textureDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "GrassTexture";

    m_grassTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    m_resourceTracking.TrackTexture( m_grassTexture.get( ), ResourceUsage::CopyDst );

    std::vector<uint8_t> textureData( textureDesc.Width * textureDesc.Height * 4 );
    for ( uint32_t y = 0; y < textureDesc.Height; ++y )
    {
        for ( uint32_t x = 0; x < textureDesc.Width; ++x )
        {
            uint32_t idx = ( y * textureDesc.Width + x ) * 4;

            float centerX = static_cast<float>( x ) / textureDesc.Width - 0.5f;
            float centerY = static_cast<float>( y ) / textureDesc.Height;

            float distanceFromCenter = std::abs( centerX ) * 2.0f;
            float alphaEdge          = 1.0f - std::min( 1.0f, distanceFromCenter * 1.8f );

            // Height-based alpha with slower fade
            float alphaHeight = std::pow( std::max( 0.0f, 1.0f - centerY ), 0.7f );

            // Add subtle noise for texture variation
            float noiseValue = std::sin( x * 0.2f + y * 0.3f ) * 0.1f + 0.9f;

            // Tip detail - add some subtle fraying at the tip
            if ( centerY > 0.85f )
            {
                // Add frayed edges at the tip
                float tipDetail = std::sin( x * 0.8f ) * 0.7f + 0.3f;
                alphaEdge *= tipDetail;
            }

            // Inner blade detail - add subtle veins
            float veinPattern = std::abs( centerX ) < 0.1f ? 1.1f : 1.0f;

            // Combine all factors for final alpha
            float alpha = alphaEdge * alphaHeight * noiseValue;

            // Base color with subtle variation - slightly greenish instead of pure white
            // to allow for better tinting in the shader
            textureData[ idx + 0 ] = static_cast<uint8_t>( 220 * noiseValue );               // R
            textureData[ idx + 1 ] = static_cast<uint8_t>( 225 * noiseValue * veinPattern ); // G - slightly higher for subtle green tint
            textureData[ idx + 2 ] = static_cast<uint8_t>( 215 * noiseValue );               // B
            textureData[ idx + 3 ] = static_cast<uint8_t>( alpha * 255.0f );                 // A
        }
    }

    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    CopyDataToTextureDesc copyDesc{ };
    copyDesc.DstTexture       = m_grassTexture.get( );
    copyDesc.Data.Elements    = textureData.data( );
    copyDesc.Data.NumElements = textureData.size( );
    batchResourceCopy.CopyDataToTexture( copyDesc );
    batchResourceCopy.Submit( );

    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { m_graphicsQueue.get( ) } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).Elements[ 0 ];
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_grassTexture.get( ), ResourceUsage::ShaderResource, QueueType::Graphics );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = syncFence.get( );
    m_graphicsQueue->ExecuteCommandLists( executeDesc );

    syncFence->Wait( );
}

void MeshShaderGrassExample::CreateTerrainGeometry( )
{
    QuadDesc quadDesc;
    quadDesc.Width     = 100.0f;
    quadDesc.Height    = 100.0f;
    quadDesc.BuildDesc = BuildDesc::BuildNormal | BuildDesc::BuildTexCoord;
    m_terrainGeometry.reset( Geometry::BuildQuadXZ( quadDesc ) );

    BufferDesc vertexDesc{ };
    vertexDesc.HeapType   = HeapType::GPU;
    vertexDesc.Descriptor = ResourceDescriptor::VertexBuffer;
    vertexDesc.NumBytes   = m_terrainGeometry->Vertices.NumElements * sizeof( GeometryVertexData );
    vertexDesc.Usages     = ResourceUsage::CopyDst | ResourceUsage::VertexAndConstantBuffer;
    vertexDesc.DebugName  = "TerrainVertexBuffer";
    m_terrainVertexBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vertexDesc ) );

    BufferDesc indexDesc{ };
    indexDesc.HeapType   = HeapType::GPU;
    indexDesc.Descriptor = ResourceDescriptor::IndexBuffer;
    indexDesc.NumBytes   = m_terrainGeometry->Indices.NumElements * sizeof( uint32_t );
    indexDesc.Usages     = ResourceUsage::CopyDst | ResourceUsage::IndexBuffer;
    indexDesc.DebugName  = "TerrainIndexBuffer";
    m_terrainIndexBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( indexDesc ) );

    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    CopyToGpuBufferDesc vertexCopyDesc{ };
    vertexCopyDesc.DstBuffer        = m_terrainVertexBuffer.get( );
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( m_terrainGeometry->Vertices.Elements );
    vertexCopyDesc.Data.NumElements = m_terrainGeometry->Vertices.NumElements * sizeof( GeometryVertexData );
    batchResourceCopy.CopyToGPUBuffer( vertexCopyDesc );

    CopyToGpuBufferDesc indexCopyDesc{ };
    indexCopyDesc.DstBuffer        = m_terrainIndexBuffer.get( );
    indexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( m_terrainGeometry->Indices.Elements );
    indexCopyDesc.Data.NumElements = m_terrainGeometry->Indices.NumElements * sizeof( uint32_t );
    batchResourceCopy.CopyToGPUBuffer( indexCopyDesc );

    batchResourceCopy.Submit( );
}

void MeshShaderGrassExample::LoadTerrainTexture( )
{
    SamplerDesc samplerDesc{ };
    samplerDesc.MinFilter    = Filter::Linear;
    samplerDesc.MagFilter    = Filter::Linear;
    samplerDesc.MipmapMode   = MipmapMode::Linear;
    samplerDesc.AddressModeU = SamplerAddressMode::Repeat;
    samplerDesc.AddressModeV = SamplerAddressMode::Repeat;
    samplerDesc.AddressModeW = SamplerAddressMode::Repeat;
    samplerDesc.DebugName    = "TerrainSampler";
    m_terrainSampler         = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );

    TextureDesc textureDesc{ };
    textureDesc.Width        = 256;
    textureDesc.Height       = 256;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = ResourceDescriptor::Texture;
    textureDesc.InitialUsage = ResourceUsage::CopyDst;
    textureDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "TerrainTexture";

    m_terrainTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    m_resourceTracking.TrackTexture( m_terrainTexture.get( ), ResourceUsage::CopyDst );

    std::vector<uint8_t> textureData( textureDesc.Width * textureDesc.Height * 4 );
    for ( uint32_t y = 0; y < textureDesc.Height; ++y )
    {
        for ( uint32_t x = 0; x < textureDesc.Width; ++x )
        {
            uint32_t idx = ( y * textureDesc.Width + x ) * 4;

            // Create a noise-based texture for terrain
            float noiseX = static_cast<float>( x ) / textureDesc.Width;
            float noiseY = static_cast<float>( y ) / textureDesc.Height;

            // Simple Perlin-like noise function
            float noise = sin( noiseX * 10 ) * cos( noiseY * 10 ) * 0.25f + sin( noiseX * 25 + noiseY * 20 ) * cos( noiseY * 15 - noiseX * 15 ) * 0.15f + 0.6f;

            // Soil base color (brown)
            float soilR = 0.35f + noise * 0.15f;
            float soilG = 0.25f + noise * 0.1f;
            float soilB = 0.15f + noise * 0.05f;

            // Add some green for grass patches
            float grassNoise = sin( noiseX * 35 + 1.3f ) * cos( noiseY * 35 + 2.4f ) * 0.5f + 0.5f;
            if ( grassNoise > 0.55f )
            {
                // Blend with grass color
                float grassBlend = ( grassNoise - 0.55f ) * 2.2f; // 0-1 range
                grassBlend       = std::min( grassBlend, 0.8f );

                soilR = soilR * ( 1 - grassBlend ) + 0.2f * grassBlend;
                soilG = soilG * ( 1 - grassBlend ) + 0.5f * grassBlend;
                soilB = soilB * ( 1 - grassBlend ) + 0.1f * grassBlend;
            }

            // Store final color
            textureData[ idx + 0 ] = static_cast<uint8_t>( soilR * 255.0f );
            textureData[ idx + 1 ] = static_cast<uint8_t>( soilG * 255.0f );
            textureData[ idx + 2 ] = static_cast<uint8_t>( soilB * 255.0f );
            textureData[ idx + 3 ] = 255; // Fully opaque
        }
    }

    // Copy data to texture
    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    CopyDataToTextureDesc copyDesc{ };
    copyDesc.DstTexture       = m_terrainTexture.get( );
    copyDesc.Data.Elements    = textureData.data( );
    copyDesc.Data.NumElements = textureData.size( );
    batchResourceCopy.CopyDataToTexture( copyDesc );
    batchResourceCopy.Submit( );

    // Transition texture to shader resource
    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { m_graphicsQueue.get( ) } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).Elements[ 0 ];
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_terrainTexture.get( ), ResourceUsage::ShaderResource, QueueType::Graphics );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = syncFence.get( );
    m_graphicsQueue->ExecuteCommandLists( executeDesc );

    syncFence->Wait( );
}

void MeshShaderGrassExample::CreateTerrainPipeline( )
{
    std::array<ShaderStageDesc, 2> shaderStages( { } );

    ShaderStageDesc &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Stage            = ShaderStage::Vertex;
    vertexShaderDesc.Path             = "Assets/Shaders/TerrainShader/TerrainVS.hlsl";
    vertexShaderDesc.EntryPoint       = "main";

    ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.Path             = "Assets/Shaders/TerrainShader/TerrainPS.hlsl";
    pixelShaderDesc.EntryPoint       = "main";

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_terrainShaderProgram               = std::make_unique<ShaderProgram>( programDesc );

    auto reflection = m_terrainShaderProgram->Reflect( );

    m_terrainRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );
    m_terrainInputLayout   = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflection.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint     = BindPoint::Graphics;
    pipelineDesc.RootSignature = m_terrainRootSignature.get( );
    pipelineDesc.InputLayout   = m_terrainInputLayout.get( );
    pipelineDesc.ShaderProgram = m_terrainShaderProgram.get( );

    pipelineDesc.Graphics.PrimitiveTopology            = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode                     = CullMode::BackFace;
    pipelineDesc.Graphics.FillMode                     = FillMode::Solid;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = Format::D32Float;
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = CompareOp::Less;

    RenderTargetDesc rtDesc{ };
    rtDesc.Format                                   = Format::B8G8R8A8Unorm;
    pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    m_terrainPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_terrainRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    m_terrainBindGroup          = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    m_terrainBindGroup->BeginUpdate( );
    m_terrainBindGroup->Cbv( 0, m_grassConstantsBuffer.get( ) ); // Use same constants buffer
    m_terrainBindGroup->Srv( 0, m_terrainTexture.get( ) );
    m_terrainBindGroup->Sampler( 0, m_terrainSampler.get( ) );
    m_terrainBindGroup->EndUpdate( );
}

void MeshShaderGrassExample::UpdateConstants( ) const
{
    m_grassConstants->Time = m_elapsedTime;

    // Create more natural wind patterns with multiple frequencies
    const float primaryWindAngle   = m_elapsedTime * 0.3f;
    const float secondaryWindAngle = m_elapsedTime * 0.17f; // Different frequency for variation

    // Primary wind direction - smooth circular motion
    float windX = cos( primaryWindAngle );
    float windZ = sin( primaryWindAngle );

    // Add secondary wind component for more natural movement
    windX += cos( secondaryWindAngle + 0.5f ) * 0.2f;
    windZ += sin( secondaryWindAngle * 1.2f ) * 0.15f;

    // Normalize the direction
    const float windLength = sqrt( windX * windX + windZ * windZ );
    windX /= windLength;
    windZ /= windLength;

    // Apply a pulsing wind strength for gusts (subtle)
    const float gustStrength = 0.8f + sin( m_elapsedTime * 0.5f ) * 0.15f + sin( m_elapsedTime * 1.3f ) * 0.05f;

    // Set wind direction and strength
    m_grassConstants->WindDirection.x = windX;
    m_grassConstants->WindDirection.y = 0.1f * sin( m_elapsedTime * 0.4f ); // Small vertical component
    m_grassConstants->WindDirection.z = windZ;
    m_grassConstants->WindDirection.w = m_animateWind ? gustStrength * 0.6f : 0.0f; // Base strength with gusts

    // Update matrices
    m_grassConstants->ViewProjection = m_camera->ViewProjectionMatrix( );

    // Create a model matrix that positions the grass field
    // Center the grass field at the origin with expanded scale for denser coverage
    m_grassConstants->Model = XMMatrixScaling( 50.0f, 1.0f, 50.0f ) * XMMatrixTranslation( 0.0f, 0.0f, 0.0f );
}
