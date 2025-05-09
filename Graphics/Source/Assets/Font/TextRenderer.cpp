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
#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <DirectXMath.h>

#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h"
#include "DenOfIzGraphics/Assets/Font/EmbeddedTextRendererShaders.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

TextRenderer::TextRenderer( const TextRendererDesc &desc ) : m_desc( desc )
{
    DZ_NOT_NULL( desc.LogicalDevice );
    m_logicalDevice = desc.LogicalDevice;

    XMStoreFloat4x4( &m_projectionMatrix, XMMatrixIdentity( ) );
}

TextRenderer::~TextRenderer( ) = default;

void TextRenderer::Initialize( )
{
    BinaryReader      binaryReader{ EmbeddedTextRendererShaders::ShaderAssetBytes };
    ShaderAssetReader assetReader{ { &binaryReader } };
    m_fontShaderProgram           = std::make_unique<ShaderProgram>( assetReader.Read( ) );
    ShaderReflectDesc reflectDesc = m_fontShaderProgram->Reflect( );

    SamplerDesc samplerDesc;
    samplerDesc.AddressModeU  = SamplerAddressMode::ClampToEdge;
    samplerDesc.AddressModeV  = SamplerAddressMode::ClampToEdge;
    samplerDesc.MipLodBias    = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.CompareOp     = CompareOp::Never;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 0.0f;
    m_fontSampler             = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );

    m_fontAtlasTextureDesc              = { };
    m_fontAtlasTextureDesc.Width        = m_desc.InitialAtlasWidth;
    m_fontAtlasTextureDesc.Height       = m_desc.InitialAtlasHeight;
    m_fontAtlasTextureDesc.Format       = Format::R8G8B8A8Unorm;
    m_fontAtlasTextureDesc.Descriptor   = BitSet( ResourceDescriptor::Texture );
    m_fontAtlasTextureDesc.InitialUsage = ResourceUsage::ShaderResource;
    m_fontAtlasTextureDesc.DebugName    = "Font MTSDF Atlas Texture";
    m_fontAtlasTexture                  = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( m_fontAtlasTextureDesc ) );
    m_resourceTracking.TrackTexture( m_fontAtlasTexture.get( ), ResourceUsage::ShaderResource );

    auto alignedPitch = Utilities::Align( m_desc.InitialAtlasWidth * FontAsset::NumChannels, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureRowAlignment );
    auto alignedSlice = Utilities::Align( m_desc.InitialAtlasHeight, m_logicalDevice->DeviceInfo( ).Constants.BufferTextureAlignment );

    BufferDesc stagingDesc;
    stagingDesc.NumBytes     = alignedPitch * alignedSlice;
    stagingDesc.Descriptor   = BitSet( ResourceDescriptor::Buffer );
    stagingDesc.InitialUsage = ResourceUsage::CopySrc;
    stagingDesc.DebugName    = "Font MSDF Atlas Staging Buffer";
    stagingDesc.HeapType     = HeapType::CPU;
    m_fontAtlasStagingBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( stagingDesc ) );

    m_vertexBufferDesc            = { };
    m_vertexBufferDesc.NumBytes   = m_maxVertices * sizeof( GlyphVertex );
    m_vertexBufferDesc.Descriptor = BitSet( ResourceDescriptor::VertexBuffer );
    m_vertexBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    m_vertexBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_vertexBufferDesc.DebugName  = "Font Vertex Buffer";
    m_vertexBuffer                = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_vertexBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );

    m_indexBufferDesc            = { };
    m_indexBufferDesc.NumBytes   = m_maxIndices * sizeof( uint32_t );
    m_indexBufferDesc.Descriptor = BitSet( ResourceDescriptor::IndexBuffer );
    m_indexBufferDesc.Usages     = ResourceUsage::IndexBuffer;
    m_indexBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_indexBufferDesc.DebugName  = "Font Index Buffer";
    m_indexBuffer                = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_indexBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_indexBuffer.get( ), ResourceUsage::IndexBuffer );

    BufferDesc uniformBufferDesc;
    uniformBufferDesc.NumBytes   = sizeof( FontShaderUniforms );
    uniformBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
    uniformBufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType   = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName  = "Font Uniform Buffer";
    m_uniformBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( uniformBufferDesc ) );
    m_resourceTracking.TrackBuffer( m_uniformBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );

    m_rootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.ShaderProgram     = m_fontShaderProgram.get( );
    pipelineDesc.RootSignature     = m_rootSignature.get( );
    pipelineDesc.InputLayout       = m_inputLayout.get( );
    pipelineDesc.Graphics.FillMode = FillMode::Solid;

    RenderTargetDesc &renderTarget          = pipelineDesc.Graphics.RenderTargets.EmplaceElement( );
    renderTarget.Blend.Enable               = true;
    renderTarget.Blend.SrcBlend             = Blend::SrcAlpha;
    renderTarget.Blend.DstBlend             = Blend::InvSrcAlpha;
    renderTarget.Blend.BlendOp              = BlendOp::Add;
    renderTarget.Blend.SrcBlendAlpha        = Blend::One;
    renderTarget.Blend.DstBlendAlpha        = Blend::Zero;
    renderTarget.Blend.BlendOpAlpha         = BlendOp::Add;
    renderTarget.Format                     = Format::B8G8R8A8Unorm;
    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;

    m_fontPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;

    m_resourceBindGroup = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );
    m_resourceBindGroup->BeginUpdate( )->Cbv( 0, m_uniformBuffer.get( ) )->Srv( 0, m_fontAtlasTexture.get( ) )->Sampler( 0, m_fontSampler.get( ) )->EndUpdate( );
}

void TextRenderer::SetFont( Font *font )
{
    DZ_NOT_NULL( font );
    m_currentFont      = font;
    m_atlasNeedsUpdate = true;

    for ( const auto &textLayout : m_textLayouts )
    {
        textLayout->SetFont( m_currentFont );
    }
}

void TextRenderer::SetAntiAliasingMode( const AntiAliasingMode antiAliasingMode )
{
    m_antiAliasingMode = antiAliasingMode;
}

void TextRenderer::SetProjectionMatrix( const Float_4x4 &projectionMatrix )
{
    m_projectionMatrix = InteropMathConverter::Float_4x4ToXMFLOAT4X4( projectionMatrix );
}

void TextRenderer::BeginBatch( )
{
    m_glyphVertices.Clear( );
    m_indexData.Clear( );
    m_currentVertexCount     = 0;
    m_currentIndexCount      = 0;
    m_currentTextLayoutIndex = 0;
}

void TextRenderer::AddText( const TextRenderDesc &params )
{
    if ( !m_currentFont || params.Text.NumChars( ) == 0 )
    {
        return;
    }

    if ( m_textLayouts.size( ) <= m_currentTextLayoutIndex )
    {
        m_textLayouts.resize( m_currentTextLayoutIndex + 1 );

        TextLayoutDesc textLayoutDesc{ m_currentFont };
        m_textLayouts[ m_currentTextLayoutIndex ] = std::make_unique<TextLayout>( textLayoutDesc );
    }
    const auto &textLayout = m_textLayouts[ m_currentTextLayoutIndex ];
    m_currentTextLayoutIndex++;

    TextRenderDesc modifiedParams = params;

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text      = params.Text;
    shapeDesc.Direction = params.Direction;
    textLayout->ShapeText( shapeDesc );

    if ( params.HorizontalCenter || params.VerticalCenter )
    {
        if ( params.HorizontalCenter )
        {
            modifiedParams.X -= m_currentFont->Asset( )->Metrics.LineHeight * params.Scale / 2.0f;
        }

        if ( params.VerticalCenter )
        {
            modifiedParams.Y -= m_currentFont->Asset( )->Metrics.LineHeight * params.Scale / 2.0f;
        }
    }

    GenerateTextVerticesDesc generateDesc{ };
    generateDesc.StartPosition = { modifiedParams.X, modifiedParams.Y };
    generateDesc.Color         = modifiedParams.Color;
    generateDesc.OutVertices   = &m_glyphVertices;
    generateDesc.OutIndices    = &m_indexData;
    generateDesc.Scale         = params.Scale;

    textLayout->GenerateTextVertices( generateDesc );

    m_currentVertexCount = m_glyphVertices.NumElements( );
    m_currentIndexCount  = m_indexData.NumElements( );

    // Resize buffers if needed
    if ( m_currentVertexCount > m_maxVertices || m_currentIndexCount > m_maxIndices )
    {
        m_maxVertices = std::max( m_maxVertices * 2, m_currentVertexCount );
        m_maxIndices  = std::max( m_maxIndices * 2, m_currentIndexCount );
        LOG( INFO ) << "Font render buffers resized: vertices=" << m_maxVertices << ", indices=" << m_maxIndices;
    }
}

void TextRenderer::EndBatch( ICommandList *commandList )
{
    if ( m_currentVertexCount == 0 || m_currentIndexCount == 0 || !m_currentFont )
    {
        return; // Nothing to render
    }

    if ( m_atlasNeedsUpdate )
    {
        UpdateAtlasTexture( commandList );
        m_atlasNeedsUpdate = false;
    }

    UpdateBuffers( );

    FontShaderUniforms uniforms{ };
    uniforms.Projection = m_projectionMatrix;
    uniforms.TextColor  = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

    const auto *fontAsset      = m_currentFont->Asset( );
    uniforms.TextureSizeParams = XMFLOAT4( static_cast<float>( fontAsset->AtlasWidth ), static_cast<float>( fontAsset->AtlasHeight ), Font::MsdfPixelRange,
                                           static_cast<float>( static_cast<uint32_t>( m_antiAliasingMode ) ) );

    void *mappedData = m_uniformBuffer->MapMemory( );
    memcpy( mappedData, &uniforms, sizeof( FontShaderUniforms ) );
    m_uniformBuffer->UnmapMemory( );

    commandList->BindPipeline( m_fontPipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    commandList->DrawIndexed( m_currentIndexCount, 1, 0, 0, 0 );
}

void TextRenderer::UpdateAtlasTexture( ICommandList *commandList )
{
    const auto *fontAsset = m_currentFont->Asset( );

    // Check if texture needs resizing
    if ( m_fontAtlasTextureDesc.Width != fontAsset->AtlasWidth || m_fontAtlasTextureDesc.Height != fontAsset->AtlasHeight )
    {
        TextureDesc newDesc = m_fontAtlasTextureDesc;
        newDesc.Width       = fontAsset->AtlasWidth;
        newDesc.Height      = fontAsset->AtlasHeight;
        newDesc.Format      = Format::R8G8B8A8Unorm;

        auto newTexture = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( newDesc ) );
        m_resourceTracking.TrackTexture( newTexture.get( ), ResourceUsage::ShaderResource );
        m_fontAtlasTexture = std::move( newTexture );
        m_resourceBindGroup->BeginUpdate( )->Srv( 0, m_fontAtlasTexture.get( ) )->EndUpdate( );
    }

    m_resourceTracking.TrackBuffer( m_fontAtlasStagingBuffer.get( ), ResourceUsage::CopySrc );

    LoadAtlasIntoGpuTextureDesc loadDesc{ };
    loadDesc.Device        = m_logicalDevice;
    loadDesc.StagingBuffer = m_fontAtlasStagingBuffer.get( );
    loadDesc.CommandList   = commandList;
    loadDesc.Texture       = m_fontAtlasTexture.get( );
    FontAssetReader::LoadAtlasIntoGpuTexture( *fontAsset, loadDesc );

    // Transition the texture to copy destination state
    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_fontAtlasTexture.get( ), ResourceUsage::CopyDst );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    // Copy from the staging buffer to the texture
    CopyBufferToTextureDesc copyDesc{ };
    copyDesc.SrcBuffer  = m_fontAtlasStagingBuffer.get( );
    copyDesc.DstTexture = m_fontAtlasTexture.get( );
    copyDesc.RowPitch   = fontAsset->AtlasWidth * 4; // 4 bytes per pixel (RGBA)
    copyDesc.Format     = m_fontAtlasTexture->GetFormat( );

    commandList->CopyBufferToTexture( copyDesc );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( m_fontAtlasTexture.get( ), ResourceUsage::ShaderResource );
    m_resourceTracking.BatchTransition( batchTransitionDesc );
}

void TextRenderer::UpdateBuffers( )
{
    // Check if we need to resize vertex buffer
    if ( m_vertexBufferDesc.NumBytes < m_glyphVertices.NumElements( ) )
    {
        BufferDesc newDesc = m_vertexBufferDesc;
        newDesc.NumBytes   = m_maxVertices * sizeof( GlyphVertex );

        auto newBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( newDesc ) );
        m_resourceTracking.TrackBuffer( newBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
        m_vertexBuffer = std::move( newBuffer );
    }

    // Check if we need to resize index buffer
    if ( m_indexBufferDesc.NumBytes < m_indexData.NumElements( ) * sizeof( uint32_t ) )
    {
        BufferDesc newDesc = m_indexBufferDesc;
        newDesc.NumBytes   = m_maxIndices * sizeof( uint32_t );

        auto newBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( newDesc ) );
        m_resourceTracking.TrackBuffer( newBuffer.get( ), ResourceUsage::IndexBuffer );
        m_indexBuffer = std::move( newBuffer );
    }

    void *vertexData = m_vertexBuffer->MapMemory( );
    memcpy( vertexData, m_glyphVertices.Data( ), m_glyphVertices.NumElements( ) * sizeof( GlyphVertex ) );
    m_vertexBuffer->UnmapMemory( );

    void *indexData = m_indexBuffer->MapMemory( );
    memcpy( indexData, m_indexData.Data( ), m_indexData.NumElements( ) * sizeof( uint32_t ) );
    m_indexBuffer->UnmapMemory( );
}
