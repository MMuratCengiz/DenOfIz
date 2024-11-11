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
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceUsage::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceUsage::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceUsage::UnorderedAccess ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 0, m_rayGenCBResource.get( ), ResourceUsage::VertexAndConstantBuffer ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 1, m_rayGenCBResource.get( ), ResourceUsage::VertexAndConstantBuffer ) );
    raytracingNode.RequiredStates.AddElement( NodeResourceUsageDesc::BufferState( 2, m_rayGenCBResource.get( ), ResourceUsage::VertexAndConstantBuffer ) );
    raytracingNode.Execute = this;

    // !!! Note to be able to do this in Vulkan you need to set the line: `swapChainDesc.ImageUsages  = ResourceUsage::CopyDst;` from IExample.h
    NodeDesc copyToRenderTargetNode{ };
    copyToRenderTargetNode.Name = "CopyToRenderTarget";
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_raytracingOutput[ 0 ].get( ), ResourceUsage::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_raytracingOutput[ 1 ].get( ), ResourceUsage::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_raytracingOutput[ 2 ].get( ), ResourceUsage::CopySrc ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_swapChain->GetRenderTarget( 0 ), ResourceUsage::CopyDst ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_swapChain->GetRenderTarget( 1 ), ResourceUsage::CopyDst ) );
    copyToRenderTargetNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_swapChain->GetRenderTarget( 2 ), ResourceUsage::CopyDst ) );
    copyToRenderTargetNode.Dependencies.AddElement( "RayTracing" );

    m_copyToPresentCallback = std::make_unique<NodeExecutionCallbackHolder>(
        [ this ]( const uint32_t frameIndex, ICommandList *commandList )
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
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 0, m_swapChain->GetRenderTarget( 0 ), ResourceUsage::Present ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 1, m_swapChain->GetRenderTarget( 1 ), ResourceUsage::Present ) );
    presentNode.RequiredStates.AddElement( NodeResourceUsageDesc::TextureState( 2, m_swapChain->GetRenderTarget( 2 ), ResourceUsage::Present ) );
    presentNode.Dependencies.AddElement( "CopyToRenderTarget" );
    presentNode.Execute = this;

    m_renderGraph->AddNode( raytracingNode );
    m_renderGraph->AddNode( copyToRenderTargetNode );
    m_renderGraph->SetPresentNode( presentNode );
    m_renderGraph->BuildGraph( );

    m_time.OnEachSecond = []( const double fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

// Node execution
void RayTracedTriangleExample::Execute( const uint32_t frameIndex, ICommandList *commandList )
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
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::RWTexture );
    textureDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    textureDesc.Usages       = BitSet( ResourceUsage::CopySrc ) | ResourceUsage::UnorderedAccess;
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
    closestHitShaderDesc.Stage                     = ShaderStage::ClosestHit;
    closestHitShaderDesc.Path                      = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    closestHitShaderDesc.EntryPoint                = "MyClosestHitShader";
    closestHitShaderDesc.RayTracing.HitGroupExport = "MyHitGroup";
    closestHitShaderDesc.RayTracing.MarkCbvAsLocal( 0, 29 );

    shaderDescs.AddElement( closestHitShaderDesc );
    ShaderDesc missShaderDesc{ };
    missShaderDesc.Stage      = ShaderStage::Miss;
    missShaderDesc.Path       = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    missShaderDesc.EntryPoint = "MyMissShader";
    shaderDescs.AddElement( missShaderDesc );

    ProgramDesc programDesc{ };
    programDesc.Shaders       = shaderDescs;
    programDesc.EnableCaching = false;

    m_rayTracingProgram       = std::unique_ptr<ShaderProgram>( m_graphicsApi->CreateShaderProgram( programDesc ) );
    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );
    m_hgShaderLayout          = std::unique_ptr<IShaderLocalDataLayout>( m_logicalDevice->CreateShaderRecordLayout( reflection.ShaderLocalDataLayouts.GetElement( 1 ) ) );

    m_hgData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderRecordData( { m_hgShaderLayout.get( ) } ) );

    auto     redData = InteropArray<Byte>( sizeof( float ) * 4 );
    XMFLOAT4 red     = { 1.0f, 0.0f, 0.0f, 1.0f };
    redData.MemCpy( &red, sizeof( float ) * 4 );

    m_hgData->Cbv( 0, redData );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rayTracingRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    for ( int i = 0; i < 3; ++i )
    {
        m_rayTracingBindGroups[ i ] = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
        m_rayTracingBindGroups[ i ]->BeginUpdate( );
        m_rayTracingBindGroups[ i ]->Srv( 0, m_topLevelAS.get( ) );
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
    pipelineDesc.RayTracing.ShaderLocalDataLayouts.Resize( pipelineDesc.ShaderProgram->CompiledShaders( ).NumElements( ) );
    pipelineDesc.RayTracing.ShaderLocalDataLayouts.SetElement( 1, m_hgShaderLayout.get( ) );

    m_rayTracingPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void RayTracedTriangleExample::CreateResources( )
{
    constexpr uint16_t indices[]  = { 0, 1, 2 };
    constexpr float    depthValue = 1.0;
    constexpr float    offset     = 0.7f;

    struct Vertex
    {
        float x, y, z;
    };
    constexpr Vertex vertices[] = { { 0, -offset, depthValue }, { -offset, offset, depthValue }, { offset, offset, depthValue } };

    BufferDesc vbDesc{ };
    vbDesc.Descriptor   = BitSet( ResourceDescriptor::VertexBuffer );
    vbDesc.InitialUsage = ResourceUsage::CopyDst;
    vbDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::VertexAndConstantBuffer | ResourceUsage::AccelerationStructureGeometry;
    vbDesc.NumBytes     = sizeof( vertices );
    vbDesc.DebugName    = "VertexBuffer";
    m_vertexBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc{ };
    ibDesc.Descriptor   = BitSet( ResourceDescriptor::IndexBuffer );
    ibDesc.NumBytes     = sizeof( indices );
    ibDesc.InitialUsage = ResourceUsage::CopyDst;
    ibDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::IndexBuffer | ResourceUsage::AccelerationStructureGeometry;
    ibDesc.DebugName    = "IndexBuffer";
    m_indexBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

    BufferDesc rayGenCbDesc{ };
    rayGenCbDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    rayGenCbDesc.NumBytes     = sizeof( m_rayGenCB );
    rayGenCbDesc.InitialUsage = ResourceUsage::CopyDst;
    rayGenCbDesc.DebugName    = "RayGenCB";
    m_rayGenCBResource        = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( rayGenCbDesc ) );

    InteropArray<Byte> vertexArray( sizeof( vertices ) );
    vertexArray.MemCpy( vertices, sizeof( vertices ) );

    InteropArray<Byte> indexArray( sizeof( indices ) );
    indexArray.MemCpy( indices, sizeof( indices ) );

    float border        = 0.1f;
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
    geometryDesc.Type                   = ASGeometryType::Triangles;
    geometryDesc.Triangles.IndexBuffer  = m_indexBuffer.get( );
    geometryDesc.Triangles.NumIndices   = 3;
    geometryDesc.Triangles.IndexType    = IndexType::Uint16;
    geometryDesc.Triangles.VertexFormat = Format::R32G32B32Float;
    geometryDesc.Triangles.NumVertices  = 3;
    geometryDesc.Triangles.VertexBuffer = m_vertexBuffer.get( );
    geometryDesc.Triangles.VertexStride = 3 * sizeof( float );
    geometryDesc.Flags                  = GeometryFlags::Opaque;

    BottomLevelASDesc bottomLevelASDesc{ };
    bottomLevelASDesc.Geometries.AddElement( geometryDesc );
    bottomLevelASDesc.BuildFlags = ASBuildFlags::PreferFastTrace;
    m_bottomLevelAS              = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelASDesc ) );

    ASInstanceDesc instanceDesc{ };
    instanceDesc.BLAS = m_bottomLevelAS.get( );
    instanceDesc.Mask = 255;

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
    commandList->PipelineBarrier( PipelineBarrierDesc{ }.MemoryBarrier( MemoryBarrierDesc{
        .BottomLevelAS = m_bottomLevelAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } ) );
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
    bindingTableDesc.MaxHitGroupDataBytes  = sizeof( float ) * 4;

    m_shaderBindingTable = std::unique_ptr<IShaderBindingTable>( m_logicalDevice->CreateShaderBindingTable( bindingTableDesc ) );

    RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = "MyRaygenShader";
    m_shaderBindingTable->BindRayGenerationShader( rayGenDesc );

    MissBindingDesc missDesc{ };
    missDesc.ShaderName = "MyMissShader";
    m_shaderBindingTable->BindMissShader( missDesc );

    HitGroupBindingDesc hitGroupDesc{ };
    hitGroupDesc.HitGroupExportName = "MyHitGroup";
    hitGroupDesc.Data               = m_hgData.get( );

    m_shaderBindingTable->BindHitGroup( hitGroupDesc );
    m_shaderBindingTable->Build( );
}
