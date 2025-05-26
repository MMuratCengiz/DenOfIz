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
#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DenOfIz;
using namespace DirectX;

TextRenderer::TextRenderer( const TextRendererDesc &desc ) : m_desc( desc )
{
    if ( !desc.LogicalDevice )
    {
        LOG( FATAL ) << "TextRendererDesc::LogicalDevice must not be null";
        return;
    }

    m_logicalDevice = desc.LogicalDevice;
    BinaryReader      binaryReader{ EmbeddedTextRendererShaders::ShaderAssetBytes };
    ShaderAssetReader assetReader{ { &binaryReader } };
    m_fontShaderProgram           = std::make_unique<ShaderProgram>( assetReader.Read( ) );
    ShaderReflectDesc reflectDesc = m_fontShaderProgram->Reflect( );

    m_fontSampler             = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( {} ) );

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

    m_vertexBufferDesc                           = { };
    m_vertexBufferDesc.NumBytes                  = m_maxVertices * sizeof( GlyphVertex );
    m_vertexBufferDesc.Descriptor                = BitSet( ResourceDescriptor::VertexBuffer ) | ResourceDescriptor::StructuredBuffer;
    m_vertexBufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    m_vertexBufferDesc.HeapType                  = HeapType::CPU_GPU;
    m_vertexBufferDesc.DebugName                 = "Font Vertex Buffer";
    m_vertexBufferDesc.StructureDesc.NumElements = m_maxVertices;
    m_vertexBufferDesc.StructureDesc.Stride      = sizeof( GlyphVertex );
    m_vertexBuffer                               = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
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
    uniformBufferDesc.NumBytes                  = 3 * sizeof( FontShaderUniforms );
    uniformBufferDesc.Descriptor                = BitSet( ResourceDescriptor::UniformBuffer );
    uniformBufferDesc.Usages                    = ResourceUsage::VertexAndConstantBuffer;
    uniformBufferDesc.HeapType                  = HeapType::CPU_GPU;
    uniformBufferDesc.DebugName                 = "Font Uniform Buffer";
    uniformBufferDesc.StructureDesc.NumElements = 1;
    uniformBufferDesc.StructureDesc.Stride      = sizeof( FontShaderUniforms );
    m_uniformBuffer                             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( uniformBufferDesc ) );
    m_uniformBufferData                         = static_cast<FontShaderUniforms *>( m_uniformBuffer->MapMemory( ) );
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

    static FontLibrary defaultFontLibrary;
    static auto        defaultFont = std::unique_ptr<Font>( defaultFontLibrary.LoadFont( { EmbeddedFonts::GetInconsolataRegular( ) } ) );
    SetFont( defaultFont.get( ) );
    RegisterFont( defaultFont.get( ), 0 );

    SetAntiAliasingMode( m_desc.AntiAliasingMode );
    if ( m_desc.Width == 0 || m_desc.Height == 0 )
    {
        LOG( WARNING ) << "Invalid viewport size, call TextRenderer::SetProjection or TextRenderer::SetViewport before rendering";
    }
    else
    {
        SetViewport( Viewport{ 0, 0, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) } );
    }
}

TextRenderer::~TextRenderer( ) = default;

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

uint16_t TextRenderer::RegisterFont( Font *font, uint16_t fontId )
{
    DZ_NOT_NULL( font );
    if ( fontId == 0 )
    {
        fontId = 1;
        while ( m_fontRegistry.contains( fontId ) )
        {
            fontId++;
        }
    }

    m_fontRegistry[ fontId ] = font;
    LOG( INFO ) << "Font registered with ID: " << fontId;
    return fontId;
}

Font *TextRenderer::GetFont( const uint16_t fontId ) const
{
    const auto it = m_fontRegistry.find( fontId );
    if ( it != m_fontRegistry.end( ) )
    {
        return it->second;
    }
    return m_currentFont;
}

void TextRenderer::UnregisterFont( const uint16_t fontId )
{
    const auto it = m_fontRegistry.find( fontId );
    if ( it != m_fontRegistry.end( ) )
    {
        m_fontRegistry.erase( it );
        LOG( INFO ) << "Font ID " << fontId << " unregistered";
    }
    else
    {
        LOG( WARNING ) << "Attempted to unregister non-existent font ID: " << fontId;
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

void TextRenderer::SetViewport( const Viewport &viewport )
{
    if ( viewport.Width == 0 || viewport.Height == 0 )
    {
        LOG( WARNING ) << "Viewport::Width or Viewport::Height is zero, cannot set projection matrix";
        return;
    }
    const XMMATRIX projection = XMMatrixOrthographicOffCenterLH( viewport.X, viewport.Width, viewport.Height, viewport.Y, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_projectionMatrix, projection );
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
    Font *font = params.FontId != 0 ? GetFont( params.FontId ) : m_currentFont;
    if ( !font )
    {
        LOG( WARNING ) << "No font available for rendering";
        return;
    }

    if ( m_textLayouts.size( ) <= m_currentTextLayoutIndex )
    {
        m_textLayouts.resize( m_currentTextLayoutIndex + 1 );

        TextLayoutDesc textLayoutDesc{ font };
        m_textLayouts[ m_currentTextLayoutIndex ] = std::make_unique<TextLayout>( textLayoutDesc );
    }
    const auto &textLayout = m_textLayouts[ m_currentTextLayoutIndex ];
    m_currentTextLayoutIndex++;

    if ( textLayout->GetFont( ) != font )
    {
        textLayout->SetFont( font );
    }

    TextRenderDesc modifiedParams = params;

    float effectiveScale = params.Scale;
    if ( params.FontSize > 0 )
    {
        const float baseSize   = static_cast<float>( font->Asset( )->InitialFontSize );
        const float targetSize = params.FontSize;
        effectiveScale         = targetSize / baseSize * params.Scale;
    }

    ShapeTextDesc shapeDesc{ };
    shapeDesc.Text      = params.Text;
    shapeDesc.Direction = params.Direction;
    shapeDesc.FontSize  = params.FontSize > 0 ? params.FontSize : font->Asset( )->InitialFontSize;

    textLayout->ShapeText( shapeDesc );

    if ( params.HorizontalCenter || params.VerticalCenter )
    {
        if ( params.HorizontalCenter )
        {
            modifiedParams.X -= font->Asset( )->Metrics.LineHeight * effectiveScale / 2.0f;
        }

        if ( params.VerticalCenter )
        {
            modifiedParams.Y -= font->Asset( )->Metrics.LineHeight * effectiveScale / 2.0f;
        }
    }

    GenerateTextVerticesDesc generateDesc{ };
    generateDesc.StartPosition = { modifiedParams.X, modifiedParams.Y };
    generateDesc.Color         = modifiedParams.Color;
    generateDesc.OutVertices   = &m_glyphVertices;
    generateDesc.OutIndices    = &m_indexData;
    generateDesc.Scale         = effectiveScale;
    generateDesc.LetterSpacing = params.LetterSpacing;
    generateDesc.LineHeight    = params.LineHeight;

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

    FontShaderUniforms* uniforms = m_uniformBufferData;
    uniforms->Projection = m_projectionMatrix;
    uniforms->TextColor  = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

    const auto *fontAsset      = m_currentFont->Asset( );
    uniforms->TextureSizeParams = XMFLOAT4( static_cast<float>( fontAsset->AtlasWidth ), static_cast<float>( fontAsset->AtlasHeight ), Font::MsdfPixelRange,
                                           static_cast<float>( static_cast<uint32_t>( m_antiAliasingMode ) ) );

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
    if ( m_vertexBufferDesc.NumBytes < m_glyphVertices.NumElements( ) * sizeof( GlyphVertex ) )
    {
        m_vertexBufferDesc.NumBytes = m_maxVertices * sizeof( GlyphVertex );
        auto newBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_vertexBufferDesc ) );
        m_resourceTracking.TrackBuffer( newBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
        m_vertexBuffer = std::move( newBuffer );
    }

    // Check if we need to resize index buffer
    if ( m_indexBufferDesc.NumBytes < m_indexData.NumElements( ) * sizeof( uint32_t ) )
    {
        m_indexBufferDesc.NumBytes = m_maxIndices * sizeof( uint32_t );
        auto newBuffer             = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( m_indexBufferDesc ) );
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

Float_2 TextRenderer::MeasureText( const InteropString &text, const float scale, const uint32_t fontSize ) const
{
    return MeasureText( text, m_currentFont, scale, fontSize );
}

Float_2 TextRenderer::MeasureText( const InteropString &text, Font *font, const float scale, const uint32_t fontSize ) const
{
    if ( !font )
    {
        LOG( ERROR ) << "Cannot measure text: font is null";
        return Float_2{ 0.0f, 0.0f };
    }

    TextLayoutDesc desc;
    desc.Font = font;
    TextLayout layout( desc );

    ShapeTextDesc shapeDesc;
    shapeDesc.Text     = text;
    shapeDesc.FontSize = fontSize;
    layout.ShapeText( shapeDesc );

    const Float_2 textSize = layout.GetTextSize( );
    return Float_2{ textSize.X * scale, textSize.Y * scale };
}

Float_2 TextRenderer::MeasureText( const InteropString &text, const TextRenderDesc &desc ) const
{
    if ( text.NumChars( ) == 0 )
    {
        return Float_2{ 0.0f, 0.0f };
    }
    Font *font = desc.FontId != 0 ? GetFont( desc.FontId ) : m_currentFont;
    if ( !font )
    {
        LOG( ERROR ) << "Cannot measure text: no font available";
        return Float_2{ 0.0f, 0.0f };
    }

    float effectiveScale = desc.Scale;
    if ( desc.FontSize > 0 )
    {
        const float baseSize   = static_cast<float>( font->Asset( )->InitialFontSize );
        const float targetSize = desc.FontSize;
        effectiveScale         = targetSize / baseSize * desc.Scale;
    }

    // Get or create a TextLayout from the pool
    if ( m_measureTextLayouts.size( ) <= m_currentMeasureLayoutIndex )
    {
        m_measureTextLayouts.resize( m_currentMeasureLayoutIndex + 1 );
        TextLayoutDesc layoutDesc{ font };
        m_measureTextLayouts[ m_currentMeasureLayoutIndex ] = std::make_unique<TextLayout>( layoutDesc );
    }

    const auto &layout          = m_measureTextLayouts[ m_currentMeasureLayoutIndex ];
    m_currentMeasureLayoutIndex = ( m_currentMeasureLayoutIndex + 1 ) % 16; // Cycle through 16 layouts

    if ( layout->GetFont( ) != font )
    {
        layout->SetFont( font );
    }

    ShapeTextDesc shapeDesc;
    shapeDesc.Text      = text;
    shapeDesc.Direction = desc.Direction;
    shapeDesc.FontSize  = desc.FontSize > 0 ? desc.FontSize : font->Asset( )->InitialFontSize;
    layout->ShapeText( shapeDesc );

    const Float_2 textSize = layout->GetTextSize( );

    float adjustedWidth = textSize.X;
    if ( desc.LetterSpacing > 0 && text.NumChars( ) > 0 )
    {
        adjustedWidth += static_cast<float>( desc.LetterSpacing ) * ( text.NumChars( ) - 1 ) * effectiveScale;
    }

    float adjustedHeight = textSize.Y;
    if ( desc.LineHeight > 0 )
    {
        adjustedHeight = static_cast<float>( desc.LineHeight ) * effectiveScale;
    }

    return Float_2{ adjustedWidth * effectiveScale, adjustedHeight * effectiveScale };
}
