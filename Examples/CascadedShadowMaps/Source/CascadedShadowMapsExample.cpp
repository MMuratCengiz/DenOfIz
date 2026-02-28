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
#include "DenOfIzExamples/CascadedShadowMapsExample.h"
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <DenOfIzGraphics/Utilities/InteropUtilities.h>
#include <algorithm>
#include <cmath>

using namespace DenOfIz;
using namespace DirectX;

// ─── Shader sources ──────────────────────────────────────────────────────────

static const char *ShadowVS = R"(
struct VSInput {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position               : SV_POSITION;
    uint   RenderTargetArrayIndex : SV_RenderTargetArrayIndex;
};

cbuffer ShadowCB : register(b0) {
    float4x4 LightViewProj[4];
    float4x4 Models[6];
};

struct RootConstants {
    uint ObjectIndex;
};

[[vk::push_constant]] ConstantBuffer<RootConstants> RootConstants : register(b0, space31);

VSOutput main(VSInput input, uint instID : SV_InstanceID) {
    VSOutput output;
    float4 worldPos = mul(float4(input.Position, 1.0), Models[RootConstants.ObjectIndex]);
    output.Position = mul(worldPos, LightViewProj[instID]);
    output.RenderTargetArrayIndex = instID;
    return output;
}
)";

static const char *MainVS = R"(
struct VSInput {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float  ViewDepth : TEXCOORD3;
};

cbuffer SceneCB : register(b0) {
    float4x4 ViewProj;
    float4x4 Models[6];
    float4x4 LightViewProj[4];
    float4   CascadeSplits;
    float4   LightDir;
    float4   CameraPos;
};


struct RootConstants {
    uint ObjectIndex;
};

[[vk::push_constant]] ConstantBuffer<RootConstants> RootConstants : register(b0, space31);

VSOutput main(VSInput input) {
    VSOutput output;
    float4 worldPos = mul(float4(input.Position, 1.0), Models[RootConstants.ObjectIndex]);
    output.Position = mul(worldPos, ViewProj);
    output.WorldPos = worldPos.xyz;
    output.Normal   = mul(float4(input.Normal, 0.0), Models[RootConstants.ObjectIndex]).xyz;
    output.TexCoord = input.TexCoord;
    output.ViewDepth = output.Position.w;
    return output;
}
)";

static const char *MainPS = R"(
struct PSInput {
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float  ViewDepth : TEXCOORD3;
};

cbuffer SceneCB : register(b0) {
    float4x4 ViewProj;
    float4x4 Models[6];
    float4x4 LightViewProj[4];
    float4   CascadeSplits;
    float4   LightDir;
    float4   CameraPos;
};

Texture2DArray<float> ShadowMap : register(t0);
SamplerComparisonState ShadowSampler : register(s0);

float SampleShadow(float3 worldPos, uint cascadeIndex) {
    float4 lightSpacePos = mul(float4(worldPos, 1.0), LightViewProj[cascadeIndex]);
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    float2 shadowUV;
    shadowUV.x = projCoords.x * 0.5 + 0.5;
    shadowUV.y = -projCoords.y * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0 || currentDepth > 1.0)
        return 1.0;

    float shadow = 0.0;
    float texelSize = 1.0 / 2048.0;
    [unroll]
    for (int x = -1; x <= 1; x++) {
        [unroll]
        for (int y = -1; y <= 1; y++) {
            float2 offset = float2(x, y) * texelSize;
            shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(shadowUV + offset, cascadeIndex), currentDepth);
        }
    }
    return shadow / 9.0;
}

float4 main(PSInput input) : SV_TARGET {
    float splits[4] = { CascadeSplits.x, CascadeSplits.y, CascadeSplits.z, CascadeSplits.w };

    uint cascadeIndex = 0;
    for (uint i = 0; i < 4; i++) {
        if (input.ViewDepth > splits[i]) {
            cascadeIndex = i + 1;
        }
    }
    cascadeIndex = min(cascadeIndex, 3);

    float shadow = SampleShadow(input.WorldPos, cascadeIndex);

    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-LightDir.xyz);
    float NdotL = max(dot(normal, lightDir), 0.0);

    // Checkerboard pattern for ground plane
    float3 baseColor = float3(0.8, 0.8, 0.8);
    float checker = step(0.0, sin(input.WorldPos.x * 3.14159) * sin(input.WorldPos.z * 3.14159));
    if (abs(input.Normal.y) > 0.9) {
        baseColor = lerp(float3(0.4, 0.5, 0.4), float3(0.6, 0.7, 0.6), checker);
    }

    float3 ambient = baseColor * 0.2;
    float3 diffuse = baseColor * NdotL * shadow;
    float3 color = ambient + diffuse;

    // Cascade debug tint
    float3 cascadeColors[4] = {
        float3(0.05, 0.0, 0.0),
        float3(0.0, 0.05, 0.0),
        float3(0.0, 0.0, 0.05),
        float3(0.05, 0.05, 0.0)
    };
    color += cascadeColors[cascadeIndex];

    return float4(color, 1.0);
}
)";

void CascadedShadowMapsExample::CreateGeometry( )
{
    DenOfIz_QuadDesc quadDesc{};
    quadDesc.Width     = 50.0f;
    quadDesc.Height    = 50.0f;
    quadDesc.BuildDesc = DENOFIZ_BUILD_DESC_BUILD_NORMAL | DENOFIZ_BUILD_DESC_BUILD_TEXCOORD;

    DenOfIz_GeometryData groundGeometry = DENOFIZ_NULL_HANDLE;
    DenOfIz_Geometry_BuildQuadXZ( &quadDesc, &groundGeometry );

    uint32_t groundVertexCount = 0, groundIndexCount = 0;
    DenOfIz_GeometryData_GetVertexCount( groundGeometry, &groundVertexCount );
    DenOfIz_GeometryData_GetIndexCount( groundGeometry, &groundIndexCount );

    std::vector<DenOfIz_GeometryVertexData> groundVerts( groundVertexCount );
    std::vector<uint32_t>                   groundIndices( groundIndexCount );
    DenOfIz_GeometryData_GetVertexData( groundGeometry, groundVerts.data( ) );
    DenOfIz_GeometryData_GetIndexData( groundGeometry, groundIndices.data( ) );

    DenOfIz_BoxDesc boxDesc{};
    boxDesc.Width     = 1.0f;
    boxDesc.Height    = 1.0f;
    boxDesc.Depth     = 1.0f;
    boxDesc.BuildDesc = DENOFIZ_BUILD_DESC_BUILD_NORMAL | DENOFIZ_BUILD_DESC_BUILD_TEXCOORD;

    DenOfIz_GeometryData boxGeometry = DENOFIZ_NULL_HANDLE;
    DenOfIz_Geometry_BuildBox( &boxDesc, &boxGeometry );

    uint32_t boxVertexCount = 0, boxIndexCount = 0;
    DenOfIz_GeometryData_GetVertexCount( boxGeometry, &boxVertexCount );
    DenOfIz_GeometryData_GetIndexCount( boxGeometry, &boxIndexCount );

    std::vector<DenOfIz_GeometryVertexData> boxVerts( boxVertexCount );
    std::vector<uint32_t>                   boxIndices( boxIndexCount );
    DenOfIz_GeometryData_GetVertexData( boxGeometry, boxVerts.data( ) );
    DenOfIz_GeometryData_GetIndexData( boxGeometry, boxIndices.data( ) );

    // Pack into single buffer: ground (obj 0) + 5 cubes (obj 1-5)
    std::vector<SceneVertex> allVertices;
    std::vector<uint32_t>    allIndices;

    auto addMesh = [ & ]( const std::vector<DenOfIz_GeometryVertexData> &verts, const std::vector<uint32_t> &indices, uint32_t objIdx )
    {
        ObjectInfo &obj  = m_objects[ objIdx ];
        obj.VertexOffset = static_cast<uint32_t>( allVertices.size( ) );
        obj.IndexOffset  = static_cast<uint32_t>( allIndices.size( ) );
        obj.IndexCount   = static_cast<uint32_t>( indices.size( ) );

        for ( const auto &v : verts )
        {
            SceneVertex sv{};
            sv.Position[ 0 ] = v.Position.X;
            sv.Position[ 1 ] = v.Position.Y;
            sv.Position[ 2 ] = v.Position.Z;
            sv.Normal[ 0 ]   = v.Normal.X;
            sv.Normal[ 1 ]   = v.Normal.Y;
            sv.Normal[ 2 ]   = v.Normal.Z;
            sv.TexCoord[ 0 ] = v.TextureCoordinate.U;
            sv.TexCoord[ 1 ] = v.TextureCoordinate.V;
            allVertices.push_back( sv );
        }

        for ( auto idx : indices )
        {
            allIndices.push_back( idx );
        }
    };

    addMesh( groundVerts, groundIndices, 0 );
    for ( uint32_t i = 1; i <= 5; ++i )
    {
        addMesh( boxVerts, boxIndices, i );
    }

    m_totalVertices = static_cast<uint32_t>( allVertices.size( ) );
    m_totalIndices  = static_cast<uint32_t>( allIndices.size( ) );

    DenOfIz_GeometryData_Destroy( groundGeometry );
    DenOfIz_GeometryData_Destroy( boxGeometry );

    DenOfIz_BufferDesc vertexBufferDesc{};
    vertexBufferDesc.NumBytes  = allVertices.size( ) * sizeof( SceneVertex );
    vertexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    vertexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    vertexBufferDesc.DebugName = DENOFIZ_STRING( "CSM Vertex Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vertexBufferDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc indexBufferDesc{};
    indexBufferDesc.NumBytes  = allIndices.size( ) * sizeof( uint32_t );
    indexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    indexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    indexBufferDesc.DebugName = DENOFIZ_STRING( "CSM Index Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &indexBufferDesc, &m_indexBuffer );

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{};
    batchDesc.Device        = m_logicalDevice;
    batchDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    DenOfIz_CopyToGpuBufferDesc vertexCopyDesc{};
    vertexCopyDesc.DstBuffer        = m_vertexBuffer;
    vertexCopyDesc.DstBufferOffset  = 0;
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( allVertices.data( ) );
    vertexCopyDesc.Data.NumElements = allVertices.size( ) * sizeof( SceneVertex );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &vertexCopyDesc );

    DenOfIz_CopyToGpuBufferDesc indexCopyDesc{};
    indexCopyDesc.DstBuffer        = m_indexBuffer;
    indexCopyDesc.DstBufferOffset  = 0;
    indexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( allIndices.data( ) );
    indexCopyDesc.Data.NumElements = allIndices.size( ) * sizeof( uint32_t );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &indexCopyDesc );

    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );
}

void CascadedShadowMapsExample::CreateShadowResources( )
{
    DenOfIz_TextureDesc shadowTexDesc{};
    shadowTexDesc.Width                 = SHADOW_MAP_SIZE;
    shadowTexDesc.Height                = SHADOW_MAP_SIZE;
    shadowTexDesc.Depth                 = 1;
    shadowTexDesc.ArraySize             = NUM_CASCADES;
    shadowTexDesc.MipLevels             = 1;
    shadowTexDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
    shadowTexDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    shadowTexDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
    shadowTexDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
    shadowTexDesc.DebugName             = DENOFIZ_STRING( "CSM Shadow Map Array" );
    DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &shadowTexDesc, &m_shadowMapArray );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_shadowMapArray, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    std::array<DenOfIz_ShaderStageDesc, 1> shadowStages{};
    shadowStages[ 0 ].Stage      = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    shadowStages[ 0 ].EntryPoint = DENOFIZ_STRING( "main" );
    shadowStages[ 0 ].Data       = DenOfIz_InteropUtilities_StringToBytes( ShadowVS );

    DenOfIz_ShaderProgramDesc shadowProgramDesc{};
    shadowProgramDesc.ShaderStages.Elements    = shadowStages.data( );
    shadowProgramDesc.ShaderStages.NumElements = shadowStages.size( );
    m_shadowShaderProgram                      = DenOfIz_ShaderProgram_Create( &shadowProgramDesc );

    DenOfIz_ShaderReflectDesc shadowReflect{};
    DenOfIz_ShaderProgram_Reflect( m_shadowShaderProgram, &shadowReflect );

    m_shadowBindGroupLayouts.resize( shadowReflect.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < shadowReflect.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &shadowReflect.BindGroupLayouts.Elements[ i ], &m_shadowBindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc shadowRootSigDesc{};
    shadowRootSigDesc.BindGroupLayouts.Elements    = m_shadowBindGroupLayouts.data( );
    shadowRootSigDesc.BindGroupLayouts.NumElements = m_shadowBindGroupLayouts.size( );
    shadowRootSigDesc.RootConstants                = shadowReflect.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &shadowRootSigDesc, &m_shadowRootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &shadowReflect.InputLayout, &m_shadowInputLayout );

    DenOfIz_PipelineDesc shadowPipelineDesc{};
    shadowPipelineDesc.RootSignature = m_shadowRootSignature;
    shadowPipelineDesc.InputLayout   = m_shadowInputLayout;
    shadowPipelineDesc.ShaderProgram = m_shadowShaderProgram;
    shadowPipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

    shadowPipelineDesc.Graphics.PrimitiveTopology                  = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    shadowPipelineDesc.Graphics.CullMode                           = DENOFIZ_CULL_MODE_BACK_FACE;
    shadowPipelineDesc.Graphics.FillMode                           = DENOFIZ_FILL_MODE_SOLID;
    shadowPipelineDesc.Graphics.DepthStencilAttachmentFormat       = DENOFIZ_FORMAT_D32_FLOAT;
    shadowPipelineDesc.Graphics.DepthTest.Enable                   = true;
    shadowPipelineDesc.Graphics.DepthTest.Write                    = true;
    shadowPipelineDesc.Graphics.DepthTest.CompareOp                = DENOFIZ_COMPARE_OP_LESS;
    shadowPipelineDesc.Graphics.Rasterization.DepthBias            = 2;
    shadowPipelineDesc.Graphics.Rasterization.SlopeScaledDepthBias = 2.0f;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &shadowPipelineDesc, &m_shadowPipeline );

    DenOfIz_BufferDesc shadowUBDesc{};
    shadowUBDesc.NumBytes  = sizeof( ShadowCB );
    shadowUBDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    shadowUBDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    shadowUBDesc.DebugName = DENOFIZ_STRING( "CSM Shadow Uniform Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &shadowUBDesc, &m_shadowUniformBuffer );
    DenOfIz_Buffer_MapMemory( m_shadowUniformBuffer, reinterpret_cast<void **>( &m_shadowCBData ) );

    if ( !m_shadowBindGroupLayouts.empty( ) )
    {
        DenOfIz_BindGroupDesc shadowBGDesc{};
        shadowBGDesc.Layout = m_shadowBindGroupLayouts[ 0 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &shadowBGDesc, &m_shadowBindGroup );

        DenOfIz_BindGroup_BeginUpdate( m_shadowBindGroup );
        DenOfIz_BindGroup_Cbv( m_shadowBindGroup, 0, m_shadowUniformBuffer );
        DenOfIz_BindGroup_EndUpdate( m_shadowBindGroup );
    }
}

void CascadedShadowMapsExample::CreateMainResources( )
{
    // Screen-sized depth buffer
    DenOfIz_TextureDesc depthDesc{};
    depthDesc.Width                 = m_pixelWidth;
    depthDesc.Height                = m_pixelHeight;
    depthDesc.Depth                 = 1;
    depthDesc.ArraySize             = 1;
    depthDesc.MipLevels             = 1;
    depthDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
    depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
    depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
    depthDesc.DebugName             = DENOFIZ_STRING( "CSM Depth Buffer" );
    DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &depthDesc, &m_depthBuffer );
    DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_depthBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    std::array<DenOfIz_ShaderStageDesc, 2> mainStages{};
    mainStages[ 0 ].Stage      = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    mainStages[ 0 ].EntryPoint = DENOFIZ_STRING( "main" );
    mainStages[ 0 ].Data       = DenOfIz_InteropUtilities_StringToBytes( MainVS );
    mainStages[ 1 ].Stage      = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    mainStages[ 1 ].EntryPoint = DENOFIZ_STRING( "main" );
    mainStages[ 1 ].Data       = DenOfIz_InteropUtilities_StringToBytes( MainPS );

    DenOfIz_ShaderProgramDesc mainProgramDesc{};
    mainProgramDesc.ShaderStages.Elements    = mainStages.data( );
    mainProgramDesc.ShaderStages.NumElements = mainStages.size( );
    m_mainShaderProgram                      = DenOfIz_ShaderProgram_Create( &mainProgramDesc );

    DenOfIz_ShaderReflectDesc mainReflect{};
    DenOfIz_ShaderProgram_Reflect( m_mainShaderProgram, &mainReflect );

    m_mainBindGroupLayouts.resize( mainReflect.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < mainReflect.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &mainReflect.BindGroupLayouts.Elements[ i ], &m_mainBindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc mainRootSigDesc{};
    mainRootSigDesc.BindGroupLayouts.Elements    = m_mainBindGroupLayouts.data( );
    mainRootSigDesc.BindGroupLayouts.NumElements = m_mainBindGroupLayouts.size( );
    mainRootSigDesc.RootConstants                = mainReflect.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &mainRootSigDesc, &m_mainRootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &mainReflect.InputLayout, &m_mainInputLayout );

    DenOfIz_PipelineDesc mainPipelineDesc{};
    mainPipelineDesc.RootSignature = m_mainRootSignature;
    mainPipelineDesc.InputLayout   = m_mainInputLayout;
    mainPipelineDesc.ShaderProgram = m_mainShaderProgram;
    mainPipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

    mainPipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    mainPipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_BACK_FACE;
    mainPipelineDesc.Graphics.FillMode          = DENOFIZ_FILL_MODE_SOLID;

    DenOfIz_RenderTargetDesc rtDesc{};
    rtDesc.Format                                       = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    rtDesc.Blend.RenderTargetWriteMask                  = 0x0F;
    mainPipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
    mainPipelineDesc.Graphics.RenderTargets.NumElements = 1;

    mainPipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;
    mainPipelineDesc.Graphics.DepthTest.Enable             = true;
    mainPipelineDesc.Graphics.DepthTest.Write              = true;
    mainPipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_LESS;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &mainPipelineDesc, &m_mainPipeline );

    DenOfIz_BufferDesc sceneUBDesc{};
    sceneUBDesc.NumBytes  = sizeof( SceneCB );
    sceneUBDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    sceneUBDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    sceneUBDesc.DebugName = DENOFIZ_STRING( "CSM Scene Uniform Buffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &sceneUBDesc, &m_sceneUniformBuffer );
    DenOfIz_Buffer_MapMemory( m_sceneUniformBuffer, reinterpret_cast<void **>( &m_sceneCBData ) );

    DenOfIz_SamplerDesc samplerDesc{};
    samplerDesc.MinFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MagFilter    = DENOFIZ_FILTER_LINEAR;
    samplerDesc.AddressModeU = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.CompareOp    = DENOFIZ_COMPARE_OP_LESS_OR_EQUAL;
    samplerDesc.DebugName    = DENOFIZ_STRING( "CSM Shadow Comparison Sampler" );
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_shadowSampler );

    if ( !m_mainBindGroupLayouts.empty( ) )
    {
        DenOfIz_BindGroupDesc mainBGDesc{};
        mainBGDesc.Layout = m_mainBindGroupLayouts[ 0 ];
        DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &mainBGDesc, &m_mainBindGroup );

        DenOfIz_BindGroup_BeginUpdate( m_mainBindGroup );
        DenOfIz_BindGroup_Cbv( m_mainBindGroup, 0, m_sceneUniformBuffer );
        DenOfIz_BindGroup_SrvTexture( m_mainBindGroup, 0, m_shadowMapArray );
        DenOfIz_BindGroup_Sampler( m_mainBindGroup, 0, m_shadowSampler );
        DenOfIz_BindGroup_EndUpdate( m_mainBindGroup );
    }
}

void CascadedShadowMapsExample::ComputeCascades( )
{
    // Practical split scheme (blend of logarithmic and linear)
    m_cascadeSplits[ 0 ] = CAMERA_NEAR;
    for ( uint32_t i = 1; i <= NUM_CASCADES; ++i )
    {
        const float p        = static_cast<float>( i ) / NUM_CASCADES;
        const float log      = CAMERA_NEAR * std::pow( CAMERA_FAR / CAMERA_NEAR, p );
        const float lin      = CAMERA_NEAR + ( CAMERA_FAR - CAMERA_NEAR ) * p;
        m_cascadeSplits[ i ] = CASCADE_LAMBDA * log + ( 1.0f - CASCADE_LAMBDA ) * lin;
    }
}

void CascadedShadowMapsExample::UpdateUniforms( )
{
    const XMMATRIX viewProj    = m_camera->ViewProjectionMatrix( );
    const XMMATRIX invViewProj = XMMatrixInverse( nullptr, viewProj );

    const XMVECTOR lightDir = XMVector3Normalize( XMVectorSet( -0.5f, -1.0f, -0.3f, 0.0f ) );

    XMMATRIX models[ NUM_OBJECTS ];
    models[ 0 ] = XMMatrixIdentity( );
    models[ 1 ] = XMMatrixTranslation( 0.0f, 0.5f, 3.0f );
    models[ 2 ] = XMMatrixTranslation( 3.0f, 0.5f, 6.0f );
    models[ 3 ] = XMMatrixTranslation( -2.0f, 0.5f, 10.0f );
    models[ 4 ] = XMMatrixScaling( 1.0f, 2.0f, 1.0f ) * XMMatrixTranslation( 4.0f, 1.0f, 15.0f );
    models[ 5 ] = XMMatrixTranslation( -3.0f, 0.5f, 25.0f );

    // Compute light VP for each cascade
    XMMATRIX lightViewProj[ NUM_CASCADES ];
    for ( uint32_t c = 0; c < NUM_CASCADES; ++c )
    {
        const float nearSplit = m_cascadeSplits[ c ];
        const float farSplit  = m_cascadeSplits[ c + 1 ];

        // Get frustum corners in world space for this cascade slice
        XMVECTOR frustumCorners[ 8 ];
        // NDC corners: near plane z=0, far plane z=1 (LH)
        const float ndcNear = 0.0f;
        const float ndcFar  = 1.0f;

        // Map cascade split distances to NDC Z
        // For perspective projection: z_ndc = (far * (z - near)) / (z * (far - near))
        float ndcNearSlice, ndcFarSlice;
        {
            const float n = CAMERA_NEAR;
            const float f = CAMERA_FAR;
            ndcNearSlice  = ( f * ( nearSplit - n ) ) / ( nearSplit * ( f - n ) );
            ndcFarSlice   = ( f * ( farSplit - n ) ) / ( farSplit * ( f - n ) );
        }

        uint32_t idx = 0;
        for ( uint32_t z = 0; z < 2; ++z )
        {
            const float nz = ( z == 0 ) ? ndcNearSlice : ndcFarSlice;
            for ( uint32_t y = 0; y < 2; ++y )
            {
                for ( uint32_t x = 0; x < 2; ++x )
                {
                    const XMVECTOR ndcPoint   = XMVectorSet( x == 0 ? -1.0f : 1.0f, y == 0 ? -1.0f : 1.0f, nz, 1.0f );
                    XMVECTOR       worldPoint = XMVector4Transform( ndcPoint, invViewProj );
                    worldPoint                = XMVectorDivide( worldPoint, XMVectorSplatW( worldPoint ) );
                    frustumCorners[ idx++ ]   = worldPoint;
                }
            }
        }

        // Compute bounding sphere center
        XMVECTOR center = XMVectorZero( );
        for ( uint32_t i = 0; i < 8; ++i )
        {
            center = XMVectorAdd( center, frustumCorners[ i ] );
        }
        center = XMVectorScale( center, 1.0f / 8.0f );

        // Compute bounding sphere radius
        float radius = 0.0f;
        for ( uint32_t i = 0; i < 8; ++i )
        {
            const float dist = XMVectorGetX( XMVector3Length( XMVectorSubtract( frustumCorners[ i ], center ) ) );
            radius           = std::max( radius, dist );
        }
        // Round up to reduce shadow shimmer
        radius = std::ceil( radius * 16.0f ) / 16.0f;

        // Snap to texel grid to reduce shadow shimmer
        const XMVECTOR lightUp   = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        const XMVECTOR lightEye  = XMVectorSubtract( center, XMVectorScale( lightDir, radius ) );
        const XMMATRIX lightView = XMMatrixLookAtLH( lightEye, center, lightUp );
        XMMATRIX       lightProj = XMMatrixOrthographicLH( radius * 2.0f, radius * 2.0f, 0.0f, radius * 2.0f );

        // Snap to shadow map texels
        const XMMATRIX shadowMatrix = lightView * lightProj;
        XMVECTOR       origin       = XMVector4Transform( XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f ), shadowMatrix );
        origin                      = XMVectorScale( origin, SHADOW_MAP_SIZE / 2.0f );
        const XMVECTOR rounded      = XMVectorRound( origin );
        XMVECTOR       offset       = XMVectorSubtract( rounded, origin );
        offset                      = XMVectorScale( offset, 2.0f / SHADOW_MAP_SIZE );
        offset                      = XMVectorSetZ( offset, 0.0f );
        offset                      = XMVectorSetW( offset, 0.0f );

        lightProj.r[ 3 ] = XMVectorAdd( lightProj.r[ 3 ], offset );

        lightViewProj[ c ] = lightView * lightProj;
    }

    // Fill shadow CB
    for ( uint32_t c = 0; c < NUM_CASCADES; ++c )
    {
        XMStoreFloat4x4( &m_shadowCBData->LightViewProj[ c ], lightViewProj[ c ] );
    }
    for ( uint32_t i = 0; i < NUM_OBJECTS; ++i )
    {
        XMStoreFloat4x4( &m_shadowCBData->Models[ i ], models[ i ] );
    }

    // Fill scene CB
    XMStoreFloat4x4( &m_sceneCBData->ViewProj, viewProj );
    for ( uint32_t i = 0; i < NUM_OBJECTS; ++i )
    {
        XMStoreFloat4x4( &m_sceneCBData->Models[ i ], models[ i ] );
    }
    for ( uint32_t c = 0; c < NUM_CASCADES; ++c )
    {
        XMStoreFloat4x4( &m_sceneCBData->LightViewProj[ c ], lightViewProj[ c ] );
    }
    m_sceneCBData->CascadeSplits = XMFLOAT4( m_cascadeSplits[ 1 ], m_cascadeSplits[ 2 ], m_cascadeSplits[ 3 ], m_cascadeSplits[ 4 ] );

    XMFLOAT4 ld;
    XMStoreFloat4( &ld, lightDir );
    m_sceneCBData->LightDir = ld;

    XMFLOAT4 camPos;
    XMStoreFloat4( &camPos, m_camera->Position( ) );
    m_sceneCBData->CameraPos = camPos;
}

void CascadedShadowMapsExample::Init( )
{
    ComputeCascades( );
    CreateGeometry( );
    CreateShadowResources( );
    CreateMainResources( );

    m_camera->SetPosition( XMVectorSet( 0.0f, 5.0f, -10.0f, 1.0f ) );
    m_camera->SetFront( XMVector3Normalize( XMVectorSet( 0.0f, -0.3f, 1.0f, 0.0f ) ) );
}

void CascadedShadowMapsExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    UpdateUniforms( );

    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, m_shadowMapArray, DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingDesc shadowRenderingDesc{};
    shadowRenderingDesc.DepthAttachment.Resource          = m_shadowMapArray;
    shadowRenderingDesc.DepthAttachment.LoadOp            = DENOFIZ_LOAD_OP_CLEAR;
    shadowRenderingDesc.DepthAttachment.StoreOp           = DENOFIZ_STORE_OP_STORE;
    shadowRenderingDesc.DepthAttachment.ClearDepthStencil = { 1.0f, 0.0f };
    shadowRenderingDesc.RenderAreaWidth                   = static_cast<float>( SHADOW_MAP_SIZE );
    shadowRenderingDesc.RenderAreaHeight                  = static_cast<float>( SHADOW_MAP_SIZE );
    shadowRenderingDesc.NumLayers                         = NUM_CASCADES;

    DenOfIz_CommandList_BeginRendering( commandList, &shadowRenderingDesc );
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, static_cast<float>( SHADOW_MAP_SIZE ), static_cast<float>( SHADOW_MAP_SIZE ) );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, static_cast<float>( SHADOW_MAP_SIZE ), static_cast<float>( SHADOW_MAP_SIZE ) );
    DenOfIz_CommandList_BindPipeline( commandList, m_shadowPipeline );

    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowBindGroup ) )
    {
        DenOfIz_CommandList_BindGroup( commandList, m_shadowBindGroup );
    }

    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, sizeof( SceneVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    for ( uint32_t obj = 0; obj < NUM_OBJECTS; ++obj )
    {
        uint32_t          objIndex = obj;
        DenOfIz_ByteArray pushData{};
        pushData.Elements    = reinterpret_cast<Byte *>( &objIndex );
        pushData.NumElements = sizeof( uint32_t );
        DenOfIz_CommandList_SetRootConstants( commandList, 0, &pushData );
        DenOfIz_CommandList_DrawIndexed( commandList, m_objects[ obj ].IndexCount, NUM_CASCADES, m_objects[ obj ].IndexOffset, m_objects[ obj ].VertexOffset, 0 );
    }

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_TransitionTextureDesc textureTransitions[ 3 ] = {};
    textureTransitions[ 0 ].Texture                       = m_shadowMapArray;
    textureTransitions[ 0 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT;
    textureTransitions[ 0 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    textureTransitions[ 1 ].Texture                       = renderTarget;
    textureTransitions[ 1 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT;
    textureTransitions[ 1 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    textureTransitions[ 2 ].Texture                       = m_depthBuffer;
    textureTransitions[ 2 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT;
    textureTransitions[ 2 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{};
    batchTransitionDesc.Textures.Elements    = textureTransitions;
    batchTransitionDesc.Textures.NumElements = 3;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    DenOfIz_RenderingAttachmentDesc colorAttachment{};
    colorAttachment.Resource   = renderTarget;
    colorAttachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    colorAttachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    colorAttachment.ClearColor = { 0.4f, 0.6f, 0.8f, 1.0f };

    DenOfIz_RenderingDesc mainRenderingDesc{};
    mainRenderingDesc.RTAttachments.Elements            = &colorAttachment;
    mainRenderingDesc.RTAttachments.NumElements         = 1;
    mainRenderingDesc.DepthAttachment.Resource          = m_depthBuffer;
    mainRenderingDesc.DepthAttachment.LoadOp            = DENOFIZ_LOAD_OP_CLEAR;
    mainRenderingDesc.DepthAttachment.StoreOp           = DENOFIZ_STORE_OP_DONT_CARE;
    mainRenderingDesc.DepthAttachment.ClearDepthStencil = { 1.0f, 0.0f };
    mainRenderingDesc.RenderAreaWidth                   = static_cast<float>( m_pixelWidth );
    mainRenderingDesc.RenderAreaHeight                  = static_cast<float>( m_pixelHeight );
    mainRenderingDesc.NumLayers                         = 1;

    DenOfIz_CommandList_BeginRendering( commandList, &mainRenderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindPipeline( commandList, m_mainPipeline );

    if ( DENOFIZ_HANDLE_IS_VALID( m_mainBindGroup ) )
    {
        DenOfIz_CommandList_BindGroup( commandList, m_mainBindGroup );
    }

    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, sizeof( SceneVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );

    for ( uint32_t obj = 0; obj < NUM_OBJECTS; ++obj )
    {
        uint32_t          objIndex = obj;
        DenOfIz_ByteArray pushData{};
        pushData.Elements    = reinterpret_cast<Byte *>( &objIndex );
        pushData.NumElements = sizeof( uint32_t );
        DenOfIz_CommandList_SetRootConstants( commandList, 0, &pushData );
        DenOfIz_CommandList_DrawIndexed( commandList, m_objects[ obj ].IndexCount, 1, m_objects[ obj ].IndexOffset, m_objects[ obj ].VertexOffset, 0 );
    }

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void CascadedShadowMapsExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
}

void CascadedShadowMapsExample::Update( )
{
    m_worldData.DeltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    RenderAndPresentFrame( );
}

void CascadedShadowMapsExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

CascadedShadowMapsExample::~CascadedShadowMapsExample( )
{
}

void CascadedShadowMapsExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    if ( m_shadowCBData )
    {
        DenOfIz_Buffer_UnmapMemory( m_shadowUniformBuffer );
        m_shadowCBData = nullptr;
    }
    if ( m_sceneCBData )
    {
        DenOfIz_Buffer_UnmapMemory( m_sceneUniformBuffer );
        m_sceneCBData = nullptr;
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowMapArray ) )
    {
        DenOfIz_TextureResource_Destroy( m_shadowMapArray );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowUniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_shadowUniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowBindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_shadowBindGroup );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowPipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_shadowPipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowRootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_shadowRootSignature );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowInputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_shadowInputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowShaderProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_shadowShaderProgram );
    }
    for ( auto &layout : m_shadowBindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
            DenOfIz_BindGroupLayout_Destroy( layout );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_depthBuffer ) )
    {
        DenOfIz_TextureResource_Destroy( m_depthBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_sceneUniformBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_sceneUniformBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_shadowSampler ) )
    {
        DenOfIz_Sampler_Destroy( m_shadowSampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_mainBindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_mainBindGroup );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_mainPipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_mainPipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_mainRootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_mainRootSignature );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_mainInputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_mainInputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_mainShaderProgram ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_mainShaderProgram );
    }
    for ( auto &layout : m_mainBindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }

    IExample::Quit( );
}
