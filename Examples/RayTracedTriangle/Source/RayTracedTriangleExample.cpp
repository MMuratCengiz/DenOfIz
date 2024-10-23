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
#include <DenOfIzExamples/RayTracedTriangleExample.h>
#include <DenOfIzGraphics/Utilities/Time.h>

using namespace DenOfIz;

void RayTracedTriangleExample::Init( )
{
    CreateGeometry( );
    CreateRayTracingPipeline( );
    CreateAccelerationStructures( );

    m_presentOutputPipeline = std::make_unique<QuadPipeline>( m_graphicsApi, m_logicalDevice, "Assets/Shaders/SampleBasic.ps.hlsl" );

    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor   = ResourceDescriptor::RWTexture;
    textureDesc.InitialState = ResourceState::ShaderResource;
    textureDesc.DebugName    = "RayTracing Output";
    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName.Append( "RayTracing Output " ).Append( std::to_string( i ).c_str( ) );
        m_raytracingOutput.push_back( std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) ) );
    }
    m_defaultSampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( SamplerDesc{ } ) );
    m_presentOutputPipeline->BindGroup( 0 )->BeginUpdate( )->Srv( 0, m_raytracingOutput[ 0 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_presentOutputPipeline->BindGroup( 1 )->BeginUpdate( )->Srv( 0, m_raytracingOutput[ 1 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );
    m_presentOutputPipeline->BindGroup( 2 )->BeginUpdate( )->Srv( 0, m_raytracingOutput[ 2 ].get( ) )->Sampler( 0, m_defaultSampler.get( ) )->EndUpdate( );

    RenderGraphDesc renderGraphDesc{ };
    renderGraphDesc.GraphicsApi   = m_graphicsApi;
    renderGraphDesc.LogicalDevice = m_logicalDevice;
    renderGraphDesc.SwapChain     = m_swapChain.get( );

    m_renderGraph = std::make_unique<RenderGraph>( renderGraphDesc );

    NodeDesc raytracingNode{ };
    raytracingNode.Name = "RayTracing";
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.Execute = this;

    PresentNodeDesc presentNode{ };
    presentNode.SwapChain = m_swapChain.get( );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceState::ShaderResource ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceState::ShaderResource ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceState::ShaderResource ) );
    presentNode.Dependencies.AddElement( "Deferred" );
    presentNode.Execute = this;

    m_renderGraph->AddNode( raytracingNode );
    m_renderGraph->SetPresentNode( presentNode );
    m_renderGraph->BuildGraph( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

// Node execution
void RayTracedTriangleExample::Execute( uint32_t frameIndex, ICommandList *commandList )
{
    RenderingAttachmentDesc renderingAttachmentDesc{ };
    renderingAttachmentDesc.Resource = m_raytracingOutput[ frameIndex ].get( );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( renderingAttachmentDesc );

    const Viewport &viewport = m_swapChain->GetViewport( );

    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );

    commandList->EndRendering( );
}

void RayTracedTriangleExample::Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget )
{
    RenderingAttachmentDesc quadAttachmentDesc{ };
    quadAttachmentDesc.Resource = renderTarget;

    RenderingDesc quadRenderingDesc{ };
    quadRenderingDesc.RTAttachments.AddElement( quadAttachmentDesc );

    commandList->BeginRendering( quadRenderingDesc );

    const Viewport &viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    m_presentOutputPipeline->Render( commandList, frameIndex );
    commandList->EndRendering( );
}

void RayTracedTriangleExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::DirectX12;
}

void RayTracedTriangleExample::Update( )
{
    m_time.Tick( );
    m_worldData.DeltaTime = m_time.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_renderGraph->Update( );
}

void RayTracedTriangleExample::HandleEvent( SDL_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RayTracedTriangleExample::Quit( )
{
    m_renderGraph->WaitIdle( );
    IExample::Quit( );
}

void RayTracedTriangleExample::CreateRayTracingPipeline( )
{
    InteropArray<ShaderDesc> shaderDescs;
    ShaderDesc               rayGenShaderDesc{ };
    rayGenShaderDesc.Stage      = ShaderStage::Raygen;
    rayGenShaderDesc.Path       = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    rayGenShaderDesc.EntryPoint = "MyRaygenShader";
    shaderDescs.AddElement( rayGenShaderDesc );
    ShaderDesc closestHitShaderDesc{ };
    closestHitShaderDesc.Stage      = ShaderStage::ClosestHit;
    closestHitShaderDesc.Path       = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    closestHitShaderDesc.EntryPoint = "MyClosestHitShader";
    shaderDescs.AddElement( closestHitShaderDesc );
    ShaderDesc missShaderDesc{ };
    missShaderDesc.Stage      = ShaderStage::Miss;
    missShaderDesc.Path       = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    missShaderDesc.EntryPoint = "MyMissShader";
    shaderDescs.AddElement( missShaderDesc );
    m_rayTracingProgram       = std::unique_ptr<ShaderProgram>( m_graphicsApi->CreateShaderProgram( shaderDescs ) );
    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                       = BindPoint::RayTracing;
    pipelineDesc.RootSignature                   = m_rayTracingRootSignature.get( );
    pipelineDesc.ShaderProgram                   = m_rayTracingProgram.get( );
    pipelineDesc.RayTracing.MaxNumPayloadBytes   = 4 * sizeof( float );
    pipelineDesc.RayTracing.MaxNumAttributeBytes = 2 * sizeof( float );
    m_rayTracingPipeline                         = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void RayTracedTriangleExample::CreateGeometry( ) const
{
    constexpr uint32_t indices[]  = { 0, 1, 2 };
    constexpr float    depthValue = 1.0;
    constexpr float    offset     = 0.7f;

    struct Vertex
    {
        float x, y, z;
    };
    constexpr Vertex vertices[] = { { 0, -offset, depthValue }, { -offset, offset, depthValue }, { offset, offset, depthValue } };

    InteropArray<Byte> vertexArray( sizeof( vertices ) );
    vertexArray.MemCpy( vertices, sizeof( vertices ) );

    InteropArray<Byte> indexArray( sizeof( indices ) );
    indexArray.MemCpy( indices, sizeof( indices ) );

    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    CopyToGpuBufferDesc vertexCopy{ };
    vertexCopy.DstBuffer = m_vertexBuffer.get( );
    vertexCopy.Data      = vertexArray;
    batchResourceCopy.CopyToGPUBuffer( vertexCopy );

    CopyToGpuBufferDesc indexCopy{ };
    indexCopy.DstBuffer = m_indexBuffer.get( );
    indexCopy.Data      = indexArray;
    batchResourceCopy.CopyToGPUBuffer( indexCopy );
    batchResourceCopy.Submit( );
}

void RayTracedTriangleExample::CreateAccelerationStructures( )
{
    ASGeometryDesc geometryDesc{ };
    geometryDesc.Type        = ASGeometryType::Triangles;
    geometryDesc.IndexBuffer = m_indexBuffer.get( );
    geometryDesc.NumIndices  = 3;
    geometryDesc.IndexType   = IndexType::Uint32;
    //    geometryDesc.Transform3x4               = 0;
    geometryDesc.VertexFormat = Format::R32G32B32Float;
    geometryDesc.NumVertices  = 3;
    geometryDesc.VertexBuffer = m_vertexBuffer.get( );
    geometryDesc.VertexStride = 3 * sizeof( float );
    geometryDesc.IsOpaque     = true; // Todo does nothing

    BottomLevelASDesc bottomLevelASDesc{ };
    bottomLevelASDesc.Geometries.AddElement( geometryDesc );
    bottomLevelASDesc.BuildFlags = ASBuildFlags::PreferFastTrace;
    m_bottomLevelAS              = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelASDesc ) );

    ASInstanceDesc instanceDesc{ };
    instanceDesc.Buffer                      = m_bottomLevelAS->Buffer( );
    instanceDesc.ID                          = 0;
    instanceDesc.Mask                        = 1;
    instanceDesc.ContributionToHitGroupIndex = 0;

    constexpr float transform[ 3 ][ 4 ] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } };
    instanceDesc.Transform.MemCpy( transform, sizeof( transform ) );

    TopLevelASDesc topLevelASDesc{ };
    topLevelASDesc.BuildFlags = ASBuildFlags::PreferFastTrace;
    topLevelASDesc.Instances.AddElement( instanceDesc );
    m_topLevelAS = std::unique_ptr<ITopLevelAS>( m_logicalDevice->CreateTopLevelAS( topLevelASDesc ) );

    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { QueueType::RayTracing, 1 } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).GetElement( 0 );
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );
    commandList->BuildBottomLevelAS( BuildBottomLevelASDesc{ m_bottomLevelAS.get( ) } );
    commandList->BuildTopLevelAS( BuildTopLevelASDesc{ m_topLevelAS.get( ) } );
    ExecuteDesc executeDesc{ };
    executeDesc.Notify = syncFence.get( );
    commandList->Execute( executeDesc );

    syncFence->Wait( );
}

void RayTracedTriangleExample::CreateShaderBindingTable( )
{
    ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline              = m_rayTracingPipeline.get( );
    bindingTableDesc.SizeDesc.NumInstances = 1;
    bindingTableDesc.SizeDesc.NumRayTypes  = 1;
    bindingTableDesc.SizeDesc.NumHitGroups = 1;

    m_shaderBindingTable = std::unique_ptr<IShaderBindingTable>( m_logicalDevice->CreateShaderBindingTable( bindingTableDesc ) );

    RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = "MyRaygenShader";
    m_shaderBindingTable->BindRayGenerationShader( rayGenDesc );

    MissBindingDesc missDesc{ };
    missDesc.ShaderName = "MyMissShader";
    m_shaderBindingTable->BindMissShader( missDesc );

    HitGroupBindingDesc hitGroupDesc{ };
    hitGroupDesc.ClosestHitShaderName = "MyClosestHitShader";
    m_shaderBindingTable->BindHitGroup( hitGroupDesc );
    m_shaderBindingTable->Build( );
}
