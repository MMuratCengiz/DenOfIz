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
#include <DenOfIzGraphics/Utilities/InteropUtilities.h>

using namespace DenOfIz;
using namespace DirectX;

Spinning3DCubeWidget::Spinning3DCubeWidget( IClayContext *clayContext, const uint32_t id ) : Widget( clayContext, id )
{
    m_hasPipeline = true;
}

Spinning3DCubeWidget::~Spinning3DCubeWidget( )
{
    if ( m_uniformData )
    {
        m_uniformBuffer->UnmapMemory( );
        m_uniformData = nullptr;
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
    // For row-major matrices, multiply vector on the left
    output.position = mul(float4(input.position, 1.0), MVP);
    // Transform normal with the upper 3x3 of model matrix only (not MVP)
    output.normal = input.normal;  // Keep normal in model space for now
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

    std::array<ShaderStageDesc, 2> shaderStages( { } );
    ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                          = ShaderStage::Vertex;
    vsDesc.EntryPoint                     = InteropString( "main" );
    vsDesc.Data                           = InteropUtilities::StringToBytes( vertexShaderHLSL );

    ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.EntryPoint       = InteropString( "main" );
    psDesc.Data             = InteropUtilities::StringToBytes( pixelShaderHLSL );

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_shaderProgram                      = std::make_unique<ShaderProgram>( programDesc );
}

void Spinning3DCubeWidget::CreatePipeline( )
{
    const ShaderReflectDesc reflectDesc = m_shaderProgram->Reflect( );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_device->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout                       = std::unique_ptr<IInputLayout>( m_device->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.ShaderProgram = m_shaderProgram.get( );
    pipelineDesc.BindPoint     = BindPoint::Graphics;

    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode          = CullMode::BackFace;
    pipelineDesc.Graphics.FillMode          = FillMode::Solid;

    RenderTargetDesc &renderTarget = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Format            = Format::B8G8R8A8Unorm;

    m_pipeline = std::unique_ptr<IPipeline>( m_device->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    m_resourceBindGroup         = std::unique_ptr<IResourceBindGroup>( m_device->CreateResourceBindGroup( bindGroupDesc ) );
}

void Spinning3DCubeWidget::CreateGeometry( )
{
    BoxDesc boxDesc{ };
    boxDesc.Width         = 1.0f;
    boxDesc.Height        = 1.0f;
    boxDesc.Depth         = 1.0f;
    boxDesc.BuildDesc     = BuildDesc::BuildNormal;
    GeometryData geometry = Geometry::BuildBox( boxDesc );

    std::vector<CubeVertex> formattedVertices( geometry.Vertices.NumElements( ) );
    for ( int i = 0; i < geometry.Vertices.NumElements( ); i++ )
    {
        const GeometryVertexData &vertexData = geometry.Vertices.GetElement( i );
        formattedVertices[ i ]               = { { vertexData.Position.X, vertexData.Position.Y, vertexData.Position.Z },
                                                 { vertexData.Normal.X, vertexData.Normal.Y, vertexData.Normal.Z },
                                                 m_cubeColor };
    }

    BufferDesc vertexBufferDesc{ };
    vertexBufferDesc.NumBytes   = formattedVertices.size( ) * sizeof( CubeVertex );
    vertexBufferDesc.Descriptor = ResourceDescriptor::VertexBuffer;
    vertexBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    vertexBufferDesc.HeapType   = HeapType::GPU;
    vertexBufferDesc.DebugName  = InteropString( "3D Cube Vertex Buffer" );
    m_vertexBuffer              = std::unique_ptr<IBufferResource>( m_device->CreateBufferResource( vertexBufferDesc ) );

    m_indexCount = geometry.Indices.NumElements( );
    BufferDesc indexBufferDesc{ };
    indexBufferDesc.NumBytes   = m_indexCount * sizeof( uint32_t ); // Fix: multiply by size of index
    indexBufferDesc.Descriptor = ResourceDescriptor::IndexBuffer;
    indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    indexBufferDesc.HeapType   = HeapType::GPU;
    indexBufferDesc.DebugName  = InteropString( "3D Cube Index Buffer" );
    m_indexBuffer              = std::unique_ptr<IBufferResource>( m_device->CreateBufferResource( indexBufferDesc ) );

    BatchResourceCopy batchCopy( m_device );
    batchCopy.Begin( );

    CopyToGpuBufferDesc vertexCopyDesc;
    vertexCopyDesc.DstBuffer        = m_vertexBuffer.get( );
    vertexCopyDesc.DstBufferOffset  = 0;
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( formattedVertices.data( ) );
    vertexCopyDesc.Data.NumElements = formattedVertices.size( ) * sizeof( CubeVertex );
    batchCopy.CopyToGPUBuffer( vertexCopyDesc );

    CopyToGpuBufferDesc indexCopyDesc;
    indexCopyDesc.DstBuffer        = m_indexBuffer.get( );
    indexCopyDesc.DstBufferOffset  = 0;
    indexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( geometry.Indices.Data( ) );
    indexCopyDesc.Data.NumElements = geometry.Indices.NumElements( ) * sizeof( uint32_t );
    batchCopy.CopyToGPUBuffer( indexCopyDesc );

    batchCopy.Submit( );

    BufferDesc uniformBufferDesc{ };
    uniformBufferDesc.NumBytes   = sizeof( CubeUniforms );
    uniformBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    uniformBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType   = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName  = InteropString( "3D Cube Uniform Buffer" );
    m_uniformBuffer              = std::unique_ptr<IBufferResource>( m_device->CreateBufferResource( uniformBufferDesc ) );
    m_uniformData                = static_cast<CubeUniforms *>( m_uniformBuffer->MapMemory( ) );

    XMStoreFloat4x4( &m_uniformData->MVP, XMMatrixIdentity( ) );
    m_uniformData->LightDirection = XMFLOAT4( 0.5f, -0.7f, 0.5f, 0.0f );

    BindBufferDesc bindUniformDesc{ };
    bindUniformDesc.Resource = m_uniformBuffer.get( );
    m_resourceBindGroup->BeginUpdate( )->Cbv( bindUniformDesc )->EndUpdate( );
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

void Spinning3DCubeWidget::InitializeRenderResources( ILogicalDevice *device, const uint32_t width, const uint32_t height )
{
    m_device = device;

    constexpr uint32_t rtSize = 512;
    for ( uint32_t frameIdx = 0; frameIdx < m_numFrames; ++frameIdx )
    {
        TextureDesc rtDesc{ };
        rtDesc.Width                = rtSize;
        rtDesc.Height               = rtSize;
        rtDesc.Format               = Format::B8G8R8A8Unorm;
        rtDesc.Usages               = ResourceUsage::RenderTarget | ResourceUsage::ShaderResource;
        rtDesc.InitialUsage         = ResourceUsage::ShaderResource;
        rtDesc.Descriptor           = ResourceDescriptor::RenderTarget | ResourceDescriptor::Texture;
        rtDesc.HeapType             = HeapType::GPU;
        rtDesc.DebugName            = InteropString( "3D Cube Widget Render Target Frame " ).Append( std::to_string( frameIdx ).c_str( ) );
        m_renderTargets[ frameIdx ] = std::unique_ptr<ITextureResource>( device->CreateTextureResource( rtDesc ) );
        m_resourceTracking.TrackTexture( m_renderTargets[ frameIdx ].get( ), ResourceUsage::ShaderResource );

        TextureDesc depthDesc{ };
        depthDesc.Width            = rtSize;
        depthDesc.Height           = rtSize;
        depthDesc.Format           = Format::D32Float;
        depthDesc.Usages           = ResourceUsage::DepthWrite | ResourceUsage::DepthRead;
        depthDesc.InitialUsage     = ResourceUsage::DepthWrite | ResourceUsage::DepthRead;
        depthDesc.Descriptor       = ResourceDescriptor::DepthStencil;
        depthDesc.HeapType         = HeapType::GPU;
        depthDesc.DebugName        = InteropString( "3D Cube Widget Depth Buffer Frame " ).Append( std::to_string( frameIdx ).c_str( ) );
        m_depthBuffers[ frameIdx ] = std::unique_ptr<ITextureResource>( device->CreateTextureResource( depthDesc ) );
        m_resourceTracking.TrackTexture( m_depthBuffers[ frameIdx ].get( ), ResourceUsage::DepthWrite );
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
        TextureDesc rtDesc{ };
        rtDesc.Width                = rtSize;
        rtDesc.Height               = rtSize;
        rtDesc.Format               = Format::B8G8R8A8Unorm;
        rtDesc.Usages               = ResourceUsage::RenderTarget | ResourceUsage::ShaderResource;
        rtDesc.InitialUsage         = ResourceUsage::ShaderResource;
        rtDesc.Descriptor           = ResourceDescriptor::RenderTarget | ResourceDescriptor::Texture;
        rtDesc.HeapType             = HeapType::GPU;
        rtDesc.DebugName            = InteropString( "3D Cube Widget Render Target Frame " ).Append( std::to_string( frameIdx ).c_str( ) );
        m_renderTargets[ frameIdx ] = std::unique_ptr<ITextureResource>( m_device->CreateTextureResource( rtDesc ) );
        m_resourceTracking.TrackTexture( m_renderTargets[ frameIdx ].get( ), ResourceUsage::ShaderResource );

        // Create depth buffer
        TextureDesc depthDesc{ };
        depthDesc.Width            = rtSize;
        depthDesc.Height           = rtSize;
        depthDesc.Format           = Format::D32Float;
        depthDesc.Usages           = ResourceUsage::DepthWrite | ResourceUsage::DepthRead;
        depthDesc.InitialUsage     = ResourceUsage::DepthWrite | ResourceUsage::DepthRead;
        depthDesc.Descriptor       = ResourceDescriptor::DepthStencil;
        depthDesc.HeapType         = HeapType::GPU;
        depthDesc.DebugName        = InteropString( "3D Cube Widget Depth Buffer Frame " ).Append( std::to_string( frameIdx ).c_str( ) );
        m_depthBuffers[ frameIdx ] = std::unique_ptr<ITextureResource>( m_device->CreateTextureResource( depthDesc ) );
        m_resourceTracking.TrackTexture( m_depthBuffers[ frameIdx ].get( ), ResourceUsage::DepthWrite );
    }

    UpdateUniforms( rtSize, rtSize );
}

void Spinning3DCubeWidget::ExecuteCustomPipeline( const WidgetExecutePipelineDesc &context )
{
    constexpr uint32_t rtSize = 512;
    UpdateUniforms( rtSize, rtSize );
    context.CommandList->Begin( );

    auto *renderTarget = m_renderTargets[ context.FrameIndex ].get( );
    auto *depthBuffer  = m_depthBuffers[ context.FrameIndex ].get( );

    BatchTransitionDesc batchTransitionDesc{ context.CommandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    batchTransitionDesc.TransitionTexture( depthBuffer, ResourceUsage::DepthWrite );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.DepthAttachment.Resource  = depthBuffer;
    renderingDesc.RenderAreaWidth           = 512;
    renderingDesc.RenderAreaHeight          = 512;

    context.CommandList->BeginRendering( renderingDesc );
    context.CommandList->BindViewport( 0.0f, 0.0f, rtSize, rtSize );
    context.CommandList->BindScissorRect( 0.0f, 0.0f, rtSize, rtSize );
    context.CommandList->BindPipeline( m_pipeline.get( ) );
    context.CommandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    context.CommandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    context.CommandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    context.CommandList->DrawIndexed( m_indexCount, 1, 0, 0, 0 );
    context.CommandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ context.CommandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::ShaderResource );
    batchTransitionDesc.TransitionTexture( depthBuffer, ResourceUsage::DepthRead );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    context.CommandList->End( );
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

void Spinning3DCubeWidget::CreateLayoutElement( )
{
    ClayElementDeclaration decl = { };
    decl.Id                     = m_id;
    decl.Layout.Sizing.Width    = ClaySizingAxis::Fixed( 200 );
    decl.Layout.Sizing.Height   = ClaySizingAxis::Fixed( 200 );
    decl.BackgroundColor        = ClayColor( 50, 50, 50, 255 );
    decl.CornerRadius           = ClayCornerRadius( 8 );
    decl.Border.Color           = ClayColor( 100, 100, 100, 255 );
    decl.Border.Width           = ClayBorderWidth( 2 );
    decl.Custom.CustomData      = this;

    m_clayContext->OpenElement( decl );
    m_clayContext->CloseElement( );
}

void Spinning3DCubeWidget::Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch )
{
    m_bounds = boundingBox;
}

void Spinning3DCubeWidget::HandleEvent( const Event &event )
{
}
