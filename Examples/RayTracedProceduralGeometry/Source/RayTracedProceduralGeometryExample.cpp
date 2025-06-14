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
#include "DenOfIzExamples/RayTracedProceduralGeometryExample.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"

using namespace DirectX;
using namespace DenOfIz;

void RayTracedProceduralGeometryExample::Init( )
{
    CreateRenderTargets( );
    BuildProceduralGeometryAABBs( );
    CreateResources( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );
}

void RayTracedProceduralGeometryExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void RayTracedProceduralGeometryExample::BuildProceduralGeometryAABBs( )
{
    constexpr auto     aabbGrid     = XMINT3( 4, 1, 4 );
    constexpr XMFLOAT3 basePosition = {
        -( aabbGrid.x * c_aabbWidth + ( aabbGrid.x - 1 ) * c_aabbDistance ) / 2.0f,
        -( aabbGrid.y * c_aabbWidth + ( aabbGrid.y - 1 ) * c_aabbDistance ) / 2.0f,
        -( aabbGrid.z * c_aabbWidth + ( aabbGrid.z - 1 ) * c_aabbDistance ) / 2.0f,
    };

    constexpr XMFLOAT3 stride         = { c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance };
    auto               InitializeAABB = [ & ]( const XMFLOAT3 &offsetIndex, const XMFLOAT3 &size )
    {
        AABBBoundingBox aabb{ };
        aabb.MinX = basePosition.x + offsetIndex.x * stride.x;
        aabb.MinY = basePosition.y + offsetIndex.y * stride.y;
        aabb.MinZ = basePosition.z + offsetIndex.z * stride.z;
        aabb.MaxX = aabb.MinX + size.x;
        aabb.MaxY = aabb.MinY + size.y;
        aabb.MaxZ = aabb.MinZ + size.z;
        return aabb;
    };

    m_aabbs.resize( IntersectionShaderType::TotalPrimitiveCount );
    UINT offset = 0;

    // Analytic primitives
    {
        m_aabbs[ offset + AnalyticPrimitive::AABB ]    = InitializeAABB( XMFLOAT3( 3, 0, 0 ), XMFLOAT3( 2, 3, 2 ) );
        m_aabbs[ offset + AnalyticPrimitive::Spheres ] = InitializeAABB( XMFLOAT3( 2.25f, 0, 0.75f ), XMFLOAT3( 3, 3, 3 ) );
        offset += AnalyticPrimitive::Count;
    }

    // Volumetric primitives
    {
        m_aabbs[ offset + VolumetricPrimitive::MetaBalls ] = InitializeAABB( XMFLOAT3( 0, 0, 0 ), XMFLOAT3( 3, 3, 3 ) );
        offset += VolumetricPrimitive::Count;
    }

    // Signed distance primitives
    {
        m_aabbs[ offset + SignedDistancePrimitive::MiniSpheres ]          = InitializeAABB( XMFLOAT3( 2, 0, 0 ), XMFLOAT3( 2, 2, 2 ) );
        m_aabbs[ offset + SignedDistancePrimitive::TwistedTorus ]         = InitializeAABB( XMFLOAT3( 0, 0, 1 ), XMFLOAT3( 2, 2, 2 ) );
        m_aabbs[ offset + SignedDistancePrimitive::IntersectedRoundCube ] = InitializeAABB( XMFLOAT3( 0, 0, 2 ), XMFLOAT3( 2, 2, 2 ) );
        m_aabbs[ offset + SignedDistancePrimitive::SquareTorus ]          = InitializeAABB( XMFLOAT3( 0.75f, -0.1f, 2.25f ), XMFLOAT3( 3, 3, 3 ) );
        m_aabbs[ offset + SignedDistancePrimitive::Cog ]                  = InitializeAABB( XMFLOAT3( 1, 0, 0 ), XMFLOAT3( 2, 2, 2 ) );
        m_aabbs[ offset + SignedDistancePrimitive::Cylinder ]             = InitializeAABB( XMFLOAT3( 0, 0, 3 ), XMFLOAT3( 2, 3, 2 ) );
        m_aabbs[ offset + SignedDistancePrimitive::FractalPyramid ]       = InitializeAABB( XMFLOAT3( 2, 0, 2 ), XMFLOAT3( 6, 6, 6 ) );
    }
}

void RayTracedProceduralGeometryExample::HandleEvent( Event &event )
{
    switch ( event.Type )
    {
    case EventType::KeyDown:
        {
            switch ( event.Key.Keycode )
            {
            case KeyCode::G:
                m_animateGeometry = !m_animateGeometry;
                break;
            default:;
            }
            break;
        }
    default:;
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RayTracedProceduralGeometryExample::UpdateAABBPrimitiveAttributes( )
{
    const XMMATRIX mIdentity = XMMatrixIdentity( );
    const XMMATRIX mScale15y = XMMatrixScaling( 1, 1.5f, 1 );
    const XMMATRIX mScale15  = XMMatrixScaling( 1.5f, 1.5f, 1.5f );
    const XMMATRIX mScale3   = XMMatrixScaling( 3, 3, 3 );
    const XMMATRIX mRotation = XMMatrixRotationY( -2 * m_animateGeometryTime );

    auto SetTransformForAABB = [ & ]( const UINT primitiveIndex, const XMMATRIX &mScale, const XMMATRIX &mRotation )
    {
        const XMVECTOR vTranslation = 0.5f * ( XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MinX ) ) +
                                               XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MaxX ) ) );
        const XMMATRIX mTranslation = XMMatrixTranslationFromVector( vTranslation );

        const XMMATRIX mTransform                                                        = mScale * mRotation * mTranslation;
        m_aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].localSpaceToBottomLevelAS = mTransform;
        m_aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].bottomLevelASToLocalSpace = XMMatrixInverse( nullptr, mTransform );
    };

    UINT offset = 0;
    // Analytic primitives
    {
        SetTransformForAABB( offset + AnalyticPrimitive::AABB, mScale15y, mIdentity );
        SetTransformForAABB( offset + AnalyticPrimitive::Spheres, mScale15, mRotation );
        offset += AnalyticPrimitive::Count;
    }

    // Volumetric primitives
    {
        SetTransformForAABB( offset + VolumetricPrimitive::MetaBalls, mScale15, mRotation );
        offset += VolumetricPrimitive::Count;
    }

    // Signed distance primitives
    {
        SetTransformForAABB( offset + SignedDistancePrimitive::MiniSpheres, mIdentity, mIdentity );
        SetTransformForAABB( offset + SignedDistancePrimitive::IntersectedRoundCube, mIdentity, mIdentity );
        SetTransformForAABB( offset + SignedDistancePrimitive::SquareTorus, mScale15, mIdentity );
        SetTransformForAABB( offset + SignedDistancePrimitive::TwistedTorus, mIdentity, mRotation );
        SetTransformForAABB( offset + SignedDistancePrimitive::Cog, mIdentity, mRotation );
        SetTransformForAABB( offset + SignedDistancePrimitive::Cylinder, mScale15y, mIdentity );
        SetTransformForAABB( offset + SignedDistancePrimitive::FractalPyramid, mScale3, mIdentity );
    }
}

void RayTracedProceduralGeometryExample::Update( )
{
    m_camera->Update( m_stepTimer.GetDeltaTime( ) );
    const float elapsedTime = static_cast<float>( m_stepTimer.GetElapsedSeconds( ) );

    if ( m_animateGeometry )
    {
        m_animateGeometryTime += elapsedTime;
        UpdateAABBPrimitiveAttributes( );
    }

    m_sceneConstants->cameraPosition    = m_camera->Position( );
    m_sceneConstants->projectionToWorld = XMMatrixInverse( nullptr, m_camera->ViewProjectionMatrix( ) );
    m_sceneConstants->elapsedTime       = elapsedTime;

    RenderAndPresentFrame( );
}

void RayTracedProceduralGeometryExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_raytracingOutput[ frameIndex ].get( ), ResourceUsage::UnorderedAccess );
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

    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_raytracingOutput[ frameIndex ].get( ), ResourceUsage::CopySrc );
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

void RayTracedProceduralGeometryExample::CreateRenderTargets( )
{
    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor   = ResourceDescriptor::RWTexture;
    textureDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    textureDesc.Usages       = ResourceUsage::CopySrc | ResourceUsage::UnorderedAccess;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName   = InteropString( "RayTracing Output " ).Append( std::to_string( i ).c_str( ) );
        m_raytracingOutput[ i ] = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
        m_resourceTracking.TrackTexture( m_raytracingOutput[ i ].get( ), ResourceUsage::Common );
    }
}

void RayTracedProceduralGeometryExample::CreateAccelerationStructures( )
{
    {
        BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags = ASBuildFlags::PreferFastTrace;

        std::array<ASGeometryDesc, IntersectionShaderType::TotalPrimitiveCount> aabbGeometries{ {} };
        for ( UINT i = 0; i < IntersectionShaderType::TotalPrimitiveCount; i++ )
        {
            ASGeometryDesc &aabbGeometryDesc = aabbGeometries[ i ];
            aabbGeometryDesc.Type            = HitGroupType::AABBs;
            aabbGeometryDesc.AABBs.Buffer    = m_aabbBuffer.get( );
            aabbGeometryDesc.AABBs.Stride    = sizeof( AABBBoundingBox );
            aabbGeometryDesc.AABBs.NumAABBs  = 1;
            aabbGeometryDesc.AABBs.Offset    = i * sizeof( AABBBoundingBox );
            aabbGeometryDesc.Flags           = GeometryFlags::Opaque;
        }

        bottomLevelDesc.Geometries.Elements    = aabbGeometries.data( );
        bottomLevelDesc.Geometries.NumElements = aabbGeometries.size( );

        m_aabbAS = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelDesc ) );
    }

    {
        ASGeometryDesc triangleGeometryDesc{ };
        triangleGeometryDesc.Type                   = HitGroupType::Triangles;
        triangleGeometryDesc.Triangles.IndexBuffer  = m_indexBuffer.get( );
        triangleGeometryDesc.Triangles.NumIndices   = 6;
        triangleGeometryDesc.Triangles.IndexType    = IndexType::Uint16;
        triangleGeometryDesc.Triangles.VertexFormat = Format::R32G32B32Float;
        triangleGeometryDesc.Triangles.VertexBuffer = m_vertexBuffer.get( );
        triangleGeometryDesc.Triangles.VertexStride = sizeof( Vertex );
        triangleGeometryDesc.Triangles.NumVertices  = 4;
        triangleGeometryDesc.Flags                  = GeometryFlags::Opaque;

        BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags             = ASBuildFlags::PreferFastTrace;
        bottomLevelDesc.Geometries.Elements    = &triangleGeometryDesc;
        bottomLevelDesc.Geometries.NumElements = 1;
        m_triangleAS                           = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelDesc ) );
    }

    {
        ASInstanceDesc aabbInstanceDesc{ };
        aabbInstanceDesc.Mask                        = 1;
        aabbInstanceDesc.BLAS                        = m_aabbAS.get( );
        aabbInstanceDesc.ContributionToHitGroupIndex = 2;
        aabbInstanceDesc.ID                          = 1;

        std::array<float, 12> aabbTransform( { } );
        aabbTransform[ 0 ]  = 1.0f;
        aabbTransform[ 5 ]  = 1.0f;
        aabbTransform[ 10 ] = 1.0f;
        // Move float:
        aabbTransform[ 7 ] = 1.0f;

        aabbInstanceDesc.Transform.Elements    = aabbTransform.data( );
        aabbInstanceDesc.Transform.NumElements = aabbTransform.size( );

        ASInstanceDesc triangleInstanceDesc{ };
        triangleInstanceDesc.Mask                        = 1;
        triangleInstanceDesc.BLAS                        = m_triangleAS.get( );
        triangleInstanceDesc.ContributionToHitGroupIndex = 0;
        triangleInstanceDesc.ID                          = 0;

        constexpr XMUINT3 NUM_AABB( 700, 1, 700 );
        constexpr auto    fWidth = XMFLOAT3( NUM_AABB.x * c_aabbWidth + ( NUM_AABB.x - 1 ) * c_aabbDistance, NUM_AABB.y * c_aabbWidth + ( NUM_AABB.y - 1 ) * c_aabbDistance,
                                             NUM_AABB.z * c_aabbWidth + ( NUM_AABB.z - 1 ) * c_aabbDistance );

        XMFLOAT3       basePosition( fWidth.x * -0.35f, 0.0f, fWidth.z * -0.35f );
        const XMVECTOR vBasePosition = XMLoadFloat3( &basePosition );
        XMMATRIX       mScale        = XMMatrixScaling( fWidth.x, fWidth.y, fWidth.z );
        XMMATRIX       mTranslation  = XMMatrixTranslationFromVector( vBasePosition );
        XMMATRIX       mTransform    = mScale * mTranslation;

        float transform[ 12 ];
        XMStoreFloat3x4( reinterpret_cast<XMFLOAT3X4 *>( transform ), mTransform );
        std::array<float, 12> triangleTransform( { } );
        for ( int i = 0; i < 12; i++ )
        {
            triangleTransform[ i ] = transform[ i ];
        }
        triangleInstanceDesc.Transform.Elements    = triangleTransform.data( );
        triangleInstanceDesc.Transform.NumElements = triangleTransform.size( );

        std::array<ASInstanceDesc, 2> instances{ {} };
        instances[ 0 ] = aabbInstanceDesc;
        instances[ 1 ] = triangleInstanceDesc;

        TopLevelASDesc topLevelDesc{ };
        topLevelDesc.BuildFlags            = ASBuildFlags::PreferFastTrace;
        topLevelDesc.Instances.Elements    = instances.data( );
        topLevelDesc.Instances.NumElements = instances.size( );

        m_topLevelAS = std::unique_ptr<ITopLevelAS>( m_logicalDevice->CreateTopLevelAS( topLevelDesc ) );
    }

    CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = QueueType::Compute;
    auto commandQueue          = std::unique_ptr<ICommandQueue>( m_logicalDevice->CreateCommandQueue( commandQueueDesc ) );

    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { commandQueue.get( ) } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).Elements[ 0 ];
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );
    commandList->BuildBottomLevelAS( { m_triangleAS.get( ) } );
    commandList->BuildBottomLevelAS( { m_aabbAS.get( ) } );

    PipelineBarrierDesc barrier{ };
    // Issue UAV barriers for BLAS, their initial state is always: AccelerationStructureWrite
    barrier.MemoryBarrier( { .BottomLevelAS = m_aabbAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } );
    barrier.MemoryBarrier( { .BottomLevelAS = m_triangleAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } );
    commandList->PipelineBarrier( barrier );

    commandList->BuildTopLevelAS( { m_topLevelAS.get( ) } );

    barrier = { };
    // Issue UAV barriers for TLAS, their initial state is always: AccelerationStructureWrite
    barrier.MemoryBarrier( { .TopLevelAS = m_topLevelAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } );
    commandList->PipelineBarrier( barrier );
    commandList->End( );

    ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = syncFence.get( );
    commandQueue->ExecuteCommandLists( executeDesc );

    syncFence->Wait( );
    m_graphicsQueue->WaitIdle( );
}

void RayTracedProceduralGeometryExample::CreateResources( )
{
    constexpr uint16_t indices[] = { 0, 1, 2, 0, 3, 1 };

    constexpr Vertex vertices[] = { { { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 1.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } } };

    BufferDesc vbDesc{ };
    vbDesc.Descriptor   = ResourceDescriptor::Buffer; // These are not real vertex buffers
    vbDesc.InitialUsage = ResourceUsage::CopyDst;
    vbDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::AccelerationStructureGeometry;
    vbDesc.NumBytes     = sizeof( vertices );
    vbDesc.DebugName    = "Plane_VertexBuffer";
    m_vertexBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc{ };
    ibDesc.Descriptor   = ResourceDescriptor::Buffer; // Not real index buffer
    ibDesc.InitialUsage = ResourceUsage::CopyDst;
    ibDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::AccelerationStructureGeometry;
    ibDesc.NumBytes     = sizeof( indices );
    ibDesc.DebugName    = "Plane_IndexBuffer";
    m_indexBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

    BufferDesc aabbDesc{ };
    aabbDesc.Descriptor   = ResourceDescriptor::Buffer;
    aabbDesc.InitialUsage = ResourceUsage::CopyDst;
    aabbDesc.Usages       = ResourceUsage::CopyDst | ResourceUsage::AccelerationStructureGeometry;
    aabbDesc.NumBytes     = sizeof( AABBBoundingBox ) * IntersectionShaderType::TotalPrimitiveCount;
    aabbDesc.DebugName    = "AABB_Buffer";
    m_aabbBuffer          = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( aabbDesc ) );

    BufferDesc attributesDesc{ };
    attributesDesc.HeapType                  = HeapType::CPU_GPU;
    attributesDesc.Descriptor                = ResourceDescriptor::Buffer | ResourceDescriptor::StructuredBuffer;
    attributesDesc.NumBytes                  = sizeof( PrimitiveInstancePerFrameBuffer ) * IntersectionShaderType::TotalPrimitiveCount;
    attributesDesc.StructureDesc.NumElements = IntersectionShaderType::TotalPrimitiveCount;
    attributesDesc.StructureDesc.Stride      = sizeof( PrimitiveInstancePerFrameBuffer );
    attributesDesc.DebugName                 = "AABB_Attributes_Buffer";
    m_aabbPrimitiveAttributeBuffer           = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( attributesDesc ) );
    m_aabbPrimitiveAttributeBufferMemory     = static_cast<PrimitiveInstancePerFrameBuffer *>( m_aabbPrimitiveAttributeBuffer->MapMemory( ) );

    BatchResourceCopy batchResourceCopy( m_logicalDevice );
    batchResourceCopy.Begin( );

    for ( int i = 0; i < IntersectionShaderType::TotalPrimitiveCount; ++i )
    {
        m_aabbPrimitiveAttributeBufferMemory[ i ].localSpaceToBottomLevelAS = XMMatrixIdentity( );
        m_aabbPrimitiveAttributeBufferMemory[ i ].bottomLevelASToLocalSpace = XMMatrixIdentity( );
    }

    CopyToGpuBufferDesc copyToGpuBufferDesc{ };
    copyToGpuBufferDesc.DstBuffer        = m_vertexBuffer.get( );
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    copyToGpuBufferDesc.Data.NumElements = sizeof( vertices );
    batchResourceCopy.CopyToGPUBuffer( copyToGpuBufferDesc );

    copyToGpuBufferDesc.DstBuffer        = m_indexBuffer.get( );
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( &indices[ 0 ] );
    copyToGpuBufferDesc.Data.NumElements = sizeof( indices );
    batchResourceCopy.CopyToGPUBuffer( copyToGpuBufferDesc );

    copyToGpuBufferDesc.DstBuffer        = m_aabbBuffer.get( );
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( m_aabbs.data( ) );
    copyToGpuBufferDesc.Data.NumElements = sizeof( AABBBoundingBox ) * m_aabbs.size( );
    batchResourceCopy.CopyToGPUBuffer( copyToGpuBufferDesc );
    batchResourceCopy.Submit( );

    InitializeScene( );
    UpdateAABBPrimitiveAttributes( );
}

void RayTracedProceduralGeometryExample::CreateRayTracingPipeline( )
{
    std::array<ShaderStageDesc, 8>     shaderStages( { } );
    std::array<ResourceBindingSlot, 1> localBindings( { } );
    localBindings[ 0 ].RegisterSpace = 3;
    localBindings[ 0 ].Binding       = 1;
    localBindings[ 0 ].Type          = ResourceBindingType::ConstantBuffer;

    { // Create shaders
        int32_t shaderIndex = 0;

        ShaderStageDesc &rayGenShaderDesc = shaderStages[ shaderIndex++ ];
        rayGenShaderDesc.Stage            = ShaderStage::Raygen;
        rayGenShaderDesc.Path             = "Assets/Shaders/RTProceduralGeometry/RayGen.hlsl";
        rayGenShaderDesc.EntryPoint       = "MyRaygenShader";

        ShaderStageDesc &missShaderDesc = shaderStages[ shaderIndex++ ];
        missShaderDesc.Stage            = ShaderStage::Miss;
        missShaderDesc.Path             = "Assets/Shaders/RTProceduralGeometry/Miss.hlsl";
        missShaderDesc.EntryPoint       = "MyMissShader";

        ShaderStageDesc &shadowMissShaderDesc = shaderStages[ shaderIndex++ ];
        shadowMissShaderDesc.Stage            = ShaderStage::Miss;
        shadowMissShaderDesc.Path             = "Assets/Shaders/RTProceduralGeometry/Miss.hlsl";
        shadowMissShaderDesc.EntryPoint       = "MyMissShader_ShadowRay";

        m_closestHitTriangleIndex                                  = shaderIndex++;
        ShaderStageDesc &triangleHitShaderDesc                     = shaderStages[ m_closestHitTriangleIndex ];
        triangleHitShaderDesc.Stage                                = ShaderStage::ClosestHit;
        triangleHitShaderDesc.Path                                 = "Assets/Shaders/RTProceduralGeometry/ClosestHit.hlsl";
        triangleHitShaderDesc.EntryPoint                           = "MyClosestHitShader_Triangle";
        triangleHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
        triangleHitShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );

        m_closestHitAABBIndex                                  = shaderIndex++;
        ShaderStageDesc &aabbHitShaderDesc                     = shaderStages[ m_closestHitAABBIndex ];
        aabbHitShaderDesc.Stage                                = ShaderStage::ClosestHit;
        aabbHitShaderDesc.Path                                 = "Assets/Shaders/RTProceduralGeometry/ClosestHit.hlsl";
        aabbHitShaderDesc.EntryPoint                           = "MyClosestHitShader_AABB";
        aabbHitShaderDesc.RayTracing.HitGroupType              = HitGroupType::AABBs;
        aabbHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
        aabbHitShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );
        m_firstIntersectionShaderIndex                         = shaderIndex;

        const char *aabbHitGroupTypes[] = { "AnalyticPrimitive", "VolumetricPrimitive", "SignedDistancePrimitive" };
        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            ShaderStageDesc &intersectionShaderDesc = shaderStages[ shaderIndex++ ];
            intersectionShaderDesc.Stage            = ShaderStage::Intersection;
            auto aabbShaderPath                     = InteropString( "Assets/Shaders/RTProceduralGeometry/Intersection_" ).Append( aabbHitGroupTypes[ i ] ).Append( ".hlsl" );
            intersectionShaderDesc.Path             = aabbShaderPath;
            intersectionShaderDesc.EntryPoint       = InteropString( "MyIntersectionShader_" ).Append( aabbHitGroupTypes[ i ] );
            intersectionShaderDesc.RayTracing.HitGroupType              = HitGroupType::AABBs;
            intersectionShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
            intersectionShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );
        }
    }

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements           = shaderStages.data( );
    programDesc.ShaderStages.NumElements        = shaderStages.size( );
    programDesc.RayTracing.MaxRecursionDepth    = MAX_RAY_RECURSION_DEPTH;
    programDesc.RayTracing.MaxNumPayloadBytes   = sizeof( RayPayload );
    programDesc.RayTracing.MaxNumAttributeBytes = sizeof( ProceduralPrimitiveAttributes );
    m_rayTracingProgram                         = std::make_unique<ShaderProgram>( programDesc );

    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    m_hgLocalRootSignature =
        std::unique_ptr<ILocalRootSignature>( m_logicalDevice->CreateLocalRootSignature( reflection.LocalRootSignatures.Elements[ m_closestHitTriangleIndex ] ) );

    std::vector<HitGroupDesc> hitGroupDescs;
    { // Create hit groups
        HitGroupDesc &hitGroupDesc1         = hitGroupDescs.emplace_back( HitGroupDesc{ } );
        hitGroupDesc1.Type                  = HitGroupType::Triangles;
        hitGroupDesc1.Name                  = "MyHitGroup_Triangle";
        hitGroupDesc1.ClosestHitShaderIndex = m_closestHitTriangleIndex;
        hitGroupDesc1.LocalRootSignature    = m_hgLocalRootSignature.get( );

        HitGroupDesc &hitGroupDesc2      = hitGroupDescs.emplace_back( HitGroupDesc{ } );
        hitGroupDesc2.Type               = HitGroupType::Triangles;
        hitGroupDesc2.Name               = "MyHitGroup_Triangle_ShadowRay";
        hitGroupDesc2.LocalRootSignature = m_hgLocalRootSignature.get( );

        const char *aabbHitGroupTypes[] = { "AnalyticPrimitive", "VolumetricPrimitive", "SignedDistancePrimitive" };
        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            for ( int rayType = 0; rayType < RayType::Count; rayType++ )
            {
                std::string hitGroupName = "MyHitGroup_AABB_" + std::string( aabbHitGroupTypes[ i ] );
                if ( rayType == 1 )
                {
                    hitGroupName += "_ShadowRay";
                }
                HitGroupDesc &hitGroupDesc           = hitGroupDescs.emplace_back( HitGroupDesc{ } );
                hitGroupDesc.Type                    = HitGroupType::AABBs;
                hitGroupDesc.Name                    = hitGroupName.c_str( );
                hitGroupDesc.ClosestHitShaderIndex   = m_closestHitAABBIndex;
                hitGroupDesc.IntersectionShaderIndex = m_firstIntersectionShaderIndex + i;
                if ( rayType == 1 )
                {
                    hitGroupDesc.ClosestHitShaderIndex = -1;
                }
                hitGroupDesc.LocalRootSignature = m_hgLocalRootSignature.get( );
            }
        }
    }

    // Create pipeline state object
    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                        = BindPoint::RayTracing;
    pipelineDesc.RootSignature                    = m_rayTracingRootSignature.get( );
    pipelineDesc.ShaderProgram                    = m_rayTracingProgram.get( );
    pipelineDesc.RayTracing.HitGroups.Elements    = hitGroupDescs.data( );
    pipelineDesc.RayTracing.HitGroups.NumElements = hitGroupDescs.size( );

    m_rayTracingPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    // Create resource bind groups
    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rayTracingRootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;

    for ( int i = 0; i < 3; ++i )
    {
        m_rayTracingBindGroups[ i ] = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
        m_rayTracingBindGroups[ i ]->BeginUpdate( );

        m_rayTracingBindGroups[ i ]->Srv( 0, m_topLevelAS.get( ) );            // g_scene
        m_rayTracingBindGroups[ i ]->Uav( 0, m_raytracingOutput[ i ].get( ) ); // g_renderTarget
        m_rayTracingBindGroups[ i ]->Cbv( 0, m_sceneConstantBuffer.get( ) );   // g_sceneCB

        m_rayTracingBindGroups[ i ]->Srv( 1, m_indexBuffer.get( ) );                  // g_indices
        m_rayTracingBindGroups[ i ]->Srv( 2, m_vertexBuffer.get( ) );                 // g_vertices
        m_rayTracingBindGroups[ i ]->Srv( 3, m_aabbPrimitiveAttributeBuffer.get( ) ); // g_AABBPrimitiveAttributes

        m_rayTracingBindGroups[ i ]->EndUpdate( );
    }
}

void RayTracedProceduralGeometryExample::InitializeScene( )
{
    m_aabbTransformsPerFrame.resize( 3 );
    for ( int i = 0; i < 3; ++i )
    {
        m_aabbTransformsPerFrame[ i ].resize( IntersectionShaderType::TotalPrimitiveCount );
    }

    // Setup materials
    m_planeMaterialCB = {
        .albedo = XMFLOAT4( 0.9f, 0.9f, 0.9f, 1.0f ), .reflectanceCoef = 0.25f, .diffuseCoef = 1.0f, .specularCoef = 0.4f, .specularPower = 50.0f, .stepScale = 1.0f
    };

    // Albedos
    constexpr auto green  = XMFLOAT4( 0.1f, 1.0f, 0.5f, 1.0f );
    constexpr auto red    = XMFLOAT4( 1.0f, 0.5f, 0.5f, 1.0f );
    constexpr auto yellow = XMFLOAT4( 1.0f, 1.0f, 0.5f, 1.0f );

    m_aabbMaterials.resize( IntersectionShaderType::TotalPrimitiveCount );
    auto SetAttributes = [ & ]( const UINT primitiveIndex, const XMFLOAT4 &albedo, const float reflectanceCoefficient = 0.0f, const float diffuseCoefficient = 0.9f,
                                const float specularCoefficient = 0.7f, const float specularPower = 50.0f, const float stepScale = 1.0f )
    {
        PrimitiveConstantBuffer &mat = m_aabbMaterials[ primitiveIndex ];
        mat.albedo                   = albedo;
        mat.reflectanceCoef          = reflectanceCoefficient;
        mat.diffuseCoef              = diffuseCoefficient;
        mat.specularCoef             = specularCoefficient;
        mat.specularPower            = specularPower;
        mat.stepScale                = stepScale;
    };

    UINT offset = 0;
    // Analytic primitives
    {
        SetAttributes( offset + AnalyticPrimitive::AABB, red );
        SetAttributes( offset + AnalyticPrimitive::Spheres, ChromiumReflectance, 1 );
        offset += AnalyticPrimitive::Count;
    }

    // Volumetric primitives
    {
        SetAttributes( offset + VolumetricPrimitive::MetaBalls, ChromiumReflectance, 1 );
        offset += VolumetricPrimitive::Count;
    }

    // Signed distance primitives
    {
        SetAttributes( offset + SignedDistancePrimitive::MiniSpheres, green );
        SetAttributes( offset + SignedDistancePrimitive::IntersectedRoundCube, green );
        SetAttributes( offset + SignedDistancePrimitive::SquareTorus, ChromiumReflectance, 1 );
        SetAttributes( offset + SignedDistancePrimitive::TwistedTorus, yellow, 0, 1.0f, 0.7f, 50, 0.5f );
        SetAttributes( offset + SignedDistancePrimitive::Cog, yellow, 0, 1.0f, 0.1f, 2 );
        SetAttributes( offset + SignedDistancePrimitive::Cylinder, red );
        SetAttributes( offset + SignedDistancePrimitive::FractalPyramid, green, 0, 1, 0.1f, 4, 0.8f );
    }

    // Create scene constant buffer
    BufferDesc sceneBufferDesc{ };
    sceneBufferDesc.HeapType   = HeapType::CPU_GPU;
    sceneBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    sceneBufferDesc.NumBytes   = sizeof( SceneConstantBuffer );
    sceneBufferDesc.Usages     = ResourceUsage::CopyDst | ResourceUsage::VertexAndConstantBuffer;
    sceneBufferDesc.DebugName  = "SceneConstantBuffer";
    m_sceneConstantBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( sceneBufferDesc ) );
    m_sceneConstants           = static_cast<SceneConstantBuffer *>( m_sceneConstantBuffer->MapMemory( ) );

    XMFLOAT4 lightPosition;
    XMFLOAT4 lightAmbientColor;
    XMFLOAT4 lightDiffuseColor;

    lightPosition                   = XMFLOAT4( 0.0f, 18.0f, -20.0f, 0.0f );
    m_sceneConstants->lightPosition = XMLoadFloat4( &lightPosition );

    lightAmbientColor                   = XMFLOAT4( 0.25f, 0.25f, 0.25f, 1.0f );
    m_sceneConstants->lightAmbientColor = XMLoadFloat4( &lightAmbientColor );

    constexpr float diffuse             = 0.6f;
    lightDiffuseColor                   = XMFLOAT4( diffuse, diffuse, diffuse, 1.0f );
    m_sceneConstants->lightDiffuseColor = XMLoadFloat4( &lightDiffuseColor );

    InitCamera( );
}

void RayTracedProceduralGeometryExample::InitCamera( ) const
{
    auto           eye    = XMVectorSet( 0.0f, 5.3f, -17.0f, 1.0f );
    const XMMATRIX rotate = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    eye                   = XMVector3Transform( eye, rotate );

    m_camera->SetPosition( eye );
    m_camera->SetFront( XMVECTOR{ 0.67f, -0.29f, 0.67f, 0.0f } );

    m_sceneConstants->cameraPosition    = m_camera->Position( );
    m_sceneConstants->projectionToWorld = XMMatrixInverse( nullptr, m_camera->ViewProjectionMatrix( ) );
}

void RayTracedProceduralGeometryExample::CreateShaderBindingTable( )
{
    uint32_t numHitGroups = RayType::Count;

    for ( uint32_t shaderType = 0; shaderType < IntersectionShaderType::Count; shaderType++ )
    {
        numHitGroups += PerPrimitiveTypeCount( static_cast<IntersectionShaderType::Enum>( shaderType ) ) * RayType::Count;
    }
    // Create shader binding table
    ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline                = m_rayTracingPipeline.get( );
    bindingTableDesc.SizeDesc.NumHitGroups   = numHitGroups;
    bindingTableDesc.SizeDesc.NumMissShaders = 2;
    bindingTableDesc.MaxHitGroupDataBytes    = sizeof( LocalData );

    m_shaderBindingTable = std::unique_ptr<IShaderBindingTable>( m_logicalDevice->CreateShaderBindingTable( bindingTableDesc ) );

    // Bind ray generation shader
    RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = "MyRaygenShader";
    m_shaderBindingTable->BindRayGenerationShader( rayGenDesc );

    // Bind miss shaders
    {
        MissBindingDesc missDesc{ };
        missDesc.ShaderName = "MyMissShader";
        m_shaderBindingTable->BindMissShader( missDesc );

        MissBindingDesc shadowMissDesc{ };
        shadowMissDesc.ShaderName = "MyMissShader_ShadowRay";
        shadowMissDesc.Offset     = 1;
        m_shaderBindingTable->BindMissShader( shadowMissDesc );
    }

    LocalData localData{ };
    uint32_t  hitGroupOffset = 0;
    {
        const auto triangleHitGroupData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderLocalData( { m_hgLocalRootSignature.get( ) } ) );

        localData.materialCB = m_planeMaterialCB;
        localData.aabbCB     = { 0, 0, 0, 0 };

        ByteArrayView localDataView( reinterpret_cast<const Byte *>( &localData ), sizeof( LocalData ) );
        triangleHitGroupData->Cbv( 1, localDataView );

        // Create separate entries for each ray type
        for ( uint32_t rayType = 0; rayType < 2; rayType++ )
        {
            HitGroupBindingDesc triangleHitGroupDesc{ };
            triangleHitGroupDesc.HitGroupExportName = rayType == 0 ? "MyHitGroup_Triangle" : "MyHitGroup_Triangle_ShadowRay";
            triangleHitGroupDesc.Data               = triangleHitGroupData.get( );
            triangleHitGroupDesc.Offset             = hitGroupOffset++;
            m_shaderBindingTable->BindHitGroup( triangleHitGroupDesc );
        }
    }

    // AABB geometry
    {
        for ( uint32_t shaderType = 0, instanceIndex = 0; shaderType < IntersectionShaderType::Count; shaderType++ )
        {
            const char *shaderTypeName = shaderType == 0 ? "AnalyticPrimitive" : shaderType == 1 ? "VolumetricPrimitive" : "SignedDistancePrimitive";

            const uint32_t numPrimitiveTypes = PerPrimitiveTypeCount( static_cast<IntersectionShaderType::Enum>( shaderType ) );
            // Primitives for each intersection shader.
            for ( uint32_t primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++, instanceIndex++ )
            {
                auto hitGroupData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderLocalData( { m_hgLocalRootSignature.get( ) } ) );

                localData.materialCB = m_aabbMaterials[ instanceIndex ];
                localData.aabbCB     = { .instanceIndex = instanceIndex, .primitiveType = primitiveIndex };

                ByteArrayView localDataView( reinterpret_cast<const Byte *>( &localData ), sizeof( LocalData ) );
                hitGroupData->Cbv( 1, localDataView );

                // Ray types.
                for ( uint32_t rayType = 0; rayType < RayType::Count; rayType++ )
                {

                    HitGroupBindingDesc hitGroupDesc{ };
                    hitGroupDesc.HitGroupExportName = InteropString( "MyHitGroup_AABB_" ).Append( shaderTypeName );
                    if ( rayType == 1 )
                    {
                        hitGroupDesc.HitGroupExportName.Append( "_ShadowRay" );
                    }
                    hitGroupDesc.Data   = hitGroupData.get( );
                    hitGroupDesc.Offset = hitGroupOffset++;
                    m_shaderBindingTable->BindHitGroup( hitGroupDesc );
                }
            }
        }
    }

    m_shaderBindingTable->Build( );
}

void RayTracedProceduralGeometryExample::Quit( )
{
    m_frameSync->WaitIdle( );
    m_aabbPrimitiveAttributeBuffer->UnmapMemory( );
    m_aabbPrimitiveAttributeBufferMemory = nullptr;
    m_sceneConstantBuffer->UnmapMemory( );
    m_sceneConstants = nullptr;
    IExample::Quit( );
}
