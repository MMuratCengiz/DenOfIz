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
#include "DenOfIzExamples/RayTracedSceneExample.h"
#include <string>

#include <spdlog/spdlog.h>
#include <unordered_map>
#include "DenOfIzExamples/Bistro.h"

using namespace DenOfIz;

void RayTracedSceneExample::Init( )
{
    m_bistro = std::make_unique<Bistro>( m_logicalDevice );

    DenOfIz_SamplerDesc samplerDesc{ };
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_sampler );

    CreateRenderTargets( );
    CreateSceneConstantBuffer( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );

    // Looking at the cute moped
    const XMVECTOR cameraPosition = XMVectorSet( -26.77f, 6.38f, -7.45f, 1.0f );
    const XMVECTOR cameraRotation = XMVectorSet( 0.122f, 0.60f, -0.07f, 0.78f );

    m_worldData.Camera->SetPosition( cameraPosition );
    m_worldData.Camera->SetRotation( cameraRotation );
}

void RayTracedSceneExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
}

void RayTracedSceneExample::Update( )
{
    m_worldData.DeltaTime = DenOfIz_StepTimer_GetDeltaTime( m_stepTimer );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    const XMMATRIX currentViewProjMatrix = m_camera->ViewProjectionMatrix( );

    DenOfIz_Float4x4 currentMatrix;
    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &currentMatrix ), currentViewProjMatrix );

    constexpr float epsilon        = 0.0001f;
    bool            cameraChanged  = false;
    const float    *currentValues  = reinterpret_cast<const float *>( &currentMatrix );
    const float    *previousValues = reinterpret_cast<const float *>( &m_lastViewProjectionMatrix );

    for ( int i = 0; i < 16; ++i )
    {
        if ( std::abs( currentValues[ i ] - previousValues[ i ] ) > epsilon )
        {
            cameraChanged = true;
            break;
        }
    }

    m_cameraMovedThisFrame = cameraChanged;

    if ( m_cameraMovedThisFrame )
    {
        m_accumulatedFrames.fill( 0 );
    }

    m_lastViewProjectionMatrix = currentMatrix;

    RenderAndPresentFrame( );
}

void RayTracedSceneExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    UpdateCamera( frameIndex );

    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );
    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    const DenOfIz_Texture raytracingTarget = m_raytracingOutput[ frameIndex ];
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, raytracingTarget, DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    DenOfIz_CommandList_BindPipeline( commandList, m_rayTracingPipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_bindlessBindGroups[ frameIndex ] );
    DenOfIz_CommandList_BindGroup( commandList, m_rayTracingBindGroups[ frameIndex ] );

    DenOfIz_DispatchRaysDesc dispatchRaysDesc{ };
    dispatchRaysDesc.Width              = viewport->Width;
    dispatchRaysDesc.Height             = viewport->Height;
    dispatchRaysDesc.Depth              = 1;
    dispatchRaysDesc.ShaderBindingTable = m_shaderBindingTable;
    DenOfIz_CommandList_DispatchRays( commandList, &dispatchRaysDesc );

    DenOfIz_TransitionTextureDesc textureTransitions[ 2 ] = { };
    textureTransitions[ 0 ].Texture                       = raytracingTarget;
    textureTransitions[ 0 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    textureTransitions[ 1 ].Texture                       = renderTarget;
    textureTransitions[ 1 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Textures.Elements    = textureTransitions;
    batchTransitionDesc.Textures.NumElements = 2;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    DenOfIz_CopyTextureRegionDesc copyTextureRegionDesc{ };
    copyTextureRegionDesc.SrcTexture = raytracingTarget;
    copyTextureRegionDesc.DstTexture = renderTarget;
    copyTextureRegionDesc.Width      = m_pixelWidth;
    copyTextureRegionDesc.Height     = m_pixelHeight;
    copyTextureRegionDesc.Depth      = 1;
    DenOfIz_CommandList_CopyTextureRegion( commandList, &copyTextureRegionDesc );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void RayTracedSceneExample::HandleEvent( DenOfIz_Event &event )
{
    m_camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RayTracedSceneExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}

RayTracedSceneExample::~RayTracedSceneExample( )
{
    for ( auto &raytracingOutput : m_raytracingOutput )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( raytracingOutput ) )
        {
            DenOfIz_TextureResource_Destroy( raytracingOutput );
        }
    }

    for ( auto &accumulationBuffer : m_accumulationBuffer )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( accumulationBuffer ) )
        {
            DenOfIz_TextureResource_Destroy( accumulationBuffer );
        }
    }

    for ( auto &blas : m_bottomLevelASInstances )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( blas ) )
        {
            DenOfIz_BottomLevelAS_Destroy( blas );
        }
    }

    for ( auto &bindGroup : m_rayTracingBindGroups )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( bindGroup );
        }
    }

    for ( auto &bindGroup : m_bindlessBindGroups )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            DenOfIz_BindGroup_Destroy( bindGroup );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_sceneCBRingBuffer ) )
    {
        DenOfIz_RingBuffer_Destroy( m_sceneCBRingBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_rayTracingProgram );
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
    if ( DENOFIZ_HANDLE_IS_VALID( m_sampler ) )
    {
        DenOfIz_Sampler_Destroy( m_sampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shaderBindingTable ) )
    {
        DenOfIz_ShaderBindingTable_Destroy( m_shaderBindingTable );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_topLevelAS ) )
    {
        DenOfIz_TopLevelAS_Destroy( m_topLevelAS );
    }
}

void RayTracedSceneExample::CreateRenderTargets( )
{
    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = m_pixelWidth;
    textureDesc.Height    = m_pixelHeight;
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
    }

    DenOfIz_TextureDesc accumulationDesc{ };
    accumulationDesc.Width     = m_pixelWidth;
    accumulationDesc.Height    = m_pixelHeight;
    accumulationDesc.Depth     = 1;
    accumulationDesc.ArraySize = 1;
    accumulationDesc.MipLevels = 1;
    accumulationDesc.Format    = DENOFIZ_FORMAT_R32G32B32A32_FLOAT;
    accumulationDesc.Usage     = DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT;
    for ( uint32_t i = 0; i < 3; ++i )
    {
        const std::string debugName = "Accumulation Buffer " + std::to_string( i );
        accumulationDesc.DebugName  = DENOFIZ_STRING( debugName.c_str( ) );
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &accumulationDesc, &m_accumulationBuffer[ i ] );
    }
}

void RayTracedSceneExample::CreateRayTracingPipeline( )
{
    std::array<DenOfIz_ShaderStageDesc, 5> shaderStages( { } );

    std::array<DenOfIz_BindlessSlot, 1> bindlessResources( { } );
    bindlessResources[ 0 ].RegisterSpace = 0;
    bindlessResources[ 0 ].Binding       = 0;
    bindlessResources[ 0 ].MaxArraySize  = 2048;
    bindlessResources[ 0 ].Descriptor    = DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;

    DenOfIz_ShaderStageDesc &rayGenShaderDesc = shaderStages[ 0 ];
    rayGenShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_RAY_GEN_BIT;
    rayGenShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedScene.hlsl" );
    rayGenShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyRaygenShader" );

    DenOfIz_ShaderStageDesc &closestHitShaderDesc = shaderStages[ 1 ];
    closestHitShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT;
    closestHitShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedScene.hlsl" );
    closestHitShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyClosestHitShader" );

    closestHitShaderDesc.Bindless.BindlessArrays.Elements    = bindlessResources.data( );
    closestHitShaderDesc.Bindless.BindlessArrays.NumElements = bindlessResources.size( );

    DenOfIz_ShaderStageDesc &missShaderDesc = shaderStages[ 2 ];
    missShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_MISS_BIT;
    missShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedScene.hlsl" );
    missShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyMissShader" );

    DenOfIz_ShaderStageDesc &anyHitShaderDesc = shaderStages[ 3 ];
    anyHitShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_ANY_HIT_BIT;
    anyHitShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedScene.hlsl" );
    anyHitShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyAnyHitShader" );

    anyHitShaderDesc.Bindless.BindlessArrays.Elements    = bindlessResources.data( );
    anyHitShaderDesc.Bindless.BindlessArrays.NumElements = bindlessResources.size( );

    DenOfIz_ShaderStageDesc &shadowMissShaderDesc = shaderStages[ 4 ];
    shadowMissShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_MISS_BIT;
    shadowMissShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedScene.hlsl" );
    shadowMissShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyShadowMissShader" );

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements           = shaderStages.data( );
    programDesc.ShaderStages.NumElements        = shaderStages.size( );
    programDesc.RayTracing.MaxNumPayloadBytes   = 28;
    programDesc.RayTracing.MaxNumAttributeBytes = 2 * sizeof( float );
    programDesc.RayTracing.MaxRecursionDepth    = 3;

    m_rayTracingProgram = DenOfIz_ShaderProgram_Create( &programDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_rayTracingProgram, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        m_spaceToLayoutIndex[ reflectDesc.BindGroupLayouts.Elements[ i ].RegisterSpace ] = i;
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_rayTracingRootSignature );

    DenOfIz_BindGroupDesc bindlessBindGroupDesc{ };
    bindlessBindGroupDesc.Layout = m_bindGroupLayouts[ m_spaceToLayoutIndex[ 0 ] ];

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_bindGroupLayouts[ m_spaceToLayoutIndex[ 1 ] ];

    for ( int i = 0; i < 3; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_rayTracingBindGroups[ i ] );
        DenOfIz_BindGroup_BeginUpdate( m_rayTracingBindGroups[ i ] );

        DenOfIz_BindGroup_SrvTopLevelAS( m_rayTracingBindGroups[ i ], 1, m_topLevelAS );
        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 2, m_bistro->GetMaterialBuffer( ) );
        DenOfIz_BindGroup_UavTexture( m_rayTracingBindGroups[ i ], 3, m_raytracingOutput[ i ] );
        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 4, m_bistro->GetVertexBuffer( ) );
        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 5, m_bistro->GetIndexBuffer( ) );
        DenOfIz_BindGroup_SrvBuffer( m_rayTracingBindGroups[ i ], 6, m_bistro->GetMeshInfoBuffer( ) );
        DenOfIz_BindGroup_UavTexture( m_rayTracingBindGroups[ i ], 7, m_accumulationBuffer[ i ] );

        DenOfIz_GPUBufferView  sceneCBView = DenOfIz_RingBuffer_GetBufferView( m_sceneCBRingBuffer, i );
        DenOfIz_BindBufferDesc sceneCBBindDesc{ };
        sceneCBBindDesc.Binding        = 0;
        sceneCBBindDesc.Resource       = sceneCBView.Buffer;
        sceneCBBindDesc.ResourceOffset = sceneCBView.Offset;
        DenOfIz_BindGroup_CbvWithDesc( m_rayTracingBindGroups[ i ], &sceneCBBindDesc );

        DenOfIz_BindGroup_EndUpdate( m_rayTracingBindGroups[ i ] );

        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindlessBindGroupDesc, &m_bindlessBindGroups[ i ] );
        DenOfIz_BindGroup_BeginUpdate( m_bindlessBindGroups[ i ] );

        DenOfIz_TextureArray textures = m_bistro->GetTextures( );
        DenOfIz_BindGroup_SrvArray( m_bindlessBindGroups[ i ], 0, &textures );
        DenOfIz_BindGroup_Sampler( m_bindlessBindGroups[ i ], 1, m_sampler );

        DenOfIz_BindGroup_EndUpdate( m_bindlessBindGroups[ i ] );
    }

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_RAYTRACING;
    pipelineDesc.RootSignature = m_rayTracingRootSignature;
    pipelineDesc.ShaderProgram = m_rayTracingProgram;

    DenOfIz_HitGroupDesc hitGroupDesc{ };
    hitGroupDesc.Name                    = DENOFIZ_STRING( "MyHitGroup" );
    hitGroupDesc.IntersectionShaderIndex = -1;
    hitGroupDesc.AnyHitShaderIndex       = 3;
    hitGroupDesc.ClosestHitShaderIndex   = 1;
    hitGroupDesc.Type                    = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;

    pipelineDesc.RayTracing.HitGroups.Elements    = &hitGroupDesc;
    pipelineDesc.RayTracing.HitGroups.NumElements = 1;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_rayTracingPipeline );
}

void RayTracedSceneExample::CreateSceneConstantBuffer( )
{
    DenOfIz_RingBufferDesc sceneRingDesc{ };
    sceneRingDesc.LogicalDevice      = m_logicalDevice;
    sceneRingDesc.DataNumBytes       = sizeof( SceneConstantBuffer );
    sceneRingDesc.NumEntries         = 3;
    sceneRingDesc.IsStructuredBuffer = false;
    sceneRingDesc.MaxChunkNumBytes   = 8192;
    m_sceneCBRingBuffer              = DenOfIz_RingBuffer_Create( &sceneRingDesc );
}

void RayTracedSceneExample::CreateAccelerationStructures( )
{
    const auto    &bistroData = m_bistro->GetBistroData( );
    DenOfIz_Buffer vb         = m_bistro->GetVertexBuffer( );
    DenOfIz_Buffer ib         = m_bistro->GetIndexBuffer( );

    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType        = DENOFIZ_QUEUE_TYPE_COMPUTE;
    DenOfIz_CommandQueue commandQueue = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &commandQueue );

    std::unordered_map<std::pair<size_t, size_t>, size_t, decltype( []( const auto &p ) { return std::hash<size_t>{ }( p.first ) ^ ( std::hash<size_t>{ }( p.second ) << 1 ); } )>
        meshToBLASIndex;

    size_t maxScratchSize = 0;

    for ( size_t i = 0; i < bistroData.size( ); ++i )
    {
        const auto &submeshData = bistroData[ i ];
        auto        meshKey     = std::make_pair( submeshData.DrawData.VertexOffset, submeshData.DrawData.IndexOffset );

        if ( !meshToBLASIndex.contains( meshKey ) )
        {
            DenOfIz_ASGeometryDesc geometryDesc{ };
            geometryDesc.Type                   = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
            geometryDesc.Triangles.IndexBuffer  = ib;
            geometryDesc.Triangles.IndexOffset  = submeshData.DrawData.IndexOffset * sizeof( uint32_t );
            geometryDesc.Triangles.NumIndices   = submeshData.DrawData.NumIndices;
            geometryDesc.Triangles.IndexType    = DENOFIZ_INDEX_TYPE_UINT32;
            geometryDesc.Triangles.VertexFormat = DENOFIZ_FORMAT_R32G32B32_FLOAT;
            geometryDesc.Triangles.VertexOffset = submeshData.DrawData.VertexOffset * sizeof( BistroVertex );
            geometryDesc.Triangles.NumVertices  = submeshData.DrawData.NumVertices;
            geometryDesc.Triangles.VertexBuffer = vb;
            geometryDesc.Triangles.VertexStride = sizeof( BistroVertex );
            geometryDesc.Flags                  = 0;

            DenOfIz_BottomLevelASDesc bottomLevelASDesc{ };
            bottomLevelASDesc.Geometries.Elements    = &geometryDesc;
            bottomLevelASDesc.Geometries.NumElements = 1;
            bottomLevelASDesc.BuildFlags             = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;

            DenOfIz_BottomLevelAS blas = DENOFIZ_NULL_HANDLE;
            DenOfIz_LogicalDevice_CreateBottomLevelAS( m_logicalDevice, &bottomLevelASDesc, &blas );
            size_t buildBytes = 0;
            DenOfIz_BottomLevelAS_BuildNumBytes( blas, &buildBytes );
            maxScratchSize = std::max( maxScratchSize, buildBytes );

            meshToBLASIndex[ meshKey ] = m_bottomLevelASInstances.size( );
            m_bottomLevelASInstances.push_back( blas );
        }
    }

    std::vector<DenOfIz_ASInstanceDesc> instances;
    instances.reserve( bistroData.size( ) );

    for ( size_t i = 0; i < bistroData.size( ); ++i )
    {
        const auto &submeshData = bistroData[ i ];
        auto        meshKey     = std::make_pair( submeshData.DrawData.VertexOffset, submeshData.DrawData.IndexOffset );
        size_t      blasIndex   = meshToBLASIndex[ meshKey ];

        DenOfIz_ASInstanceDesc instanceDesc{ };
        instanceDesc.BLAS                        = m_bottomLevelASInstances[ blasIndex ];
        instanceDesc.Mask                        = 255;
        instanceDesc.ID                          = static_cast<uint32_t>( i );
        instanceDesc.ContributionToHitGroupIndex = 0;
        instanceDesc.Transform                   = submeshData.Object.Transform;
        instances.push_back( instanceDesc );
    }

    DenOfIz_TopLevelASDesc topLevelASDesc{ };
    topLevelASDesc.BuildFlags            = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
    topLevelASDesc.Instances.Elements    = instances.data( );
    topLevelASDesc.Instances.NumElements = instances.size( );
    DenOfIz_LogicalDevice_CreateTopLevelAS( m_logicalDevice, &topLevelASDesc, &m_topLevelAS );

    size_t topLevelBuildBytes = 0;
    DenOfIz_TopLevelAS_BuildNumBytes( m_topLevelAS, &topLevelBuildBytes );
    maxScratchSize = std::max( maxScratchSize, topLevelBuildBytes );

    DenOfIz_CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue        = commandQueue;
    commandListPoolDesc.NumCommandLists     = 1;
    DenOfIz_CommandListPool commandListPool = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &commandListPoolDesc, &commandListPool );
    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];
    DenOfIz_Fence       syncFence   = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateFence( m_logicalDevice, &syncFence );

    DenOfIz_BufferDesc scratchBufferDesc{ };
    scratchBufferDesc.HeapType   = DENOFIZ_HEAP_TYPE_GPU;
    scratchBufferDesc.Usage      = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.NumBytes   = maxScratchSize;
    scratchBufferDesc.DebugName  = DENOFIZ_STRING( "ScratchBuffer" );
    DenOfIz_Buffer scratchBuffer = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &scratchBufferDesc, &scratchBuffer );

    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_MemoryBarrierDesc scratchBarrier{ };
    scratchBarrier.BufferResource = scratchBuffer;
    scratchBarrier.OldState       = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    scratchBarrier.NewState       = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;

    DenOfIz_PipelineBarrierDesc scratchBarrierDesc{ };
    scratchBarrierDesc.MemoryBarriers.Elements    = &scratchBarrier;
    scratchBarrierDesc.MemoryBarriers.NumElements = 1;

    for ( size_t i = 0; i < m_bottomLevelASInstances.size( ); ++i )
    {
        DenOfIz_BuildBottomLevelASDesc buildDesc{ };
        buildDesc.BottomLevelAS       = m_bottomLevelASInstances[ i ];
        buildDesc.ScratchBuffer       = scratchBuffer;
        buildDesc.ScratchBufferOffset = 0;
        DenOfIz_CommandList_BuildBottomLevelAS( commandList, &buildDesc );
        DenOfIz_CommandList_PipelineBarrier( commandList, &scratchBarrierDesc );
    }

    std::vector<DenOfIz_MemoryBarrierDesc> blasBarriers;
    for ( auto &blas : m_bottomLevelASInstances )
    {
        DenOfIz_MemoryBarrierDesc barrier{ };
        barrier.BottomLevelAS = blas;
        barrier.OldState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
        barrier.NewState      = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;
        blasBarriers.push_back( barrier );
    }

    DenOfIz_PipelineBarrierDesc blasBarrierDesc{ };
    blasBarrierDesc.MemoryBarriers.Elements    = blasBarriers.data( );
    blasBarrierDesc.MemoryBarriers.NumElements = blasBarriers.size( );
    DenOfIz_CommandList_PipelineBarrier( commandList, &blasBarrierDesc );

    DenOfIz_BuildTopLevelASDesc topLevelBuildDesc{ };
    topLevelBuildDesc.TopLevelAS          = m_topLevelAS;
    topLevelBuildDesc.ScratchBuffer       = scratchBuffer;
    topLevelBuildDesc.ScratchBufferOffset = 0;
    DenOfIz_CommandList_BuildTopLevelAS( commandList, &topLevelBuildDesc );

    DenOfIz_MemoryBarrierDesc tlasBarrier{ };
    tlasBarrier.TopLevelAS = m_topLevelAS;
    tlasBarrier.OldState   = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
    tlasBarrier.NewState   = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;

    DenOfIz_PipelineBarrierDesc tlasBarrierDesc{ };
    tlasBarrierDesc.MemoryBarriers.Elements    = &tlasBarrier;
    tlasBarrierDesc.MemoryBarriers.NumElements = 1;
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

void RayTracedSceneExample::CreateShaderBindingTable( )
{
    DenOfIz_ShaderBindingTableDesc bindingTableDesc{ };
    bindingTableDesc.Pipeline                         = m_rayTracingPipeline;
    bindingTableDesc.MaxHitGroupDataBytes             = 64;
    bindingTableDesc.SizeDesc.NumRayGenerationShaders = 1;
    bindingTableDesc.SizeDesc.NumMissShaders          = 2;
    bindingTableDesc.SizeDesc.NumHitGroups            = 1;

    DenOfIz_LogicalDevice_CreateShaderBindingTable( m_logicalDevice, &bindingTableDesc, &m_shaderBindingTable );

    DenOfIz_RayGenerationBindingDesc rayGenDesc{ };
    rayGenDesc.ShaderName = DENOFIZ_STRING( "MyRaygenShader" );
    DenOfIz_ShaderBindingTable_BindRayGenerationShader( m_shaderBindingTable, &rayGenDesc );

    DenOfIz_MissBindingDesc missDesc{ };
    missDesc.ShaderName = DENOFIZ_STRING( "MyMissShader" );
    DenOfIz_ShaderBindingTable_BindMissShader( m_shaderBindingTable, &missDesc );

    DenOfIz_MissBindingDesc shadowMissDesc{ };
    shadowMissDesc.ShaderName = DENOFIZ_STRING( "MyShadowMissShader" );
    shadowMissDesc.Offset     = 1;
    DenOfIz_ShaderBindingTable_BindMissShader( m_shaderBindingTable, &shadowMissDesc );

    DenOfIz_HitGroupBindingDesc hitGroupDesc{ };
    hitGroupDesc.HitGroupExportName = DENOFIZ_STRING( "MyHitGroup" );
    DenOfIz_ShaderBindingTable_BindHitGroup( m_shaderBindingTable, &hitGroupDesc );

    DenOfIz_ShaderBindingTable_Build( m_shaderBindingTable );
}

void RayTracedSceneExample::UpdateCamera( const uint32_t frameIndex )
{
    auto *sceneConstants = reinterpret_cast<SceneConstantBuffer *>( DenOfIz_RingBuffer_GetMappedMemory( m_sceneCBRingBuffer, frameIndex ) );

    const XMMATRIX viewProjMatrix = m_camera->ViewProjectionMatrix( );
    const XMMATRIX projToWorld    = XMMatrixInverse( nullptr, viewProjMatrix );

    XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4 *>( &sceneConstants->ProjectionToWorld ), projToWorld );

    const XMVECTOR cameraPos = m_camera->Position( );
    XMStoreFloat4( reinterpret_cast<XMFLOAT4 *>( &sceneConstants->CameraPosition ), cameraPos );
    constexpr float sunLightIntensity = 3.5f;
    constexpr float sunOrientation    = -0.3f;
    constexpr float sunInclination    = 0.6f;
    constexpr float ambientIntensity  = 0.3f;

    const float cosInc = cosf( sunInclination * XM_PIDIV2 );
    const float sinInc = sinf( sunInclination * XM_PIDIV2 );
    const float cosOri = cosf( sunOrientation );
    const float sinOri = sinf( sunOrientation );

    const XMVECTOR sunDirection = XMVector3Normalize( XMVectorSet( cosInc * sinOri, sinInc, cosInc * cosOri, 0.0f ) );
    XMStoreFloat4( reinterpret_cast<XMFLOAT4 *>( &sceneConstants->SunDirection ), sunDirection );

    const XMVECTOR sunColor = XMVectorSet( sunLightIntensity * 1.0f, sunLightIntensity * 0.98f, sunLightIntensity * 0.92f, 1.0f );
    XMStoreFloat4( reinterpret_cast<XMFLOAT4 *>( &sceneConstants->SunColor ), sunColor );

    const XMVECTOR ambientColor = XMVectorSet( ambientIntensity * 0.5f, ambientIntensity * 0.65f, ambientIntensity * 0.8f, 1.0f );
    XMStoreFloat4( reinterpret_cast<XMFLOAT4 *>( &sceneConstants->AmbientColor ), ambientColor );

    sceneConstants->Reflectance   = 0.3f;
    sceneConstants->ElapsedTime   = static_cast<float>( DenOfIz_StepTimer_GetElapsedSeconds( m_stepTimer ) );
    sceneConstants->UseShadowRays = 1;
    sceneConstants->FrameCount    = m_accumulatedFrames[ frameIndex ];
    m_accumulatedFrames[ frameIndex ]++;
}
