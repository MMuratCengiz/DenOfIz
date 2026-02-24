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
#include <algorithm>
#include <string>

using namespace DenOfIz;

RayTracedTriangleExample::~RayTracedTriangleExample( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    for ( int i = 0; i < 3; ++i )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_raytracingOutput[ i ] ) )
        {
            DenOfIz_TextureResource_Destroy( m_raytracingOutput[ i ] );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingBindGroups[ i ] ) )
        {
            DenOfIz_BindGroup_Destroy( m_rayTracingBindGroups[ i ] );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_shaderBindingTable ) )
    {
        DenOfIz_ShaderBindingTable_Destroy( m_shaderBindingTable );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_hgData ) )
    {
        DenOfIz_ShaderLocalData_Destroy( m_hgData );
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
    if ( DENOFIZ_HANDLE_IS_VALID( m_hgShaderLayout ) )
    {
        DenOfIz_LocalRootSignature_Destroy( m_hgShaderLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rayTracingProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_rayTracingProgram );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_topLevelAS ) )
    {
        DenOfIz_TopLevelAS_Destroy( m_topLevelAS );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_bottomLevelAS ) )
    {
        DenOfIz_BottomLevelAS_Destroy( m_bottomLevelAS );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rayGenCBResource ) )
    {
        DenOfIz_Buffer_Destroy( m_rayGenCBResource );
    }
}

void RayTracedTriangleExample::Init( )
{
    CreateRenderTargets( );
    CreateResources( );
    CreateAccelerationStructures( );
    CreateRayTracingPipeline( );
    CreateShaderBindingTable( );
}

void RayTracedTriangleExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
}

void RayTracedTriangleExample::Update( )
{
    m_worldData.DeltaTime = DenOfIz_StepTimer_GetDeltaTime( m_stepTimer );
    m_worldData.Camera->Update( m_worldData.DeltaTime );

    RenderAndPresentFrame( );
}

void RayTracedTriangleExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_TransitionBufferDesc bufferTransition = { };
    bufferTransition.Buffer                       = m_rayGenCBResource;
    bufferTransition.NewUsage                     = DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT;
    bufferTransition.QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_TransitionTextureDesc textureTransition = { };
    textureTransition.Texture                       = m_raytracingOutput[ frameIndex ];
    textureTransition.NewUsage                      = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    textureTransition.QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc = { };
    batchTransitionDesc.Buffers.Elements            = &bufferTransition;
    batchTransitionDesc.Buffers.NumElements         = 1;
    batchTransitionDesc.Textures.Elements           = &textureTransition;
    batchTransitionDesc.Textures.NumElements        = 1;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    DenOfIz_CommandList_BindPipeline( commandList, m_rayTracingPipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_rayTracingBindGroups[ frameIndex ] );

    DenOfIz_DispatchRaysDesc dispatchRaysDesc = { };
    dispatchRaysDesc.Width                    = static_cast<uint32_t>( viewport->Width );
    dispatchRaysDesc.Height                   = static_cast<uint32_t>( viewport->Height );
    dispatchRaysDesc.Depth                    = 1;
    dispatchRaysDesc.ShaderBindingTable       = m_shaderBindingTable;
    DenOfIz_CommandList_DispatchRays( commandList, &dispatchRaysDesc );

    DenOfIz_TransitionTextureDesc textureTransitions[ 2 ] = { };
    textureTransitions[ 0 ].Texture                       = m_raytracingOutput[ frameIndex ];
    textureTransitions[ 0 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT;
    textureTransitions[ 0 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    textureTransitions[ 1 ].Texture                       = renderTarget;
    textureTransitions[ 1 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
    textureTransitions[ 1 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc2 = { };
    batchTransitionDesc2.Textures.Elements           = textureTransitions;
    batchTransitionDesc2.Textures.NumElements        = 2;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc2 );

    DenOfIz_CopyTextureRegionDesc copyTextureRegionDesc = { };
    copyTextureRegionDesc.SrcTexture                    = m_raytracingOutput[ frameIndex ];
    copyTextureRegionDesc.DstTexture                    = renderTarget;
    copyTextureRegionDesc.Width                         = m_windowDesc.Width;
    copyTextureRegionDesc.Height                        = m_windowDesc.Height;
    copyTextureRegionDesc.Depth                         = 1;
    DenOfIz_CommandList_CopyTextureRegion( commandList, &copyTextureRegionDesc );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void RayTracedTriangleExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void RayTracedTriangleExample::CreateRenderTargets( )
{
    DenOfIz_TextureDesc textureDesc = { };
    textureDesc.Width               = m_windowDesc.Width;
    textureDesc.Height              = m_windowDesc.Height;
    textureDesc.Depth               = 1;
    textureDesc.ArraySize           = 1;
    textureDesc.MipLevels           = 1;
    textureDesc.Format              = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage               = DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        const std::string debugName = "RayTracing Output " + std::to_string( i );
        textureDesc.DebugName       = DENOFIZ_STRING( debugName.c_str( ) );
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &m_raytracingOutput[ i ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_raytracingOutput[ i ], DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }

    DenOfIz_CommandQueueDesc commandQueueDesc = { };
    commandQueueDesc.QueueType                = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_CommandQueue commandQueue         = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &commandQueue );

    DenOfIz_CommandListPoolDesc poolDesc    = { };
    poolDesc.CommandQueue                   = commandQueue;
    poolDesc.NumCommandLists                = 1;
    DenOfIz_CommandListPool commandListPool = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &poolDesc, &commandListPool );

    DenOfIz_CommandListArray commandLists = { };
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];

    DenOfIz_Fence syncFence = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateFence( m_logicalDevice, &syncFence );

    DenOfIz_CommandList_Begin( commandList );

    for ( uint32_t i = 0; i < 3; ++i )
    {
        DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, m_raytracingOutput[ i ], DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT,
                                                    DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }

    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc = { };
    executeDesc.CommandLists.Elements           = &commandList;
    executeDesc.CommandLists.NumElements        = 1;
    executeDesc.Signal                          = syncFence;
    DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );

    DenOfIz_Fence_Wait( syncFence );

    DenOfIz_Fence_Destroy( syncFence );
    DenOfIz_CommandListPool_Destroy( commandListPool );
    DenOfIz_CommandQueue_Destroy( commandQueue );
}

void RayTracedTriangleExample::CreateRayTracingPipeline( )
{
    DenOfIz_ShaderStageDesc shaderStages[ 3 ] = { };

    DenOfIz_ResourceBindingSlot localBindings[ 1 ] = { };
    localBindings[ 0 ].Binding                     = 0;
    localBindings[ 0 ].RegisterSpace               = 29;
    localBindings[ 0 ].Type                        = DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER;

    DenOfIz_ShaderStageDesc &rayGenShaderDesc = shaderStages[ 0 ];
    rayGenShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_RAY_GEN_BIT;
    rayGenShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl" );
    rayGenShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyRaygenShader" );

    DenOfIz_ShaderStageDesc &closestHitShaderDesc             = shaderStages[ 1 ];
    closestHitShaderDesc.Stage                                = DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT;
    closestHitShaderDesc.Path                                 = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl" );
    closestHitShaderDesc.EntryPoint                           = DENOFIZ_STRING( "MyClosestHitShader" );
    closestHitShaderDesc.RayTracing.LocalBindings.Elements    = localBindings;
    closestHitShaderDesc.RayTracing.LocalBindings.NumElements = 1;

    DenOfIz_ShaderStageDesc &missShaderDesc = shaderStages[ 2 ];
    missShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_MISS_BIT;
    missShaderDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/RayTracing/RayTracedTriangle.hlsl" );
    missShaderDesc.EntryPoint               = DENOFIZ_STRING( "MyMissShader" );

    DenOfIz_ShaderProgramDesc programDesc       = { };
    programDesc.ShaderStages.Elements           = shaderStages;
    programDesc.ShaderStages.NumElements        = 3;
    programDesc.RayTracing.MaxNumPayloadBytes   = 4 * sizeof( float );
    programDesc.RayTracing.MaxNumAttributeBytes = 2 * sizeof( float );
    programDesc.RayTracing.MaxRecursionDepth    = 1;

    m_rayTracingProgram = DenOfIz_ShaderProgram_Create( &programDesc );

    DenOfIz_ShaderReflectDesc reflectDesc = { };
    DenOfIz_ShaderProgram_Reflect( m_rayTracingProgram, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc    = { };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_rayTracingRootSignature );
    DenOfIz_LogicalDevice_CreateLocalRootSignature( m_logicalDevice, &reflectDesc.LocalRootSignatures.Elements[ 1 ], &m_hgShaderLayout );

    DenOfIz_ShaderLocalDataDesc localDataDesc = { };
    localDataDesc.Layout                      = m_hgShaderLayout;
    DenOfIz_LogicalDevice_CreateShaderLocalData( m_logicalDevice, &localDataDesc, &m_hgData );

    float red[ 4 ] = { 1.0f, 0.0f, 0.0f, 1.0f };

    DenOfIz_ByteArrayView redData = { };
    redData.Elements              = reinterpret_cast<const Byte *>( &red );
    redData.NumElements           = 4 * sizeof( float );

    DenOfIz_ShaderLocalData_Begin( m_hgData );
    DenOfIz_ShaderLocalData_CbvData( m_hgData, 0, &redData );
    DenOfIz_ShaderLocalData_End( m_hgData );

    DenOfIz_BindGroupDesc bindGroupDesc = { };
    bindGroupDesc.Layout                = m_bindGroupLayouts[ 0 ];

    for ( int i = 0; i < 3; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_rayTracingBindGroups[ i ] );
        DenOfIz_BindGroup_BeginUpdate( m_rayTracingBindGroups[ i ] );
        DenOfIz_BindGroup_SrvTopLevelAS( m_rayTracingBindGroups[ i ], 0, m_topLevelAS );
        DenOfIz_BindGroup_UavTexture( m_rayTracingBindGroups[ i ], 0, m_raytracingOutput[ i ] );
        DenOfIz_BindGroup_Cbv( m_rayTracingBindGroups[ i ], 0, m_rayGenCBResource );
        DenOfIz_BindGroup_EndUpdate( m_rayTracingBindGroups[ i ] );
    }

    DenOfIz_HitGroupDesc hitGroupDesc    = { };
    hitGroupDesc.Name                    = DENOFIZ_STRING( "MyHitGroup" );
    hitGroupDesc.IntersectionShaderIndex = -1;
    hitGroupDesc.AnyHitShaderIndex       = -1;
    hitGroupDesc.ClosestHitShaderIndex   = 1;
    hitGroupDesc.LocalRootSignature      = m_hgShaderLayout;
    hitGroupDesc.Type                    = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;

    DenOfIz_PipelineDesc pipelineDesc             = { };
    pipelineDesc.BindPoint                        = DENOFIZ_BIND_POINT_RAYTRACING;
    pipelineDesc.RootSignature                    = m_rayTracingRootSignature;
    pipelineDesc.ShaderProgram                    = m_rayTracingProgram;
    pipelineDesc.RayTracing.HitGroups.Elements    = &hitGroupDesc;
    pipelineDesc.RayTracing.HitGroups.NumElements = 1;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_rayTracingPipeline );
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

    DenOfIz_BufferDesc vbDesc = { };
    vbDesc.Usage              = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    vbDesc.NumBytes           = sizeof( vertices );
    vbDesc.DebugName          = DENOFIZ_STRING( "VertexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vbDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc ibDesc = { };
    ibDesc.Usage              = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    ibDesc.NumBytes           = sizeof( indices );
    ibDesc.DebugName          = DENOFIZ_STRING( "IndexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &ibDesc, &m_indexBuffer );

    DenOfIz_BufferDesc rayGenCbDesc = { };
    rayGenCbDesc.Usage              = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    rayGenCbDesc.NumBytes           = sizeof( m_rayGenCB );
    rayGenCbDesc.DebugName          = DENOFIZ_STRING( "RayGenCB" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &rayGenCbDesc, &m_rayGenCBResource );

    float border        = 0.1f;
    float aspect        = static_cast<float>( m_windowDesc.Width ) / static_cast<float>( m_windowDesc.Height );
    m_rayGenCB.Viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
    m_rayGenCB.Stencil  = { -1 + border / aspect, -1 + border, 1 - border / aspect, 1.0f - border };

    DenOfIz_BatchResourceCopy     batchResourceCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc = { };
    batchDesc.Device                        = m_logicalDevice;
    batchDesc.IssueBarriers                 = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchResourceCopy );
    DenOfIz_BatchResourceCopy_Begin( batchResourceCopy );

    DenOfIz_CopyToGpuBufferDesc vertexCopy = { };
    vertexCopy.DstBuffer                   = m_vertexBuffer;
    vertexCopy.Data.Elements               = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    vertexCopy.Data.NumElements            = sizeof( vertices );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &vertexCopy );

    DenOfIz_CopyToGpuBufferDesc indexCopy = { };
    indexCopy.DstBuffer                   = m_indexBuffer;
    indexCopy.Data.Elements               = reinterpret_cast<const Byte *>( &indices[ 0 ] );
    indexCopy.Data.NumElements            = sizeof( indices );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &indexCopy );

    DenOfIz_CopyToGpuBufferDesc rayGenCBCopy = { };
    rayGenCBCopy.DstBuffer                   = m_rayGenCBResource;
    rayGenCBCopy.Data.Elements               = reinterpret_cast<Byte *>( &m_rayGenCB );
    rayGenCBCopy.Data.NumElements            = sizeof( m_rayGenCB );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchResourceCopy, &rayGenCBCopy );
    DenOfIz_BatchResourceCopy_Submit( batchResourceCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchResourceCopy );
}

void RayTracedTriangleExample::CreateAccelerationStructures( )
{
    DenOfIz_CommandQueueDesc commandQueueDesc = { };
    commandQueueDesc.QueueType                = DENOFIZ_QUEUE_TYPE_COMPUTE;
    DenOfIz_CommandQueue commandQueue         = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_logicalDevice, &commandQueueDesc, &commandQueue );

    DenOfIz_ASGeometryDesc geometryDesc = { };
    geometryDesc.Type                   = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
    geometryDesc.Triangles.IndexBuffer  = m_indexBuffer;
    geometryDesc.Triangles.NumIndices   = 3;
    geometryDesc.Triangles.IndexType    = DENOFIZ_INDEX_TYPE_UINT16;
    geometryDesc.Triangles.VertexFormat = DENOFIZ_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.NumVertices  = 3;
    geometryDesc.Triangles.VertexBuffer = m_vertexBuffer;
    geometryDesc.Triangles.VertexStride = 3 * sizeof( float );
    geometryDesc.Flags                  = DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;

    DenOfIz_BottomLevelASDesc bottomLevelASDesc = { };
    bottomLevelASDesc.Geometries.Elements       = &geometryDesc;
    bottomLevelASDesc.Geometries.NumElements    = 1;
    bottomLevelASDesc.BuildFlags                = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
    DenOfIz_LogicalDevice_CreateBottomLevelAS( m_logicalDevice, &bottomLevelASDesc, &m_bottomLevelAS );

    DenOfIz_ASInstanceDesc instanceDesc = { };
    instanceDesc.BLAS                   = m_bottomLevelAS;
    instanceDesc.Transform              = DENOFIZ_FLOAT4X4_IDENTITY;
    instanceDesc.Mask                   = 255;

    DenOfIz_TopLevelASDesc topLevelASDesc = { };
    topLevelASDesc.BuildFlags             = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
    topLevelASDesc.Instances.Elements     = &instanceDesc;
    topLevelASDesc.Instances.NumElements  = 1;
    DenOfIz_LogicalDevice_CreateTopLevelAS( m_logicalDevice, &topLevelASDesc, &m_topLevelAS );

    size_t blasBuildNumBytes = 0;
    size_t tlasBuildNumBytes = 0;
    DenOfIz_BottomLevelAS_BuildNumBytes( m_bottomLevelAS, &blasBuildNumBytes );
    DenOfIz_TopLevelAS_BuildNumBytes( m_topLevelAS, &tlasBuildNumBytes );

    DenOfIz_BufferDesc scratchBufferDesc = { };
    scratchBufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_GPU;
    scratchBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.NumBytes           = std::max( blasBuildNumBytes, tlasBuildNumBytes );
    DenOfIz_Buffer scratchBuffer         = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &scratchBufferDesc, &scratchBuffer );

    DenOfIz_CommandListPoolDesc poolDesc    = { };
    poolDesc.CommandQueue                   = commandQueue;
    poolDesc.NumCommandLists                = 1;
    DenOfIz_CommandListPool commandListPool = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_logicalDevice, &poolDesc, &commandListPool );

    DenOfIz_CommandListArray commandLists = { };
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];

    DenOfIz_Fence syncFence = DENOFIZ_NULL_HANDLE;
    DenOfIz_LogicalDevice_CreateFence( m_logicalDevice, &syncFence );

    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_BuildBottomLevelASDesc buildBlasDesc = { };
    buildBlasDesc.BottomLevelAS                  = m_bottomLevelAS;
    buildBlasDesc.ScratchBuffer                  = scratchBuffer;
    DenOfIz_CommandList_BuildBottomLevelAS( commandList, &buildBlasDesc );

    DenOfIz_MemoryBarrierDesc blasBarrier = { };
    blasBarrier.BottomLevelAS             = m_bottomLevelAS;
    blasBarrier.OldState                  = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT;
    blasBarrier.NewState                  = DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT;

    DenOfIz_PipelineBarrierDesc blasBarrierDesc = { };
    blasBarrierDesc.MemoryBarriers.Elements     = &blasBarrier;
    blasBarrierDesc.MemoryBarriers.NumElements  = 1;
    DenOfIz_CommandList_PipelineBarrier( commandList, &blasBarrierDesc );

    DenOfIz_BuildTopLevelASDesc buildTlasDesc = { };
    buildTlasDesc.TopLevelAS                  = m_topLevelAS;
    buildTlasDesc.ScratchBuffer               = scratchBuffer;
    DenOfIz_CommandList_BuildTopLevelAS( commandList, &buildTlasDesc );
    DenOfIz_CommandList_End( commandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc = { };
    executeDesc.CommandLists.Elements           = &commandList;
    executeDesc.CommandLists.NumElements        = 1;
    executeDesc.Signal                          = syncFence;
    DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );

    DenOfIz_Fence_Wait( syncFence );
    DenOfIz_CommandQueue_WaitIdle( commandQueue );

    DenOfIz_Fence_Destroy( syncFence );
    DenOfIz_CommandListPool_Destroy( commandListPool );
    DenOfIz_Buffer_Destroy( scratchBuffer );
    DenOfIz_CommandQueue_Destroy( commandQueue );
}

void RayTracedTriangleExample::CreateShaderBindingTable( )
{
    DenOfIz_ShaderBindingTableDesc bindingTableDesc   = { };
    bindingTableDesc.Pipeline                         = m_rayTracingPipeline;
    bindingTableDesc.SizeDesc.NumRayGenerationShaders = 1;
    bindingTableDesc.SizeDesc.NumMissShaders          = 1;
    bindingTableDesc.SizeDesc.NumHitGroups            = 1;
    bindingTableDesc.MaxHitGroupDataBytes             = sizeof( float ) * 4;

    DenOfIz_LogicalDevice_CreateShaderBindingTable( m_logicalDevice, &bindingTableDesc, &m_shaderBindingTable );

    DenOfIz_RayGenerationBindingDesc rayGenDesc = { };
    rayGenDesc.ShaderName                       = DENOFIZ_STRING( "MyRaygenShader" );
    DenOfIz_ShaderBindingTable_BindRayGenerationShader( m_shaderBindingTable, &rayGenDesc );

    DenOfIz_MissBindingDesc missDesc = { };
    missDesc.ShaderName              = DENOFIZ_STRING( "MyMissShader" );
    DenOfIz_ShaderBindingTable_BindMissShader( m_shaderBindingTable, &missDesc );

    DenOfIz_HitGroupBindingDesc hitGroupBindingDesc = { };
    hitGroupBindingDesc.HitGroupExportName          = DENOFIZ_STRING( "MyHitGroup" );
    hitGroupBindingDesc.Data                        = m_hgData;
    DenOfIz_ShaderBindingTable_BindHitGroup( m_shaderBindingTable, &hitGroupBindingDesc );

    DenOfIz_ShaderBindingTable_Build( m_shaderBindingTable );
}
