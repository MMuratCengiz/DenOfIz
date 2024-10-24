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
    CreateRenderTargets( );
    CreateResources( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );

    RenderGraphDesc renderGraphDesc{ };
    renderGraphDesc.GraphicsApi   = m_graphicsApi;
    renderGraphDesc.LogicalDevice = m_logicalDevice;
    renderGraphDesc.SwapChain     = m_swapChain.get( );

    m_renderGraph = std::make_unique<RenderGraph>( renderGraphDesc );

    NodeDesc raytracingNode{ };
    raytracingNode.Name      = "RayTracing";
    raytracingNode.QueueType = QueueType::RayTracing;
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceState::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 0, m_rayGenCBResource.get( ), ResourceState::VertexAndConstantBuffer ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 1, m_rayGenCBResource.get( ), ResourceState::VertexAndConstantBuffer ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 2, m_rayGenCBResource.get( ), ResourceState::VertexAndConstantBuffer ) );
    raytracingNode.Execute = this;

    NodeDesc copyToRenderTargetNode{ };
    copyToRenderTargetNode.Name = "CopyToRenderTarget";
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceState::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceState::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceState::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_swapChain->GetRenderTarget( 0 ), ResourceState::CopyDst ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_swapChain->GetRenderTarget( 1 ), ResourceState::CopyDst ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_swapChain->GetRenderTarget( 2 ), ResourceState::CopyDst ) );
    copyToRenderTargetNode.Dependencies.AddElement( "RayTracing" );

    m_copyToPresentCallback = std::make_unique<NodeExecutionCallbackHolder>(
        [ this ]( uint32_t frameIndex, ICommandList *commandList )
        {
            CopyTextureRegionDesc copyTextureRegionDesc{ };
            copyTextureRegionDesc.SrcTexture = m_raytracingOutput[ frameIndex ].get( );
            copyTextureRegionDesc.DstTexture = m_swapChain->GetRenderTarget( frameIndex );
            copyTextureRegionDesc.Width      = m_windowDesc.Width;
            copyTextureRegionDesc.Height     = m_windowDesc.Height;
            copyTextureRegionDesc.Depth      = 1;
            commandList->CopyTextureRegion( copyTextureRegionDesc );
        } );
    copyToRenderTargetNode.Execute = m_copyToPresentCallback.get( );

    PresentNodeDesc presentNode{ };
    presentNode.SwapChain = m_swapChain.get( );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_swapChain->GetRenderTarget( 0 ), ResourceState::Present ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_swapChain->GetRenderTarget( 1 ), ResourceState::Present ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_swapChain->GetRenderTarget( 2 ), ResourceState::Present ) );
    presentNode.Dependencies.AddElement( "CopyToRenderTarget" );
    presentNode.Execute = this;

    m_renderGraph->AddNode( raytracingNode );
    m_renderGraph->AddNode( copyToRenderTargetNode );
    m_renderGraph->SetPresentNode( presentNode );
    m_renderGraph->BuildGraph( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

// Node execution
void RayTracedTriangleExample::Execute( uint32_t frameIndex, ICommandList *commandList )
{
    const Viewport &viewport = m_swapChain->GetViewport( );

    commandList->BindPipeline( m_rayTracingPipeline.get( ) );
    commandList->BindResourceGroup( m_rayTracingBindGroups[ frameIndex ].get( ) );

    DispatchRaysDesc dispatchRaysDesc{ };
    dispatchRaysDesc.Width              = viewport.Width;
    dispatchRaysDesc.Height             = viewport.Height;
    dispatchRaysDesc.Depth              = 1;
    dispatchRaysDesc.ShaderBindingTable = m_shaderBindingTable.get( );
    commandList->DispatchRays( dispatchRaysDesc );
}

void RayTracedTriangleExample::Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget )
{
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

void RayTracedTriangleExample::CreateRenderTargets( )
{
    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor   = ResourceDescriptor::RWTexture;
    textureDesc.InitialState = ResourceState::ShaderResource;
    textureDesc.DebugName    = "RayTracing Output";
    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName = InteropString( );
        textureDesc.DebugName.Append( "RayTracing Output " ).Append( std::to_string( i ).c_str( ) );
        m_raytracingOutput[ i ] = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    }
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
    m_rayTracingProgram       = std::unique_ptr<ShaderProgram>( m_graphicsApi->CreateShaderProgram( shaderDescs, false ) );
    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rayTracingRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    for ( int i = 0; i < 3; ++i )
    {
        m_rayTracingBindGroups[ i ] = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
        m_rayTracingBindGroups[ i ]->BeginUpdate( );
        m_rayTracingBindGroups[ i ]->Srv( 0, m_topLevelAS->Buffer( ) );
        m_rayTracingBindGroups[ i ]->Uav( 0, m_raytracingOutput[ i ].get( ) );
        m_rayTracingBindGroups[ i ]->Cbv( 0, m_rayGenCBResource.get( ) );
        m_rayTracingBindGroups[ i ]->EndUpdate( );
    }

    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                       = BindPoint::RayTracing;
    pipelineDesc.RootSignature                   = m_rayTracingRootSignature.get( );
    pipelineDesc.ShaderProgram                   = m_rayTracingProgram.get( );
    pipelineDesc.RayTracing.MaxNumPayloadBytes   = 4 * sizeof( float );
    pipelineDesc.RayTracing.MaxNumAttributeBytes = 2 * sizeof( float );
    m_rayTracingPipeline                         = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void RayTracedTriangleExample::CreateResources( )
{
    constexpr uint32_t indices[]  = { 0, 1, 2 };
    constexpr float    depthValue = 1.0;
    constexpr float    offset     = 0.7f;

    struct Vertex
    {
        float x, y, z;
    };
    constexpr Vertex vertices[] = { { 0, -offset, depthValue }, { -offset, offset, depthValue }, { offset, offset, depthValue } };

    BufferDesc vbDesc{ };
    vbDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vbDesc.NumBytes     = sizeof( vertices );
    vbDesc.InitialState = ResourceState::CopyDst;
    vbDesc.DebugName    = "VertexBuffer";
    m_vertexBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc{ };
    ibDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
    ibDesc.NumBytes     = sizeof( indices );
    ibDesc.InitialState = ResourceState::CopyDst;
    ibDesc.DebugName    = "IndexBuffer";
    m_indexBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

    BufferDesc rayGenCbDesc{ };
    rayGenCbDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    rayGenCbDesc.NumBytes     = sizeof( m_rayGenCB );
    rayGenCbDesc.InitialState = ResourceState::CopyDst;
    rayGenCbDesc.DebugName    = "RayGenCB";
    m_rayGenCBResource        = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( rayGenCbDesc ) );

    InteropArray<Byte> vertexArray( sizeof( vertices ) );
    vertexArray.MemCpy( vertices, sizeof( vertices ) );

    InteropArray<Byte> indexArray( sizeof( indices ) );
    indexArray.MemCpy( indices, sizeof( indices ) );

    float border        = 1.0f;
    float aspect        = static_cast<float>( m_windowDesc.Width ) / static_cast<float>( m_windowDesc.Height );
    m_rayGenCB.Viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
    m_rayGenCB.Stencil  = { -1 + border / aspect, -1 + border, 1 - border / aspect, 1.0f - border };

    InteropArray<Byte> rayGenCBArray( sizeof( m_rayGenCB ) );
    rayGenCBArray.MemCpy( &m_rayGenCB, sizeof( m_rayGenCB ) );

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

    CopyToGpuBufferDesc rayGenCBCopy{ };
    rayGenCBCopy.DstBuffer = m_rayGenCBResource.get( );
    rayGenCBCopy.Data      = rayGenCBArray;
    batchResourceCopy.CopyToGPUBuffer( rayGenCBCopy );

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
    instanceDesc.BLASBuffer = m_bottomLevelAS->Buffer( );
    instanceDesc.Mask       = 1;

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
    // Pipeline barriers:
    // - Transition bottom level AS to AS state
    // - Transition top level AS to AS state
    //    commandList->PipelineBarrier( PipelineBarrierDesc{ }.BufferBarrier(
    //        BufferBarrierDesc{ .Resource = m_bottomLevelAS->Buffer( ), .OldState = ResourceState::AccelerationStructureWrite, .NewState = ResourceState::UnorderedAccess } ) );
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

    m_shaderBindingTable = std::unique_ptr<IShaderBindingTable>( m_logicalDevice->CreateShaderBindingTable( bindingTableDesc ) );

    RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = "MyRaygenShader";
    m_shaderBindingTable->BindRayGenerationShader( rayGenDesc );

    MissBindingDesc missDesc{ };
    missDesc.ShaderName = "MyMissShader";
    m_shaderBindingTable->BindMissShader( missDesc );

    HitGroupBindingDesc hitGroupDesc{ };
    hitGroupDesc.HitGroupExportName = "HitGroup";
    m_shaderBindingTable->BindHitGroup( hitGroupDesc );
    m_shaderBindingTable->Build( );
}
