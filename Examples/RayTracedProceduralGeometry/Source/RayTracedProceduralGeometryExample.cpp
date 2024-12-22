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
#include <DenOfIzExamples/RayTracedProceduralGeometryExample.h>

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
    raytracingNode.Execute = this;

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

void RayTracedProceduralGeometryExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
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
        D3D12_RAYTRACING_AABB aabb;
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

void RayTracedProceduralGeometryExample::HandleEvent( SDL_Event &event )
{
    switch ( event.type )
    {
    case SDL_KEYDOWN:
        {
            switch ( event.key.keysym.sym )
            {
            case SDLK_g:
                m_animateGeometry = !m_animateGeometry;
                break;
            default:;
            }
            break;
        }
    default:;
    }

    IExample::HandleEvent( event );
}

void RayTracedProceduralGeometryExample::UpdateAABBPrimitiveAttributes( const float animationTime ) const
{
    const XMMATRIX mIdentity = XMMatrixIdentity( );
    const XMMATRIX mScale15y = XMMatrixScaling( 1, 1.5f, 1 );
    const XMMATRIX mScale15  = XMMatrixScaling( 1.5f, 1.5f, 1.5f );
    const XMMATRIX mScale3   = XMMatrixScaling( 3, 3, 3 );
    const XMMATRIX mRotation = XMMatrixRotationY( -2 * animationTime );

    std::vector<PrimitiveInstancePerFrameBuffer> attributeData( IntersectionShaderType::TotalPrimitiveCount );
    // Helper to convert matrix to transform array
    auto AddTransform = [ &attributeData ]( const uint32_t index, const XMMATRIX &transform )
    {
        PrimitiveInstancePerFrameBuffer &attr = attributeData[ index ];
        XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &attr.localSpaceToBottomLevelAS ), transform );
        XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &attr.bottomLevelASToLocalSpace ), XMMatrixInverse( nullptr, transform ) );
    };

    UINT offset = 0;

    // Analytic primitives
    {
        XMMATRIX transform = mScale15y * mIdentity;
        AddTransform( offset + AnalyticPrimitive::AABB, transform );

        transform = mScale15 * mRotation;
        AddTransform( offset + AnalyticPrimitive::Spheres, transform );
        offset += AnalyticPrimitive::Count;
    }

    // Volumetric primitives
    {
        const XMMATRIX transform = mScale15 * mRotation;
        AddTransform( offset + VolumetricPrimitive::MetaBalls, transform );
        offset += VolumetricPrimitive::Count;
    }

    // Signed distance primitives
    {
        AddTransform( offset + SignedDistancePrimitive::MiniSpheres, mIdentity );
        AddTransform( offset + SignedDistancePrimitive::IntersectedRoundCube, mIdentity );
        AddTransform( offset + SignedDistancePrimitive::SquareTorus, mScale15 );
        AddTransform( offset + SignedDistancePrimitive::TwistedTorus, mRotation );
        AddTransform( offset + SignedDistancePrimitive::Cog, mRotation );
        AddTransform( offset + SignedDistancePrimitive::Cylinder, mScale15y );
        AddTransform( offset + SignedDistancePrimitive::FractalPyramid, mScale3 );
    }

    memcpy( m_aabbPrimitiveAttributeBufferMemory, attributeData.data( ), sizeof( PrimitiveInstancePerFrameBuffer ) * IntersectionShaderType::TotalPrimitiveCount );
}

void RayTracedProceduralGeometryExample::Update( )
{
    m_time.Tick( );
    if ( m_animateGeometry )
    {
        m_animateGeometryTime += m_time.GetDeltaTime( );
        UpdateAABBPrimitiveAttributes( m_animateGeometryTime );
    }
    m_renderGraph->Update( );
}

void RayTracedProceduralGeometryExample::Execute( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->BindPipeline( m_rayTracingPipeline.get( ) );
    commandList->BindResourceGroup( m_rayTracingBindGroups[ frameIndex ].get( ) );

    DispatchRaysDesc dispatchRaysDesc{ };
    dispatchRaysDesc.Width              = m_windowDesc.Width;
    dispatchRaysDesc.Height             = m_windowDesc.Height;
    dispatchRaysDesc.Depth              = 1;
    dispatchRaysDesc.ShaderBindingTable = m_shaderBindingTable.get( );
    commandList->DispatchRays( dispatchRaysDesc );
}

void RayTracedProceduralGeometryExample::Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *renderTarget )
{
    // Not used
}

void RayTracedProceduralGeometryExample::CreateRenderTargets( )
{
    TextureDesc textureDesc{ };
    textureDesc.Width        = m_windowDesc.Width;
    textureDesc.Height       = m_windowDesc.Height;
    textureDesc.Format       = Format::B8G8R8A8Unorm;
    textureDesc.Descriptor   = BitSet( ResourceDescriptor::RWTexture );
    textureDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    textureDesc.Usages       = BitSet( ResourceUsage::CopySrc ) | ResourceUsage::UnorderedAccess;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        textureDesc.DebugName   = InteropString( "RayTracing Output " ).Append( std::to_string( i ).c_str( ) );
        m_raytracingOutput[ i ] = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );
    }
}

void RayTracedProceduralGeometryExample::CreateAccelerationStructures( )
{
    {
        ASGeometryDesc aabbGeometryDesc{ };
        aabbGeometryDesc.Type           = HitGroupType::AABBs;
        aabbGeometryDesc.AABBs.Buffer   = m_aabbBuffer.get( );
        aabbGeometryDesc.AABBs.Stride   = sizeof( D3D12_RAYTRACING_AABB );
        aabbGeometryDesc.AABBs.NumAABBs = IntersectionShaderType::TotalPrimitiveCount;
        aabbGeometryDesc.Flags          = GeometryFlags::Opaque;

        BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags = ASBuildFlags::PreferFastTrace;
        bottomLevelDesc.Geometries.AddElement( aabbGeometryDesc );
        m_aabbAS = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelDesc ) );
    }

    {
        ASGeometryDesc triangleGeometryDesc{ };
        triangleGeometryDesc.Type                   = HitGroupType::Triangles;
        triangleGeometryDesc.Triangles.IndexBuffer  = m_indexBuffer.get( );
        triangleGeometryDesc.Triangles.NumIndices   = 6; // Two triangles for plane
        triangleGeometryDesc.Triangles.IndexType    = IndexType::Uint16;
        triangleGeometryDesc.Triangles.VertexFormat = Format::R32G32B32Float;
        triangleGeometryDesc.Triangles.VertexBuffer = m_vertexBuffer.get( );
        triangleGeometryDesc.Triangles.VertexStride = sizeof( float ) * 3;
        triangleGeometryDesc.Triangles.NumVertices  = 4; // Four vertices for plane
        triangleGeometryDesc.Flags                  = GeometryFlags::Opaque;

        BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags = ASBuildFlags::PreferFastTrace;
        bottomLevelDesc.Geometries.AddElement( triangleGeometryDesc );
        m_triangleAS = std::unique_ptr<IBottomLevelAS>( m_logicalDevice->CreateBottomLevelAS( bottomLevelDesc ) );
    }

    {
        ASInstanceDesc aabbInstanceDesc{ };
        aabbInstanceDesc.BLAS                        = m_aabbAS.get( );
        aabbInstanceDesc.ContributionToHitGroupIndex = 1;
        aabbInstanceDesc.ID                          = 1;
        aabbInstanceDesc.Transform.Resize( 12 );

        aabbInstanceDesc.Transform.SetElement( 0, 1.0f );
        aabbInstanceDesc.Transform.SetElement( 5, 1.0f );
        aabbInstanceDesc.Transform.SetElement( 10, 1.0f );

        ASInstanceDesc triangleInstanceDesc{ };
        triangleInstanceDesc.BLAS                        = m_triangleAS.get( );
        triangleInstanceDesc.ContributionToHitGroupIndex = 0;
        triangleInstanceDesc.ID                          = 0;
        triangleInstanceDesc.Transform.Resize( 12 );

        float scale = 10.0f;
        triangleInstanceDesc.Transform.SetElement( 0, scale );
        triangleInstanceDesc.Transform.SetElement( 5, 1.0f );
        triangleInstanceDesc.Transform.SetElement( 10, scale );

        triangleInstanceDesc.Transform.SetElement( 3, -scale / 2.0f );
        triangleInstanceDesc.Transform.SetElement( 7, -1.0f );
        triangleInstanceDesc.Transform.SetElement( 11, -scale / 2.0f );

        TopLevelASDesc topLevelDesc{ };
        topLevelDesc.BuildFlags = BitSet( ASBuildFlags::PreferFastTrace ) | ASBuildFlags::AllowUpdate;
        topLevelDesc.Instances.AddElement( triangleInstanceDesc );
        topLevelDesc.Instances.AddElement( aabbInstanceDesc );
        m_topLevelAS = std::unique_ptr<ITopLevelAS>( m_logicalDevice->CreateTopLevelAS( topLevelDesc ) );
    }

    const auto    commandListPool = std::unique_ptr<ICommandListPool>( m_logicalDevice->CreateCommandListPool( { QueueType::RayTracing, 1 } ) );
    ICommandList *commandList     = commandListPool->GetCommandLists( ).GetElement( 0 );
    const auto    syncFence       = std::unique_ptr<IFence>( m_logicalDevice->CreateFence( ) );

    commandList->Begin( );

    commandList->BuildBottomLevelAS( { m_triangleAS.get( ) } );
    commandList->BuildBottomLevelAS( { m_aabbAS.get( ) } );

    PipelineBarrierDesc barrier{ };
    barrier.MemoryBarrier( { .BottomLevelAS = m_triangleAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } );
    barrier.MemoryBarrier( { .BottomLevelAS = m_aabbAS.get( ), .OldState = ResourceUsage::AccelerationStructureWrite, .NewState = ResourceUsage::AccelerationStructureRead } );

    commandList->PipelineBarrier( barrier );
    commandList->BuildTopLevelAS( { m_topLevelAS.get( ) } );

    ExecuteDesc executeDesc{ };
    executeDesc.Notify = syncFence.get( );
    commandList->Execute( executeDesc );
    syncFence->Wait( );
}

void RayTracedProceduralGeometryExample::CreateResources( )
{
    {
        constexpr uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };

        struct Vertex
        {
            float x, y, z;
        };
        constexpr Vertex vertices[] = { { -1.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 1.0f } };

        BufferDesc vbDesc{ };
        vbDesc.Descriptor   = BitSet( ResourceDescriptor::VertexBuffer );
        vbDesc.InitialUsage = ResourceUsage::CopyDst;
        vbDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::VertexAndConstantBuffer | ResourceUsage::AccelerationStructureGeometry;
        vbDesc.NumBytes     = sizeof( vertices );
        vbDesc.DebugName    = "Plane_VertexBuffer";
        m_vertexBuffer      = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

        BufferDesc ibDesc{ };
        ibDesc.Descriptor   = BitSet( ResourceDescriptor::IndexBuffer );
        ibDesc.InitialUsage = ResourceUsage::CopyDst;
        ibDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::IndexBuffer | ResourceUsage::AccelerationStructureGeometry;
        ibDesc.NumBytes     = sizeof( indices );
        ibDesc.DebugName    = "Plane_IndexBuffer";
        m_indexBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

        BufferDesc aabbDesc{ };
        aabbDesc.Descriptor   = BitSet( ResourceDescriptor::Buffer );
        aabbDesc.InitialUsage = ResourceUsage::CopyDst;
        aabbDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::AccelerationStructureGeometry;
        aabbDesc.NumBytes     = sizeof( D3D12_RAYTRACING_AABB ) * IntersectionShaderType::TotalPrimitiveCount;
        aabbDesc.DebugName    = "AABB_Buffer";
        m_aabbBuffer          = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( aabbDesc ) );

        BufferDesc attributesDesc{ };
        attributesDesc.HeapType              = HeapType::CPU_GPU;
        attributesDesc.Descriptor            = BitSet( ResourceDescriptor::Buffer ) | ResourceDescriptor::StructuredBuffer;
        attributesDesc.InitialUsage          = ResourceUsage::CopyDst;
        attributesDesc.Usages                = BitSet( ResourceUsage::CopyDst );
        attributesDesc.NumBytes              = sizeof( PrimitiveInstancePerFrameBuffer ) * IntersectionShaderType::TotalPrimitiveCount;
        attributesDesc.DebugName             = "AABB_Attributes_Buffer";
        m_aabbPrimitiveAttributeBuffer       = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( attributesDesc ) );
        m_aabbPrimitiveAttributeBufferMemory = m_aabbPrimitiveAttributeBuffer->MapMemory( );

        BatchResourceCopy batchResourceCopy( m_logicalDevice );
        batchResourceCopy.Begin( );

        std::vector<PrimitiveInstancePerFrameBuffer> initialAttributes( IntersectionShaderType::TotalPrimitiveCount );
        for ( auto &attr : initialAttributes )
        {
            XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &attr.localSpaceToBottomLevelAS ), XMMatrixIdentity( ) );
            XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &attr.bottomLevelASToLocalSpace ), XMMatrixIdentity( ) );
        }

        memcpy( m_aabbPrimitiveAttributeBufferMemory, initialAttributes.data( ), sizeof( PrimitiveInstancePerFrameBuffer ) * IntersectionShaderType::TotalPrimitiveCount );

        InteropArray<Byte> vertexArray( sizeof( vertices ) );
        vertexArray.MemCpy( vertices, sizeof( vertices ) );
        batchResourceCopy.CopyToGPUBuffer( { m_vertexBuffer.get( ), vertexArray } );

        InteropArray<Byte> indexArray( sizeof( indices ) );
        indexArray.MemCpy( indices, sizeof( indices ) );
        batchResourceCopy.CopyToGPUBuffer( { m_indexBuffer.get( ), indexArray } );

        InteropArray<Byte> aabbArray( sizeof( D3D12_RAYTRACING_AABB ) * m_aabbs.size( ) );
        aabbArray.MemCpy( m_aabbs.data( ), sizeof( D3D12_RAYTRACING_AABB ) * m_aabbs.size( ) );
        batchResourceCopy.CopyToGPUBuffer( { m_aabbBuffer.get( ), aabbArray } );

        InitializeScene( batchResourceCopy );

        batchResourceCopy.Submit( );
    }
}

void RayTracedProceduralGeometryExample::CreateRayTracingPipeline( )
{
    // Create shader program
    InteropArray<ShaderDesc>   shaderDescs( 8 );
    InteropArray<HitGroupDesc> hitGroupDescs( 4 );

    int32_t shaderIndex = 0;
    // Ray generation shader
    {
        ShaderDesc &rayGenShaderDesc = shaderDescs.GetElement( shaderIndex++ );
        rayGenShaderDesc.Stage       = ShaderStage::Raygen;
        rayGenShaderDesc.Path        = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
        rayGenShaderDesc.EntryPoint  = "MyRaygenShader";
    }

    // Miss shaders
    {
        ShaderDesc &missShaderDesc = shaderDescs.GetElement( shaderIndex++ );
        missShaderDesc.Stage       = ShaderStage::Miss;
        missShaderDesc.Path        = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
        missShaderDesc.EntryPoint  = "MyMissShader";

        ShaderDesc &shadowMissShaderDesc = shaderDescs.GetElement( shaderIndex++ );
        shadowMissShaderDesc.Stage       = ShaderStage::Miss;
        shadowMissShaderDesc.Path        = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
        shadowMissShaderDesc.EntryPoint  = "MyMissShader_ShadowRay";
    }
    {
        {
            // Primary ray
            ShaderDesc triangleHitShaderDesc{ };
            triangleHitShaderDesc.Stage                     = ShaderStage::ClosestHit;
            triangleHitShaderDesc.Path                      = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
            triangleHitShaderDesc.EntryPoint                = "MyClosestHitShader_Triangle";
            triangleHitShaderDesc.RayTracing.HitGroupExport = "MyHitGroup_Triangle";
            triangleHitShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );
            shaderDescs.AddElement( triangleHitShaderDesc );

            // Shadow ray
            ShaderDesc triangleShadowHitShaderDesc{ };
            triangleShadowHitShaderDesc.Stage                     = ShaderStage::ClosestHit;
            triangleShadowHitShaderDesc.Path                      = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
            triangleShadowHitShaderDesc.EntryPoint                = "MyClosestHitShader_Triangle";
            triangleShadowHitShaderDesc.RayTracing.HitGroupExport = "MyHitGroup_Triangle_ShadowRay";
            triangleShadowHitShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );
            shaderDescs.AddElement( triangleShadowHitShaderDesc );
        }

        // AABB geometry
        const char *aabbHitGroupTypes[] = { "AnalyticPrimitive", "VolumetricPrimitive", "SignedDistancePrimitive" };

        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            for ( int rayType = 0; rayType < 2; rayType++ ) // Primary and shadow
            {
                ShaderDesc aabbHitShaderDesc{ };
                aabbHitShaderDesc.Stage      = ShaderStage::ClosestHit;
                aabbHitShaderDesc.Path       = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
                aabbHitShaderDesc.EntryPoint = "MyClosestHitShader_AABB";

                // Create correct hit group name
                std::string hitGroupName = "MyHitGroup_AABB_" + std::string( aabbHitGroupTypes[ i ] );
                if ( rayType == 1 )
                    hitGroupName += "_ShadowRay";

                aabbHitShaderDesc.RayTracing.HitGroupExport = hitGroupName.c_str( );
                aabbHitShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );
                aabbHitShaderDesc.RayTracing.MarkCbvAsLocal( 2, 0 );
                shaderDescs.AddElement( aabbHitShaderDesc );
            }
        }
    }
    // Closest hit shaders
    {
        // Triangle geometry
        ShaderDesc &triangleHitShaderDesc               = shaderDescs.GetElement( shaderIndex++ );
        triangleHitShaderDesc.Stage                     = ShaderStage::ClosestHit;
        triangleHitShaderDesc.Path                      = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
        triangleHitShaderDesc.EntryPoint                = "MyClosestHitShader_Triangle";
        triangleHitShaderDesc.RayTracing.HitGroupExport = "MyHitGroup_Triangle";
        triangleHitShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );

        HitGroupDesc &hitGroupDesc         = hitGroupDescs.GetElement( 0 );
        hitGroupDesc.Type                  = HitGroupType::Triangles;
        hitGroupDesc.Name                  = "MyHitGroup_Triangle";
        hitGroupDesc.ClosestHitShaderIndex = shaderIndex - 1;

        // AABB geometry
        m_closestHitAABBIndex                       = shaderIndex++;
        ShaderDesc &aabbHitShaderDesc               = shaderDescs.GetElement( m_closestHitAABBIndex );
        aabbHitShaderDesc.Stage                     = ShaderStage::ClosestHit;
        aabbHitShaderDesc.Path                      = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
        aabbHitShaderDesc.EntryPoint                = "MyClosestHitShader_AABB";
        aabbHitShaderDesc.RayTracing.HitGroupExport = "MyHitGroup_AABB";
        aabbHitShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );
        aabbHitShaderDesc.RayTracing.MarkCbvAsLocal( 2, 0 );
    }

    m_firstIntersectionShaderIndex = shaderIndex;
    // Intersection shaders
    {
        const char *intersectionShaderNames[] = { "MyIntersectionShader_AnalyticPrimitive", "MyIntersectionShader_VolumetricPrimitive",
                                                  "MyIntersectionShader_SignedDistancePrimitive" };

        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            ShaderDesc &intersectionShaderDesc = shaderDescs.GetElement( shaderIndex );

            intersectionShaderDesc.Stage      = ShaderStage::Intersection;
            intersectionShaderDesc.Path       = "Assets/Shaders/RTProceduralGeometry/RayTracing.hlsl";
            intersectionShaderDesc.EntryPoint = intersectionShaderNames[ i ];
            intersectionShaderDesc.RayTracing.MarkCbvAsLocal( 1, 0 );
            intersectionShaderDesc.RayTracing.MarkCbvAsLocal( 2, 0 );

            HitGroupDesc &hitGroupDesc = hitGroupDescs.GetElement( 1 + i ); // +1 for Triangle HitGroup
            hitGroupDesc.Type          = HitGroupType::AABBs;
            hitGroupDesc.Name.Append( "MyHitGroup_Intersection_" ).Append( std::to_string( i ).c_str( ) );
            hitGroupDesc.ClosestHitShaderIndex   = m_closestHitAABBIndex;
            hitGroupDesc.IntersectionShaderIndex = shaderIndex;

            ++shaderIndex;
        }
    }

    ProgramDesc programDesc{ };
    programDesc.Shaders       = shaderDescs;
    programDesc.EnableCaching = false;
    m_rayTracingProgram       = std::unique_ptr<ShaderProgram>( m_graphicsApi->CreateShaderProgram( programDesc ) );

    // Create root signature and pipeline
    auto reflection           = m_rayTracingProgram->Reflect( );
    m_rayTracingRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    // Create hit group shader layouts
    m_shaderLocalDataLayouts.resize( shaderDescs.NumElements( ) );
    for ( uint32_t i = 0; i < reflection.ShaderLocalDataLayouts.NumElements( ); i++ )
    {
        auto &layout                  = reflection.ShaderLocalDataLayouts.GetElement( i );
        m_shaderLocalDataLayouts[ i ] = std::unique_ptr<IShaderLocalDataLayout>( m_logicalDevice->CreateShaderRecordLayout( layout ) );
    }

    // Create pipeline state object
    PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                       = BindPoint::RayTracing;
    pipelineDesc.RootSignature                   = m_rayTracingRootSignature.get( );
    pipelineDesc.ShaderProgram                   = m_rayTracingProgram.get( );
    pipelineDesc.RayTracing.MaxNumPayloadBytes   = sizeof( RayPayload );
    pipelineDesc.RayTracing.MaxNumAttributeBytes = sizeof( ProceduralPrimitiveAttributes );
    pipelineDesc.RayTracing.HitGroups            = std::move( hitGroupDescs );
    for ( uint32_t i = 0; i < m_shaderLocalDataLayouts.size( ); i++ )
    {
        pipelineDesc.RayTracing.ShaderLocalDataLayouts.AddElement( m_shaderLocalDataLayouts[ i ].get( ) );
    }

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

void RayTracedProceduralGeometryExample::InitializeScene( BatchResourceCopy &batchResourceCopy )
{
    m_aabbTransformsPerFrame.resize( 3 );
    for ( int i = 0; i < 3; ++i )
    {
        m_aabbTransformsPerFrame[ i ].Resize( IntersectionShaderType::TotalPrimitiveCount );
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
    auto SetAttributes = [ & ]( UINT primitiveIndex, const XMFLOAT4 &albedo, float reflectanceCoef = 0.0f, float diffuseCoef = 0.9f, float specularCoef = 0.7f,
                                float specularPower = 50.0f, float stepScale = 1.0f )
    {
        PrimitiveConstantBuffer &mat = m_aabbMaterials[ primitiveIndex ];
        mat.albedo                   = albedo;
        mat.reflectanceCoef          = reflectanceCoef;
        mat.diffuseCoef              = diffuseCoef;
        mat.specularCoef             = specularCoef;
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

    // Setup scene constants
    m_sceneConstants = SceneConstantBuffer{ .lightPosition     = XMVECTOR( { 0.0f, 18.0f, -20.0f, 0.0f } ),
                                            .lightAmbientColor = XMVECTOR( { 0.25f, 0.25f, 0.25f, 1.0f } ),
                                            .lightDiffuseColor = XMVECTOR( { 0.6f, 0.6f, 0.6f, 1.0f } ) };
    // Setup Camera
    m_eye          = XMVectorSet( 0.0f, 5.3f, -17.0f, 1.0f );
    m_at           = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
    XMVECTOR right = XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f );

    XMVECTOR direction = XMVector4Normalize( m_at - m_eye );
    m_up               = XMVector3Normalize( XMVector3Cross( direction, right ) );

    // Rotate camera around Y axis
    XMMATRIX rotate = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    m_eye           = XMVector3Transform( m_eye, rotate );
    m_up            = XMVector3Transform( m_up, rotate );

    UpdateCameraMatrices( );
    // Create scene constant buffer
    BufferDesc sceneBufferDesc{ };
    sceneBufferDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
    sceneBufferDesc.NumBytes     = sizeof( SceneConstantBuffer );
    sceneBufferDesc.InitialUsage = ResourceUsage::CopyDst;
    sceneBufferDesc.Usages       = BitSet( ResourceUsage::CopyDst ) | ResourceUsage::VertexAndConstantBuffer;
    sceneBufferDesc.DebugName    = "SceneConstantBuffer";
    m_sceneConstantBuffer        = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( sceneBufferDesc ) );

    // Copy scene constants
    InteropArray<Byte> sceneData( sizeof( SceneConstantBuffer ) );
    sceneData.MemCpy( &m_sceneConstants, sizeof( SceneConstantBuffer ) );
    batchResourceCopy.CopyToGPUBuffer( { m_sceneConstantBuffer.get( ), sceneData } );
}

void RayTracedProceduralGeometryExample::UpdateCameraMatrices( )
{
    // Update camera matrices in scene constant buffer
    m_sceneConstants.cameraPosition = m_eye;

    constexpr float fovAngleY = 45.0f;
    const XMMATRIX  view      = XMMatrixLookAtLH( m_eye, m_at, m_up );
    const XMMATRIX  proj =
        XMMatrixPerspectiveFovLH( XMConvertToRadians( fovAngleY ), static_cast<float>( m_windowDesc.Width ) / static_cast<float>( m_windowDesc.Height ), 0.01f, 125.0f );
    const XMMATRIX viewProj            = view * proj;
    m_sceneConstants.projectionToWorld = XMMatrixInverse( nullptr, viewProj );
}

void RayTracedProceduralGeometryExample::CreateShaderBindingTable( )
{
    // Create shader binding table
    ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline              = m_rayTracingPipeline.get( );
    bindingTableDesc.SizeDesc.NumRayTypes  = 2; // Primary and shadow rays
    bindingTableDesc.SizeDesc.NumInstances = IntersectionShaderType::TotalPrimitiveCount;
    bindingTableDesc.MaxHitGroupDataBytes  = sizeof( PrimitiveInstanceConstantBuffer ) + sizeof( PrimitiveConstantBuffer );

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
        m_shaderBindingTable->BindMissShader( shadowMissDesc );
    }

    {
        const auto triangleHitGroupData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderRecordData( { m_shaderLocalDataLayouts[ 3 ].get( ) } ) );

        InteropArray<Byte> materialData( sizeof( PrimitiveConstantBuffer ) );
        materialData.MemCpy( &m_planeMaterialCB, sizeof( PrimitiveConstantBuffer ) );
        triangleHitGroupData->Cbv( 1, materialData );

        HitGroupBindingDesc triangleHitGroupDesc{ };
        triangleHitGroupDesc.HitGroupExportName = "MyHitGroup_Triangle";
        triangleHitGroupDesc.Data               = triangleHitGroupData.get( );
        m_shaderBindingTable->BindHitGroup( triangleHitGroupDesc );
    }

    // AABB geometry hit groups
    {
        uint32_t instanceIndex = 0;
        for ( uint32_t shaderType = 0; shaderType < IntersectionShaderType::Count; shaderType++ )
        {
            const uint32_t numPrimitives = PerPrimitiveTypeCount( static_cast<IntersectionShaderType::Enum>( shaderType ) );
            for ( uint32_t primitiveIndex = 0; primitiveIndex < numPrimitives; primitiveIndex++ )
            {
                auto hitGroupData = std::unique_ptr<IShaderLocalData>( m_logicalDevice->CreateShaderRecordData( { m_shaderLocalDataLayouts[ m_closestHitAABBIndex ].get( ) } ) );

                InteropArray<Byte> materialData( sizeof( PrimitiveConstantBuffer ) );
                materialData.MemCpy( &m_aabbMaterials[ instanceIndex ], sizeof( PrimitiveConstantBuffer ) );
                hitGroupData->Cbv( 1, materialData );

                PrimitiveInstanceConstantBuffer instanceData{ .instanceIndex = instanceIndex, .primitiveType = primitiveIndex };
                InteropArray<Byte>              instanceDataArray( sizeof( PrimitiveInstanceConstantBuffer ) );
                instanceDataArray.MemCpy( &instanceData, sizeof( PrimitiveInstanceConstantBuffer ) );
                hitGroupData->Cbv( 2, instanceDataArray );

                HitGroupBindingDesc hitGroupDesc{ };
                hitGroupDesc.HitGroupExportName = InteropString( "MyHitGroup_Intersection_" ).Append( std::to_string( shaderType ).c_str( ) );
                hitGroupDesc.Data               = hitGroupData.get( );
                hitGroupDesc.InstanceIndex      = 1;
                hitGroupDesc.GeometryIndex      = instanceIndex++;
                hitGroupDesc.RayTypeIndex       = -1;
                m_shaderBindingTable->BindHitGroup( hitGroupDesc );
            }
        }
    }

    m_shaderBindingTable->Build( );
}

RayTracedProceduralGeometryExample::~RayTracedProceduralGeometryExample( )
{
    m_aabbPrimitiveAttributeBuffer->UnmapMemory( );
    m_aabbPrimitiveAttributeBufferMemory = nullptr;
}
