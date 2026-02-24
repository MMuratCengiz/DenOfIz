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

#include <array>
#include <string>

using namespace DirectX;
using namespace DenOfIz;

RayTracedProceduralGeometryExample::~RayTracedProceduralGeometryExample( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_shaderBindingTable ) )
    {
        DenOfIz_ShaderBindingTable_Destroy( m_shaderBindingTable );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_hgLocalRootSignature ) )
    {
        DenOfIz_LocalRootSignature_Destroy( m_hgLocalRootSignature );
    }

    for ( int i = 0; i < 3; ++i )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingBindGroups[ i ] ) )
        {
            DenOfIz_BindGroup_Destroy( m_rayTracingBindGroups[ i ] );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_raytracingOutput[ i ] ) )
        {
            DenOfIz_TextureResource_Destroy( m_raytracingOutput[ i ] );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingPipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_rayTracingPipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingRootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rayTracingRootSignature );
    }
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_rayTracingProgram );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_topLevelAS ) )
    {
        DenOfIz_TopLevelAS_Destroy( m_topLevelAS );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_triangleAS ) )
    {
        DenOfIz_BottomLevelAS_Destroy( m_triangleAS );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_aabbAS ) )
    {
        DenOfIz_BottomLevelAS_Destroy( m_aabbAS );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_aabbBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_aabbBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_sceneConstantRingBuffer ) )
    {
        DenOfIz_RingBuffer_Destroy( m_sceneConstantRingBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_aabbPrimitiveAttributeRingBuffer ) )
    {
        DenOfIz_RingBuffer_Destroy( m_aabbPrimitiveAttributeRingBuffer );
    }
}

void RayTracedProceduralGeometryExample::Init( )
{
    CreateRenderTargets( );
    BuildProceduralGeometryAABBs( );
    CreateResources( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );
}

void RayTracedProceduralGeometryExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
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
        DenOfIz_AABBBoundingBox aabb{ };
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

void RayTracedProceduralGeometryExample::HandleEvent( DenOfIz_Event &event )
{
    switch ( event.Type )
    {
    case DENOFIZ_EVENT_TYPE_KEY_DOWN:
        {
            switch ( event.Key.KeyCode )
            {
            case DENOFIZ_KEY_CODE_G:
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

    for ( uint32_t frameIndex = 0; frameIndex < 3; ++frameIndex )
    {
        auto *aabbPrimitiveAttributeBufferMemory =
            reinterpret_cast<PrimitiveInstancePerFrameBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_aabbPrimitiveAttributeRingBuffer, frameIndex ) );

        auto SetTransformForAABB = [ & ]( const UINT primitiveIndex, const XMMATRIX &mScale, const XMMATRIX &mRotation )
        {
            const XMVECTOR vTranslation = 0.5f * ( XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MinX ) ) +
                                                   XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MaxX ) ) );
            const XMMATRIX mTranslation = XMMatrixTranslationFromVector( vTranslation );

            const XMMATRIX mTransform                                                      = mScale * mRotation * mTranslation;
            aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].localSpaceToBottomLevelAS = mTransform;
            aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].bottomLevelASToLocalSpace = XMMatrixInverse( nullptr, mTransform );
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
}

void RayTracedProceduralGeometryExample::UpdateAABBPrimitiveAttributesForFrame( const uint32_t frameIndex )
{
    const XMMATRIX mIdentity = XMMatrixIdentity( );
    const XMMATRIX mScale15y = XMMatrixScaling( 1, 1.5f, 1 );
    const XMMATRIX mScale15  = XMMatrixScaling( 1.5f, 1.5f, 1.5f );
    const XMMATRIX mScale3   = XMMatrixScaling( 3, 3, 3 );
    const XMMATRIX mRotation = XMMatrixRotationY( -2 * m_animateGeometryTime );

    auto *aabbPrimitiveAttributeBufferMemory =
        reinterpret_cast<PrimitiveInstancePerFrameBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_aabbPrimitiveAttributeRingBuffer, frameIndex ) );

    auto SetTransformForAABB = [ & ]( const UINT primitiveIndex, const XMMATRIX &mScale, const XMMATRIX &mRotation )
    {
        const XMVECTOR vTranslation = 0.5f * ( XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MinX ) ) +
                                               XMLoadFloat3( reinterpret_cast<XMFLOAT3 *>( &m_aabbs[ primitiveIndex ].MaxX ) ) );
        const XMMATRIX mTranslation = XMMatrixTranslationFromVector( vTranslation );

        const XMMATRIX mTransform                                                      = mScale * mRotation * mTranslation;
        aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].localSpaceToBottomLevelAS = mTransform;
        aabbPrimitiveAttributeBufferMemory[ primitiveIndex ].bottomLevelASToLocalSpace = XMMatrixInverse( nullptr, mTransform );
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
    m_camera->Update( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    const float elapsedTime = static_cast<float>( DenOfIz_StepTimer_GetElapsedSeconds( m_stepTimer ) );

    if ( m_animateGeometry )
    {
        m_animateGeometryTime += elapsedTime;
    }

    RenderAndPresentFrame( );
}

void RayTracedProceduralGeometryExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    const float elapsedTime = static_cast<float>( DenOfIz_StepTimer_GetElapsedSeconds( m_stepTimer ) );

    auto *sceneConstants              = reinterpret_cast<SceneConstantBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_sceneConstantRingBuffer, frameIndex ) );
    sceneConstants->cameraPosition    = m_camera->Position( );
    sceneConstants->projectionToWorld = XMMatrixInverse( nullptr, m_camera->ViewProjectionMatrix( ) );
    sceneConstants->elapsedTime       = elapsedTime;

    if ( m_animateGeometry )
    {
        UpdateAABBPrimitiveAttributesForFrame( frameIndex );
    }

    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, m_raytracingOutput[ frameIndex ], DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT,
                                                DENOFIZ_QUEUE_TYPE_GRAPHICS );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    DenOfIz_CommandList_BindPipeline( commandList, m_rayTracingPipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_rayTracingBindGroups[ frameIndex ] );

    DenOfIz_DispatchRaysDesc dispatchRaysDesc{ };
    dispatchRaysDesc.Width              = viewport->Width;
    dispatchRaysDesc.Height             = viewport->Height;
    dispatchRaysDesc.Depth              = 1;
    dispatchRaysDesc.ShaderBindingTable = m_shaderBindingTable;
    DenOfIz_CommandList_DispatchRays( commandList, &dispatchRaysDesc );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );
    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    std::array<DenOfIz_TransitionTextureDesc, 2> textureTransitions{ };
    textureTransitions[ 0 ].Texture   = m_raytracingOutput[ frameIndex ];
    textureTransitions[ 0 ].NewUsage  = DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT;
    textureTransitions[ 0 ].QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    textureTransitions[ 1 ].Texture   = renderTarget;
    textureTransitions[ 1 ].NewUsage  = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
    textureTransitions[ 1 ].QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Textures.Elements    = textureTransitions.data( );
    batchTransitionDesc.Textures.NumElements = textureTransitions.size( );
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    DenOfIz_CopyTextureRegionDesc copyTextureRegionDesc{ };
    copyTextureRegionDesc.SrcTexture = m_raytracingOutput[ frameIndex ];
    copyTextureRegionDesc.DstTexture = renderTarget;
    copyTextureRegionDesc.Width      = m_windowDesc.Width;
    copyTextureRegionDesc.Height     = m_windowDesc.Height;
    copyTextureRegionDesc.Depth      = 1;
    DenOfIz_CommandList_CopyTextureRegion( commandList, &copyTextureRegionDesc );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void RayTracedProceduralGeometryExample::CreateRenderTargets( )
{
    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = m_windowDesc.Width;
    textureDesc.Height    = m_windowDesc.Height;
    textureDesc.Depth     = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format    = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        const std::string debugName = "RayTracing Output " + std::to_string( i );
        textureDesc.DebugName       = DENOFIZ_STRING( debugName.c_str( ) );
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &m_raytracingOutput[ i ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_raytracingOutput[ i ], DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }
}

void RayTracedProceduralGeometryExample::CreateAccelerationStructures( )
{
    {
        DenOfIz_BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;

        std::array<DenOfIz_ASGeometryDesc, IntersectionShaderType::TotalPrimitiveCount> aabbGeometries{ {} };
        for ( UINT i = 0; i < IntersectionShaderType::TotalPrimitiveCount; i++ )
        {
            DenOfIz_ASGeometryDesc &aabbGeometryDesc = aabbGeometries[ i ];
            aabbGeometryDesc.Type                    = DENOFIZ_HIT_GROUP_TYPE_AABBS;
            aabbGeometryDesc.AABBs.Buffer            = m_aabbBuffer;
            aabbGeometryDesc.AABBs.Stride            = sizeof( DenOfIz_AABBBoundingBox );
            aabbGeometryDesc.AABBs.NumAABBs          = 1;
            aabbGeometryDesc.AABBs.Offset            = i * sizeof( DenOfIz_AABBBoundingBox );
            aabbGeometryDesc.Flags                   = DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;
        }

        bottomLevelDesc.Geometries.Elements    = aabbGeometries.data( );
        bottomLevelDesc.Geometries.NumElements = aabbGeometries.size( );

        DenOfIz_LogicalDevice_CreateBottomLevelAS( m_logicalDevice, &bottomLevelDesc, &m_aabbAS );
    }

    {
        DenOfIz_ASGeometryDesc triangleGeometryDesc{ };
        triangleGeometryDesc.Type                   = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
        triangleGeometryDesc.Triangles.IndexBuffer  = m_indexBuffer;
        triangleGeometryDesc.Triangles.NumIndices   = 6;
        triangleGeometryDesc.Triangles.IndexType    = DENOFIZ_INDEX_TYPE_UINT16;
        triangleGeometryDesc.Triangles.VertexFormat = DENOFIZ_FORMAT_R32G32B32_FLOAT;
        triangleGeometryDesc.Triangles.VertexBuffer = m_vertexBuffer;
        triangleGeometryDesc.Triangles.VertexStride = sizeof( Vertex );
        triangleGeometryDesc.Triangles.NumVertices  = 4;
        triangleGeometryDesc.Flags                  = DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;

        DenOfIz_BottomLevelASDesc bottomLevelDesc{ };
        bottomLevelDesc.BuildFlags             = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
        bottomLevelDesc.Geometries.Elements    = &triangleGeometryDesc;
        bottomLevelDesc.Geometries.NumElements = 1;
        DenOfIz_LogicalDevice_CreateBottomLevelAS( m_logicalDevice, &bottomLevelDesc, &m_triangleAS );
    }

    {
        DenOfIz_ASInstanceDesc aabbInstanceDesc{ };
        aabbInstanceDesc.Mask                        = 1;
        aabbInstanceDesc.BLAS                        = m_aabbAS;
        aabbInstanceDesc.ContributionToHitGroupIndex = 2;
        aabbInstanceDesc.ID                          = 1;

        DenOfIz_Float4x4 aabbTransform = DENOFIZ_FLOAT4X4_IDENTITY;

        aabbInstanceDesc.Transform = aabbTransform;

        DenOfIz_ASInstanceDesc triangleInstanceDesc{ };
        triangleInstanceDesc.Mask                        = 1;
        triangleInstanceDesc.BLAS                        = m_triangleAS;
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

        DenOfIz_Float4x4 triangleTransform{ };
        XMStoreFloat3x4( reinterpret_cast<XMFLOAT3X4 *>( &triangleTransform._11 ), mTransform );
        triangleInstanceDesc.Transform = triangleTransform;

        std::array<DenOfIz_ASInstanceDesc, 2> instances{ {} };
        instances[ 0 ] = aabbInstanceDesc;
        instances[ 1 ] = triangleInstanceDesc;

        DenOfIz_TopLevelASDesc topLevelDesc{ };
        topLevelDesc.BuildFlags            = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
        topLevelDesc.Instances.Elements    = instances.data( );
        topLevelDesc.Instances.NumElements = instances.size( );

        DenOfIz_LogicalDevice_CreateTopLevelAS( m_logicalDevice, &topLevelDesc, &m_topLevelAS );
    }

    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType        = DENOFIZ_QUEUE_TYPE_COMPUTE;
    DenOfIz_CommandQueue commandQueue = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &commandQueue );

    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue                   = commandQueue;
    poolDesc.NumCommandLists                = 1;
    DenOfIz_CommandListPool commandListPool = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &poolDesc, &commandListPool );

    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];

    DenOfIz_Fence syncFence = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateFence( m_logicalDevice, &syncFence );

    size_t triangleASBuildNumBytes = 0;
    size_t aabbASBuildNumBytes     = 0;
    size_t topLevelASBuildNumBytes = 0;
    DenOfIz_BottomLevelAS_BuildNumBytes( m_triangleAS, &triangleASBuildNumBytes );
    DenOfIz_BottomLevelAS_BuildNumBytes( m_aabbAS, &aabbASBuildNumBytes );
    DenOfIz_TopLevelAS_BuildNumBytes( m_topLevelAS, &topLevelASBuildNumBytes );

    DenOfIz_BufferDesc scratchBufferDesc{ };
    scratchBufferDesc.HeapType   = DENOFIZ_HEAP_TYPE_GPU;
    scratchBufferDesc.Usage      = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.NumBytes   = std::max( { triangleASBuildNumBytes, aabbASBuildNumBytes, topLevelASBuildNumBytes } );
    scratchBufferDesc.DebugName  = DENOFIZ_STRING( "AS_ScratchBuffer" );
    DenOfIz_Buffer scratchBuffer = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &scratchBufferDesc, &scratchBuffer );

    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_BuildBottomLevelASDesc buildTriangleASDesc{ };
    buildTriangleASDesc.BottomLevelAS = m_triangleAS;
    buildTriangleASDesc.ScratchBuffer = scratchBuffer;
    DenOfIz_CommandList_BuildBottomLevelAS( commandList, &buildTriangleASDesc );

    std::array<DenOfIz_MemoryBarrierDesc, 1> scratchBarrier{ };
    scratchBarrier[ 0 ].BufferResource = scratchBuffer;
    scratchBarrier[ 0 ].OldState       = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    scratchBarrier[ 0 ].NewState       = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    DenOfIz_PipelineBarrierDesc scratchBarrierDesc{ };
    scratchBarrierDesc.MemoryBarriers.Elements    = scratchBarrier.data( );
    scratchBarrierDesc.MemoryBarriers.NumElements = scratchBarrier.size( );
    DenOfIz_CommandList_PipelineBarrier( commandList, &scratchBarrierDesc );

    DenOfIz_BuildBottomLevelASDesc buildAabbASDesc{ };
    buildAabbASDesc.BottomLevelAS = m_aabbAS;
    buildAabbASDesc.ScratchBuffer = scratchBuffer;
    DenOfIz_CommandList_BuildBottomLevelAS( commandList, &buildAabbASDesc );

    std::array<DenOfIz_MemoryBarrierDesc, 2> blasBarriers{ };
    blasBarriers[ 0 ].BottomLevelAS = m_aabbAS;
    blasBarriers[ 0 ].OldState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
    blasBarriers[ 0 ].NewState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;
    blasBarriers[ 1 ].BottomLevelAS = m_triangleAS;
    blasBarriers[ 1 ].OldState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
    blasBarriers[ 1 ].NewState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;
    DenOfIz_PipelineBarrierDesc blasBarrierDesc{ };
    blasBarrierDesc.MemoryBarriers.Elements    = blasBarriers.data( );
    blasBarrierDesc.MemoryBarriers.NumElements = blasBarriers.size( );
    DenOfIz_CommandList_PipelineBarrier( commandList, &blasBarrierDesc );

    DenOfIz_BuildTopLevelASDesc buildTopLevelASDesc{ };
    buildTopLevelASDesc.TopLevelAS    = m_topLevelAS;
    buildTopLevelASDesc.ScratchBuffer = scratchBuffer;
    DenOfIz_CommandList_BuildTopLevelAS( commandList, &buildTopLevelASDesc );

    std::array<DenOfIz_MemoryBarrierDesc, 1> tlasBarrier{ };
    tlasBarrier[ 0 ].TopLevelAS = m_topLevelAS;
    tlasBarrier[ 0 ].OldState   = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
    tlasBarrier[ 0 ].NewState   = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;
    DenOfIz_PipelineBarrierDesc tlasBarrierDesc{ };
    tlasBarrierDesc.MemoryBarriers.Elements    = tlasBarrier.data( );
    tlasBarrierDesc.MemoryBarriers.NumElements = tlasBarrier.size( );
    DenOfIz_CommandList_PipelineBarrier( commandList, &tlasBarrierDesc );
    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    executeDesc.Signal                   = syncFence;
    DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );

    DenOfIz_Fence_Wait( syncFence );
    DenOfIz_CommandQueue_WaitIdle( commandQueue );

    DenOfIz_Buffer_Destroy( scratchBuffer );
    DenOfIz_Fence_Destroy( syncFence );
    DenOfIz_CommandListPool_Destroy( commandListPool );
    DenOfIz_CommandQueue_Destroy( commandQueue );
}

void RayTracedProceduralGeometryExample::CreateResources( )
{
    constexpr uint16_t indices[] = { 0, 1, 2, 0, 3, 1 };

    constexpr Vertex vertices[] = { { { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                                    { { 1.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } } };

    DenOfIz_BufferDesc vbDesc{ };
    vbDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    vbDesc.NumBytes  = sizeof( vertices );
    vbDesc.DebugName = DENOFIZ_STRING( "Plane_VertexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vbDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc ibDesc{ };
    ibDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    ibDesc.NumBytes  = sizeof( indices );
    ibDesc.DebugName = DENOFIZ_STRING( "Plane_IndexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &ibDesc, &m_indexBuffer );

    DenOfIz_BufferDesc aabbDesc{ };
    aabbDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    aabbDesc.NumBytes  = sizeof( DenOfIz_AABBBoundingBox ) * IntersectionShaderType::TotalPrimitiveCount;
    aabbDesc.DebugName = DENOFIZ_STRING( "AABB_Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &aabbDesc, &m_aabbBuffer );

    DenOfIz_RingBufferDesc aabbAttributesRingDesc{ };
    aabbAttributesRingDesc.LogicalDevice      = m_logicalDevice;
    aabbAttributesRingDesc.DataNumBytes       = sizeof( PrimitiveInstancePerFrameBuffer ) * IntersectionShaderType::TotalPrimitiveCount;
    aabbAttributesRingDesc.NumEntries         = 3;
    aabbAttributesRingDesc.IsStructuredBuffer = true;
    aabbAttributesRingDesc.MaxChunkNumBytes   = 8192;
    m_aabbPrimitiveAttributeRingBuffer        = DenOfIz_RingBuffer_Create( &aabbAttributesRingDesc );

    for ( uint32_t frameIndex = 0; frameIndex < 3; ++frameIndex )
    {
        auto *aabbPrimitiveAttributeBufferMemory =
            reinterpret_cast<PrimitiveInstancePerFrameBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_aabbPrimitiveAttributeRingBuffer, frameIndex ) );
        for ( int i = 0; i < IntersectionShaderType::TotalPrimitiveCount; ++i )
        {
            aabbPrimitiveAttributeBufferMemory[ i ].localSpaceToBottomLevelAS = XMMatrixIdentity( );
            aabbPrimitiveAttributeBufferMemory[ i ].bottomLevelASToLocalSpace = XMMatrixIdentity( );
        }
    }

    DenOfIz_BatchResourceCopy     batchResourceCopy = DENOFIZ_NULL_HANDLE;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device        = m_logicalDevice;
    batchDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchResourceCopy );
    DenOfIz_BatchResourceCopy_Begin( batchResourceCopy );

    DenOfIz_CopyToGpuBufferDesc copyToGpuBufferDesc{ };
    copyToGpuBufferDesc.DstBuffer        = m_vertexBuffer;
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    copyToGpuBufferDesc.Data.NumElements = sizeof( vertices );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &copyToGpuBufferDesc );

    copyToGpuBufferDesc.DstBuffer        = m_indexBuffer;
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( &indices[ 0 ] );
    copyToGpuBufferDesc.Data.NumElements = sizeof( indices );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &copyToGpuBufferDesc );

    copyToGpuBufferDesc.DstBuffer        = m_aabbBuffer;
    copyToGpuBufferDesc.Data.Elements    = reinterpret_cast<const Byte *>( m_aabbs.data( ) );
    copyToGpuBufferDesc.Data.NumElements = sizeof( DenOfIz_AABBBoundingBox ) * m_aabbs.size( );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &copyToGpuBufferDesc );
    DenOfIz_BatchResourceCopy_Submit( batchResourceCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchResourceCopy );

    InitializeScene( );
    UpdateAABBPrimitiveAttributes( );
}

void RayTracedProceduralGeometryExample::CreateRayTracingPipeline( )
{
    std::array<DenOfIz_ShaderStageDesc, 8>     shaderStages( { } );
    std::array<DenOfIz_ResourceBindingSlot, 1> localBindings( { } );
    localBindings[ 0 ].RegisterSpace = 3;
    localBindings[ 0 ].Binding       = 1;
    localBindings[ 0 ].Type          = DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;

    std::array<std::string, IntersectionShaderType::Count> aabbShaderPaths;
    std::array<std::string, IntersectionShaderType::Count> aabbEntryPoints;
    for ( int i = 0; i < IntersectionShaderType::Count; i++ )
    {
        std::string aabbHitGroupTypes[] = { "AnalyticPrimitive", "VolumetricPrimitive", "SignedDistancePrimitive" };
        aabbShaderPaths[ i ]            = "Assets/Shaders/RTProceduralGeometry/Intersection_" + aabbHitGroupTypes[ i ] + ".hlsl";
        aabbEntryPoints[ i ]            = "MyIntersectionShader_" + aabbHitGroupTypes[ i ];
    }

    {
        int32_t shaderIndex = 0;

        DenOfIz_ShaderStageDesc &rayGenShaderDesc = shaderStages[ shaderIndex++ ];
        rayGenShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_RAY_GEN_BIT;
        rayGenShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RTProceduralGeometry/RayGen.hlsl" );
        rayGenShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyRaygenShader" );

        DenOfIz_ShaderStageDesc &missShaderDesc = shaderStages[ shaderIndex++ ];
        missShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_MISS_BIT;
        missShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RTProceduralGeometry/Miss.hlsl" );
        missShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyMissShader" );

        DenOfIz_ShaderStageDesc &shadowMissShaderDesc = shaderStages[ shaderIndex++ ];
        shadowMissShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_MISS_BIT;
        shadowMissShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RTProceduralGeometry/Miss.hlsl" );
        shadowMissShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyMissShader_ShadowRay" );

        m_closestHitTriangleIndex                                  = shaderIndex++;
        DenOfIz_ShaderStageDesc &triangleHitShaderDesc             = shaderStages[ m_closestHitTriangleIndex ];
        triangleHitShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT;
        triangleHitShaderDesc.Path                                 = DENOFIZ_STRING( "Assets/Shaders/RTProceduralGeometry/ClosestHit.hlsl" );
        triangleHitShaderDesc.EntryPoint                           = DENOFIZ_STRING( "MyClosestHitShader_Triangle" );
        triangleHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
        triangleHitShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );

        m_closestHitAABBIndex                                  = shaderIndex++;
        DenOfIz_ShaderStageDesc &aabbHitShaderDesc             = shaderStages[ m_closestHitAABBIndex ];
        aabbHitShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT;
        aabbHitShaderDesc.Path                                 = DENOFIZ_STRING( "Assets/Shaders/RTProceduralGeometry/ClosestHit.hlsl" );
        aabbHitShaderDesc.EntryPoint                           = DENOFIZ_STRING( "MyClosestHitShader_AABB" );
        aabbHitShaderDesc.RayTracing.HitGroupType              = DENOFIZ_HIT_GROUP_TYPE_AABBS;
        aabbHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
        aabbHitShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );
        m_firstIntersectionShaderIndex                         = shaderIndex;

        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            DenOfIz_ShaderStageDesc &intersectionShaderDesc             = shaderStages[ shaderIndex++ ];
            intersectionShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_INTERSECTION_BIT;
            intersectionShaderDesc.Path                                 = DENOFIZ_STRING( aabbShaderPaths[ i ].c_str( ) );
            intersectionShaderDesc.EntryPoint                           = DENOFIZ_STRING( aabbEntryPoints[ i ].c_str( ) );
            intersectionShaderDesc.RayTracing.HitGroupType              = DENOFIZ_HIT_GROUP_TYPE_AABBS;
            intersectionShaderDesc.RayTracing.LocalBindings.Elements    = localBindings.data( );
            intersectionShaderDesc.RayTracing.LocalBindings.NumElements = localBindings.size( );
        }
    }

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements           = shaderStages.data( );
    programDesc.ShaderStages.NumElements        = shaderStages.size( );
    programDesc.RayTracing.MaxRecursionDepth    = MAX_RAY_RECURSION_DEPTH;
    programDesc.RayTracing.MaxNumPayloadBytes   = sizeof( RayPayload );
    programDesc.RayTracing.MaxNumAttributeBytes = sizeof( ProceduralPrimitiveAttributes );
    m_rayTracingProgram                         = DenOfIz_ShaderProgram_Create( &programDesc );

    DenOfIz_ShaderReflectDesc reflection{ };
    DenOfIz_ShaderProgram_Reflect( m_rayTracingProgram, &reflection );

    m_bindGroupLayouts.resize( reflection.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflection.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflection.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflection.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_rayTracingRootSignature );

    DenOfIz_LogicalDevice_CreateLocalRootSignature( m_logicalDevice, &reflection.LocalRootSignatures.Elements[ m_closestHitTriangleIndex ], &m_hgLocalRootSignature );

    std::vector<DenOfIz_HitGroupDesc> hitGroupDescs;
    std::vector<std::string>          hitGroupNames( IntersectionShaderType::Count * RayType::Count );

    {
        DenOfIz_HitGroupDesc &hitGroupDesc1   = hitGroupDescs.emplace_back( DenOfIz_HitGroupDesc{ } );
        hitGroupDesc1.Type                    = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
        hitGroupDesc1.Name                    = DENOFIZ_STRING( "MyHitGroup_Triangle" );
        hitGroupDesc1.ClosestHitShaderIndex   = m_closestHitTriangleIndex;
        hitGroupDesc1.IntersectionShaderIndex = -1;
        hitGroupDesc1.AnyHitShaderIndex       = -1;
        hitGroupDesc1.LocalRootSignature      = m_hgLocalRootSignature;

        DenOfIz_HitGroupDesc &hitGroupDesc2   = hitGroupDescs.emplace_back( DenOfIz_HitGroupDesc{ } );
        hitGroupDesc2.Type                    = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
        hitGroupDesc2.Name                    = DENOFIZ_STRING( "MyHitGroup_Triangle_ShadowRay" );
        hitGroupDesc2.ClosestHitShaderIndex   = -1;
        hitGroupDesc2.IntersectionShaderIndex = -1;
        hitGroupDesc2.AnyHitShaderIndex       = -1;
        hitGroupDesc2.LocalRootSignature      = m_hgLocalRootSignature;

        for ( int i = 0; i < IntersectionShaderType::Count; i++ )
        {
            for ( int rayType = 0; rayType < RayType::Count; rayType++ )
            {
                const char  *aabbHitGroupTypes[] = { "AnalyticPrimitive", "VolumetricPrimitive", "SignedDistancePrimitive" };
                std::string &hitGroupName        = hitGroupNames.emplace_back( "MyHitGroup_AABB_" + std::string( aabbHitGroupTypes[ i ] ) );
                if ( rayType == 1 )
                {
                    hitGroupName += "_ShadowRay";
                }
                DenOfIz_HitGroupDesc &hitGroupDesc   = hitGroupDescs.emplace_back( DenOfIz_HitGroupDesc{ } );
                hitGroupDesc.Type                    = DENOFIZ_HIT_GROUP_TYPE_AABBS;
                hitGroupDesc.Name                    = DENOFIZ_STRING( hitGroupName.c_str( ) );
                hitGroupDesc.ClosestHitShaderIndex   = m_closestHitAABBIndex;
                hitGroupDesc.IntersectionShaderIndex = m_firstIntersectionShaderIndex + i;
                hitGroupDesc.AnyHitShaderIndex       = -1;
                if ( rayType == 1 )
                {
                    hitGroupDesc.ClosestHitShaderIndex = -1;
                }
                hitGroupDesc.LocalRootSignature = m_hgLocalRootSignature;
            }
        }
    }

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                        = DENOFIZ_BIND_POINT_RAYTRACING;
    pipelineDesc.RootSignature                    = m_rayTracingRootSignature;
    pipelineDesc.ShaderProgram                    = m_rayTracingProgram;
    pipelineDesc.RayTracing.HitGroups.Elements    = hitGroupDescs.data( );
    pipelineDesc.RayTracing.HitGroups.NumElements = hitGroupDescs.size( );

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_rayTracingPipeline );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_bindGroupLayouts[ 0 ];

    for ( int i = 0; i < 3; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_rayTracingBindGroups[ i ] );
        DenOfIz_BindGroup_BeginUpdate( m_rayTracingBindGroups[ i ] );

        DenOfIz_BindGroup_SrvTopLevelAS( m_rayTracingBindGroups[ i ], 0, m_topLevelAS );
        DenOfIz_BindGroup_UavTexture( m_rayTracingBindGroups[ i ], 0, m_raytracingOutput[ i ] );

        DenOfIz_GPUBufferView  sceneConstantView = DenOfIz_RingBuffer_GetBufferView( m_sceneConstantRingBuffer, i );
        DenOfIz_BindBufferDesc sceneConstantBindDesc{ };
        sceneConstantBindDesc.Binding        = 0;
        sceneConstantBindDesc.Resource       = sceneConstantView.Buffer;
        sceneConstantBindDesc.ResourceOffset = sceneConstantView.Offset;
        DenOfIz_BindGroup_CbvWithDesc( m_rayTracingBindGroups[ i ], &sceneConstantBindDesc );

        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 1, m_indexBuffer );
        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 2, m_vertexBuffer );

        DenOfIz_GPUBufferView  aabbAttributeView = DenOfIz_RingBuffer_GetBufferView( m_aabbPrimitiveAttributeRingBuffer, i );
        DenOfIz_BindBufferDesc aabbAttributeBindDesc{ };
        aabbAttributeBindDesc.Binding        = 3;
        aabbAttributeBindDesc.Resource       = aabbAttributeView.Buffer;
        aabbAttributeBindDesc.ResourceOffset = aabbAttributeView.Offset;
        DenOfIz_BindGroup_SrvBufferWithDesc( m_rayTracingBindGroups[ i ], &aabbAttributeBindDesc );

        DenOfIz_BindGroup_EndUpdate( m_rayTracingBindGroups[ i ] );
    }
}

void RayTracedProceduralGeometryExample::InitializeScene( )
{
    m_aabbTransformsPerFrame.resize( 3 );
    for ( int i = 0; i < 3; ++i )
    {
        m_aabbTransformsPerFrame[ i ].resize( IntersectionShaderType::TotalPrimitiveCount );
    }

    m_planeMaterialCB = {
        .albedo = XMFLOAT4( 0.9f, 0.9f, 0.9f, 1.0f ), .reflectanceCoef = 0.25f, .diffuseCoef = 1.0f, .specularCoef = 0.4f, .specularPower = 50.0f, .stepScale = 1.0f
    };

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

    DenOfIz_RingBufferDesc sceneRingDesc{ };
    sceneRingDesc.LogicalDevice      = m_logicalDevice;
    sceneRingDesc.DataNumBytes       = sizeof( SceneConstantBuffer );
    sceneRingDesc.NumEntries         = 3;
    sceneRingDesc.IsStructuredBuffer = false;
    sceneRingDesc.MaxChunkNumBytes   = 8192;
    m_sceneConstantRingBuffer        = DenOfIz_RingBuffer_Create( &sceneRingDesc );

    constexpr auto  lightPosition     = XMFLOAT4( 0.0f, 18.0f, -20.0f, 0.0f );
    constexpr auto  lightAmbientColor = XMFLOAT4( 0.25f, 0.25f, 0.25f, 1.0f );
    constexpr float diffuse           = 0.6f;
    constexpr auto  lightDiffuseColor = XMFLOAT4( diffuse, diffuse, diffuse, 1.0f );

    for ( uint32_t frameIndex = 0; frameIndex < 3; ++frameIndex )
    {
        auto *sceneConstants              = reinterpret_cast<SceneConstantBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_sceneConstantRingBuffer, frameIndex ) );
        sceneConstants->lightPosition     = XMLoadFloat4( &lightPosition );
        sceneConstants->lightAmbientColor = XMLoadFloat4( &lightAmbientColor );
        sceneConstants->lightDiffuseColor = XMLoadFloat4( &lightDiffuseColor );
    }

    InitCamera( );
}

void RayTracedProceduralGeometryExample::InitCamera( ) const
{
    auto           eye    = XMVectorSet( 0.0f, 5.3f, -17.0f, 1.0f );
    const XMMATRIX rotate = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    eye                   = XMVector3Transform( eye, rotate );

    m_camera->SetPosition( eye );
    m_camera->SetFront( XMVECTOR{ 0.67f, -0.29f, 0.67f, 0.0f } );

    for ( uint32_t frameIndex = 0; frameIndex < 3; ++frameIndex )
    {
        auto *sceneConstants              = reinterpret_cast<SceneConstantBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_sceneConstantRingBuffer, frameIndex ) );
        sceneConstants->cameraPosition    = m_camera->Position( );
        sceneConstants->projectionToWorld = XMMatrixInverse( nullptr, m_camera->ViewProjectionMatrix( ) );
    }
}

void RayTracedProceduralGeometryExample::CreateShaderBindingTable( )
{
    uint32_t numHitGroups = RayType::Count;

    for ( uint32_t shaderType = 0; shaderType < IntersectionShaderType::Count; shaderType++ )
    {
        numHitGroups += PerPrimitiveTypeCount( static_cast<IntersectionShaderType::Enum>( shaderType ) ) * RayType::Count;
    }

    DenOfIz_ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline                         = m_rayTracingPipeline;
    bindingTableDesc.SizeDesc.NumRayGenerationShaders = 1;
    bindingTableDesc.SizeDesc.NumHitGroups            = numHitGroups;
    bindingTableDesc.SizeDesc.NumMissShaders          = 2;
    bindingTableDesc.MaxHitGroupDataBytes             = sizeof( LocalData );

    DenOfIz_LogicalDevice_CreateShaderBindingTable( m_logicalDevice, &bindingTableDesc, &m_shaderBindingTable );

    DenOfIz_RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = DENOFIZ_STRING( "MyRaygenShader" );
    DenOfIz_ShaderBindingTable_BindRayGenerationShader( m_shaderBindingTable, &rayGenDesc );

    {
        DenOfIz_MissBindingDesc missDesc{ };
        missDesc.ShaderName = DENOFIZ_STRING( "MyMissShader" );
        DenOfIz_ShaderBindingTable_BindMissShader( m_shaderBindingTable, &missDesc );

        DenOfIz_MissBindingDesc shadowMissDesc{ };
        shadowMissDesc.ShaderName = DENOFIZ_STRING( "MyMissShader_ShadowRay" );
        shadowMissDesc.Offset     = 1;
        DenOfIz_ShaderBindingTable_BindMissShader( m_shaderBindingTable, &shadowMissDesc );
    }

    LocalData localData{ };
    uint32_t  hitGroupOffset = 0;
    {
        DenOfIz_ShaderLocalDataDesc localDataDesc{ };
        localDataDesc.Layout                         = m_hgLocalRootSignature;
        DenOfIz_ShaderLocalData triangleHitGroupData = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateShaderLocalData( m_logicalDevice, &localDataDesc, &triangleHitGroupData );

        localData.materialCB = m_planeMaterialCB;
        localData.aabbCB     = { 0, 0, 0, 0 };

        DenOfIz_ByteArrayView localDataView{ };
        localDataView.Elements    = reinterpret_cast<const Byte *>( &localData );
        localDataView.NumElements = sizeof( LocalData );
        DenOfIz_ShaderLocalData_CbvData( triangleHitGroupData, 1, &localDataView );

        for ( uint32_t rayType = 0; rayType < 2; rayType++ )
        {
            DenOfIz_HitGroupBindingDesc triangleHitGroupDesc{ };
            triangleHitGroupDesc.HitGroupExportName = DENOFIZ_STRING( rayType == 0 ? "MyHitGroup_Triangle" : "MyHitGroup_Triangle_ShadowRay" );
            triangleHitGroupDesc.Data               = triangleHitGroupData;
            triangleHitGroupDesc.Offset             = hitGroupOffset++;
            DenOfIz_ShaderBindingTable_BindHitGroup( m_shaderBindingTable, &triangleHitGroupDesc );
        }

        DenOfIz_ShaderLocalData_Destroy( triangleHitGroupData );
    }

    {
        for ( uint32_t shaderType = 0, instanceIndex = 0; shaderType < IntersectionShaderType::Count; shaderType++ )
        {
            const char *shaderTypeName = shaderType == 0 ? "AnalyticPrimitive" : shaderType == 1 ? "VolumetricPrimitive" : "SignedDistancePrimitive";

            const uint32_t numPrimitiveTypes = PerPrimitiveTypeCount( static_cast<IntersectionShaderType::Enum>( shaderType ) );
            for ( uint32_t primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++, instanceIndex++ )
            {
                DenOfIz_ShaderLocalDataDesc localDataDesc{ };
                localDataDesc.Layout                 = m_hgLocalRootSignature;
                DenOfIz_ShaderLocalData hitGroupData = DENOFIZ_NULL_HANDLE;
                DenOfIz_LogicalDevice_CreateShaderLocalData( m_logicalDevice, &localDataDesc, &hitGroupData );

                localData.materialCB = m_aabbMaterials[ instanceIndex ];
                localData.aabbCB     = { .instanceIndex = instanceIndex, .primitiveType = primitiveIndex };

                DenOfIz_ByteArrayView localDataView{ };
                localDataView.Elements    = reinterpret_cast<const Byte *>( &localData );
                localDataView.NumElements = sizeof( LocalData );
                DenOfIz_ShaderLocalData_CbvData( hitGroupData, 1, &localDataView );

                for ( uint32_t rayType = 0; rayType < RayType::Count; rayType++ )
                {
                    std::string hitGroupName = std::string( "MyHitGroup_AABB_" ) + shaderTypeName;
                    if ( rayType == 1 )
                    {
                        hitGroupName += "_ShadowRay";
                    }

                    DenOfIz_HitGroupBindingDesc hitGroupDesc{ };
                    hitGroupDesc.HitGroupExportName = DENOFIZ_STRING( hitGroupName.c_str( ) );
                    hitGroupDesc.Data               = hitGroupData;
                    hitGroupDesc.Offset             = hitGroupOffset++;
                    DenOfIz_ShaderBindingTable_BindHitGroup( m_shaderBindingTable, &hitGroupDesc );
                }

                DenOfIz_ShaderLocalData_Destroy( hitGroupData );
            }
        }
    }

    DenOfIz_ShaderBindingTable_Build( m_shaderBindingTable );
}

void RayTracedProceduralGeometryExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}
