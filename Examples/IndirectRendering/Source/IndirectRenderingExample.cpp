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
#include "DenOfIzExamples/IndirectRenderingExample.h"
#include <DirectXMath.h>
#include <random>

using namespace DirectX;

#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

IndirectRenderingExample::~IndirectRenderingExample( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    if ( DENOFIZ_HANDLE_IS_VALID( m_simulationDataBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_simulationDataBuffer );
        DenOfIz_Buffer_Destroy( m_simulationDataBuffer );
    }

    DenOfIz_Buffer_Destroy( m_vertexBuffer );
    DenOfIz_Buffer_Destroy( m_indexBuffer );
    DenOfIz_Buffer_Destroy( m_indirectBuffer );
    DenOfIz_Buffer_Destroy( m_perDrawDataBuffer );

    DenOfIz_BindGroup_Destroy( m_computeBindGroup );

    DenOfIz_Pipeline_Destroy( m_graphicsPipeline );
    DenOfIz_Pipeline_Destroy( m_computePipeline );

    DenOfIz_RootSignature_Destroy( m_graphicsRootSignature );
    DenOfIz_RootSignature_Destroy( m_computeRootSignature );
    for ( auto &layout : m_graphicsBindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    for ( auto &layout : m_computeBindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }

    DenOfIz_InputLayout_Destroy( m_inputLayout );

    DenOfIz_ShaderProgram_Destroy( m_graphicsProgram );
    DenOfIz_ShaderProgram_Destroy( m_computeProgram );
}

void IndirectRenderingExample::Init( )
{
    DenOfIz_BufferDesc simDataDesc{ };
    simDataDesc.Usage                     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    simDataDesc.NumBytes                  = sizeof( SimulationData );
    simDataDesc.DebugName                 = DENOFIZ_STRING( "SimulationDataBuffer" );
    simDataDesc.HeapType                  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    simDataDesc.StructureDesc.NumElements = 1;
    simDataDesc.StructureDesc.Stride      = sizeof( SimulationData );
    simDataDesc.StructureDesc.Offset      = 0;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &simDataDesc, &m_simulationDataBuffer );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_simulationDataBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    void *mappedMem = nullptr;
    DenOfIz_Buffer_MapMemory( m_simulationDataBuffer, &mappedMem );
    m_simulationDataMapped = static_cast<SimulationData *>( mappedMem );

    CreateBuffers( );
    CreateIndirectBuffer( );
    CreateComputePipeline( );
    CreateGraphicsPipeline( );
}

void IndirectRenderingExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
}

void IndirectRenderingExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_elapsedTime += m_worldData.DeltaTime;

    if ( m_simulationDataMapped )
    {
        if ( m_normalizedMousePos.X == 0.0f && m_normalizedMousePos.Y == 0.0f )
        {
            m_simulationDataMapped->MousePosition = { 0.0f, 0.0f };
        }
        else
        {
            m_simulationDataMapped->MousePosition = m_normalizedMousePos;
        }

        m_simulationDataMapped->DeltaTime   = std::clamp( m_worldData.DeltaTime, 0.001f, 0.1f );
        m_simulationDataMapped->ElapsedTime = m_elapsedTime;

        const DenOfIz_Viewport *viewport = nullptr;
        DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
        m_simulationDataMapped->ScreenDimensions = { viewport->Width, viewport->Height };
    }

    RenderAndPresentFrame( );
}

void IndirectRenderingExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    RunComputeShader( commandList );

    DenOfIz_TransitionBufferDesc bufferTransitions[ 2 ] = { };
    bufferTransitions[ 0 ].Buffer                       = m_vertexBuffer;
    bufferTransitions[ 0 ].NewUsage                     = DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT;
    bufferTransitions[ 0 ].QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    bufferTransitions[ 1 ].Buffer                       = m_indexBuffer;
    bufferTransitions[ 1 ].NewUsage                     = DENOFIZ_RESOURCE_USAGE_INDEX_BUFFER_BIT;
    bufferTransitions[ 1 ].QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Buffers.Elements    = bufferTransitions;
    batchTransitionDesc.Buffers.NumElements = 2;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindPipeline( commandList, m_graphicsPipeline );
    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, sizeof( Vertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    DenOfIz_CommandList_DrawIndexedIndirect( commandList, m_indirectBuffer, 0, NUM_TRIANGLES, sizeof( DenOfIz_DrawIndexedIndirectCommand ) );

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void IndirectRenderingExample::HandleEvent( DenOfIz_Event &event )
{
    if ( event.Type == DENOFIZ_EVENT_TYPE_MOUSE_MOTION )
    {
        m_mousePosition = { static_cast<float>( event.MouseMotion.X ), static_cast<float>( event.MouseMotion.Y ) };

        const DenOfIz_Viewport *viewport = nullptr;
        DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

        DenOfIz_Float2 newMousePos{ };
        newMousePos.X = m_mousePosition.X / viewport->Width * 2.0f - 1.0f;
        newMousePos.Y = 1.0f - m_mousePosition.Y / viewport->Height * 2.0f;

        newMousePos.X = std::clamp( newMousePos.X, -1.0f, 1.0f );
        newMousePos.Y = std::clamp( newMousePos.Y, -1.0f, 1.0f );

        if ( m_normalizedMousePos.X == 0.0f && m_normalizedMousePos.Y == 0.0f )
        {
            m_normalizedMousePos = newMousePos;
        }
        else
        {
            constexpr float smoothing = 0.1f;
            m_normalizedMousePos.X    = m_normalizedMousePos.X * ( 1.0f - smoothing ) + newMousePos.X * smoothing;
            m_normalizedMousePos.Y    = m_normalizedMousePos.Y * ( 1.0f - smoothing ) + newMousePos.Y * smoothing;
        }
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void IndirectRenderingExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    IExample::Quit( );
}

void IndirectRenderingExample::CreateBuffers( )
{
    constexpr uint32_t totalVertices = NUM_TRIANGLES * 3;
    constexpr uint32_t totalIndices  = NUM_TRIANGLES * 3;

    DenOfIz_BufferDesc vertexBufferDesc{ };
    vertexBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    vertexBufferDesc.NumBytes                  = totalVertices * sizeof( Vertex );
    vertexBufferDesc.DebugName                 = DENOFIZ_STRING( "IndirectVertexBuffer" );
    vertexBufferDesc.StructureDesc.NumElements = totalVertices;
    vertexBufferDesc.StructureDesc.Stride      = sizeof( Vertex );
    vertexBufferDesc.StructureDesc.Offset      = 0;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vertexBufferDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc indexBufferDesc{ };
    indexBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    indexBufferDesc.NumBytes                  = totalIndices * sizeof( uint32_t );
    indexBufferDesc.DebugName                 = DENOFIZ_STRING( "IndirectIndexBuffer" );
    indexBufferDesc.StructureDesc.NumElements = totalIndices;
    indexBufferDesc.StructureDesc.Stride      = sizeof( uint32_t );
    indexBufferDesc.StructureDesc.Offset      = 0;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &indexBufferDesc, &m_indexBuffer );

    DenOfIz_BufferDesc perDrawBufferDesc{ };
    perDrawBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    perDrawBufferDesc.NumBytes                  = NUM_TRIANGLES * sizeof( PerDrawData );
    perDrawBufferDesc.DebugName                 = DENOFIZ_STRING( "PerDrawDataBuffer" );
    perDrawBufferDesc.HeapType                  = DENOFIZ_HEAP_TYPE_GPU;
    perDrawBufferDesc.StructureDesc.NumElements = NUM_TRIANGLES;
    perDrawBufferDesc.StructureDesc.Stride      = sizeof( PerDrawData );
    perDrawBufferDesc.StructureDesc.Offset      = 0;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &perDrawBufferDesc, &m_perDrawDataBuffer );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_vertexBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_indexBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_perDrawDataBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    std::vector<PerDrawData> perDrawData( NUM_TRIANGLES );

    std::mt19937                   rng( std::random_device{ }( ) );
    std::uniform_real_distribution posDist( -1.0f, 1.0f );
    std::uniform_real_distribution velDist( -0.5f, 0.5f );
    std::uniform_real_distribution hueDist( 0.0f, 1.0f );

    for ( uint32_t i = 0; i < NUM_TRIANGLES; ++i )
    {
        PerDrawData &currentData = perDrawData[ i ];

        float x = posDist( rng );
        float y = posDist( rng );

        currentData.Velocity = { velDist( rng ), velDist( rng ) };

        currentData.GroupType = static_cast<uint32_t>( BoidGroup::MouseFollower );

        float hue = hueDist( rng );

        float h     = hue * 6.0f;
        float s     = 0.8f;
        float v     = 0.9f;
        float c     = v * s;
        float x_hsv = c * ( 1.0f - fabsf( fmodf( h, 2.0f ) - 1.0f ) );
        float m     = v - c;

        DenOfIz_Float4 color;
        if ( h < 1.0f )
        {
            color = { c + m, x_hsv + m, m, 0.8f };
        }
        else if ( h < 2.0f )
        {
            color = { x_hsv + m, c + m, m, 0.8f };
        }
        else if ( h < 3.0f )
        {
            color = { m, c + m, x_hsv + m, 0.8f };
        }
        else if ( h < 4.0f )
        {
            color = { m, x_hsv + m, c + m, 0.8f };
        }
        else if ( h < 5.0f )
        {
            color = { x_hsv + m, m, c + m, 0.8f };
        }
        else
        {
            color = { c + m, m, x_hsv + m, 0.8f };
        }

        currentData.Color = color;

        constexpr float scale     = 0.012f;
        const XMMATRIX  transform = XMMatrixScaling( scale, scale, scale ) * XMMatrixTranslation( x, y, 0.0f );
        XMStoreFloat4x4( &currentData.Transform, transform );
    }

    DenOfIz_BatchResourceCopy     perDrawCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device        = m_logicalDevice;
    batchDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &perDrawCopy );
    DenOfIz_BatchResourceCopy_Begin( perDrawCopy );

    DenOfIz_CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer        = m_perDrawDataBuffer;
    copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( perDrawData.data( ) );
    copyDesc.Data.NumElements = perDrawData.size( ) * sizeof( PerDrawData );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( perDrawCopy, &copyDesc );
    DenOfIz_BatchResourceCopy_Submit( perDrawCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( perDrawCopy );
}

void IndirectRenderingExample::CreateIndirectBuffer( )
{
    std::vector<DenOfIz_DrawIndexedIndirectCommand> indirectCommands( NUM_TRIANGLES );

    for ( uint32_t i = 0; i < NUM_TRIANGLES; ++i )
    {
        indirectCommands[ i ].NumIndices    = 3;
        indirectCommands[ i ].NumInstances  = 1;
        indirectCommands[ i ].FirstIndex    = i * 3;
        indirectCommands[ i ].VertexOffset  = 0;
        indirectCommands[ i ].FirstInstance = 0;
    }

    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDIRECT_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    bufferDesc.NumBytes  = indirectCommands.size( ) * sizeof( DenOfIz_DrawIndexedIndirectCommand );
    bufferDesc.DebugName = DENOFIZ_STRING( "IndirectCommandBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &bufferDesc, &m_indirectBuffer );

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchCopyDesc{ };
    batchCopyDesc.Device        = m_logicalDevice;
    batchCopyDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchCopyDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    DenOfIz_CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer        = m_indirectBuffer;
    copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( indirectCommands.data( ) );
    copyDesc.Data.NumElements = indirectCommands.size( ) * sizeof( DenOfIz_DrawIndexedIndirectCommand );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &copyDesc );
    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_indirectBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
}

void IndirectRenderingExample::CreateComputePipeline( )
{
    DenOfIz_ShaderStageDesc computeShaderDesc{ };
    computeShaderDesc.Stage      = DENOFIZ_SHADER_STAGE_COMPUTE_BIT;
    computeShaderDesc.EntryPoint = DENOFIZ_STRING( "CSMain" );
    computeShaderDesc.Data       = ComputeShader( );

    DenOfIz_ShaderProgramDesc shaderProgramDesc{ };
    shaderProgramDesc.ShaderStages.Elements    = &computeShaderDesc;
    shaderProgramDesc.ShaderStages.NumElements = 1;
    m_computeProgram                           = DenOfIz_ShaderProgram_Create( &shaderProgramDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_computeProgram, &reflectDesc );

    m_computeBindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_computeBindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_computeBindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_computeBindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_computeRootSignature );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_computeBindGroupLayouts[ 0 ];
    DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_computeBindGroup );

    DenOfIz_BindGroup_BeginUpdate( m_computeBindGroup );
    DenOfIz_BindGroup_UavBuffer( m_computeBindGroup, 0, m_vertexBuffer );
    DenOfIz_BindGroup_UavBuffer( m_computeBindGroup, 1, m_indexBuffer );
    DenOfIz_BindGroup_UavBuffer( m_computeBindGroup, 2, m_perDrawDataBuffer );
    DenOfIz_BindGroup_SrvBuffer( m_computeBindGroup, 3, m_simulationDataBuffer );
    DenOfIz_BindGroup_EndUpdate( m_computeBindGroup );

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.ShaderProgram = m_computeProgram;
    pipelineDesc.RootSignature = m_computeRootSignature;
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_COMPUTE;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_computePipeline );
}

void IndirectRenderingExample::CreateGraphicsPipeline( )
{
    DenOfIz_ShaderStageDesc  shaderStages[ 2 ]{ };
    DenOfIz_ShaderStageDesc &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vertexShaderDesc.EntryPoint               = DENOFIZ_STRING( "VSMain" );
    vertexShaderDesc.Data                     = VertexShader( );

    DenOfIz_ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    pixelShaderDesc.EntryPoint               = DENOFIZ_STRING( "PSMain" );
    pixelShaderDesc.Data                     = PixelShader( );

    DenOfIz_ShaderProgramDesc shaderProgramDesc{ };
    shaderProgramDesc.ShaderStages.Elements    = shaderStages;
    shaderProgramDesc.ShaderStages.NumElements = 2;
    m_graphicsProgram                          = DenOfIz_ShaderProgram_Create( &shaderProgramDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_graphicsProgram, &reflectDesc );

    m_graphicsBindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_graphicsBindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_graphicsBindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_graphicsBindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;

    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &reflectDesc.InputLayout, &m_inputLayout );
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_graphicsRootSignature );

    DenOfIz_RenderTargetDesc renderTargetDesc{ };
    renderTargetDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTargetDesc.Blend.RenderTargetWriteMask = 0x0F;

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                        = m_inputLayout;
    pipelineDesc.ShaderProgram                      = m_graphicsProgram;
    pipelineDesc.RootSignature                      = m_graphicsRootSignature;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTargetDesc;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_graphicsPipeline );
}

void IndirectRenderingExample::RunComputeShader( DenOfIz_CommandList commandList )
{
    DenOfIz_TransitionBufferDesc bufferTransitions[ 3 ] = { };
    bufferTransitions[ 0 ].Buffer                       = m_vertexBuffer;
    bufferTransitions[ 0 ].NewUsage                     = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    bufferTransitions[ 0 ].QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    bufferTransitions[ 1 ].Buffer                       = m_indexBuffer;
    bufferTransitions[ 1 ].NewUsage                     = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    bufferTransitions[ 1 ].QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    bufferTransitions[ 2 ].Buffer                       = m_perDrawDataBuffer;
    bufferTransitions[ 2 ].NewUsage                     = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    bufferTransitions[ 2 ].QueueType                    = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Buffers.Elements    = bufferTransitions;
    batchTransitionDesc.Buffers.NumElements = 3;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    DenOfIz_CommandList_BindPipeline( commandList, m_computePipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_computeBindGroup );

    constexpr uint32_t threadsPerGroup = 64;
    constexpr uint32_t numGroups       = ( NUM_TRIANGLES + threadsPerGroup - 1 ) / threadsPerGroup;
    DenOfIz_CommandList_Dispatch( commandList, numGroups, 1, 1 );

    DenOfIz_MemoryBarrierDesc uavBarrier{ };
    uavBarrier.OldState = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;
    uavBarrier.NewState = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT;

    DenOfIz_PipelineBarrierDesc uavBarrierDesc{ };
    uavBarrierDesc.MemoryBarriers.Elements    = &uavBarrier;
    uavBarrierDesc.MemoryBarriers.NumElements = 1;
    DenOfIz_CommandList_PipelineBarrier( commandList, &uavBarrierDesc );
}

DenOfIz_ByteArray IndirectRenderingExample::ComputeShader( )
{
    const auto shaderCode = R"(
        struct Vertex
        {
            float4 Position;
            float4 Color;
        };

        struct PerDrawData
        {
            float4x4 Transform;
            float4 Color;
            float2 Velocity;
            uint GroupType;
            float Padding;
        };

        struct SimulationData
        {
            float2 MousePosition;
            float DeltaTime;
            float ElapsedTime;
            float2 ScreenDimensions;
            float2 Padding;
        };

        RWStructuredBuffer<Vertex> g_VertexBuffer : register(u0, space0);
        RWStructuredBuffer<uint> g_IndexBuffer : register(u1, space0);
        RWStructuredBuffer<PerDrawData> g_PerDrawData : register(u2, space0);
        StructuredBuffer<SimulationData> g_SimData : register(t3, space0);

        // Boids parameters
        static const float MAX_SPEED = 2.0f, MIN_SPEED = 0.2f, MOUSE_ATTRACTION = 0.4f;
        static const float SEP_RADIUS = 0.045f, ALN_RADIUS = 0.11f, COH_RADIUS = 0.15f;
        static const float SEP_WEIGHT = 1.0f, ALN_WEIGHT = 1.0f, COH_WEIGHT = 1.3f;
        static const float MIN_VEL_ROT = 0.1f, MAX_FORCE = 1.0f;

        float3 GetPositionFromTransform(float4x4 transform)
        {
            return float3(transform._41, transform._42, transform._43);
        }

        float2 Normalize2D(float2 v)
        {
            float len = length(v);
            return len > 0.0001f ? v / len : float2(0, 0);
        }

        [numthreads(64, 1, 1)]
        void CSMain(uint3 id : SV_DispatchThreadID)
        {
            uint triangleIndex = id.x;
            if (triangleIndex >= 150) return;

            PerDrawData drawData = g_PerDrawData[triangleIndex];

            float3 currentPos = GetPositionFromTransform(drawData.Transform);
            float2 pos2D = currentPos.xy;

            float2 force = float2(0, 0);

            float seed = g_SimData[0].ElapsedTime * 100.0f + float(triangleIndex);
            force += float2(sin(seed), cos(seed * 1.3f)) * 0.02f;

            if (drawData.GroupType == 0) // MouseFollower
            {
                float2 toMouse = g_SimData[0].MousePosition - pos2D;
                float distToMouse = length(toMouse);

                if (distToMouse > 0.001f)
                {
                    float2 desiredVel;
                    if (distToMouse > 0.25f)
                    {
                        desiredVel = Normalize2D(toMouse) * MAX_SPEED;
                    }
                    else
                    {
                        float2 perpendicular = float2(-toMouse.y, toMouse.x); // 90-degree rotation

                        float orbitalStrength = 1.0f - (distToMouse / 0.25f); // Stronger when closer
                        float indexOffset = float(triangleIndex) * 0.1f; // Unique offset per triangle
                        float orbitalDirection = sin(g_SimData[0].ElapsedTime * 2.0f + indexOffset);

                        float2 orbitalVel = Normalize2D(perpendicular) * orbitalDirection * orbitalStrength * MAX_SPEED * 0.8f;
                        float2 approachVel = Normalize2D(toMouse) * (1.0f - orbitalStrength) * MAX_SPEED * 0.6f;

                        desiredVel = approachVel + orbitalVel;
                    }

                    float trailFactor = 1.0f - (float(triangleIndex) / 150.0f) * 0.3f;
                    float2 steeringForce = (desiredVel - drawData.Velocity) * trailFactor;

                    float mouseScale = (distToMouse < 0.15f) ? MOUSE_ATTRACTION * 0.3f : MOUSE_ATTRACTION;

                    if (length(steeringForce) > MAX_FORCE)
                        steeringForce = Normalize2D(steeringForce) * MAX_FORCE;

                    force += steeringForce * mouseScale;
                }

                float2 sepForce = float2(0, 0), avgVel = float2(0, 0), centerMass = float2(0, 0);
                int sepCount = 0, alnCount = 0, cohCount = 0;
                for (uint i = 0; i < 40; i++)
                {
                    uint idx = (triangleIndex + i * 13) % 150;
                    if (idx == triangleIndex) continue;

                    float2 otherPos = GetPositionFromTransform(g_PerDrawData[idx].Transform).xy;
                    float2 diff = pos2D - otherPos;
                    float dist = length(diff);

                    if (dist < SEP_RADIUS && dist > 0.0001f)
                    {
                        float sepStrength = ((SEP_RADIUS - dist) / SEP_RADIUS);
                        float distToMouse = length(g_SimData[0].MousePosition - pos2D);
                        if (distToMouse < 0.2f)
                            sepStrength *= 1.5f; // Extra separation for better distribution

                        sepForce += Normalize2D(diff) * sepStrength;
                        sepCount++;
                    }
                    if (dist < ALN_RADIUS && dist > 0.0001f)
                    {
                        avgVel += g_PerDrawData[idx].Velocity;
                        alnCount++;
                    }
                    if (dist < COH_RADIUS && dist > 0.0001f)
                    {
                        centerMass += otherPos;
                        cohCount++;
                    }
                }

                if (sepCount > 0)
                {
                    sepForce /= float(sepCount);
                    if (length(sepForce) > MAX_FORCE * 1.2f)
                        sepForce = Normalize2D(sepForce) * (MAX_FORCE * 1.2f);
                    force += sepForce * SEP_WEIGHT;
                }

                if (alnCount > 0)
                {
                    avgVel /= float(alnCount);
                    float2 alnForce = avgVel - drawData.Velocity;
                    if (length(alnForce) > MAX_FORCE)
                        alnForce = Normalize2D(alnForce) * MAX_FORCE;
                    force += alnForce * ALN_WEIGHT;
                }

                if (cohCount > 0)
                {
                    centerMass /= float(cohCount);
                    float2 cohForce = centerMass - pos2D;
                    if (length(cohForce) > 0.001f)
                    {
                        float distToMouse = length(g_SimData[0].MousePosition - pos2D);

                        // When close to mouse, encourage circular distribution instead of center pull
                        if (distToMouse < 0.2f)
                        {
                            // Find ideal orbital position
                            float2 mouseToCenter = centerMass - g_SimData[0].MousePosition;
                            if (length(mouseToCenter) > 0.001f)
                            {
                                float idealDist = 0.12f; // Preferred distance from mouse
                                float2 idealPos = g_SimData[0].MousePosition + Normalize2D(mouseToCenter) * idealDist;
                                cohForce = idealPos - pos2D;
                            }
                        }

                        if (length(cohForce) > 0.3f)
                            cohForce = Normalize2D(cohForce) * 0.3f;
                        if (length(cohForce) > MAX_FORCE * 1.2f)
                            cohForce = Normalize2D(cohForce) * (MAX_FORCE * 1.2f);

                        float cohScale = (distToMouse < 0.15f) ? 0.8f : 1.0f; // Reduce when close
                        force += cohForce * COH_WEIGHT * cohScale;
                    }
                }
            }

            // Update physics
            float deltaTime = clamp(g_SimData[0].DeltaTime, 0.0001f, 0.1f);

            if (length(force) > MAX_FORCE * 1.5f)
                force = Normalize2D(force) * (MAX_FORCE * 1.5f);

            drawData.Velocity *= 0.95f; // Dampening
            drawData.Velocity += force * deltaTime;

            float speed = length(drawData.Velocity);
            if (speed > MAX_SPEED)
                drawData.Velocity = Normalize2D(drawData.Velocity) * MAX_SPEED;
            else if (speed < MIN_SPEED && speed > 0.001f)
            {
                float distToMouse = length(g_SimData[0].MousePosition - pos2D);
                if (distToMouse > 0.2f)
                    drawData.Velocity = Normalize2D(drawData.Velocity) * MIN_SPEED;
            }

            pos2D += drawData.Velocity * deltaTime;

            // Escape mechanism and wrapping
            float2 toMouse = g_SimData[0].MousePosition - pos2D;
            if (length(toMouse) > 0.8f)
                pos2D += Normalize2D(toMouse) * 0.5f * deltaTime;

            if (pos2D.x > 1.0f) pos2D.x = -1.0f;
            if (pos2D.x < -1.0f) pos2D.x = 1.0f;
            if (pos2D.y > 1.0f) pos2D.y = -1.0f;
            if (pos2D.y < -1.0f) pos2D.y = 1.0f;

            // Rotation and transform
            float scale = 0.012f;
            float angle = (speed > MIN_VEL_ROT) ?
                         atan2(drawData.Velocity.y, drawData.Velocity.x) - 1.57079632679f :
                         atan2(toMouse.y, toMouse.x) - 1.57079632679f;

            float cosA = cos(angle), sinA = sin(angle);

            drawData.Transform._11 = cosA * scale; drawData.Transform._12 = sinA * scale;
            drawData.Transform._13 = 0.0f; drawData.Transform._14 = 0.0f;
            drawData.Transform._21 = -sinA * scale; drawData.Transform._22 = cosA * scale;
            drawData.Transform._23 = 0.0f; drawData.Transform._24 = 0.0f;
            drawData.Transform._31 = 0.0f; drawData.Transform._32 = 0.0f;
            drawData.Transform._33 = scale; drawData.Transform._34 = 0.0f;
            drawData.Transform._41 = pos2D.x; drawData.Transform._42 = pos2D.y;
            drawData.Transform._43 = 0.0f; drawData.Transform._44 = 1.0f;

            // Update color based on speed for visual effect
            float speedNorm = speed / MAX_SPEED;
            drawData.Color.w = 0.3f + speedNorm * 0.7f; // Vary alpha based on speed

            // Write back updated data
            g_PerDrawData[triangleIndex] = drawData;

            // Generate triangle vertices
            uint baseVertexIndex = triangleIndex * 3;
            uint baseIndexIndex = triangleIndex * 3;

            // Triangle vertices (clockwise winding for left-handed system)
            float3 baseTriangle[3] = {
                float3(0.0, 0.8, 0.0),    // Top vertex
                float3(-0.5, -0.5, 0.0),  // Bottom left
                float3(0.5, -0.5, 0.0)    // Bottom right
            };

            // Transform vertices and generate geometry
            for (int i = 0; i < 3; i++)
            {
                // Apply transform (following DirectX convention: position * matrix)
                float4 localPos = float4(baseTriangle[i], 1.0);
                float4 worldPos = mul(localPos, drawData.Transform);

                g_VertexBuffer[baseVertexIndex + i].Position = worldPos;
                g_VertexBuffer[baseVertexIndex + i].Color = drawData.Color;
                g_IndexBuffer[baseIndexIndex + i] = baseVertexIndex + i;
            }
        }
        )";
    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}

DenOfIz_ByteArray IndirectRenderingExample::VertexShader( )
{
    const auto shaderCode = R"(
        struct VSInput
        {
            float4 Position : POSITION;
            float4 Color : TEXCOORD0;
        };

        struct PSInput
        {
            float4 Position : SV_POSITION;
            float4 Color : TEXCOORD0;
        };

        PSInput VSMain(VSInput input)
        {
            PSInput output;
            output.Position = input.Position;
            output.Color = input.Color;
            return output;
        }
        )";
    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}

DenOfIz_ByteArray IndirectRenderingExample::PixelShader( )
{
    const auto shaderCode = R"(
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float4 Color : TEXCOORD0;
        };

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return input.Color;
        }
        )";
    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}
