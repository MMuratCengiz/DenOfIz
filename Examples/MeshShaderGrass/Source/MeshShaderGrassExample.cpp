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

#include <DenOfIzExamples/MeshShaderGrassExample.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>

using namespace DirectX;
using namespace DenOfIz;

void MeshShaderGrassExample::Init( )
{
    m_camera->SetPosition( { 0.0f, 5.0f, -10.0f, 1.0f } );
    m_camera->SetFront( { 0.0f, -0.2f, 1.0f, 0.0f } );

    CreateConstantsBuffer( );
    LoadGrassTexture( );
    CreateMeshShaderPipeline( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

void MeshShaderGrassExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // Use DirectX12 for mesh shader support
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void MeshShaderGrassExample::Update( )
{
    m_time.Tick( );
    m_stepTimer.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
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

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource        = renderTarget;
    attachmentDesc.LoadOp          = LoadOp::Clear;
    attachmentDesc.ClearColor[ 0 ] = 0.1f; // Sky blue
    attachmentDesc.ClearColor[ 1 ] = 0.4f;
    attachmentDesc.ClearColor[ 2 ] = 0.7f;
    attachmentDesc.ClearColor[ 3 ] = 1.0f;

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( attachmentDesc );

    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    // Bind the mesh shader pipeline
    commandList->BindPipeline( m_meshPipeline.get( ) );

    // Bind resources
    commandList->BindResourceGroup( m_meshBindGroup.get( ) );

    // Dispatch mesh shader with the desired patch count
    // Parameters are grid dimensions X, Y, Z
    // For a 20x20 grid of grass patches (400 patches total):
    commandList->DispatchMesh( 20, 20, 1 );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void MeshShaderGrassExample::HandleEvent( SDL_Event &event )
{
    switch ( event.type )
    {
    case SDL_KEYDOWN:
        {
            switch ( event.key.keysym.sym )
            {
            case SDLK_w:
                m_animateWind = !m_animateWind;
                LOG( INFO ) << "Wind animation " << ( m_animateWind ? "enabled" : "disabled" );
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
    // Initialize shader stages for mesh shader pipeline
    InteropArray<ShaderStageDesc> shaderStages( 2 );

    // Mesh shader
    ShaderStageDesc &meshShaderDesc = shaderStages.GetElement( 0 );
    meshShaderDesc.Stage            = ShaderStage::Mesh;
    meshShaderDesc.Path             = "Assets/Shaders/GrassShader/GrassMS.hlsl";
    meshShaderDesc.EntryPoint       = "main";

    // Pixel shader
    ShaderStageDesc &pixelShaderDesc = shaderStages.GetElement( 1 );
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.Path             = "Assets/Shaders/GrassShader/GrassPS.hlsl";
    pixelShaderDesc.EntryPoint       = "main";

    // Create shader program
    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages = shaderStages;
    m_meshShaderProgram      = std::make_unique<ShaderProgram>( programDesc );

    // Get shader reflection data
    auto reflection = m_meshShaderProgram->Reflect( );

    // Create root signature
    m_meshRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    // Create pipeline
    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint     = BindPoint::Mesh; // Use mesh shader pipeline
    pipelineDesc.RootSignature = m_meshRootSignature.get( );
    pipelineDesc.ShaderProgram = m_meshShaderProgram.get( );

    // Configure graphics pipeline details
    pipelineDesc.Graphics.PrimitiveTopology            = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode                     = CullMode::None; // No culling for grass as it's double-sided
    pipelineDesc.Graphics.FillMode                     = FillMode::Solid;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = Format::D32Float;
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = CompareOp::Less;

    // Alpha blending for grass
    RenderTargetDesc rtDesc{ };
    rtDesc.Format              = Format::B8G8R8A8Unorm;
    rtDesc.Blend.Enable        = true;
    rtDesc.Blend.SrcBlend      = Blend::SrcAlpha;
    rtDesc.Blend.DstBlend      = Blend::InvSrcAlpha;
    rtDesc.Blend.SrcBlendAlpha = Blend::One;
    rtDesc.Blend.DstBlendAlpha = Blend::Zero;
    pipelineDesc.Graphics.RenderTargets.AddElement( rtDesc );

    // Create pipeline
    m_meshPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    // Create resource bind group
    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_meshRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    m_meshBindGroup             = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    // Bind resources
    m_meshBindGroup->BeginUpdate( );
    m_meshBindGroup->Cbv( 0, m_grassConstantsBuffer.get( ) ); // Grass constants
    m_meshBindGroup->Srv( 0, m_grassTexture.get( ) );         // Grass texture
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
    constantsDesc.Usages     = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::VertexAndConstantBuffer;
    constantsDesc.DebugName  = "GrassConstantsBuffer";
    m_grassConstantsBuffer   = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( constantsDesc ) );
    m_grassConstants         = static_cast<GrassConstants *>( m_grassConstantsBuffer->MapMemory( ) );

    // Initialize default values
    m_grassConstants->WindDirection       = { 1.0f, 0.0f, 0.0f, 0.8f };  // X-direction wind with 0.8 strength
    m_grassConstants->GrassColor          = { 0.4f, 0.8f, 0.2f, 1.0f };  // Green
    m_grassConstants->GrassColorVariation = { 0.1f, 0.1f, 0.05f, 0.0f }; // Slight color variation
    m_grassConstants->Time                = 0.0f;
    m_grassConstants->DensityFactor       = 4.0f;  // Number of grass blades per unit area
    m_grassConstants->HeightScale         = 0.8f;  // Height of grass blades
    m_grassConstants->WidthScale          = 0.1f;  // Width of grass blades
    m_grassConstants->MaxDistance         = 30.0f; // Maximum distance for LOD

    // Identity matrices initially
    m_grassConstants->Model          = XMMatrixIdentity( );
    m_grassConstants->ViewProjection = XMMatrixIdentity( );
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

    // Create a simple texture for grass (could be replaced with a loaded texture)
    TextureDesc textureDesc{ };
    textureDesc.Width        = 128;
    textureDesc.Height       = 128;
    textureDesc.Format       = Format::R8G8B8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    textureDesc.InitialUsage = ResourceUsage::CopyDst;
    textureDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::ShaderResource;
    textureDesc.DebugName    = "GrassTexture";

    m_grassTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    m_resourceTracking.TrackTexture( m_grassTexture.get( ), ResourceUsage::CopyDst );

    // Create texture data with a simple gradient alpha
    std::vector<uint8_t> textureData( textureDesc.Width * textureDesc.Height * 4 );
    for ( uint32_t y = 0; y < textureDesc.Height; ++y )
    {
        for ( uint32_t x = 0; x < textureDesc.Width; ++x )
        {
            uint32_t idx = ( y * textureDesc.Width + x ) * 4;

            // Create a gradient that fades out at the top and edges
            float centerX = static_cast<float>( x ) / textureDesc.Width - 0.5f;
            float centerY = static_cast<float>( y ) / textureDesc.Height;

            float distanceFromCenter = std::abs( centerX ) * 2.0f;
            float alphaEdge          = 1.0f - std::min( 1.0f, distanceFromCenter * 1.5f );
            float alphaHeight        = std::max( 0.0f, 1.0f - centerY * 1.2f );

            // Make tip more transparent
            float alpha = alphaEdge * alphaHeight;

            // RGB values - mostly white to allow tinting in shader
            textureData[ idx + 0 ] = 220;                                    // R
            textureData[ idx + 1 ] = 220;                                    // G
            textureData[ idx + 2 ] = 220;                                    // B
            textureData[ idx + 3 ] = static_cast<uint8_t>( alpha * 255.0f ); // A
        }
    }

    // Copy data to texture
    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    InteropArray<Byte> texArray;
    texArray.Resize( textureData.size( ) );
    texArray.MemCpy( textureData.data( ), textureData.size( ) );

    CopyDataToTextureDesc copyDesc{ };
    copyDesc.DstTexture = m_grassTexture.get( );
    copyDesc.Data       = texArray;
    batchResourceCopy.CopyDataToTexture( copyDesc );
    batchResourceCopy.Submit( );

    // Transition texture to shader resource
    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { m_graphicsQueue.get( ) } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).GetElement( 0 );
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.CommandList = commandList;
    batchTransitionDesc.TransitionTexture( m_grassTexture.get( ), ResourceUsage::ShaderResource, QueueType::Graphics );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.AddElement( commandList );
    executeDesc.Signal = syncFence.get( );
    m_graphicsQueue->ExecuteCommandLists( executeDesc );

    syncFence->Wait( );
}

void MeshShaderGrassExample::UpdateConstants( )
{
    // Update time
    m_grassConstants->Time = m_elapsedTime;

    // Update wind direction with circular motion
    float windAngle                   = m_elapsedTime * 0.3f;
    m_grassConstants->WindDirection.x = cos( windAngle );
    m_grassConstants->WindDirection.z = sin( windAngle );

    // Update matrices
    m_grassConstants->ViewProjection = m_camera->ViewProjectionMatrix( );

    // Create a model matrix that positions the grass field
    // Center the grass field at the origin, scaling to 40x40 units
    m_grassConstants->Model = XMMatrixScaling( 40.0f, 1.0f, 40.0f ) * XMMatrixTranslation( 0.0f, 0.0f, 0.0f );
}
