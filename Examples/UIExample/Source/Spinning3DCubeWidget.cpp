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

#include <DenOfIzExamples/Spinning3DCubeWidget.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <DenOfIzGraphics/Utilities/InteropUtilities.h>
#include <string>
#include <vector>

using namespace DenOfIz;
using namespace DirectX;

Spinning3DCubeWidget::Spinning3DCubeWidget( const uint32_t id ) : m_id( id )
{
}

Spinning3DCubeWidget::~Spinning3DCubeWidget( )
{
    if ( m_uniformData )
    {
        DenOfIz_Buffer_UnmapMemory( m_uniformBuffer );
        m_uniformData = nullptr;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_uniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_uniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_bindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_bindGroup );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shaderProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_shaderProgram );
    }

    for ( auto &rt : m_renderTargets )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( rt ) )
        {
            DenOfIz_TextureResource_Destroy( rt );
        }
    }
    for ( auto &db : m_depthBuffers )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( db ) )
        {
            DenOfIz_TextureResource_Destroy( db );
        }
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) )
    {
        DenOfIz_CommandListPool_Destroy( m_commandListPool );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_renderSemaphore ) )
    {
        DenOfIz_Semaphore_Destroy( m_renderSemaphore );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_resourceTracking ) )
    {
        DenOfIz_ResourceTracking_Destroy( m_resourceTracking );
    }
}

void Spinning3DCubeWidget::CreateShaderProgram( )
{
    const auto vertexShaderHLSL = R"(
struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

cbuffer Constants : register(b0) {
    float4x4 MVP;
    float4 lightDirection;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(float4(input.position, 1.0), MVP);
    output.normal = input.normal;
    output.color = input.color;
    return output;
}
)";

    const auto pixelShaderHLSL = R"(
struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

cbuffer Constants : register(b0) {
    float4x4 MVP;
    float4 lightDirection;
};

float4 main(PSInput input) : SV_TARGET {
    float3 lightDir = normalize(lightDirection.xyz);
    float NdotL = max(dot(normalize(input.normal), -lightDir), 0.0);
    float3 ambient = input.color.rgb * 0.3;
    float3 diffuse = input.color.rgb * NdotL;
    return float4(ambient + diffuse, input.color.a);
}
)";

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vsDesc.EntryPoint                             = DENOFIZ_STRING( "main" );
    vsDesc.Data                                   = DenOfIz_InteropUtilities_StringToBytes( vertexShaderHLSL );

    DenOfIz_ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    psDesc.EntryPoint               = DENOFIZ_STRING( "main" );
    psDesc.Data                     = DenOfIz_InteropUtilities_StringToBytes( pixelShaderHLSL );

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_shaderProgram                      = DenOfIz_ShaderProgram_Create( &programDesc );
}

void Spinning3DCubeWidget::CreatePipeline( )
{
    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_shaderProgram, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_device, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_device, &rootSigDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_device, &reflectDesc.InputLayout, &m_inputLayout );

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature = m_rootSignature;
    pipelineDesc.InputLayout   = m_inputLayout;
    pipelineDesc.ShaderProgram = m_shaderProgram;
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

    pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_BACK_FACE;
    pipelineDesc.Graphics.FillMode          = DENOFIZ_FILL_MODE_SOLID;

    DenOfIz_RenderTargetDesc renderTarget{ };
    renderTarget.Format                             = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTarget.Blend.RenderTargetWriteMask        = 0x0F;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;

    pipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_LESS;

    DenOfIz_LogicalDevice_CreatePipeline( m_device, &pipelineDesc, &m_pipeline );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
    DenOfIz_LogicalDevice_CreateBindGroup( m_device, &bindGroupDesc, &m_bindGroup );
}

void Spinning3DCubeWidget::CreateGeometry( )
{
    DenOfIz_BoxDesc boxDesc{ };
    boxDesc.Width     = 1.0f;
    boxDesc.Height    = 1.0f;
    boxDesc.Depth     = 1.0f;
    boxDesc.BuildDesc = DENOFIZ_BUILD_DESC_BUILD_NORMAL;

    DenOfIz_GeometryData geometry = DENOFIZ_NULL_HANDLE;
    DenOfIz_Geometry_BuildBox( &boxDesc, &geometry );

    uint32_t vertexCount = 0;
    uint32_t indexCount  = 0;
    DenOfIz_GeometryData_GetVertexCount( geometry, &vertexCount );
    DenOfIz_GeometryData_GetIndexCount( geometry, &indexCount );

    std::vector<DenOfIz_GeometryVertexData> vertices( vertexCount );
    std::vector<uint32_t>                   indices( indexCount );

    DenOfIz_GeometryData_GetVertexData( geometry, vertices.data( ) );
    DenOfIz_GeometryData_GetIndexData( geometry, indices.data( ) );

    std::vector<CubeVertex> formattedVertices( vertexCount );
    for ( uint32_t i = 0; i < vertexCount; i++ )
    {
        const DenOfIz_GeometryVertexData &vertexData = vertices[ i ];
        formattedVertices[ i ]                       = { { vertexData.Position.X, vertexData.Position.Y, vertexData.Position.Z },
                                                         { vertexData.Normal.X, vertexData.Normal.Y, vertexData.Normal.Z },
                                                         m_cubeColor };
    }

    DenOfIz_BufferDesc vertexBufferDesc{ };
    vertexBufferDesc.NumBytes  = formattedVertices.size( ) * sizeof( CubeVertex );
    vertexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    vertexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    vertexBufferDesc.DebugName = DENOFIZ_STRING( "3D Cube Vertex Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &vertexBufferDesc, &m_vertexBuffer );

    m_indexCount = indexCount;
    DenOfIz_BufferDesc indexBufferDesc{ };
    indexBufferDesc.NumBytes  = m_indexCount * sizeof( uint32_t );
    indexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    indexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    indexBufferDesc.DebugName = DENOFIZ_STRING( "3D Cube Index Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &indexBufferDesc, &m_indexBuffer );

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device        = m_device;
    batchDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    DenOfIz_CopyToGpuBufferDesc vertexCopyDesc{ };
    vertexCopyDesc.DstBuffer        = m_vertexBuffer;
    vertexCopyDesc.DstBufferOffset  = 0;
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( formattedVertices.data( ) );
    vertexCopyDesc.Data.NumElements = formattedVertices.size( ) * sizeof( CubeVertex );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &vertexCopyDesc );

    DenOfIz_CopyToGpuBufferDesc indexCopyDesc{ };
    indexCopyDesc.DstBuffer        = m_indexBuffer;
    indexCopyDesc.DstBufferOffset  = 0;
    indexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( indices.data( ) );
    indexCopyDesc.Data.NumElements = indexCount * sizeof( uint32_t );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &indexCopyDesc );

    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    DenOfIz_GeometryData_Destroy( geometry );

    DenOfIz_BufferDesc uniformBufferDesc{ };
    uniformBufferDesc.NumBytes  = sizeof( CubeUniforms );
    uniformBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    uniformBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    uniformBufferDesc.DebugName = DENOFIZ_STRING( "3D Cube Uniform Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &uniformBufferDesc, &m_uniformBuffer );
    DenOfIz_Buffer_MapMemory( m_uniformBuffer, reinterpret_cast<void **>( &m_uniformData ) );

    XMStoreFloat4x4( &m_uniformData->MVP, XMMatrixIdentity( ) );
    m_uniformData->LightDirection = XMFLOAT4( 0.5f, -0.7f, 0.5f, 0.0f );

    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 0, m_uniformBuffer );
    DenOfIz_BindGroup_EndUpdate( m_bindGroup );
}

void Spinning3DCubeWidget::UpdateUniforms( const uint32_t width, const uint32_t height ) const
{
    const float    aspectRatio = static_cast<float>( width ) / static_cast<float>( height );
    const XMMATRIX projection  = XMMatrixPerspectiveFovLH( XMConvertToRadians( 60.0f ), aspectRatio, 0.1f, 10.0f );
    const XMMATRIX view        = XMMatrixLookAtLH( XMVectorSet( 0.0f, 0.0f, -2.0f, 1.0f ), XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f ), XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ) );

    const XMMATRIX rotationY = XMMatrixRotationY( m_rotation );
    const XMMATRIX rotationX = XMMatrixRotationX( m_rotation * 0.7f );
    const XMMATRIX model     = rotationX * rotationY;

    const XMMATRIX mvp = model * view * projection;

    XMStoreFloat4x4( &m_uniformData->MVP, mvp );
    m_uniformData->LightDirection = XMFLOAT4( 0.5f, -0.7f, 0.5f, 0.0f );
}

void Spinning3DCubeWidget::InitializeRenderResources( DenOfIz_LogicalDevice device, DenOfIz_CommandQueue commandQueue, const uint32_t width, const uint32_t height )
{
    m_device       = device;
    m_commandQueue = commandQueue;

    DenOfIz_ResourceTracking_Create( &m_resourceTracking );

    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.NumCommandLists = m_numFrames;
    poolDesc.CommandQueue    = commandQueue;
    DenOfIz_LogicalDevice_CreateCommandListPool( device, &poolDesc, &m_commandListPool );

    DenOfIz_LogicalDevice_CreateSemaphore( device, &m_renderSemaphore );

    constexpr uint32_t rtSize = 512;
    for ( uint32_t frameIdx = 0; frameIdx < m_numFrames; ++frameIdx )
    {
        DenOfIz_TextureDesc rtDesc{ };
        rtDesc.Width                       = rtSize;
        rtDesc.Height                      = rtSize;
        rtDesc.Depth                       = 1;
        rtDesc.MipLevels                   = 1;
        rtDesc.ArraySize                   = 1;
        rtDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        rtDesc.Usage                       = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
        rtDesc.HeapType                    = DENOFIZ_HEAP_TYPE_GPU;
        rtDesc.ClearColorHint              = { 0.0f, 0.0f, 0.0f, 1.0f };
        const std::string renderTargetName = "3D Cube Widget Render Target Frame " + std::to_string( frameIdx );
        rtDesc.DebugName                   = DenOfIz_StringView( renderTargetName.c_str( ), static_cast<uint32_t>( renderTargetName.size( ) ) );
        DenOfIz_LogicalDevice_CreateTexture( device, &rtDesc, &m_renderTargets[ frameIdx ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_renderTargets[ frameIdx ], DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_TextureDesc depthDesc{ };
        depthDesc.Width                 = rtSize;
        depthDesc.Height                = rtSize;
        depthDesc.Depth                 = 1;
        depthDesc.MipLevels             = 1;
        depthDesc.ArraySize             = 1;
        depthDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
        depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
        depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
        depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
        const std::string depthName     = "3D Cube Widget Depth Buffer Frame " + std::to_string( frameIdx );
        depthDesc.DebugName             = DenOfIz_StringView( depthName.c_str( ), static_cast<uint32_t>( depthName.size( ) ) );
        DenOfIz_LogicalDevice_CreateTexture( device, &depthDesc, &m_depthBuffers[ frameIdx ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_depthBuffers[ frameIdx ], DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }

    CreateShaderProgram( );
    CreatePipeline( );
    CreateGeometry( );
    UpdateUniforms( rtSize, rtSize );
}

void Spinning3DCubeWidget::ResizeRenderResources( const uint32_t width, const uint32_t height )
{
    constexpr uint32_t rtSize = 512;
    for ( uint32_t frameIdx = 0; frameIdx < m_numFrames; ++frameIdx )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_renderTargets[ frameIdx ] ) )
        {
            DenOfIz_TextureResource_Destroy( m_renderTargets[ frameIdx ] );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_depthBuffers[ frameIdx ] ) )
        {
            DenOfIz_TextureResource_Destroy( m_depthBuffers[ frameIdx ] );
        }

        DenOfIz_TextureDesc rtDesc{ };
        rtDesc.Width                       = rtSize;
        rtDesc.Height                      = rtSize;
        rtDesc.Depth                       = 1;
        rtDesc.MipLevels                   = 1;
        rtDesc.ArraySize                   = 1;
        rtDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        rtDesc.Usage                       = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
        rtDesc.HeapType                    = DENOFIZ_HEAP_TYPE_GPU;
        const std::string renderTargetName = "3D Cube Widget Render Target Frame " + std::to_string( frameIdx );
        rtDesc.DebugName                   = DenOfIz_StringView( renderTargetName.c_str( ), static_cast<uint32_t>( renderTargetName.size( ) ) );
        rtDesc.ClearColorHint              = { 0.0f, 0.0f, 0.0f, 1.0f };
        DenOfIz_LogicalDevice_CreateTexture( m_device, &rtDesc, &m_renderTargets[ frameIdx ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_renderTargets[ frameIdx ], DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_TextureDesc depthDesc{ };
        depthDesc.Width                 = rtSize;
        depthDesc.Height                = rtSize;
        depthDesc.Depth                 = 1;
        depthDesc.MipLevels             = 1;
        depthDesc.ArraySize             = 1;
        depthDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
        depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
        depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
        depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
        const std::string depthName     = "3D Cube Widget Depth Buffer Frame " + std::to_string( frameIdx );
        depthDesc.DebugName             = DenOfIz_StringView( depthName.c_str( ), static_cast<uint32_t>( depthName.size( ) ) );
        DenOfIz_LogicalDevice_CreateTexture( m_device, &depthDesc, &m_depthBuffers[ frameIdx ] );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_depthBuffers[ frameIdx ], DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }

    UpdateUniforms( rtSize, rtSize );
}

void Spinning3DCubeWidget::SetRotationSpeed( const float speed )
{
    m_rotationSpeed = speed;
}

void Spinning3DCubeWidget::SetCubeColor( const XMFLOAT4 &color )
{
    m_cubeColor = color;
}

void Spinning3DCubeWidget::Update( const float deltaTime )
{
    m_rotation += m_rotationSpeed * deltaTime;
    if ( m_rotation > XM_2PI )
    {
        m_rotation -= XM_2PI;
    }
}

void Spinning3DCubeWidget::RenderToTexture( uint32_t frameIndex )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_commandListPool ) || frameIndex >= m_numFrames )
    {
        return;
    }

    DenOfIz_Texture cubeRenderTarget = GetRenderTarget( frameIndex );
    if ( !DENOFIZ_HANDLE_IS_VALID( cubeRenderTarget ) )
    {
        return;
    }

    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( m_commandListPool, &commandLists );
    DenOfIz_CommandList cubeCommandList = commandLists.Elements[ frameIndex ];

    constexpr uint32_t rtSize = 512;
    UpdateUniforms( rtSize, rtSize );
    DenOfIz_CommandList_Begin( cubeCommandList );

    DenOfIz_Texture renderTarget = m_renderTargets[ frameIndex ];
    DenOfIz_Texture depthBuffer  = m_depthBuffers[ frameIndex ];

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, cubeCommandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, cubeCommandList, depthBuffer, DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource   = renderTarget;
    attachmentDesc.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    attachmentDesc.StoreOp    = DENOFIZ_STORE_OP_STORE;
    attachmentDesc.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements            = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements         = 1;
    renderingDesc.DepthAttachment.Resource          = depthBuffer;
    renderingDesc.DepthAttachment.LoadOp            = DENOFIZ_LOAD_OP_CLEAR;
    renderingDesc.DepthAttachment.StoreOp           = DENOFIZ_STORE_OP_DONT_CARE;
    renderingDesc.DepthAttachment.ClearDepthStencil = { 1.0f, 0.0f };
    renderingDesc.RenderAreaWidth                   = rtSize;
    renderingDesc.RenderAreaHeight                  = rtSize;
    renderingDesc.NumLayers                         = 1;

    DenOfIz_CommandList_BeginRendering( cubeCommandList, &renderingDesc );
    DenOfIz_CommandList_BindViewport( cubeCommandList, 0.0f, 0.0f, rtSize, rtSize );
    DenOfIz_CommandList_BindScissorRect( cubeCommandList, 0.0f, 0.0f, rtSize, rtSize );
    DenOfIz_CommandList_BindPipeline( cubeCommandList, m_pipeline );
    DenOfIz_CommandList_BindGroup( cubeCommandList, m_bindGroup );
    DenOfIz_CommandList_BindVertexBuffer( cubeCommandList, m_vertexBuffer, 0, sizeof( CubeVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( cubeCommandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );
    DenOfIz_CommandList_DrawIndexed( cubeCommandList, m_indexCount, 1, 0, 0, 0 );
    DenOfIz_CommandList_EndRendering( cubeCommandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, cubeCommandList, renderTarget, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, cubeCommandList, depthBuffer, DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( cubeCommandList );

    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements        = &cubeCommandList;
    executeDesc.CommandLists.NumElements     = 1;
    executeDesc.SignalSemaphores.Elements    = &m_renderSemaphore;
    executeDesc.SignalSemaphores.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( m_commandQueue, &executeDesc );
}

DenOfIz_Texture Spinning3DCubeWidget::GetRenderTarget( const uint32_t frameIndex ) const
{
    if ( frameIndex < m_renderTargets.size( ) )
    {
        return m_renderTargets[ frameIndex ];
    }
    return DENOFIZ_NULL_HANDLE;
}

DenOfIz_Semaphore Spinning3DCubeWidget::GetRenderSemaphore( ) const
{
    return m_renderSemaphore;
}
