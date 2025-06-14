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
#include "DenOfIzExamples/RayTracedTriangleExample.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"

using namespace DenOfIz;

void RayTracedTriangleExample::Init( )
{
    CreateRenderTargets( );
    CreateResources( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );
}

void RayTracedTriangleExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void RayTracedTriangleExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    RenderAndPresentFrame( );
}

void RayTracedTriangleExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );

    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_raytracingOutput[ frameIndex ].get( ), ResourceUsage::UnorderedAccess );
    batchTransitionDesc.TransitionBuffer( m_rayGenCBResource.get( ), ResourceUsage::VertexAndConstantBuffer );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    const Viewport &viewport = m_swapChain->GetViewport( );

    commandList->BindPipeline( m_rayTracingPipeline.get( ) );
    commandList->BindResourceGroup( m_rayTracingBindGroups[ frameIndex ].get( ) );

    DispatchRaysDesc dispatchRaysDesc{ };
    dispatchRaysDesc.Width              = viewport.Width;
    dispatchRaysDesc.Height             = viewport.Height;
    dispatchRaysDesc.Depth              = 1;
    dispatchRaysDesc.ShaderBindingTable = m_shaderBindingTable.get( );
    commandList->DispatchRays( dispatchRaysDesc );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_raytracingOutput[ frameIndex ].get( ), ResourceUsage::UnorderedAccess );
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::CopyDst );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    CopyTextureRegionDesc copyTextureRegionDesc{ };
    copyTextureRegionDesc.SrcTexture = m_raytracingOutput[ frameIndex ].get( );
    copyTextureRegionDesc.DstTexture = renderTarget;
    copyTextureRegionDesc.Width      = m_windowDesc.Width;
    copyTextureRegionDesc.Height     = m_windowDesc.Height;
    copyTextureRegionDesc.Depth      = 1;
    commandList->CopyTextureRegion( copyTextureRegionDesc );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void RayTracedTriangleExample::HandleEvent( Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RayTracedTriangleExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}

void RayTracedTriangleExample::CreateRenderTargets( )
{
    TextureDesc textureDesc{ };
    textureDesc.Width      = m_windowDesc.Width;
    textureDesc.Height     = m_windowDesc.Height;
    textureDesc.Format     = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor = ResourceDescriptor::RWTexture;
    textureDesc.Usages     = ResourceUsage::CopySrc | ResourceUsage::UnorderedAccess;
    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName   = InteropString( "RayTracing Output " ).Append( std::to_string( i ).c_str( ) );
        m_raytracingOutput[ i ] = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    }
}

void RayTracedTriangleExample::CreateRayTracingPipeline( )
{
    std::array<ShaderStageDesc, 3>     shaderStages( { } );
    std::array<ResourceBindingSlot, 1> localBindings( { } );
    localBindings[ 0 ].Binding       = 0;
    localBindings[ 0 ].RegisterSpace = 29;
    localBindings[ 0 ].Type          = ResourceBindingType::ConstantBuffer;

    ShaderStageDesc &rayGenShaderDesc = shaderStages[ 0 ];
    rayGenShaderDesc.Stage            = ShaderStage::Raygen;
    rayGenShaderDesc.Path             = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    rayGenShaderDesc.EntryPoint       = "MyRaygenShader";

    ShaderStageDesc &closestHitShaderDesc                     = shaderStages[ 1 ];
    closestHitShaderDesc.Stage                                = ShaderStage::ClosestHit;
    closestHitShaderDesc.Path                                 = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    closestHitShaderDesc.EntryPoint                           = "MyClosestHitShader";
    closestHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
    closestHitShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );

    ShaderStageDesc &missShaderDesc = shaderStages[ 2 ];
    missShaderDesc.Stage            = ShaderStage::Miss;
    missShaderDesc.Path             = "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl";
    missShaderDesc.EntryPoint       = "MyMissShader";

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements           = shaderStages.data( );
    programDesc.ShaderStages.NumElements        = shaderStages.size( );
    programDesc.RayTracing.MaxNumPayloadBytes   = 4 * sizeof( float );
    programDesc.RayTracing.MaxNumAttributeBytes = 2 * sizeof( float );
    programDesc.RayTracing.MaxRecursionDepth    = 1;

    m_rayTracingProgram       = std::make_unique<ShaderProgram>( programDesc );
    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );
    m_hgShaderLayout          = std::unique_ptr<ILocalRootSignature>( m_logicalDevice->CreateLocalRootSignature( reflection.LocalRootSignatures.Elements[ 1 ] ) );

    m_hgData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderLocalData( { m_hgShaderLayout.get( ) } ) );

    XMFLOAT4 red = { 1.0f, 0.0f, 0.0f, 1.0f };

    ByteArrayView redData{ };
    redData.Elements    = reinterpret_cast<const Byte *>( &red );
    redData.NumElements = 4 * sizeof( float );
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
    pipelineDesc.BindPoint     = BindPoint::RayTracing;
    pipelineDesc.RootSignature = m_rayTracingRootSignature.get( );
    pipelineDesc.ShaderProgram = m_rayTracingProgram.get( );
    HitGroupDesc hitGroupDesc{ .Name = "MyHitGroup", .ClosestHitShaderIndex = 1, .LocalRootSignature = m_hgShaderLayout.get( ), .Type = HitGroupType::Triangles };
    pipelineDesc.RayTracing.HitGroups.Elements    = &hitGroupDesc;
    pipelineDesc.RayTracing.HitGroups.NumElements = 1;

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
    vbDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vbDesc.InitialUsage = ResourceUsage::CopyDst;
    vbDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::VertexAndConstantBuffer | ResourceUsage::AccelerationStructureGeometry;
    vbDesc.NumBytes     = sizeof( vertices );
    vbDesc.DebugName    = "VertexBuffer";
    m_vertexBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc{ };
    ibDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
    ibDesc.NumBytes     = sizeof( indices );
    ibDesc.InitialUsage = ResourceUsage::CopyDst;
    ibDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::IndexBuffer | ResourceUsage::AccelerationStructureGeometry;
    ibDesc.DebugName    = "IndexBuffer";
    m_indexBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

    BufferDesc rayGenCbDesc{ };
    rayGenCbDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    rayGenCbDesc.NumBytes     = sizeof( m_rayGenCB );
    rayGenCbDesc.InitialUsage = ResourceUsage::CopyDst;
    rayGenCbDesc.DebugName    = "RayGenCB";
    m_rayGenCBResource        = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( rayGenCbDesc ) );

    float border        = 0.1f;
    float aspect        = static_cast<float>( m_windowDesc.Width ) / static_cast<float>( m_windowDesc.Height );
    m_rayGenCB.Viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
    m_rayGenCB.Stencil  = { -1 + border / aspect, -1 + border, 1 - border / aspect, 1.0f - border };

    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    CopyToGpuBufferDesc vertexCopy{ };
    vertexCopy.DstBuffer        = m_vertexBuffer.get( );
    vertexCopy.Data.Elements    = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    vertexCopy.Data.NumElements = sizeof( vertices );
    batchResourceCopy.CopyToGPUBuffer( vertexCopy );

    CopyToGpuBufferDesc indexCopy{ };
    indexCopy.DstBuffer        = m_indexBuffer.get( );
    indexCopy.Data.Elements    = reinterpret_cast<const Byte *>( &indices[ 0 ] );
    indexCopy.Data.NumElements = sizeof( indices );
    batchResourceCopy.CopyToGPUBuffer( indexCopy );

    CopyToGpuBufferDesc rayGenCBCopy{ };
    rayGenCBCopy.DstBuffer        = m_rayGenCBResource.get( );
    rayGenCBCopy.Data.Elements    = reinterpret_cast<Byte *>( &m_rayGenCB );
    rayGenCBCopy.Data.NumElements = sizeof( m_rayGenCB );
    batchResourceCopy.CopyToGPUBuffer( rayGenCBCopy );
    batchResourceCopy.Submit( );
}

void RayTracedTriangleExample::CreateAccelerationStructures( )
{
    CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = QueueType::Compute;
    const auto commandQueue    = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

    ASGeometryDesc geometryDesc{ };
    geometryDesc.Type                   = HitGroupType::Triangles;
    geometryDesc.Triangles.IndexBuffer  = m_indexBuffer.get( );
    geometryDesc.Triangles.NumIndices   = 3;
    geometryDesc.Triangles.IndexType    = IndexType::Uint16;
    geometryDesc.Triangles.VertexFormat = Format::R32G32B32Float;
    geometryDesc.Triangles.NumVertices  = 3;
    geometryDesc.Triangles.VertexBuffer = m_vertexBuffer.get( );
    geometryDesc.Triangles.VertexStride = 3 * sizeof( float );
    geometryDesc.Flags                  = GeometryFlags::Opaque;

    BottomLevelASDesc bottomLevelASDesc{ };
    bottomLevelASDesc.Geometries.Elements    = &geometryDesc;
    bottomLevelASDesc.Geometries.NumElements = 1;
    bottomLevelASDesc.BuildFlags             = ASBuildFlags::PreferFastTrace;
    m_bottomLevelAS                          = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelASDesc ) );

    ASInstanceDesc instanceDesc{ };
    instanceDesc.BLAS = m_bottomLevelAS.get( );
    instanceDesc.Mask = 255;

    float transform[ 3 ][ 4 ]          = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } };
    instanceDesc.Transform.Elements    = &transform[ 0 ][ 0 ];
    instanceDesc.Transform.NumElements = 3 * 4;

    TopLevelASDesc topLevelASDesc{ };
    topLevelASDesc.BuildFlags            = ASBuildFlags::PreferFastTrace;
    topLevelASDesc.Instances.Elements    = &instanceDesc;
    topLevelASDesc.Instances.NumElements = 1;
    m_topLevelAS                         = std::unique_ptr<ITopLevelAS>( m_logicalDevice->CreateTopLevelAS( topLevelASDesc ) );

    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { commandQueue.get( ), 1 } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).Elements[ 0 ];
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );
    commandList->BuildBottomLevelAS( BuildBottomLevelASDesc{ m_bottomLevelAS.get( ) } );
    commandList->PipelineBarrier( PipelineBarrierDesc{ }.MemoryBarrier( MemoryBarrierDesc{
        .BottomLevelAS = m_bottomLevelAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } ) );
    commandList->BuildTopLevelAS( BuildTopLevelASDesc{ m_topLevelAS.get( ) } );
    commandList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = syncFence.get( );
    commandQueue->ExecuteCommandLists( executeDesc );

    syncFence->Wait( );
    commandQueue->WaitIdle( );
}

void RayTracedTriangleExample::CreateShaderBindingTable( )
{
    ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline             = m_rayTracingPipeline.get( );
    bindingTableDesc.MaxHitGroupDataBytes = sizeof( float ) * 4;

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
