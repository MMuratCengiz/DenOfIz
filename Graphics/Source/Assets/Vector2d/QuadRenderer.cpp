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

#include <DenOfIzGraphics/Assets/Vector2d/QuadRenderer.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <DenOfIzGraphics/Utilities/InteropMathConverter.h>
#include <DenOfIzGraphics/Utilities/InteropUtilities.h>
#include <algorithm>

#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

using namespace DenOfIz;
using namespace DirectX;

static auto QuadVertexShader = R"(
struct VertexInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct InstanceData
{
    float4x4 Transform;
    float4 UVScaleOffset;
    uint MaterialId;
    float3 _Pad0;
    float4 _Pad1;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
    uint MaterialId : TEXCOORD1;
};

struct RootConstants {
    uint StartInstance;
    uint HasTexture;
    float2 Padding;
};

[[vk::push_constant]] ConstantBuffer<RootConstants> rootConstants : register(b0, space31);
StructuredBuffer<InstanceData> Instances : register(t0);
cbuffer FrameConstants : register(b0)
{
    float4x4 Projection;
};

VertexOutput main(VertexInput input, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    InstanceData instance = Instances[rootConstants.StartInstance + instanceID];
    float4 worldPos = mul(float4(input.Position, 1.0), instance.Transform);
    output.Position = mul(worldPos, Projection);
    output.TexCoord = input.TexCoord * instance.UVScaleOffset.xy + instance.UVScaleOffset.zw;
    output.MaterialId = instance.MaterialId;
    return output;
}
)";

static auto RasterPixelShader = R"(
struct MaterialData
{
    float4 Color;
};

struct RootConstants {
    uint StartInstance;
    uint HasTexture;
    float2 Padding;
};

[[vk::push_constant]] ConstantBuffer<RootConstants> rootConstants : register(b0, space31);

SamplerState QuadSampler : register(s0);
Texture2D QuadTexture : register(t0, space1);
StructuredBuffer<MaterialData> MaterialDatas : register(t1, space1);

struct PixelInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
    uint MaterialId : TEXCOORD1;
};

float4 main(PixelInput input) : SV_Target
{
    MaterialData materialData = MaterialDatas[ input.MaterialId ];
    if (rootConstants.HasTexture == 0) // 0 means no texture available
    {
        return materialData.Color;
    }
    float4 texColor = QuadTexture.Sample(QuadSampler, input.TexCoord);
    // For premultiplied alpha with white tint, we just return the texture color
    return texColor;
}
)";

QuadRenderer::QuadRenderer( const QuadRendererDesc &desc ) : m_desc( desc )
{
    DZ_NOT_NULL( desc.LogicalDevice );
    m_logicalDevice = desc.LogicalDevice;

    XMStoreFloat4x4( &m_projectionMatrix, XMMatrixIdentity( ) );
    m_frameData.resize( desc.NumFrames );
}

QuadRenderer::~QuadRenderer( ) = default;

void QuadRenderer::Initialize( )
{
    CreateShaderResources( );
    CreateStaticQuadGeometry( );

    SamplerDesc samplerDesc;
    samplerDesc.AddressModeU  = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeV  = SamplerAddressMode::ClampToEdge;
    samplerDesc.MipLodBias    = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.CompareOp     = CompareOp::Never;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 16.0f;
    m_sampler                 = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );

    BufferDesc instanceBufferDesc{ };
    instanceBufferDesc.NumBytes                  = m_desc.NumFrames * m_desc.MaxNumQuads * sizeof( QuadInstance );
    instanceBufferDesc.Descriptor                = BitSet( ResourceDescriptor::StructuredBuffer );
    instanceBufferDesc.Usages                    = ResourceUsage::ShaderResource;
    instanceBufferDesc.HeapType                  = HeapType::CPU_GPU;
    instanceBufferDesc.DebugName                 = InteropString( "Quad Renderer Instance Buffer" );
    instanceBufferDesc.StructureDesc.NumElements = m_desc.NumFrames * m_desc.MaxNumQuads;
    instanceBufferDesc.StructureDesc.Stride      = sizeof( QuadInstance );
    m_instanceBuffer                             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( instanceBufferDesc ) );
    m_instances                                  = static_cast<QuadInstance *>( m_instanceBuffer->MapMemory( ) );

    BufferDesc materialBufferDesc{ };
    materialBufferDesc.NumBytes                  = m_desc.NumFrames * m_desc.MaxNumMaterials * sizeof( MaterialShaderData );
    materialBufferDesc.Descriptor                = BitSet( ResourceDescriptor::StructuredBuffer );
    materialBufferDesc.Usages                    = ResourceUsage::ShaderResource;
    materialBufferDesc.HeapType                  = HeapType::CPU_GPU;
    materialBufferDesc.DebugName                 = InteropString( "Quad Renderer Material Buffer Frame" );
    materialBufferDesc.StructureDesc.NumElements = m_desc.NumFrames * m_desc.MaxNumMaterials;
    materialBufferDesc.StructureDesc.Stride      = sizeof( MaterialShaderData );
    m_materialBuffer                             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( materialBufferDesc ) );
    m_materialData                               = static_cast<Byte *>( m_materialBuffer->MapMemory( ) );

    uint32_t   alignedFrameConstants = Utilities::Align( sizeof( FrameConstants ), 256 );
    BufferDesc constantsBufferDesc;
    constantsBufferDesc.NumBytes   = m_desc.NumFrames * alignedFrameConstants;
    constantsBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
    constantsBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    constantsBufferDesc.HeapType   = HeapType::CPU_GPU;
    constantsBufferDesc.DebugName  = InteropString( "Quad Renderer Constants Buffer" );
    m_constantsBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( constantsBufferDesc ) );

    for ( uint32_t frameIdx = 0; frameIdx < m_desc.NumFrames; ++frameIdx )
    {
        FrameData &frame = m_frameData[ frameIdx ];

        ResourceBindGroupDesc instancesBindGroup{ };
        instancesBindGroup.RootSignature = m_rootSignature.get( );
        frame.InstanceBindGroup          = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( instancesBindGroup ) );

        BindBufferDesc bindInstanceBufferDesc{ };
        bindInstanceBufferDesc.Resource       = m_instanceBuffer.get( );
        bindInstanceBufferDesc.ResourceOffset = frameIdx * m_desc.MaxNumQuads * sizeof( QuadInstance );

        BindBufferDesc bindConstantsBufferDesc{ };
        bindConstantsBufferDesc.Resource       = m_constantsBuffer.get( );
        bindConstantsBufferDesc.ResourceOffset = frameIdx * alignedFrameConstants;
        frame.InstanceBindGroup->BeginUpdate( )->Cbv( bindConstantsBufferDesc )->Srv( bindInstanceBufferDesc )->Sampler( 0, m_sampler.get( ) )->EndUpdate( );

        ResourceBindGroupDesc rootConstantsBindGroupDesc{ };
        rootConstantsBindGroupDesc.RootSignature = m_rootSignature.get( );
        rootConstantsBindGroupDesc.RegisterSpace = DZConfiguration::Instance( ).RootConstantRegisterSpace;
        frame.RootConstantsBindGroup             = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( rootConstantsBindGroupDesc ) );
    }

    TextureDesc textureDesc{ };
    textureDesc.Width      = 1;
    textureDesc.Height     = 1;
    textureDesc.Format     = m_desc.RenderTargetFormat;
    textureDesc.Usages     = ResourceUsage::ShaderResource;
    textureDesc.DebugName  = InteropString( "Quad Renderer Null Texture" );
    textureDesc.Descriptor = ResourceDescriptor::Texture;
    m_nullTexture          = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );

    m_materialDescs.resize( m_desc.MaxNumMaterials );
}

void QuadRenderer::SetCanvas( const uint32_t width, const uint32_t height )
{
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
    const auto     mappedData      = static_cast<Byte *>( m_constantsBuffer->MapMemory( ) );
    const uint32_t alignedMemBytes = Utilities::Align( sizeof( FrameConstants ), 256 );
    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        std::memcpy( mappedData + i * alignedMemBytes, &m_projectionMatrix, sizeof( FrameConstants ) );
    }
    m_constantsBuffer->UnmapMemory( );
}

void QuadRenderer::CreateShaderResources( )
{
    ShaderProgramDesc shaderProgramDesc{ };

    ShaderStageDesc &vertexShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement( );
    vertexShaderDesc.Stage            = ShaderStage::Vertex;
    vertexShaderDesc.EntryPoint       = "main";
    const InteropString vsStr( QuadVertexShader );
    vertexShaderDesc.Data = InteropUtilities::StringToBytes( vsStr );

    ShaderStageDesc &pixelShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement( );
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.EntryPoint       = "main";
    const InteropString psStr( RasterPixelShader );
    pixelShaderDesc.Data = InteropUtilities::StringToBytes( psStr );

    m_shaderProgram = std::make_unique<ShaderProgram>( shaderProgramDesc );

    const ShaderReflectDesc reflectDesc = m_shaderProgram->Reflect( );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout                       = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.ShaderProgram     = m_shaderProgram.get( );
    pipelineDesc.RootSignature     = m_rootSignature.get( );
    pipelineDesc.InputLayout       = m_inputLayout.get( );
    pipelineDesc.Graphics.FillMode = FillMode::Solid;

    RenderTargetDesc &renderTarget          = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Blend.Enable               = true;
    renderTarget.Blend.SrcBlend             = Blend::One; // Premultiplied alpha
    renderTarget.Blend.DstBlend             = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOp              = BlendOp::Add;
    renderTarget.Blend.SrcBlendAlpha        = Blend::One;
    renderTarget.Blend.DstBlendAlpha        = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOpAlpha         = BlendOp::Add;
    renderTarget.Format                     = m_desc.RenderTargetFormat;
    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;

    m_rasterPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void QuadRenderer::CreateStaticQuadGeometry( )
{
    // Create a unit quad (0,0) to (1,1)
    constexpr QuadVertex vertices[ 4 ] = {
        { XMFLOAT3( 0.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // Top-left
        { XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // Top-right
        { XMFLOAT3( 1.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // Bottom-right
        { XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) }  // Bottom-left
    };

    const uint32_t indices[ 6 ] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };

    BufferDesc vertexBufferDesc{ };
    vertexBufferDesc.NumBytes                  = sizeof( vertices );
    vertexBufferDesc.Descriptor                = BitSet( ResourceDescriptor::VertexBuffer );
    vertexBufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    vertexBufferDesc.HeapType                  = HeapType::GPU;
    vertexBufferDesc.DebugName                 = "Quad Renderer Vertex Buffer";
    vertexBufferDesc.StructureDesc.NumElements = 4;
    vertexBufferDesc.StructureDesc.Stride      = sizeof( QuadVertex );
    m_vertexBuffer                             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vertexBufferDesc ) );

    BufferDesc indexBufferDesc{ };
    indexBufferDesc.NumBytes   = sizeof( indices );
    indexBufferDesc.Descriptor = BitSet( ResourceDescriptor::IndexBuffer );
    indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    indexBufferDesc.HeapType   = HeapType::GPU;
    indexBufferDesc.DebugName  = "Quad Renderer Index Buffer";
    m_indexBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( indexBufferDesc ) );

    BatchResourceCopy batchResourceCopy{ m_logicalDevice };

    batchResourceCopy.Begin( );
    CopyToGpuBufferDesc copyToGpuBufferDesc;
    copyToGpuBufferDesc.Data.MemCpy( &vertices[ 0 ], sizeof( vertices ) );
    copyToGpuBufferDesc.DstBuffer = m_vertexBuffer.get( );
    batchResourceCopy.CopyToGPUBuffer( copyToGpuBufferDesc );

    copyToGpuBufferDesc = { };
    copyToGpuBufferDesc.Data.MemCpy( &indices[ 0 ], sizeof( indices ) );
    copyToGpuBufferDesc.DstBuffer = m_indexBuffer.get( );
    batchResourceCopy.CopyToGPUBuffer( copyToGpuBufferDesc );
    batchResourceCopy.Submit( );
}

void QuadRenderer::AddMaterial( const QuadMaterialDesc &desc )
{
    if ( desc.MaterialId >= m_desc.MaxNumMaterials )
    {
        LOG( WARNING ) << "Maximum number of materials reached";
        return;
    }
    m_materialDescs[ desc.MaterialId ] = desc;
    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        auto &resourceBindGroups = m_frameData[ i ].MaterialBindGroups;

        ResourceBindGroupDesc bindGroupDesc{ };
        bindGroupDesc.RootSignature           = m_rootSignature.get( );
        bindGroupDesc.RegisterSpace           = 1;
        resourceBindGroups[ desc.MaterialId ] = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

        UpdateMaterial( i, desc );
    }
    m_drawBatches[ desc.MaterialId ] = { .StartInstance = 0, .MaterialId = desc.MaterialId };
}

void QuadRenderer::UpdateMaterial( const uint32_t frameIndex, const QuadMaterialDesc &desc ) const
{
    if ( desc.MaterialId >= m_desc.MaxNumMaterials )
    {
        LOG( WARNING ) << "Invalid material ID: " << desc.MaterialId;
        return;
    }

    MaterialShaderData shaderData{ };
    shaderData.Color = desc.Color;

    const uint64_t dstOffset = frameIndex * m_desc.MaxNumMaterials * sizeof( MaterialShaderData ) + desc.MaterialId * sizeof( MaterialShaderData );
    std::memcpy( m_materialData + dstOffset, &shaderData, sizeof( MaterialShaderData ) );

    BindBufferDesc bindMaterialShaderDataDesc{ };
    bindMaterialShaderDataDesc.Binding        = 1;
    bindMaterialShaderDataDesc.Resource       = m_materialBuffer.get( );
    bindMaterialShaderDataDesc.ResourceOffset = frameIndex * m_desc.MaxNumMaterials * sizeof( MaterialShaderData );

    auto &resourceBindGroup = m_frameData[ frameIndex ].MaterialBindGroups.at( desc.MaterialId );
    resourceBindGroup->BeginUpdate( );
    resourceBindGroup->Srv( bindMaterialShaderDataDesc );
    resourceBindGroup->Srv( 0, desc.Texture != nullptr ? desc.Texture : m_nullTexture.get( ) );
    resourceBindGroup->EndUpdate( );
}

void QuadRenderer::AddQuad( const QuadDataDesc &desc )
{
    if ( desc.QuadId >= m_desc.MaxNumQuads )
    {
        LOG( WARNING ) << "Maximum number of quads reached";
        return;
    }

    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        UpdateQuad( i, desc );
    }

    if ( !m_drawBatches.contains( desc.MaterialId ) )
    {
        LOG( ERROR ) << "Material ID does not exist, add it first using AddMaterial";
        return;
    }

    DrawBatch &batch = m_drawBatches[ desc.MaterialId ];
    batch.QuadIds.push_back( desc.QuadId );
    batch.InstanceCount = batch.QuadIds.size( );
}

void QuadRenderer::UpdateQuad( const uint32_t frameIndex, const QuadDataDesc &desc ) const
{
    if ( desc.QuadId > m_desc.MaxNumQuads )
    {
        LOG( WARNING ) << "Invalid quad ID: " << desc.QuadId << ". QuadRendererDesc::MaxNumQuads is configured to be: " << m_desc.MaxNumQuads;
        return;
    }

    QuadInstance    *instance  = &m_instances[ desc.QuadId + frameIndex * m_desc.MaxNumQuads ];
    const XMFLOAT4X4 transform = CalculateTransform( desc );
    instance->Transform        = transform;
    instance->UVScaleOffset    = XMFLOAT4( desc.UV1.X - desc.UV0.X, // U scale
                                           desc.UV1.Y - desc.UV0.Y, // V scale
                                           desc.UV0.X,              // U offset
                                           desc.UV0.Y               // V offset
       );
    instance->MaterialId       = desc.MaterialId;
}

void QuadRenderer::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    const FrameData &frame = m_frameData[ frameIndex ];

    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    commandList->BindPipeline( m_rasterPipeline.get( ) );
    commandList->BindResourceGroup( frame.InstanceBindGroup.get( ) );

    for ( auto &[ materialId, drawBatch ] : m_drawBatches )
    {
        commandList->BindResourceGroup( frame.MaterialBindGroups.at( materialId ).get( ) );
        for ( const uint32_t quadId : drawBatch.QuadIds )
        {
            RootConstants rootConstants;
            rootConstants.StartInstance = quadId;
            rootConstants.HasTexture    = m_materialDescs[ materialId ].Texture != nullptr ? 1 : 0;
            frame.RootConstantsBindGroup->SetRootConstants( 0, &rootConstants );
            commandList->BindResourceGroup( frame.RootConstantsBindGroup.get( ) );
            commandList->DrawIndexed( 6, 1, 0, 0, 0 );
        }
    }
}

XMFLOAT4X4 QuadRenderer::CalculateTransform( const QuadDataDesc &desc ) const
{
    XMMATRIX transform = XMMatrixIdentity( );
    transform          = transform * XMMatrixScaling( desc.Size.X * desc.Scale.X, desc.Size.Y * desc.Scale.Y, 1.0f );
    if ( desc.Rotation != 0.0f )
    {
        const float scaledCenterX = desc.Size.X * desc.Scale.X * 0.5f;
        const float scaledCenterY = desc.Size.Y * desc.Scale.Y * 0.5f;

        const float rotCenterX = desc.RotationCenter.X != 0.0f || desc.RotationCenter.Y != 0.0f ? desc.RotationCenter.X : scaledCenterX;
        const float rotCenterY = desc.RotationCenter.X != 0.0f || desc.RotationCenter.Y != 0.0f ? desc.RotationCenter.Y : scaledCenterY;

        transform = transform * XMMatrixTranslation( -rotCenterX, -rotCenterY, 0.0f );
        transform = transform * XMMatrixRotationZ( desc.Rotation );
        transform = transform * XMMatrixTranslation( rotCenterX, rotCenterY, 0.0f );
    }

    transform = transform * XMMatrixTranslation( desc.Position.X, desc.Position.Y, 0.0f );

    XMFLOAT4X4 result;
    XMStoreFloat4x4( &result, transform );
    return result;
}
